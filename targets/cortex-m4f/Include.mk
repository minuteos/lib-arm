#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m4f/Include.mk - Makefile additions for ARM Cortex-M4F targets
#

# everything is the same as on a non-F Cortex-M4, with added FPU
TARGETS += cortex-m4

# some targets may require the softfp ABI due to precompiled library dependencies,
# however hard is recommended as it allows floating point values to be passed in FPU registers directly
CORTEX_FLOAT_ABI ?= hard

ARCH_FLAGS ?= -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=$(CORTEX_FLOAT_ABI)
