/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/nvram/Flash.cpp
 *
 * Emulated "FLASH" in qemu-arm
 * Because qemu-system-arm does not emulate the FLASH peripheral, we just fake it in RAM
 */

#include "Flash.h"

namespace nvram
{

struct FlashImpl
{
    FlashImpl()
    {
        memset(data, 0xFF, EMULATED_FLASH_SIZE);
    }

    uint8_t data[EMULATED_FLASH_SIZE];
} flash __attribute__((aligned(EMULATED_FLASH_PAGE_SIZE)));

Span Flash::GetRange()
{
    return flash.data;
}

bool Flash::Write(const void* ptr, Span data)
{
    char* p = (char*)ptr;
    for (char ch: data)
        *p++ &= ch;
    return data == Span(ptr, data.Length());
}

#if NVRAM_FLASH_DOUBLE_WRITE

void Flash::ShredDouble(const void* ptr)
{
    ASSERT(!(uintptr_t(ptr) & 7));
    auto p = (uint32_t*)ptr;
    p[0] = p[1] = 0;
}

bool Flash::WriteDouble(const void* ptr, uint32_t lo, uint32_t hi)
{
    ASSERT(!(uintptr_t(ptr) & 7));
    auto p = (uint32_t*)ptr;
    p[0] &= lo;
    p[1] &= hi;
    return p[0] == lo && p[1] == hi;
}

#else

void Flash::ShredWord(const void* ptr)
{
    ASSERT(!(uintptr_t(ptr) & 3));
    *(uint32_t*)ptr = 0;
}

bool Flash::WriteWord(const void* ptr, uint32_t word)
{
    ASSERT(!(uintptr_t(ptr) & 3));
    *(uint32_t*)ptr &= word;
    return *(uint32_t*)ptr == word;
}

#endif

bool Flash::Erase(Span range)
{
    memset((void*)range.Pointer(), 0xFF, range.Length());
    return true;
}

async(Flash::ErasePageAsync, const void* ptr)
async_def()
{
    memset((void*)ptr, 0xFF, EMULATED_FLASH_PAGE_SIZE);
    async_delay_ms(10);
    async_return(true);
}
async_end

}
