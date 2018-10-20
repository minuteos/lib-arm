#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m/Include.mk
#
# Makefile modifications for Cortex-M targets
#

# GCC Arm Embedded Toolchain is the recommended toolchain
TOOLCHAIN_PREFIX = arm-none-eabi-

# the packages for individual cores are more specific
ARCH_FLAGS ?= -mthumb

# try to keep the output binary as small as possible
LINK_FLAGS += -Tdefault.ld -nostartfiles -Wl,--gc-sections,-Map,$(OUTPUT).map -specs=nano.specs

TARGETS += cmsis

# we also generate a raw binary image for direct flashing
.PHONY: binary

all: binary

binary: $(OUTPUT).bin

$(OUTPUT).bin: $(OUTPUT).elf
	$(OBJCOPY) -O binary $< $@
