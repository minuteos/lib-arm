/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/base/platform.h
 * 
 * Common includes for all Cortex-M MCUs
 */

#include <base/base.h>

// collect specific definitions for Cortex targets
#include <cortex_defs.h>

#include <cmsis.h>

#include_next <base/platform.h>

#define CM_PERIPHERAL(type, base) ((type*)(base))

// various macros that can be overriden in target specific cortex_defs.h
#ifndef CORTEX_HALT
#define CORTEX_HALT(n)  for(;;)
#endif
