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
#include <sys/file.h>
#include <errno.h>

enum struct SysCall
{
    Open = 0x01,
    Close = 0x02,
    WriteChar = 0x03,
    Write0 = 0x04,
    Write = 0x05,
    Read = 0x06,
    IsTty = 0x09,
    Seek = 0x0A,
    FLen = 0x0C,
    Clock = 0x10,
    ErrNo = 0x13,
    GetCmdLine = 0x15,
    Exit = 0x18,
    Elapsed = 0x30,
    TickFreq = 0x31,
};

enum struct ExitReason
{
    Success = 0x20026,
    Error = 0, // in fact anything other than the above value
};

static inline intptr_t angel(SysCall swi, intptr_t arg = 0)
{
    register intptr_t r0 asm("r0") = intptr_t(swi);
    register intptr_t r1 asm("r1") = intptr_t(arg);
    asm volatile ("bkpt 0xab" : "=r" (r0) : "r" (r0), "r" (r1) : "cc", "memory");
    return r0;
}

static inline intptr_t angel(SysCall swi, void* arg)
{
    return angel(swi, intptr_t(arg));
}

extern "C" {

int _open(const char* filename, int mode)
{
    int angelMode = 0;
    if (mode & O_BINARY)
    {
        angelMode |= 1;
    }
    if (mode & O_RDWR)
    {
        angelMode |= 2;
    }
    if (mode & O_APPEND)
    {
        angelMode |= 8;
    }
    else if (mode & (O_CREAT | O_TRUNC))
    {
        angelMode |= 4;
    }

    struct
    {
        const char* fn;
        int mode;
        size_t len;
    } arg = { filename, angelMode, strlen(filename) };

    auto res = angel(SysCall::Open, &arg);
    if (res == -1)
    {
        errno = angel(SysCall::ErrNo);
    }
    return res;
}

int _close(int fd)
{
    auto res = angel(SysCall::Close, fd);
    if (res == -1)
    {
        errno = angel(SysCall::ErrNo);
    }
    return res;
}

int _write(int fd, const void* data, size_t len)
{
    struct
    {
        int fd;
        const void* data;
        size_t len;
    } arg = { fd, data, len };

    auto res = angel(SysCall::Write, &arg);
    if (res == -1 || res == intptr_t(len))
    {
        errno = angel(SysCall::ErrNo);
        return -1;
    }

    return len - res;
}

int _read(int fd, void* data, size_t len)
{
    struct
    {
        int fd;
        void* data;
        size_t len;
    } arg = { fd, data, len };

    auto res = angel(SysCall::Read, &arg);
    if (res == -1)
    {
        errno = angel(SysCall::ErrNo);
        return -1;
    }

    return len - res;
}

int _isatty(int fd)
{
    return angel(SysCall::IsTty, fd);
}

int _lseek(int fd, intptr_t offset, int origin)
{
    switch (origin)
    {
        case SEEK_CUR: return 1;
        case SEEK_END: offset += angel(SysCall::FLen, fd); break;
    }

    struct
    {
        int fd;
        intptr_t offset;
    } arg = { fd, offset };

    auto res = angel(SysCall::Seek, &arg);
    if (res < 0)
    {
        errno = angel(SysCall::ErrNo);
    }
    return res;
}

int _fstat(int fd, struct stat* st)
{
    *st = {};
    st->st_mode = S_IFCHR;
    st->st_blksize = 1024;
    return 0;
}

void exit(int err)
{
    angel(SysCall::Exit, intptr_t(err ? ExitReason::Error : ExitReason::Success));
    for (;;);
}

void abort(void)
{
    angel(SysCall::Exit, intptr_t(ExitReason::Error));
    for (;;);
}

void angel_output(void* context, char ch)
{
    static char buf[64];
    static int len = 0;
    buf[len++] = ch;
    if (ch == 0 || ch == '\n' || len == sizeof(buf) - 1)
    {
        buf[len] = 0;
        angel(SysCall::Write0, buf);
        len = 0;
    }
}

uint64_t angel_clock()
{
    uint64_t clk;
    angel(SysCall::Elapsed, intptr_t(&clk));
    return clk;
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

int vprintf(const char* fmt, va_list va)
{
    return vformat(angel_output, NULL, fmt, va);
}

extern int main(int argc, char** argv);

void angel_main()
{
    char cmdline[1024];
    int argc = 0;
    char* argv[32];
    struct { char* p; int len; } arg = { cmdline, sizeof(cmdline) };
    angel(SysCall::GetCmdLine, intptr_t(&arg));

    const char* s = cmdline;
    char* d = cmdline;
    bool a = false;

    while (char c = *s++)
    {
        switch (c)
        {
            case ' ':
                // end of argument
                if (a) { a = false; *d++ = 0; }
                break;
            case '"':
            case '\'':
                if (!a) { a = true; argv[argc++] = d; }
                while (char cc = *s++)
                {
                    if (c == cc) { break;}
                    *d++ = cc;
                }
                break;
            default:
                if (!a) { a = true; argv[argc++] = d; }
                *d++ = c;
                break;
        }
    }

    if (a) { *d++ = 0; }

    main(argc, argv);
}

}
