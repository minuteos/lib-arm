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

#include_next <base/platform.h>

#define CM_PERIPHERAL(type, base) ((type*)(base))

// various macros that can be overriden in target specific cortex_defs.h
#ifndef CORTEX_HALT
#define CORTEX_HALT(n)  for(;;)
#endif

typedef union { uint8_t u8; uint16_t u16; uint32_t u32; } Cortex_ITM_Port_t;

EXTERN_C volatile Cortex_ITM_Port_t* Cortex_GetDebugChannel(unsigned channel);

#define PLATFORM_DBG_ACTIVE(channel)        ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) && (ITM->TER & (1 << (channel))))
#define PLATFORM_DBG_CHAR(channel, char)    ({ Cortex_GetDebugChannel(channel)->u8 = (char); })
