/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * memset.S
 *
 * Sets all bytes in a block of memory to the same value
 *
 * The newlib nano implementation is a naive byte-by-byte approach which is a waste of performance.
 *
 * The full newlib implementation is huge because it does 4x unrolling which is not that useful for
 * typical amounts of data being copied on MCUs.
 */

    .syntax unified
    .arch   armv7-m
    .section .text.memset
    .global memset

memset:
    // expand r1 for word filling (i.e. 0xxxxxxxAA > 0xAAAAAAAA)
    uxtb r1, r1             // 0xxxxxxxAA > 0x000000AA
    orr r1, r1, r1, lsl #8  // 0x000000AA > 0x0000AAAA
    orr r1, r1, r1, lsl #16 // 0x0000AAAA > 0xAAAAAAAA

    // check for and set unaligned byte
    lsrs r3, r2, #1         // C == len bit 0
    itt cs
    subcs r2, #1
    strbcs r1, [r0, r2]

    // check for and set unaligned halfword
    lsrs r3, r2, #2         // C == len bit 1, Z == no words
    itt cs
    subcs r2, #2
    strhcs r1, [r0, r2]

    // no words remaining
    it eq
    bxeq lr

.L_aligned:
    subs r2, #4
    str r1, [r0, r2]
    bne .L_aligned
    bx lr
