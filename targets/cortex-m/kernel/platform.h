/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/kernel/platform.h
 *
 * Kernel support for Cortex-M MCUs
 */

#include_next <kernel/platform.h>

// use standard CMSIS macros for CPSID/CPSIE

#ifndef PLATFORM_DISABLE_INTERRUPTS
#define PLATFORM_DISABLE_INTERRUPTS __disable_irq
#endif

#ifndef PLATFORM_ENABLE_INTERRUPTS
#define PLATFORM_ENABLE_INTERRUPTS __enable_irq
#endif

#if !defined(PLATFORM_SLEEP) && defined(CORTEX_SCHEDULE_WAKEUP) && defined(CORTEX_CLEAN_WAKEUP)
#define CORTEX_DEFAULT_SLEEP
extern void Cortex_Sleep(mono_t until);
#define PLATFORM_SLEEP(since, duration) Cortex_Sleep((since) + (duration))

#if CORTEX_DEEP_SLEEP_ENABLED
extern void Cortex_DeepSleep(mono_t until);
extern int Cortex_NoDeepSleep;
#define PLATFORM_DEEP_SLEEP_DISABLE() Cortex_NoDeepSleep++;
#define PLATFORM_DEEP_SLEEP_ENABLE() Cortex_NoDeepSleep--;
#endif

#endif
