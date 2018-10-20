/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/platform.h
 * 
 * Platform definitions for QEMU semihosting
 */

#pragma once

#define PLATFORM_DBG_CHAR(channel, ch) angel_output(NULL, ch)
#define PLATFORM_DBG_ACTIVE(channel) ((channel) == 0)

#define MONO_US (angel_clock() * 10000)

BEGIN_EXTERN_C

void angel_output(void* context, char ch);
uint32_t angel_clock();

END_EXTERN_C

#include_next <base/platform.h>
