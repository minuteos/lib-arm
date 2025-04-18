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
    // worker function has to return a complete async_res in R0/R1, just call the service handler
    __asm volatile("svc #0");
}

typedef void (*handler_t)(void);
extern "C" handler_t g_isrTableSys[];
extern "C" void Missing_Handler();

static void StopWorker();
static void StopWorker2();
static void StopWorker3(intptr_t asyncVal, AsyncResult asyncRes);
static void StartWorker2(bool noPreempt);

/*!
 * Critical context switching code when stopping/yielding workers (SVC handler)
 * we cannot trust the compiler not to mix up register/stack allocations
 */
__attribute__((naked))
#ifdef LINKER_ORDERED_SECTION
__attribute__((section(LINKER_ORDERED_SECTION ".wrk.stop.0")))
#endif
static void StopWorker()
{
    __asm volatile (
        // load R0/R1 from PSP because a possible exception hadnler could have
        // been executed right before entering this one, changing the actual registers
        "mrs r2, psp\n"
        "ldrd r0, r1, [r2]\n"
        // continue in StopWorker2
        "b %[StopWorker2]"
        : : [StopWorker2] "g" (StopWorker2)
    );
}

/*!
 * Critical context switching code when interrupting workers (SysTick handler)
 * we cannot trust the compiler not to mix up register/stack allocations
 */
__attribute__((naked))
#ifdef LINKER_ORDERED_SECTION
__attribute__((section(LINKER_ORDERED_SECTION ".wrk.stop.0")))
#endif
static void InterruptWorker()
{
    __asm volatile (
        // just call StopWorker with a yield result, i.e. SleepTicks(0)
        "movs r0, #0\n"
        "movs r1, %[SleepTicks]\n"
        "mrs r2, psp\n"
        // continue in StopWorker2
        // just let it fall through if we can rely on linker ordering
#ifndef LINKER_ORDERED_SECTION
        "b %[StopWorker2]\n"
#endif
        : : [SleepTicks] "i" (AsyncResult::SleepTicks), [StopWorker2] "g" (StopWorker2)
    );
}

/*!
 * Critical context switching code when stopping/yielding workers (shared by SysTick and SVC handler)
 * we cannot trust the compiler not to mix up register/stack allocations
 * expects async_res_t in R0/R1 and PSP in R2
 */
__attribute__((naked))
#ifdef LINKER_ORDERED_SECTION
__attribute__((section(LINKER_ORDERED_SECTION ".wrk.stop.2")))
#endif
static void StopWorker2()
{
    __asm volatile (
        // we'll be returning to MSP
        "bic lr, #4\n"
        // overwrite the stack R0/R1 values with current ones (async_res_t)
        "strd r0, r1, [sp]\n"
        // save registers not handled by handler entry below PSP
        "stmdb r2!, {r4-r11}\n"
#ifndef __SOFTFP__
        "vstmdb r2!, {s16-s31}\n"
#endif
        // end of critical part, handle the rest in StopWorker3
        // just let it fall through if we can rely on linker ordering
#ifndef LINKER_ORDERED_SECTION
        "b %[StopWorker3]"
#endif
        : : [StopWorker3] "g" (StopWorker3)
    );
}

//! Rest of the handling when stopping a worker after register switching is complete
#ifdef LINKER_ORDERED_SECTION
__attribute__((section(LINKER_ORDERED_SECTION ".wrk.stop.3")))
#endif
static void StopWorker3(intptr_t asyncVal, AsyncResult asyncRes)
{
#if TRACE && WORKER_TRACE_ENTRY_EXIT
    ITM->PORT[0].u8 = '>';
#endif

    // stop the SysTick
    SysTick->CTRL = 0;
    // clear possible pending SysTick in case the stop is called via SVC
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
};

/*!
 * Critical context switching code when starting workers (SVC handler)
 * we cannot trust the compiler not to mix up register/stack allocations
 */
__attribute__((naked))
#ifdef LINKER_ORDERED_SECTION
__attribute__((section(LINKER_ORDERED_SECTION ".wrk.start.1")))
#endif
static void StartWorker()
{
    __asm volatile (
        // we'll be returning to PSP
        "orr lr, #4\n"
        // load R0 from PSP because previous handler could have corrupted it
        "ldr r0, [sp]\n"
        // restore registers not handled by handler return from below PSP
        "mrs r2, psp\n"
        "ldmdb r2!, {r4-r11}\n"
#ifndef __SOFTFP__
        "vldmdb r2!, {s16-s31}\n"
#endif
        // end of critical part, handle the rest in StartWorker2
        // just let it fall through if we can rely on linker ordering
#ifndef LINKER_ORDERED_SECTION
        "b %[StartWorker2]"
#endif
        : : [StartWorker2] "g" (StartWorker2)
    );
}

