/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * lib-arm/targets/cortex-m/hw/SCB.cpp
 */

#include "SCB.h"

void _SCB::WaitForInterrupt()
{
    __set_BASEPRI(0);   // interrupts are disabled, but we must lift BASEPRI limit to allow any interrupt to wake us from WFI

    // now we clean all low priority IRQs except those that are backed by active signals
    auto pend = PendingIRQn();
    while (pend >= 0 && NVIC_GetPriority(pend) == MASK(__NVIC_PRIO_BITS))
    {
        NVIC_ClearPendingIRQ(pend);
        auto next = PendingIRQn();
        if (pend == next)
            break;          // unable to clear pending IRQ, leave it be...
        pend = next;
    }

    __WFI();

    __set_BASEPRI(0xFF);    // go back to blocking lowest priority interrupts
}
