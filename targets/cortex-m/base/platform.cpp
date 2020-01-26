/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * cortex-m/base/platform.cpp
 *
 * Basic platform support for Cortex-M MCUs
 */

#include <base/base.h>

int Cortex_DebugWrite(unsigned channelAndSize, uint32_t data)
{
    uint32_t channel = channelAndSize & 0x1F;

    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
        return false;
    if (!GETBIT(ITM->TER, channel))
        return false;

    auto& port = ITM->PORT[channel];
    if (!port.u32)
    {
        // wait for a while for the port to become available
        unsigned wait = 255;
        do
        {
            if (!--wait)
            {
                return false;
            }
        } while (!port.u32);
    }

    switch (channelAndSize >> 5)
    {
        case 0: port.u8 = data; break;
        case 1: port.u16 = data; break;
        case 2: port.u32 = data; break;
    }
    return true;
}
