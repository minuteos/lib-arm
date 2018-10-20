/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * ld_symbols.h
 * 
 * declarations of various symbols exported by default.ld
 */

#include <stdint.h>

extern uint32_t __boot_end;
extern uint32_t __app_start;
extern uint32_t __stack_start;
extern uint32_t __stack_end;
extern char __bss_start;
extern char __bss_end;
extern char __data_start;
extern char __data_end;
extern char __data_load;
extern char __heap_start;
extern char __heap_end;
extern char __ram_start;
extern char __ram_end;
