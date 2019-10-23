#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# cortex-m/boot/Include.mk
#
# Generic bootloader support for Cortex-M series MCUs
#

ifneq (bootloader,$(NAME))

.PHONY: bootloader

BOOT_MAKEFILE := $(call curmake)
BOOT_DIR = $(dir $(BOOT_MAKEFILE))

BOOT_OUTDIR = $(OBJDIR)boot/
BOOT_OBJDIR = $(BOOT_OUTDIR)obj/
BOOT_OUTPUT = $(BOOT_OUTDIR)bootloader
BOOT_CONFIG ?= Release

ADDITIONAL_BLOBS += $(BOOT_OUTPUT).o

COMPONENTS := $(filter-out boot%,$(COMPONENTS))

prebuild: bootloader

bootloader: $(BOOT_OUTPUT).o

$(BOOT_OUTPUT).o: $(BOOT_OUTPUT).bin
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.binboot $< $@

$(BOOT_OUTPUT).bin: export COMPONENTS := boot $(BOOT_COMPONENTS)
$(BOOT_OUTPUT).bin: export DEFINES    := BOOTLOADER
$(BOOT_OUTPUT).bin:
	$(MAKE) -f $(BASE_DIR)Base.mk PROJECT_SOURCE_DIR=$(BOOT_DIR) CONFIG=$(BOOT_CONFIG) OUTDIR=$(BOOT_OUTDIR) OBJDIR=$(BOOT_OBJDIR) NAME=bootloader LD_SCRIPT=boot.ld "TARGETS=$(TARGETS)" main binary

else

COMPONENTS += kernel-app
ARCH_FLAGS += -flto -ffunction-sections

endif
