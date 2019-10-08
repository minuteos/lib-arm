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

void Flash::ShredWord(const void* ptr)
{
    *(uint32_t*)ptr = 0;
}

bool Flash::WriteWord(const void* ptr, uint32_t word)
{
    *(uint32_t*)ptr &= word;
    return *(uint32_t*)ptr == word;
}

bool Flash::Erase(Span range)
{
    memset((void*)range.Pointer(), 0xFF, range.Length());
    return true;
}

}
