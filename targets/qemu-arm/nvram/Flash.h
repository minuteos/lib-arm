/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/nvram/Flash.h
 *
 * Emulated FLASH in QEMU
 */

#include <base/base.h>

#ifndef EMULATED_FLASH_SIZE
#define EMULATED_FLASH_SIZE     16384
#endif

#ifndef EMULATED_FLASH_PAGE_SIZE
#define EMULATED_FLASH_PAGE_SIZE    1024
#endif

namespace nvram
{

class Flash
{
public:
    static constexpr uintptr_t PageSize = EMULATED_FLASH_PAGE_SIZE;

    static Span GetRange();

    static bool Write(const void* ptr, Span data);
    static bool WriteWord(const void* ptr, uint32_t word);
    static void ShredWord(const void* ptr);
    static bool Erase(Span range);
};

}