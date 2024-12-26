/*
 * Copyright (c) 2024 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/kernel/PlatformWorkerInline.h
 */

#pragma once

#include <base/base.h>

namespace kernel
{

template<> ALWAYS_INLINE void Worker::Yield(async_res_t res)
{
    register intptr_t r0 asm("r0") = _ASYNC_RES_VALUE(res);
    register AsyncResult r1 asm("r1") = _ASYNC_RES_TYPE(res);
    __asm volatile("svc #0" : : "r" (r0), "r" (r1));
}

}
