/*
 * SCB.h
 *
 *  Created on: Sep 7, 2017
 *      Author: pista
 */

#pragma once

#include <base/base.h>

#undef SCB
#define SCB CM_PERIPHERAL(_SCB, SCB_BASE)

#ifndef NVIC_USER_IRQ_OFFSET
#define NVIC_USER_IRQ_OFFSET 16
#endif

class _SCB : public SCB_Type
{
public:
    typedef void (*IRQHandler)();

    void SetISRTable(IRQHandler* tbl) { VTOR = (uint32_t)tbl; }
    void SetIRQHandler(IRQn_Type irqn, IRQHandler h) { ((IRQHandler*)VTOR)[irqn + NVIC_USER_IRQ_OFFSET] = h; }

#ifdef __SOFTFP__
    void EnableFPU() {}
#else
    void EnableFPU()
    {
        CPACR |= 0x00F00000;
        __DSB();
        __ISB();
    }
#endif

    void Sleep() { DisableDeepSleep(); WaitForInterrupt(); }
    void DeepSleep() { EnableDeepSleep(); WaitForInterrupt(); }
    void DisableDeepSleep() { SCR &= ~SCB_SCR_SLEEPDEEP_Msk; }
    void EnableDeepSleep() { SCR |= SCB_SCR_SLEEPDEEP_Msk; }

    bool IRQPending() { return ICSR & SCB_ICSR_ISRPENDING_Msk; }
    IRQn_Type PendingIRQn() { return IRQn_Type(((ICSR & SCB_ICSR_VECTPENDING_Msk) >> SCB_ICSR_VECTPENDING_Pos) - NVIC_USER_IRQ_OFFSET); }
    IRQn_Type ActiveIRQn() { return IRQn_Type(((ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos) - NVIC_USER_IRQ_OFFSET); }

private:
    void WaitForInterrupt();
};