//! Rest of the handling when starting a worker after register switching is complete
#ifdef LINKER_ORDERED_SECTION
__attribute__((section(LINKER_ORDERED_SECTION ".wrk.start.2")))
#endif
static void StartWorker2(bool noPreempt)
{
#if TRACE && WORKER_TRACE_ENTRY_EXIT
    ITM->PORT[0].u8 = '<';
#endif

    // change the SVCall handler to StopWorker
    g_isrTableSys[SVCall_IRQn + NVIC_USER_IRQ_OFFSET] = (handler_t)StopWorker;
    __DSB();
    __ISB();

    if (!noPreempt)
    {
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    }
}

OPTIMIZE async(CortexWorker::RunWorker)
{
    // change the SVCall handler to StartWorker
    g_isrTableSys[SVCall_IRQn + NVIC_USER_IRQ_OFFSET] = StartWorker;

    // load PSP
    __set_PSP(uint32_t(sp));
    __DSB();
    __ISB();

#if CORTEX_WORKER_MPU_STACK_GUARD
    // make sure the bottom of stack falls in a no access region
    MPU->RBAR = ((uint32_t(stack) + 31) & ~31) | 0x10;
    MPU->RASR = MPU_RASR_ENABLE_Msk | (4 << MPU_RASR_SIZE_Pos);
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
#endif

    register intptr_t r0 asm ("r0") = noPreempt;
    register AsyncResult r1 asm ("r1");
    __asm volatile (
        // switch to PSP, we get back after yield or interrupt
        "svc #0\n"
        : "=r" (r0), "=r" (r1) : "r" (r0) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "cc", "memory"
    );
    auto res = _ASYNC_RES(r0, r1);

#if CORTEX_WORKER_MPU_STACK_GUARD
    MPU->CTRL = 0;
#endif
    g_isrTableSys[SVCall_IRQn + NVIC_USER_IRQ_OFFSET] = (handler_t)Missing_Handler;

    // store PSP
    sp = (uint32_t*)__get_PSP();

#if TRACE
    if (*stack != STACK_MAGIC || sp < stack)
    {
        if (sp < stack)
        {
            DBGL("STACK OVERFLOW: %p < %p", sp, stack);
        }
        if (*stack != STACK_MAGIC)
        {
            DBGL("STACK CORRUPTION @ %p", stack);
        }
        ASSERT(false);
    }
#endif

    if (_ASYNC_RES_TYPE(res) <= AsyncResult::Complete)
    {
        if (stackAlloc)
        {
            stackAlloc->Free(stack);
        }
        else
        {
            free(stack);
        }
        MemPoolFreeDynamic(this);
    }
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

async_once(Worker::Run)
{
    size_t stackSize = this->stackSize;
    void* allocd = NULL;
    if (stackAlloc)
    {
        allocd = stackAlloc->Allocate(stackSize);
    }
    if (!allocd)
    {
        stackAlloc = NULL;
        allocd = malloc(stackSize);
    }
    size_t stackWords = stackSize / 4;
    stack = (uint32_t*)allocd;

    stack[0] = STACK_MAGIC;
#if TRACE
    for (size_t i = 1; i < stackWords; i++)
    {
        stack[i] = STACK_MAGIC;
    }
#endif
    sp = stack + stackWords - ISRStack;
    // stack must be aligned to 8 bytes
    sp = (uint32_t*)((uintptr_t)sp & ~7);
    memset(sp - BelowStack, 0, (BelowStack + ISRStack) * sizeof(uint32_t));

    sp[7] = BIT(24);  // set THUMB in xPSR
    sp[6] = uint32_t((void*)run) | 1;        // initial PC
    sp[5] = uint32_t(WorkerDone);                 // LR
    sp[0] = uint32_t(this);    // initial R0

    // initialize systick
    SysTick->CTRL = 0;
    SysTick->VAL = 0;
    SysTick->LOAD = SystemCoreClock / 3000;

    Cortex_SetIRQHandler(SysTick_IRQn, InterruptWorker);
    // use lowest non-masked priority for SysTick
    // it will be masked in critical sections to prevent preemption
    NVIC_SetPriority(SysTick_IRQn, CORTEX_WORKER_PRIO);

    return async_forward(Task::Switch, GetDelegate((CortexWorker*)this, &CortexWorker::RunWorker), trySync);
}

bool Worker::CanAwait()
{
    return g_isrTableSys[SVCall_IRQn + NVIC_USER_IRQ_OFFSET] == (handler_t)StopWorker;
}

}
