/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/base/platform.h
 *
 * Platform definitions for QEMU semihosting
 */

#pragma once

#define PLATFORM_DBG_CHAR(channel, ch) angel_output(NULL, ch)
#define PLATFORM_DBG_ACTIVE(channel) ((channel) == 0)

#define CORTEX_STARTUP_MAIN()    angel_main()

#define MONO_US (angel_clock() * 10000)

BEGIN_EXTERN_C

void angel_output(void* context, char ch);
uint32_t angel_clock();
void angel_main();

END_EXTERN_C

#define SystemCoreClock 32000000

#include_next <base/platform.h>
