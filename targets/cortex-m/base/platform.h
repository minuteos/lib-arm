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

typedef void (*cortex_handler_t)(void);
typedef void (*cortex_handler_arg_t)(void* arg);

extern void Cortex_SetIRQHandler(IRQn_Type IRQn, cortex_handler_t handler);
extern void Cortex_SetIRQHandlerWithArg(IRQn_Type IRQn, cortex_handler_arg_t handler, void* arg);
extern void Cortex_ResetIRQHandler(IRQn_Type IRQn);
extern void* Cortex_GetIRQHandlerArg(IRQn_Type IRQn);
static inline void Cortex_SetIRQWakeup(IRQn_Type IRQn) { NVIC_SetPriority(IRQn, 0xFF); }

#ifdef __cplusplus
#include <base/Delegate.h>
extern void Cortex_SetIRQHandler(IRQn_Type IRQn, Delegate<void> handler);
#endif
