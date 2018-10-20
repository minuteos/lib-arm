#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m3/Include.mk
#
# Makefile modifications for ARM Cortex-M3 targets
#

# use the common Cortex-M target
TARGETS += cortex-m

# be a bit more specific about architecture
ARCH_FLAGS ?= -mcpu=cortex-m3 -mthumb
