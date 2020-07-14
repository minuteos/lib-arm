/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/sniprintf.c
 *
 * Replace (v)sniprintf implementation with a simple one using (v)format
 * to save memory
 */

#include <base/base.h>
#include <base/format.h>

#include <stdio.h>

int sniprintf(char* buf, size_t len, const char* format, ...)
{
    return va_call(vsniprintf, format, buf, len, format);
}

int vsniprintf(char* buf, size_t len, const char* format, va_list va)
{
    format_write_info fwi = { buf, buf + len };
    int res = vformat(format_output_mem, &fwi, format, va);
    if (buf && len)
    {
        buf[len - 1] = 0;
    }
    return res;
}
