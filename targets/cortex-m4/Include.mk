#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m4/Include.mk
#
# Makefile modifications for ARM Cortex-M4 targets
#

# Cortex-M4 is more or less the same as Cortex-M3, just with DSP instructions
TARGETS += cortex-m3
ARCH_FLAGS ?= -mcpu=cortex-m4 -mthumb
