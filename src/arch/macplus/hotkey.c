/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/hotkey.c                                    *
 * Created:     2010-11-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "hotkey.h"


int mac_set_hotkey (macplus_t *sim, pce_key_t key)
{
	switch (key) {
	case PCE_KEY_F1:
		mac_set_msg (sim, "mac.insert", "1");
		break;

	case PCE_KEY_F2:
		mac_set_msg (sim, "mac.insert", "2");
		break;

	case PCE_KEY_F3:
		mac_set_msg (sim, "mac.insert", "3");
		break;

	case PCE_KEY_K:
	case PCE_KEY_KP_5:
		if (sim->kbd != NULL) {
			if (sim->kbd->keypad_mode) {
				mac_log_deb ("keypad mode: motion\n");
				mac_kbd_set_keypad_mode (sim->kbd, 1);
			}
			else {
				mac_log_deb ("keypad mode: keypad\n");
				mac_kbd_set_keypad_mode (sim->kbd, 0);
			}
		}
		break;

	case PCE_KEY_I:
		mac_interrupt (sim, 7, 1);
		mac_interrupt (sim, 7, 0);
		break;

	default:
		pce_log (MSG_INF, "unhandled magic key (%u)\n",
			(unsigned) key
		);
		break;
	}

	return (0);
}
