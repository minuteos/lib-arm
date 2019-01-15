/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/kernel/platform.h
 * 
 * Kernel support for QEMU semihosting
 */

#pragma once

#define MONO_CLOCKS     angel_clock()
#define MONO_US (angel_clock() * 10000)

typedef uint32_t mono_t;

#define PLATFORM_SLEEP(...)

#include_next <kernel/platform.h>
