/*
 * Copyright (c) 2024 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/hw/IRQ.h
 */

#pragma once

#include <base/base.h>

class IRQ
{
private:
    IRQn_Type num;

public:
    constexpr IRQ(IRQn_Type num = (IRQn_Type)0)
        : num(num) { ASSERT(num >= 0); }

    ALWAYS_INLINE bool IsEnabled() const { if (num < 0) __builtin_unreachable(); return NVIC_GetEnableIRQ(num); }
    ALWAYS_INLINE void Enable() const { if (num < 0) __builtin_unreachable(); NVIC_EnableIRQ(num); }
    ALWAYS_INLINE void Disable() const { if (num < 0) __builtin_unreachable(); NVIC_DisableIRQ(num); }
    ALWAYS_INLINE void Trigger() const { NVIC->STIR = num; }
    ALWAYS_INLINE void Priority(unsigned prio) const { if (num < 0) __builtin_unreachable(); NVIC_SetPriority(num, prio); }
    ALWAYS_INLINE unsigned Priority() const { if (num < 0) __builtin_unreachable(); return NVIC_GetPriority(num); }
    ALWAYS_INLINE void SetHandler(Delegate<void> handler) const { Cortex_SetIRQHandler(num, handler); }
    ALWAYS_INLINE void SetHandler(cortex_handler_t handler) const { Cortex_SetIRQHandler(num, handler); }
    template<class T> ALWAYS_INLINE void SetHandler(T* target, void (T::*method)()) const
        { SetHandler(Delegate(target, method)); }
    template<class T> ALWAYS_INLINE void SetHandler(const T* target, void (T::*method)() const) const
        { SetHandler(Delegate(target, method)); }
    ALWAYS_INLINE void ResetHandler() const { Cortex_ResetIRQHandler(num); }
    ALWAYS_INLINE const void* HandlerArgument() const { return Cortex_GetIRQHandlerArg(num); }
};
