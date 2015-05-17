/* Copyright (c) 2010, Fabian Greif
 * All rights reserved.
 *
 * The file is part of the CAN bootloader project and is released under the
 * 3-clause BSD license. See the file `LICENSE` for the full license governing
 * this code.
 */
// ----------------------------------------------------------------------------

#include <util/atomic.h>

#include "at90can.h"

// ----------------------------------------------------------------------------
void
at90can_send_message(command_t type, uint8_t length)
{
	while (true)
	{
		// check if there is any free MOb
		if (at90can_free_buffer == 0)
		{
			continue;
		}
		
		for (uint8_t mob = 8; mob < 15; mob++)
		{
			// load MOb page
			CANPAGE = mob << 4;
			
			// check if the MOb is in use
			if ((CANCDMOB & ((1 << CONMOB1) | (1 << CONMOB0))) == 0)
			{
				// clear flags (read-write-cycle required)
				CANSTMOB &= 0;
				
				// set identifier
				CANIDT4 = 0;
				CANIDT3 = 0;
				CANIDT2 = (uint8_t) (0x7fe << 5);
				CANIDT1 = (uint8_t) (0x7fe >> 3);
				
				CANMSG = BOOTLOADER_BOARD_ID;
				CANMSG = type;
				CANMSG = message_number;
				CANMSG = message_data_counter;
				
				// copy data
				const uint8_t *p = message_data;
				for (uint8_t i = 0; i < length; i++)
				{
					CANMSG = *p++;
				}
				
				// enable MOb interrupt
				CANIE1 |= (1 << (mob - 8));
				
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
				{
					at90can_free_buffer--;
				}
				
				// enable transmission
				CANCDMOB = (1 << CONMOB0) | (length + 4);
				
				return;
			}
		}
	}
}
