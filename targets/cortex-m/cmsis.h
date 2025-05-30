/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/cmsis.h
 *
 * Fallback minimal CMSIS root header, specific targets should override it
 */

#pragma once

typedef enum IRQn
{
/* -------------------  Processor Exceptions Numbers  ----------------------------- */
  NonMaskableInt_IRQn           = -14,     /*  2 Non Maskable Interrupt */
  HardFault_IRQn                = -13,     /*  3 HardFault Interrupt */
  MemoryManagement_IRQn         = -12,     /*  4 Memory Management Interrupt */
  BusFault_IRQn                 = -11,     /*  5 Bus Fault Interrupt */
  UsageFault_IRQn               = -10,     /*  6 Usage Fault Interrupt */
  SVCall_IRQn                   =  -5,     /* 11 SV Call Interrupt */
  PendSV_IRQn                   =  -2,     /* 14 Pend SV Interrupt */
  SysTick_IRQn                  =  -1,     /* 15 System Tick Interrupt */
} IRQn_Type;

// nonstandard extension expected by startup.cpp
#define EXT_IRQ_COUNT             0U

#define __NVIC_PRIO_BITS          0U        /* Number of Bits used for Priority Levels */

#ifdef __ARM_FP
#define __FPU_PRESENT             1U
#endif

#ifdef __ARM_ARCH_6M__
#include "core_cm0.h"
#elif __ARM_ARCH_7EM__
#include "core_cm4.h"
#else
#include "core_cm3.h"
#endif
