/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * memcpy.S
 *
 * Copies a block of memory
 *
 * Acutal implementation starts from the end of the memory block,
 * which allows us to use only the scratch registers
 *
 * The newlib nano implementation is a naive byte-by-byte approach which is a waste of performance.
 *
 * The full newlib implementation is huge because it does 4x unrolling which is not that useful for
 * typical amounts of data being copied on MCUs.
 */

    .syntax unified
    .arch   armv7-m
    .section .text.memcpy
    .global memcpy

memcpy:
    // check for and copy unaligned byte
    lsrs r3, r2, #1         // C == len bit 0
    ittt cs
    subcs r2, #1
    ldrbcs r3, [r1, r2]
    strbcs r3, [r0, r2]

    // check for and copy unaligned halfword
    lsrs r3, r2, #2         // C == len bit 1, Z == no words
    ittt cs
    subcs r2, #2
    ldrhcs r3, [r1, r2]
    strhcs r3, [r0, r2]

    // no words remaining
    it eq
    bxeq lr

.L_aligned:
    subs r2, #4
    ldr r3, [r1, r2]
    str r3, [r0, r2]
    bne .L_aligned
    bx lr
