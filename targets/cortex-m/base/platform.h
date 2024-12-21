/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/base/platform.h
 *
 * Common includes for all Cortex-M MCUs
 */

#include <base/base.h>

// collect specific definitions for Cortex targets
#include <cortex_defs.h>

#include <cmsis.h>

#ifdef Ccmsis_dsp

BEGIN_EXTERN_C

#include <arm_math.h>
#include <arm_const_structs.h>
#include <arm_common_tables.h>

END_EXTERN_C

#endif

#include_next <base/platform.h>

#define CM_PERIPHERAL(type, base) ((type*)(base))

// various macros that can be overriden in target specific cortex_defs.h
#ifndef CORTEX_HALT
#define CORTEX_HALT(n)  for(;;)
#endif

EXTERN_C int Cortex_DebugWrite(unsigned channelAndSize, uint32_t data);

ALWAYS_INLINE uint32_t* Cortex_Handler_ReadSP()
{
    register uint32_t* res __asm("r0");
    __asm volatile (
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n" : : : "r0");
    return res;
}

ALWAYS_INLINE uint32_t* Cortex_Handler_SaveR4_R11()
{
    register uint32_t* res __asm("r1");
    __asm volatile (
        "push {r4-r11}\n"
        "mov r1, sp\n" : : : "r1");
    return res;
}

#if !defined(PLATFORM_DBG_ACTIVE) && !defined(PLATFORM_DBG_CHAR)
#define PLATFORM_DBG_ACTIVE(channel)        Cortex_DebugWrite((channel) | 0x60, 0)
#define PLATFORM_DBG_CHAR(channel, char)    Cortex_DebugWrite((channel), (char))
#endif
#ifndef PLATFORM_DBG_HALFWORD
#define PLATFORM_DBG_HALFWORD(channel, hw)  Cortex_DebugWrite((channel) | 0x20, (hw))
#endif
#ifndef PLATFORM_DBG_WORD
#define PLATFORM_DBG_WORD(channel, word)    Cortex_DebugWrite((channel) | 0x40, (word))
#endif

#ifndef PLATFORM_DBG_BRACKET
#define PLATFORM_DBG_BRACKET()              (__get_IPSR() ? '{' : (__get_CONTROL() & 2) ? '(' : '[')
#endif

#ifndef PLATFORM_CRITICAL_SECTION
#define PLATFORM_CRITICAL_SECTION()     ::Cortex_CriticalContext __critical ## __LINE__
#endif

#define CORTEX_GET_BASEPRI(pri)   ((uint8_t)(((pri) << (8 - __NVIC_PRIO_BITS)) & 0xFF))

//! priority level in which the default code is running (interrupts on this level are masked)
#define CORTEX_DEFAULT_PRIO     0xFF
//! priority level on which worker switching takes place (masking this prevents race condition between workers and main code)
#define CORTEX_WORKER_PRIO      0xFE
//! priority level used while detecting changes before going to sleep (so that changes caused by interrupts are properly detected)
#define CORTEX_PRESLEEP_PRIO    1
//! maximum priority, interrupts are never masked under normal circumstances (used by hardware handlers and similar)
#define CORTEX_MAXIMUM_PRIO     0

typedef void (*cortex_handler_t)(void);
typedef void (*cortex_handler_arg_t)(void* arg);

extern void Cortex_SetIRQHandler(IRQn_Type IRQn, cortex_handler_t handler);
extern void Cortex_SetIRQHandlerWithArg(IRQn_Type IRQn, cortex_handler_arg_t handler, void* arg);
extern void Cortex_ResetIRQHandler(IRQn_Type IRQn);
extern void* Cortex_GetIRQHandlerArg(IRQn_Type IRQn);

//! Configures the specified interrupt to run at the lowest priority level that is never serviced (used only for waking up the core)
static ALWAYS_INLINE void Cortex_SetIRQWakeup(IRQn_Type IRQn) { NVIC_SetPriority(IRQn, CORTEX_DEFAULT_PRIO); }

//! Unconditionally sets the current priority level to the specified value
static ALWAYS_INLINE void Cortex_SetPriorityLevel(uint8_t pri) { __set_BASEPRI(CORTEX_GET_BASEPRI(pri)); }

//! Sets the current priority level to at least the specified value
//! @returns priority level to be restored using Cortex_RestorePriority
static ALWAYS_INLINE uint8_t Cortex_MaskPriority(uint8_t pri)
{
    uint8_t bpri = CORTEX_GET_BASEPRI(pri);
    uint8_t bp = __get_BASEPRI();
    if (bp > bpri)
    {
        __set_BASEPRI(bpri);
    }
    return bp;
}

//! Restores the priority level changed by Cortex_MaskPriority
static ALWAYS_INLINE void Cortex_RestorePriority(uint8_t bp) { __set_BASEPRI(bp); }

#ifdef __cplusplus

//! Sets the execution priority in the scope where a variable of this type is defined
template <uint8_t pri> struct Cortex_PriorityContext
{
    ALWAYS_INLINE Cortex_PriorityContext() { bp = Cortex_MaskPriority(pri); }
    ALWAYS_INLINE ~Cortex_PriorityContext() { Cortex_RestorePriority(bp); }

private:
    uint8_t bp;
};

using Cortex_CriticalContext = Cortex_PriorityContext<CORTEX_WORKER_PRIO>;

#include <base/Delegate.h>
extern void Cortex_SetIRQHandler(IRQn_Type IRQn, Delegate<void> handler);

#endif
