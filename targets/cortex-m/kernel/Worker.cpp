/*
 * Copyright (c) 2024 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/kernel/Worker.cpp
 */

#include <kernel/kernel.h>

static constexpr uint32_t STACK_MAGIC = ID("STAK");

//#define WORKER_TRACE_ENTRY_EXIT 1

namespace kernel
{

__attribute__((naked, noreturn)) static void WorkerDone()
{
    // return complete async_res, keep result in R0 so it is propagated
    __asm volatile(
        "movs r1, #0\n"
        "svc #0\n"
    );
}

typedef void (*handler_t)(void);
extern "C" handler_t g_isrTableSys[];

static void StopWorker(intptr_t asyncVal, AsyncResult asyncRes);

OPTIMIZE __attribute__((naked)) static void InterruptWorker()
{
    __asm volatile (
        "movs r0, #0\n"
        "movs r1, %[SleepTicks]\n"
        "b %[StopWorker]\n"
        : : [SleepTicks] "i" (AsyncResult::SleepTicks), [StopWorker] "g" (StopWorker)
    );
}

OPTIMIZE static void StopWorker(intptr_t asyncVal, AsyncResult asyncRes)
{
    // we'll be returning to MSP
    // overwrite the stack R0/R1 values with current ones (async_res_t)
    auto msp = (uint32_t*)__get_MSP();
    msp[0] = asyncVal; msp[1] = uint32_t(asyncRes);

#if TRACE && WORKER_TRACE_ENTRY_EXIT
    ITM->PORT[0].u8 = '>';
#endif

    __asm volatile (
        // stop the SysTick
        "mov r0, #0xE000E000\n"
        "movs r1, #0\n"
        "str r1, [r0, #0x10]\n"
    );

    __asm volatile (
        // save registers not handled by handler entry below PSP
        "mrs r0, psp\n"
        "stmdb r0!, {r4-r11}\n"
#ifndef __SOFTFP__
        "vstmdb r0!, {s16-s31}\n"
#endif

        // return to MSP
        "bic lr, #4\n"
        "bx lr\n"
    );

    __builtin_unreachable();
};

OPTIMIZE static void StartWorker()
{
#if TRACE && WORKER_TRACE_ENTRY_EXIT
    ITM->PORT[0].u8 = '<';
#endif

    // change the SVCall handler to StopWorker
    g_isrTableSys[SVCall_IRQn + NVIC_USER_IRQ_OFFSET] = (handler_t)StopWorker;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    __asm volatile (
        // restore registers not handled by handler return from below PSP
        "mrs r0, psp\n"
        "ldmdb r0!, {r4-r11}\n"
#ifndef __SOFTFP__
        "vldmdb r0!, {s16-s31}\n"
#endif

        // return to PSP
        "orr lr, #4\n"
        "bx lr\n"
    );

    __builtin_unreachable();
}

OPTIMIZE static async_res_t RunWorker(AsyncFrame**, uint32_t** sp, uint32_t* spLow)
{
    // change the SVCall handler to StartWorker
    g_isrTableSys[SVCall_IRQn + NVIC_USER_IRQ_OFFSET] = StartWorker;

    // load PSP
    __set_PSP(uint32_t(*sp));

    register intptr_t r0 asm ("r0");
    register AsyncResult r1 asm ("r1");
    __asm volatile (
        // switch to PSP, we get back after yield or interrupt
        "svc #0\n"
        : "=r" (r0), "=r" (r1) : [sp] "r" (sp) : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
    );
    auto res = _ASYNC_RES(r0, r1);

    // store PSP
    *sp = (uint32_t*)__get_PSP();

#if TRACE
    if (*spLow != STACK_MAGIC || *sp < spLow)
    {
        if (*sp < spLow)
        {
            DBGL("STACK OVERFLOW: %p < %p", *sp, spLow);
        }
        if (*spLow != STACK_MAGIC)
        {
            DBGL("STACK CORRUPTION @ %p", spLow);
        }
        ASSERT(false);
    }
#endif

    return res;
}

enum
{
#ifdef __SOFTFP__
    ISRStack = 8,           // 8 regular IRS registers
    BelowStack = 8,         // R4-R11
#else
    ISRStack = 8 + 18,      // 8 regular ISR registers + 18 VFP (S0-15, FPSCR, VPR)
    BelowStack = 8 + 16,    // R4-R11, S16-S31
#endif
};

OPTIMIZE async(Worker::PlatformRun, Delegate<intptr_t> worker, size_t stack)
async_def(
    uint32_t* stack;
    size_t stackWords;
    uint32_t* sp;

    ~__FRAME() { delete[] stack; }
)
{
    f.stackWords = (stack + 3) / 4;
    f.stack = new uint32_t[f.stackWords];

    f.stack[0] = STACK_MAGIC;
    f.sp = f.stack + f.stackWords - ISRStack;
    memset(f.sp - BelowStack, 0, (BelowStack + ISRStack) * sizeof(uint32_t));

    f.sp[7] = BIT(24);  // set THUMB in xPSR
    f.sp[6] = uint32_t(_DelegateFn(worker));        // initial PC
    f.sp[5] = uint32_t(WorkerDone);                 // LR
    f.sp[0] = uint32_t(_DelegateTarget(worker));    // initial R0

    // initialize systick
    SysTick->CTRL = 0;
    SysTick->VAL = 0;
    SysTick->LOAD = SystemCoreClock / 3000;

    Cortex_SetIRQHandler(SysTick_IRQn, InterruptWorker);
    // use lowest non-masked priority for SysTick
    // it will be masked in critical sections to prevent preemption
    NVIC_SetPriority(SysTick_IRQn, 0x100 >> __NVIC_PRIO_BITS);

    auto res = await(RunWorker, &f.sp, f.stack);

    async_return(res);
}
async_end

}
