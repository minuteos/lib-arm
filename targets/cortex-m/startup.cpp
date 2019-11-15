/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * startup.cpp
 *
 * Startup code for Cortex-M based MCUs
 */

#include <base/base.h>
#include <base/Delegate.h>

#include <hw/SCB.h>

#include <ld_symbols.h>

// actual handlers
extern "C" void Reset_Handler() __attribute__((naked, noreturn, alias("Default_Reset_Handler")));
extern "C" void Interrupt_Handler() __attribute__((naked));
extern "C" void HardFault_Handler() __attribute__((naked, noreturn));
extern "C" void Missing_Handler(void*) __attribute__((naked, noreturn));

// system ISRs + IRQs
#define ISR_COUNT	(NVIC_USER_IRQ_OFFSET + EXT_IRQ_COUNT)

typedef void (*handler_t)(void);

__attribute__ ((section(".isr_vector"), externally_visible))
extern handler_t const g_initialVectors[] =
{
    (handler_t)&__stack_end,	// initial stack pointer
    &Reset_Handler,				// application entry
    // the remaining handlers can be initialized once the interrupt table is moved to RAM
};

// Delegate table for actual ISRs
// LD script must enforce this to be located at the start of RAM (0x20000000)
__attribute__ ((section(".bss.isr_vector")))
Delegate<void> g_isrTable[ISR_COUNT];

// VTABLE used by the core, actually filled with Interrupt_Handler routines
// LD script must enforce 512B alignment of this section
__attribute__ ((section(".bss.isr_vector_sys")))
handler_t g_isrTableSys[ISR_COUNT];

extern "C" void Interrupt_Handler()
{
    __asm volatile( \
        "mrs r1, ipsr\n"
        "mov r0, r1, lsl #3\n"
        "movt r0, #0x2000\n"
        "ldmia r0, {r0, pc}\n"
    );
}

extern "C" void Missing_Handler(void* arg0)
{
#if TRACE
    uint32_t* regs;
    uint32_t* regs2;

    __asm volatile (
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq %0, msp\n"
        "mrsne %0, psp\n"
        "push {r4-r11}\n"	// make the remaining registers available
        "mov %1, sp\n"
    : "=r" (regs), "=r" (regs2));

    DBG("Unhandled IRQ: %d\n", SCB->ActiveIRQn());
    DBG("R0: %08x  R1: %08x  R2: %08x  R3: %08x\n", regs[0], regs[1], regs[2], regs[3]);
    DBG("R4: %08x  R5: %08x  R6: %08x  R7: %08x\n", regs2[0], regs2[1], regs2[2], regs2[3]);
    DBG("R8: %08x  R9: %08x  R10:%08x  R11:%08x\n", regs2[4], regs2[5], regs2[6], regs2[7]);
    DBG("R12:%08x  LR: %08x  PC: %08x  PSR:%08x\n", regs[4], regs[5], regs[6], regs[7]);
#endif
    CORTEX_HALT(1);
}

extern "C" void HardFault_Handler()
{
#if TRACE
    uint32_t* regs;
    uint32_t* regs2;

    __asm volatile (
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq %0, msp\n"
        "mrsne %0, psp\n"
        "push {r4-r11}\n"	// make the remaining registers available
        "mov %1, sp\n"
    : "=r" (regs), "=r" (regs2));

    DBG("HARD FAULT!\n");
    DBG("HFSR: %08x\n", *(int*)0xE000ED2C);
    DBG("DFSR: %08x\n", *(int*)0xE000ED30);
    DBG("LFSR: %08x\n", *(int*)0xE000ED28);
    DBG("BFAR: %08x\n", *(int*)0xE000ED38);
    DBG("R0: %08x  R1: %08x  R2: %08x  R3: %08x\n", regs[0], regs[1], regs[2], regs[3]);
    DBG("R4: %08x  R5: %08x  R6: %08x  R7: %08x\n", regs2[0], regs2[1], regs2[2], regs2[3]);
    DBG("R8: %08x  R9: %08x  R10:%08x  R11:%08x\n", regs2[4], regs2[5], regs2[6], regs2[7]);
    DBG("R12:%08x  LR: %08x  PC: %08x  PSR:%08x\n", regs[4], regs[5], regs[6], regs[7]);
#endif
    CORTEX_HALT(1);
}

void Cortex_SetIRQHandler(IRQn_Type IRQn, Delegate<void> handler)
{
    g_isrTable[IRQn + NVIC_USER_IRQ_OFFSET] = handler;
}

void Cortex_SetIRQHandler(IRQn_Type IRQn, handler_t handler)
{
    g_isrTableSys[IRQn + NVIC_USER_IRQ_OFFSET] = handler;
}

// symbols provided by LD
extern handler_t __init_array_start[];
extern handler_t __init_array_end[];
extern int main(void);

extern "C" __attribute__((noreturn)) void Default_Reset_Handler()
{
#ifdef CORTEX_TRACK_STACK_USAGE
    __asm volatile(
        "mov r0,%0\n"
        "0: stmia %1!, {r0,%0}\n"
        "cmp sp, %1\n"
        "bgt 0b\n" : : "r"(STACK_MAGIC), "l"(&__stack_start) : "r0"
    );
#endif

    SCB->EnableFPU();
    __set_BASEPRI(0xFF);    // lowest priority IRQs only wake up the MCU, but don't execute handlers
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

#ifdef CORTEX_STARTUP_BEFORE_INIT
    CORTEX_STARTUP_BEFORE_INIT();
#endif

    // copy pre-initialized RW data
    memcpy(&__data_start, &__data_load, &__data_end - &__data_start);

    // zeroing of BSS
    memset(&__bss_start, 0, &__bss_end - &__bss_start);

    // prepare the ISR table
    // do not touch entry 0, as it's not a real ISR and is may be used by bootloaders for communication
    for (uint32_t i = 1; i < ISR_COUNT; i++)
    {
        g_isrTableSys[i] = &Interrupt_Handler;
        g_isrTable[i] = &Missing_Handler;
    }

#if TRACE
    Cortex_SetIRQHandler(HardFault_IRQn, &HardFault_Handler);
#endif

    // activate the ISR table prepared in RAM
    SCB->SetISRTable(g_isrTableSys);

#ifdef CORTEX_STARTUP_HARDWARE_INIT
    // hardware init (clocks etc.) before static constructors
    CORTEX_STARTUP_HARDWARE_INIT();
#endif

#ifdef CORTEX_STARTUP_BEFORE_C_INIT
    CORTEX_STARTUP_BEFORE_C_INIT();
#endif

#if BOOTLOADER
    DBG("============= BOOTLOADER =============\n");
#else
    DBG("=============== RESET ===============\n");
#endif

    // CRT init (static constructors, global initializers, etc...)
    for (handler_t* initptr = __init_array_start; initptr < __init_array_end; initptr++)
    {
        DBG("init: %08X\n", *initptr);
        (*initptr)();
    }

#ifdef CORTEX_STARTUP_BEFORE_MAIN
    CORTEX_STARTUP_BEFORE_MAIN();
#endif

    // finally
    DBG("init: starting main()\n");
    main();

#if BOOTLOADER
    // when main completes, jump to the actual application
    SCB->VTOR = 0;
    __asm volatile(
        "movs r0, #0\n"
        "ldr sp, [r0]\n"
        "ldr r0, [r0, #4]\n"
        "blx r0\n"
    );
#endif

    // main should never return, but if it does...
    CORTEX_HALT(0);
}
