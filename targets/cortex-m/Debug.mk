#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m/Debug.mk
#
# Overrides for Debug builds on Cortex-M MCUs
#

# arm-none-eabi-gcc supports a special -Og debug-friendly optimization level
OPT_FLAGS = -Og
