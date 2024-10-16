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
    constexpr IRQ(IRQn_Type num)
        : num(num) {}

    void Enable() const { NVIC_EnableIRQ(num); }
    void Disable() const { NVIC_DisableIRQ(num); }
    void Trigger() const { NVIC->STIR = num; }
};
