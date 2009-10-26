/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/msg.c                                       *
 * Created:     2007-12-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


int mac_set_msg (macplus_t *sim, const char *msg, const char *val)
{
	/* a hack, for debugging only */
	if (sim == NULL) {
		sim = par_sim;
	}

	if (msg == NULL) {
		msg = "";
	}

	if (val == NULL) {
		val = "";
	}

	if (msg_is_message ("emu.stop", msg)) {
		mac_set_msg_trm (sim, "term.release", "1");
		sim->brk = PCE_BRK_STOP;
		return (0);
	}
	else if (msg_is_message ("emu.exit", msg)) {
		sim->brk = PCE_BRK_ABORT;
		mon_set_terminate (&par_mon, 1);
		return (0);
	}
	else if (msg_is_message ("emu.disk.commit", msg)) {
		if (strcmp (val, "") == 0) {
			if (dsks_commit (sim->dsks)) {
				pce_log (MSG_ERR,
					"commit failed for at least one disk\n"
				);
				return (1);
			}
		}
		else {
			unsigned d;

			if (msg_get_uint (val, &d)) {
				return (1);
			}

			if (dsks_set_msg (sim->dsks, d, "commit", NULL)) {
				pce_log (MSG_ERR, "commit failed (%s)\n", val);
				return (1);
			}
		}

		return (0);
	}
	else if (msg_is_message ("emu.disk.eject", msg)) {
		unsigned d;
		disk_t   *dsk;

		if (msg_get_uint (val, &d)) {
			return (1);
		}

		dsk = dsks_get_disk (sim->dsks, d);
		if (dsk == NULL) {
			return (1);
		}

		dsks_rmv_disk (sim->dsks, dsk);

		dsk_del (dsk);

		return (0);
	}
	else if (msg_is_message ("emu.disk.insert", msg)) {
		if (dsk_insert (sim->dsks, val, 1)) {
			return (1);
		}

		return (0);
	}
	else if (msg_is_message ("emu.realtime", msg)) {
		int v;

		if (msg_get_bool (val, &v)) {
			return (1);
		}

		mac_set_speed (sim, v ? 1 : 0);

		return (0);
	}
	else if (msg_is_message ("emu.realtime.toggle", msg)) {
		if (sim->speed_factor > 0) {
			mac_set_speed (sim, 0);
		}
		else {
			mac_set_speed (sim, 1);
		}

		return (0);
	}
	else if (msg_is_message ("emu.cpu.model", msg)) {
		if (mac_set_cpu_model (sim, val)) {
			pce_log (MSG_ERR, "unknown CPU model (%s)\n", val);
			return (1);
		}

		return (0);
	}
	else if (msg_is_message ("emu.cpu.speed", msg)) {
		unsigned f;

		if (msg_get_uint (val, &f)) {
			return (1);
		}

		mac_set_speed (sim, f);

		return (0);
	}
	else if (msg_is_message ("emu.cpu.speed.step", msg)) {
		int v;

		if (msg_get_sint (val, &v)) {
			return (1);
		}

		v += (int) sim->speed_factor;

		if (v <= 0) {
			v = 1;
		}

		mac_set_speed (sim, v);

		return (0);
	}
	else if (msg_is_message ("emu.pause", msg)) {
		int v;

		if (msg_get_bool (val, &v)) {
			return (1);
		}

		mac_set_pause (sim, v);

		return (0);
	}
	else if (msg_is_message ("emu.pause.toggle", msg)) {
		mac_set_pause (sim, !sim->pause);
		return (0);
	}
	else if (msg_is_message ("emu.reset", msg)) {
		mac_reset (sim);
		return (0);
	}
	else if (msg_is_message ("mac.insert", msg)) {
		unsigned drv;

		if (strcmp (val, "") == 0) {
			mac_sony_insert (sim, 1);
			mac_sony_insert (sim, 2);
			mac_sony_insert (sim, 3);
			mac_sony_insert (sim, 4);
		}
		else {
			if (msg_get_uint (val, &drv)) {
				return (1);
			}

			mac_sony_insert (sim, drv);
		}

		return (0);
	}

	if (sim->trm != NULL) {
		if (trm_set_msg_trm (sim->trm, msg, val) == 0) {
			return (0);
		}
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
