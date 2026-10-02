/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/hotkey.c                                    *
 * Created:     2010-11-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2020 Hampa Hug <hampa@hampa.ch>                     *
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
#include "hook.h"
#include "hotkey.h"
#include "macplus.h"
#include "msg.h"

#include <lib/log.h>


static
int pcex_interrupt (macplus_t *sim, unsigned long val)
{
	unsigned long sp;
	unsigned long addr;

	addr = 0xf80000;

	if (mem_get_uint32_be (sim->mem, addr) != 0x50434558) {
		return (1);
	}

	if (mem_get_uint32_be (sim->mem, addr + 8) < 5) {
		return (1);
	}

	addr += mem_get_uint32_be (sim->mem, addr + 12 + 4 * 4);

	sp = e68_get_areg32 (sim->cpu, 7);
	mem_set_uint32_be (sim->mem, sp - 4, e68_get_pc (sim->cpu));
	mem_set_uint16_be (sim->mem, sp - 6, e68_get_sr (sim->cpu));
	mem_set_uint32_be (sim->mem, sp - 10, e68_get_dreg32 (sim->cpu, 0));
	e68_set_dreg32 (sim->cpu, 0, val);
	e68_set_areg32 (sim->cpu, 7, sp - 10);
	e68_set_pc_prefetch (sim->cpu, addr);

	return (0);
}

int mac_set_hotkey (macplus_t *sim, pce_key_t key)
{
	static const char *ins = "emu.mac.insert";

	switch (key) {
	case PCE_KEY_F1:
		mac_set_msg (sim, ins, "1");
		break;

	case PCE_KEY_F2:
		mac_set_msg (sim, ins, "2");
		break;

	case PCE_KEY_F3:
		mac_set_msg (sim, ins, "3");
		break;

	case PCE_KEY_F4:
		mac_set_msg (sim, ins, "4");
		break;

	case PCE_KEY_F5:
		mac_set_msg (sim, ins, "5");
		break;

	case PCE_KEY_F6:
		mac_set_msg (sim, ins, "6");
		break;

	case PCE_KEY_F7:
		mac_set_msg (sim, ins, "7");
		break;

	case PCE_KEY_F8:
		mac_set_msg (sim, ins, "8");
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

		if (sim->adb_kbd != NULL) {
			if (adb_kbd_get_keypad_mode (sim->adb_kbd)) {
				mac_log_deb ("keypad mode: keypad\n");
				adb_kbd_set_keypad_mode (sim->adb_kbd, 0);
			}
			else {
				mac_log_deb ("keypad mode: motion\n");
				adb_kbd_set_keypad_mode (sim->adb_kbd, 1);
			}
		}
		break;

	case PCE_KEY_I:
		mac_interrupt (sim, 7, 1);
		mac_interrupt (sim, 7, 0);
		break;

	case PCE_KEY_F12:
		pcex_interrupt (sim, 0);
		break;

	default:
		pce_log (MSG_INF, "unhandled magic key (%u)\n",
			(unsigned) key
		);
		break;
	}

	return (0);
}
