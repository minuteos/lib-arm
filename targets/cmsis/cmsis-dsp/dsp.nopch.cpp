/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cmsis/cmsis-dsp/dsp.nopch.cpp
 * 
 * Compiles the CMSIS-DSP as a single object
 */

#include <base/base.h>

#include <arm_math.h>

#pragma GCC optimize("O3")
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wsign-compare"

extern "C"
{

#include "BasicMathFunctions/BasicMathFunctions.c"
#include "CommonTables/CommonTables.c"
#include "ComplexMathFunctions/ComplexMathFunctions.c"
#include "ControllerFunctions/ControllerFunctions.c"
#include "FastMathFunctions/FastMathFunctions.c"
#include "FilteringFunctions/FilteringFunctions.c"
#include "MatrixFunctions/MatrixFunctions.c"
#include "StatisticsFunctions/StatisticsFunctions.c"
#include "SupportFunctions/SupportFunctions.c"
#include "TransformFunctions/TransformFunctions.c"

}
