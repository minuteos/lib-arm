/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m3/kernel/platform.h
 *
 * Kernel support for Cortex-M3/4 MCUs
 */

#include_next <kernel/platform.h>

#ifndef PLATFROM_ATOMIC_EXCHANGE_8
#define PLATFORM_ATOMIC_EXCHANGE_8(target, value) ({ uint8_t res; do { res = __LDREXB(target); } while (__STREXB(value, target)); res; })
#endif

#ifndef PLATFROM_ATOMIC_EXCHANGE_16
#define PLATFORM_ATOMIC_EXCHANGE_16(target, value) ({ uint16_t res; do { res = __LDREXH(target); } while (__STREXH(value, target)); res; })
#endif

#ifndef PLATFROM_ATOMIC_EXCHANGE_32
#define PLATFORM_ATOMIC_EXCHANGE_32(target, value) ({ uint32_t res; do { res = __LDREXW(target); } while (__STREXW(value, target)); res; })
#endif
