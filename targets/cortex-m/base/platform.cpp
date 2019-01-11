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

volatile Cortex_ITM_Port_t* Cortex_GetDebugChannel(unsigned channel)
{
	static Cortex_ITM_Port_t discard;
	volatile Cortex_ITM_Port_t* port = (Cortex_ITM_Port_t*)&ITM->PORT[channel];

	if (port->u32 != 0)
		return port;

	for (uint i = 0; i < 256; i++)
	{
		if (port->u32 != 0)
			return port;
		if (!GETBIT(ITM->TER, channel))
			break;
	}

	return &discard;
}
