/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * mem_mini.cpp
 *
 * Basic memory operations implemented with a reasonable compromise of performance and code size.
 *
 * The newlib nano implementation is a naive byte-by-byte approach which is a waste of performance.
 *
 * The full newlib implementation is huge because it does 4x unrolling which is not that useful for
 * typical amounts of data being copied on MCUs.
 */

#include <base/base.h>

/*!
 * Copies a block of memory
 *
 * Acutal implementation starts from the end of the memory block,
 * which allows us to use only the scratch registers
 */
__attribute__((naked)) void *memcpy(void *dst, const void *src, size_t len)
{
    __asm volatile(
        "cbz.n r2, 1f\n"	// do nothing when len is zero (1)
        "3: "
        "tst r2, #3\n"
        "beq.n 2f\n"		// len is non-zero and aligned, continue by words (2)
        "subs r2, #1\n"
        "ldrb r3, [r1, r2]\n"
        "strb r3, [r0, r2]\n"
        "bne.n 3b\n"		// len might be aligned but also might be zero
        // zero length at this point
        "bx lr\n"
        "2: "	// r2 (len) is definitely a multiply of 4, and not 0 (see above)
        "subs r2, #4\n"
        "ldr r3, [r1, r2]\n"
        "str r3, [r0, r2]\n"
        "bne.n 2b\n"
        "1: "
        "bx lr\n"
        );
}

/*!
 * Moves a block of memory, checking for overlap
 *
 * memcpy is used when dst >= src
 */
__attribute__((naked)) void *memmove(void *dst, const void* src, size_t len)
{
    __asm volatile(
        "cmp r0, r1\n"
        "bhs.n memcpy\n"	// memcpy works from the end, we can use it for dst >= src
        "push {r0, lr}\n"	// memmove must leave r0 untouched, cannot do it without pushing
        "1: "
        "subs r2, #4\n"
        "blo.n 2f\n"	// less than 4 bytes left, r2 = <-1,-4>
        "ldr r3, [r1], #4\n"
        "str r3, [r0], #4\n"
        "b.n 1b\n"
        "2: "
        "tst r2, #3\n"
        "beq.n 3f\n"	// -4 (11..111100) == done
        "subs r2, #1\n"
        "ldrb r3, [r1], #1\n"
        "strb r3, [r0], #1\n"
        "b.n 2b\n"
        "3: "
        "pop {r0, pc}\n"
        );
}

/*!
 * Sets all bytes in a block of memory to the same value
 */
__attribute__((naked)) void *memset(void *dst, int data, size_t len)
{
    __asm volatile(
        "cbz.n r2, 1f\n"	// do nothing when len is zero (1)
        "3: "	// setting of max. 3 unaligned bytes
        "tst.w r2, #3\n"
        "beq.n 2f\n"		// len is non-zero and aligned, continue by words (2)
        "subs r2, #1\n"
        "strb r1, [r0, r2]\n"
        "bne.n 3b\n"		// len might be aligned but also might be zero
        // zero length at this point
        "bx lr\n"
        "2: "	// prepare r1 for word filling (i.e. 0xxxxxxxAA > 0xAAAAAAAA)
        "uxtb r1, r1\n" // 0xxxxxxxAA > 0x000000AA
        "orr r1, r1, r1, lsl #8\n" // 0x000000AA > 0x0000AAAA
        "orr r1, r1, r1, lsl #16\n" // 0x0000AAAA > 0xAAAAAAAA
        "4: "	// r2 (len) is definitely a multiply of 4, and not 0 (see above)
        "subs r2, #4\n"
        "str r1, [r0, r2]\n"
        "bne.n 4b\n"
        "1: "
        "bx lr\n"
        );
}
