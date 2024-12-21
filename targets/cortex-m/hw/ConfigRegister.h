/*
 * Copyright (c) 2024 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/hw/ConfigRegister.h
 *
 * A helper for combining values for hardware configuration registers
 */

#pragma once

#include <base/base.h>

template<auto Register>
struct ConfigRegister {};

template<typename Peripheral, typename RegisterType, RegisterType Peripheral::*Register>
struct ConfigRegister<Register>
{
    using reg_t = std::remove_volatile_t<RegisterType>;

    ALWAYS_INLINE constexpr ConfigRegister(reg_t value, reg_t mask)
        : value(value & mask), mask(mask) {}

    ALWAYS_INLINE constexpr ConfigRegister(bool value, reg_t mask)
        : value(value * mask), mask(mask) {}

    ALWAYS_INLINE constexpr ConfigRegister operator |(const ConfigRegister&& other)
    {
        return { value | other.value, mask | other.mask };
    }

    ALWAYS_INLINE constexpr void Apply(Peripheral* periph) const
    {
        periph->*Register = (periph->*Register & ~mask) | value;
    }

    reg_t value, mask;
};

#define DEFINE_CONFIGURE_METHOD(peripheral) \
    template<typename TReg, TReg peripheral::*Reg> constexpr void Configure(const ConfigRegister<Reg>& cfg) { cfg.Apply(this); }
