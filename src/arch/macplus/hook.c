/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/hook.c                                      *
 * Created:     2007-12-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "macplus.h"
#include "msg.h"

#include <cpu/e68000/e68000.h>

#include <lib/log.h>


#define MAC_HOOK_NOP    0
#define MAC_HOOK_STOP   1
#define MAC_HOOK_EXIT   2
#define MAC_HOOK_INSERT 3
#define MAC_HOOK_MARK   4


/*
 * Advance the PC by cnt bytes
 */
void mac_hook_skip (macplus_t *sim, unsigned cnt)
{
	e68_set_pc (sim->cpu, e68_get_pc (sim->cpu) + cnt);
}

int mac_hook (void *ext, unsigned val)
{
	unsigned  i;
	macplus_t *sim = ext;

	switch (val) {
	case MAC_HOOK_NOP:
		mac_hook_skip (sim, 4);
		return (0);

	case MAC_HOOK_STOP:
		mac_hook_skip (sim, 4);
		mac_set_msg (sim, "emu.stop", "1");
		return (0);

	case MAC_HOOK_EXIT:
		mac_hook_skip (sim, 4);
		mac_set_msg (sim, "emu.exit", "1");
		return (0);

	case MAC_HOOK_INSERT:
		mac_hook_skip (sim, 4);
		for (i = 0; i < 4; i++) {
			mac_sony_insert (sim, i + 1);
		}
		return (0);

	case MAC_HOOK_MARK:
		mac_hook_skip (sim, 4);
		pce_log (MSG_INF, "mark: PC=%06lX\n",
			(unsigned long) e68_get_pc (sim->cpu)
		);
		return (0);

	default:
		if (mac_sony_hook (sim, val) == 0) {
			return (0);
		}
		break;
	}

	mac_log_deb ("unhandled hook (%04X)\n", val);

	return (1);
}
