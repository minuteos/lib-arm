/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/cortex_defs.h
 *
 * Additional definitions for running under QEMU
 */

#pragma once

#include_next <cortex_defs.h>

#define CORTEX_HALT(res) exit(res)
