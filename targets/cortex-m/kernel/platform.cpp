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

#ifdef CORTEX_DEFAULT_SLEEP

void Cortex_Sleep(mono_t wakeAt)
{
    mono_t sleepAt = MONO_CLOCKS;
	if (OVF_DIFF(wakeAt, sleepAt) >= 2)
    {
        CORTEX_SCHEDULE_WAKEUP(wakeAt);
        SCB->Sleep();
        CORTEX_CLEAN_WAKEUP();
    }
}

#endif

