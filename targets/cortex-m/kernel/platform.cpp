/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/kernel/platform.cpp
 *
 * Kernel support for Cortex-M MCUs
 */

#include <kernel/kernel.h>

#include <hw/SCB.h>

#if CORTEX_DEFAULT_SLEEP_AVAILABLE

#if CORTEX_DEFAULT_DEEP_SLEEP
int Cortex_NoDeepSleep;
#endif

void Cortex_Sleep(mono_t wakeAt)
{
    mono_t sleepAt = MONO_CLOCKS;

#if CORTEX_DEEP_SLEEP_ENABLED
    if (PLATFORM_DEEP_SLEEP_ENABLED())
    {
        auto wakeDelayUs = CORTEX_DEEP_SLEEP_RESTORE_US();
        auto wakeDelayTicks = MonoFromMicroseconds(wakeDelayUs);
#ifdef CORTEX_DEEP_SLEEP_BEFORE
        bool canSleep = CORTEX_DEEP_SLEEP_BEFORE();
        sleepAt = MONO_CLOCKS;
#else
        bool canSleep = true;
#endif
        if (canSleep && OVF_DIFF(wakeAt - wakeDelayTicks, sleepAt) >= 2)
        {
            Cortex_DeepSleep(wakeAt - wakeDelayTicks);

#ifdef CORTEX_DEEP_SLEEP_AFTER
            CORTEX_DEEP_SLEEP_AFTER();
#endif
            return;
        }
    }
#endif

    if (OVF_DIFF(wakeAt, sleepAt) >= 2)
    {
        CORTEX_SCHEDULE_WAKEUP(wakeAt);
        SCB->Sleep();
        CORTEX_CLEAN_WAKEUP();
    }
}

#if CORTEX_DEEP_SLEEP_ENABLED

void Cortex_DeepSleep(mono_t wakeAt)
{
    mono_t sleepAt = MONO_CLOCKS;
    if (OVF_DIFF(wakeAt, sleepAt) < 2)
        return;

    CORTEX_SCHEDULE_WAKEUP(wakeAt);
#ifdef CORTEX_DEEP_SLEEP_PREPARE
    CORTEX_DEEP_SLEEP_PREPARE();
#endif
    SCB->DeepSleep();
#ifdef CORTEX_DEEP_SLEEP_RESTORE
    CORTEX_DEEP_SLEEP_RESTORE();
#endif
}

#endif  /* CORTEX_DEEP_SLEEP_ENABLED */

#endif  /* CORTEX_DEFAULT_SLEEP */
