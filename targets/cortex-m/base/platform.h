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

typedef union { uint8_t u8; uint16_t u16; uint32_t u32; } Cortex_ITM_Port_t;

EXTERN_C volatile Cortex_ITM_Port_t* Cortex_GetDebugChannel(unsigned channel);

#if !defined(PLATFORM_DBG_ACTIVE) && !defined(PLATFORM_DBG_CHAR)
#define PLATFORM_DBG_ACTIVE(channel)        ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) && (ITM->TER & (1 << (channel))))
#define PLATFORM_DBG_CHAR(channel, char)    ({ Cortex_GetDebugChannel(channel)->u8 = (char); })
#endif

typedef void (*cortex_handler_t)(void);

extern void Cortex_SetIRQHandler(IRQn_Type IRQn, cortex_handler_t handler);
static inline void Cortex_SetIRQWakeup(IRQn_Type IRQn) { NVIC_SetPriority(IRQn, 0xFF); }

#ifdef __cplusplus
#include <base/Delegate.h>
extern void Cortex_SetIRQHandler(IRQn_Type IRQn, Delegate<void> handler);
#endif
