#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# qemu-arm/Include.mk
#
# Makefile modifications to allow using qemu-system-arm for running tests
#

# prefix for test invocations
TEST_RUN = qemu-system-arm -machine lm3s6965evb -monitor null -serial null -nographic -semihosting -kernel
TEST_RUN_ARGS = -append "$(TEST_FILTERS)"

# LM3S6965 is a Cortex-M3
TARGETS += cortex-m3

COMPONENTS += base

.PHONY: run

run: $(OUTPUT).elf
	@$(TEST_RUN) $(OUTPUT).elf $(TEST_RUN_ARGS)
