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

#ifndef PLATFORM_DISABLE_INTERRUPTS
#define PLATFORM_DISABLE_INTERRUPTS() Cortex_SetPriorityLevel(CORTEX_PRESLEEP_PRIO)
#endif

#ifndef PLATFORM_ENABLE_INTERRUPTS
#define PLATFORM_ENABLE_INTERRUPTS() Cortex_SetPriorityLevel(CORTEX_DEFAULT_PRIO)
#endif

#ifndef PLATFORM_CYCLE_COUNT
#define PLATFORM_CYCLE_COUNT       (DWT->CYCCNT)
#endif

#ifndef PLATFORM_WAKE_REASON
#define PLATFORM_WAKE_REASON       (((SCB->ICSR & SCB_ICSR_VECTPENDING_Msk) >> SCB_ICSR_VECTPENDING_Pos) - NVIC_USER_IRQ_OFFSET)
#endif

#ifndef PLATFORM_KERNEL_WORKER_SUPPORT
#define PLATFORM_KERNEL_WORKER_SUPPORT    1
#endif

#if defined(CORTEX_SCHEDULE_WAKEUP) && defined(CORTEX_CLEAN_WAKEUP)

#define CORTEX_DEFAULT_SLEEP_AVAILABLE 1

extern void Cortex_Sleep(mono_t until);
extern void Cortex_DeepSleep(mono_t until);

#if CORTEX_DEEP_SLEEP_ENABLED && !defined(PLATFORM_DEEP_SLEEP_ENABLED)

#define CORTEX_DEFAULT_DEEP_SLEEP   1
extern int Cortex_NoDeepSleep;

#define PLATFORM_DEEP_SLEEP_DISABLE() (Cortex_NoDeepSleep++)
#define PLATFORM_DEEP_SLEEP_ENABLE() (Cortex_NoDeepSleep--)
#define PLATFORM_DEEP_SLEEP_ENABLED() (!Cortex_NoDeepSleep)

#endif  /* CORTEX_DEEP_SLEEP_ENABLED && !defined(PLATFORM_DEEP_SLEEP_ENABLED) */

#endif  /* defined(CORTEX_SCHEDULE_WAKEUP) && defined(CORTEX_CLEAN_WAKEUP) */

#if !defined(PLATFORM_SLEEP) && CORTEX_DEFAULT_SLEEP_AVAILABLE
#define CORTEX_DEFAULT_SLEEP    1
#define PLATFORM_SLEEP(since, duration) Cortex_Sleep((since) + (duration))
#endif
