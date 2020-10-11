#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m/Include.mk
#
# Makefile modifications for Cortex-M targets
#

CORTEX_M_MAKEFILE := $(call curmake)
CORTEX_M_DIR := $(dir $(CORTEX_M_MAKEFILE))

# GCC Arm Embedded Toolchain is the recommended toolchain
TOOLCHAIN_PREFIX = arm-none-eabi-

# the packages for individual cores are more specific
ARCH_FLAGS ?= -mthumb

# overridable LD script
LD_SCRIPT ?= default.ld

# try to keep the output binary as small as possible
LINK_FLAGS += -T$(LD_SCRIPT) -nostartfiles -Wl,--gc-sections,-Map,$(OUTPUT).map -specs=nano.specs

# fallbacks for missing LD sections
LINK_DIRS += $(CORTEX_M_DIR)ld_fallbacks/

TARGETS += cmsis

PRIMARY_EXT = .axf

# we also generate a raw binary image for direct flashing
.PHONY: binary ihex srec

binary: $(OUTPUT).bin

ihex: $(OUTPUT).hex

srec: $(OUTPUT).s37

$(OUTPUT).bin: $(PRIMARY_OUTPUT)
	$(OBJCOPY) -O binary $< $@

$(OUTPUT).s37: $(PRIMARY_OUTPUT)
	$(OBJCOPY) -O srec --srec-forceS3 $< $@

$(OUTPUT).hex: $(PRIMARY_OUTPUT)
	$(OBJCOPY) -O ihex $< $@
