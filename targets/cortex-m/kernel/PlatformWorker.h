/*
 * Copyright (c) 2024 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/kernel/PlatformWorker.h
 */

#pragma once

#include <base/base.h>

#ifndef PLATFORM_WORKER_CLASS_BASE
#define PLATFORM_WORKER_CLASS_BASE    CortexWorker
#endif

namespace kernel
{

class CortexWorker
{
private:
    CortexWorker(const WorkerOptions& opts)
        : stackSize(opts.stack), noPreempt(opts.noPreempt), trySync(opts.trySync) {}

    union { size_t stackSize; uint32_t* stack; };
    uint32_t* sp;
    bool noPreempt, trySync;

    async(RunWorker);

    friend class Worker;
};

}

#include_next <kernel/Worker.h>
