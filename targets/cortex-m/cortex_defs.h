/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/cortex_defs.h
 *
 * Global defines specific to ARM Cortex-M MCUs
 */

#pragma once

#define HAS_MALLOC_ONCE 1

extern void* malloc_once(size_t size);
