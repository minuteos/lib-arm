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

    void EnableFaults() { SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk; }

    void Sleep() { DisableDeepSleep(); WaitForInterrupt(); }
    void DeepSleep() { EnableDeepSleep(); WaitForInterrupt(); }
    void DisableDeepSleep() { SCR &= ~SCB_SCR_SLEEPDEEP_Msk; }
    void EnableDeepSleep() { SCR |= SCB_SCR_SLEEPDEEP_Msk; }
    void EnableWake(IRQn_Type irq) { NVIC->ISER[irq >> 5] = NVIC->ICPR[irq >> 5] = BIT(irq & 31); }
    void TriggerWake(IRQn_Type irq) { NVIC->ISPR[irq >> 5] = BIT(irq & 31); }
    void DisableWake(IRQn_Type irq) { NVIC->ICER[irq >> 5] = BIT(irq & 31); }

    bool IRQPending() { return ICSR & SCB_ICSR_ISRPENDING_Msk; }
    IRQn_Type PendingIRQn() { return IRQn_Type(((ICSR & SCB_ICSR_VECTPENDING_Msk) >> SCB_ICSR_VECTPENDING_Pos) - NVIC_USER_IRQ_OFFSET); }
    IRQn_Type ActiveIRQn() { return IRQn_Type(((ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos) - NVIC_USER_IRQ_OFFSET); }

    //! interrupts must be disabled, but we must lift BASEPRI limit to allow any interrupt to wake us from WFI
    ALWAYS_INLINE void WaitForInterrupt() { auto bp = __get_BASEPRI(); __set_BASEPRI(0); __WFI(); __set_BASEPRI(bp); }
};
