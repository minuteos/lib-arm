/*
 * Copyright (c) 2024 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/malloc_internal.h
 *
 * Exposes the internals of the Cortex malloc implementation for diagnostics
 */

#pragma once

#include <base/base.h>

#include <ld_symbols.h>

struct __malloc_free_list
{
    struct __malloc_free_list* next;
    size_t size;
};

struct __malloc_heap
{
    __malloc_free_list* free;
    size_t fragments;
    void* top = &__heap_start;
    void* limit = &__heap_end;

    constexpr size_t Total() { return &__heap_end - &__heap_start; }
    constexpr size_t Committed() { return (char*)top - &__heap_start; }
    constexpr size_t Fragments() { return fragments; }
    constexpr size_t Used() { return Committed() - fragments; }
    constexpr size_t ContiguousFree() { return (char*)limit - (char*)top; }
    constexpr size_t Free() { return ContiguousFree() + fragments; }
    constexpr size_t Once() { return &__heap_end - (char*)limit; }
};

extern __malloc_heap __heap;
