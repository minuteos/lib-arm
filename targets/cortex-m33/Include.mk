#
# Copyright (c) 2020 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m33/Include.mk - Makefile additions for ARM Cortex-M33 targets
#

TARGETS += cortex-m

# some targets may require the softfp ABI due to precompiled library dependencies,
# however hard is recommended as it allows floating point values to be passed in FPU registers directly
CORTEX_FLOAT_ABI ?= hard

ARCH_FLAGS ?= -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=$(CORTEX_FLOAT_ABI)
