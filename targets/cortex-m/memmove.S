/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * memmove.S
 *
 * Moves a block of memory, checking for overlap
 * memcpy is used when dst >= src
 *
 * The newlib nano implementation is a naive byte-by-byte approach which is a waste of performance.
 *
 * The full newlib implementation is huge because it does 4x unrolling which is not that useful for
 * typical amounts of data being copied on MCUs.
 */

    .syntax unified
    .arch   armv7-m
    .section .text.memmove
    .global memmove

 memmove:
    cmp r0, r1
    bhs.n memcpy        // memcpy works from the end, we can use it for dst >= src
    mov ip, r0          // store original dst as it must be returned untouched
.L_aligned:
    subs r2, #4
    blo .L_unaligned
    ldmia r1!, {r3}
    stmia r0!, {r3}
    bne .L_aligned
    // finished at zero, no need to check unaligned parts
    mov r0, ip
    bx lr

.L_unaligned:           // the bottom two bits remained the same as they were in the beginning
    lsls r2, r2, #31    // C == bit 1, N == bit0
    itt cs
    ldrhcs r3, [r1], #2
    strhcs r3, [r0], #2

    itt mi
    ldrbmi r3, [r1]
    strbmi r3, [r0]

    mov r0, ip
    bx lr
