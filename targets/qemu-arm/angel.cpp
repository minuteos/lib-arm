/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * qemu-arm/syscalls.cpp
 *
 * Implementation of printf using the ARM Angel interface
 */

#include <base/base.h>
#include <base/format.h>

#include <sys/stat.h>

enum struct SysCall
{
    Open = 0x01,
    WriteChar = 0x03,
    Clock = 0x10,
    Exit = 0x18,
};

enum struct ExitReason
{
    Success = 0x20026,
    Error = 0, // in fact anything other than the above value
};

extern "C" {

static inline int angel(SysCall swi, const void* data = NULL)
{
    int res;
    asm volatile ("mov r0, %1; mov r1, %2; bkpt 0xab; mov %0, r0"
        : "=r" (res)
        : "r" (swi), "r" (data)
        : "r0", "r1", "memory", "cc");
    return res;
}

void exit(int err)
{
    angel(SysCall::Exit, (const void*)(err ? ExitReason::Error : ExitReason::Success));
    for (;;);
}

void abort(void)
{
    angel(SysCall::Exit, (const void*)ExitReason::Error);
    for (;;);
}

void angel_output(void* context, char ch)
{
    angel(SysCall::WriteChar, &ch);
}

uint32_t angel_clock()
{
    return angel(SysCall::Clock, 0);
}

int puts(const char* s)
{
    int i = 0;
    while (s[i])
        angel_output(NULL, s[i++]);
    angel_output(NULL, '\n');
    return i;
}

int printf(const char* fmt, ...)
{
    return va_call(vformat, fmt, angel_output, NULL, fmt);
}

}
