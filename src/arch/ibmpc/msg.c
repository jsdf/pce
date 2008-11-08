/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/msg.c                                         *
 * Created:     2004-09-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2008 Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id$ */


#include "main.h"


int pc_set_msg (ibmpc_t *pc, const char *msg, const char *val)
{
	/* a hack, for debugging only */
	if (pc == NULL) {
		pc = par_pc;
	}

	if (msg == NULL) {
		msg = "";
	}

	if (val == NULL) {
		val = "";
	}

	if (msg_is_prefix ("term", msg)) {
		if (pc->trm != NULL) {
			return (trm_set_msg_trm (pc->trm, msg, val));
		}

		return (1);
	}

	if (msg_is_message ("emu.stop", msg)) {
		pc->brk = PCE_BRK_STOP;
		return (0);
	}
	else if (msg_is_message ("emu.exit", msg)) {
		pc->brk = PCE_BRK_ABORT;
		mon_set_terminate (&par_mon, 1);
		return (0);
	}
	else if (msg_is_message ("emu.pause", msg)) {
		int v;

		if (msg_get_bool (val, &v)) {
			return (1);
		}

		pc->pause = v;

		pc_clock_discontinuity (pc);

		return (0);
	}
	else if (msg_is_message ("emu.reset", msg)) {
		pc_reset (pc);
		return (0);
	}
	else if (msg_is_message ("emu.pause.toggle", msg)) {
		pc->pause = !pc->pause;

		pc_clock_discontinuity (pc);

		return (0);
	}
	else if (msg_is_message ("emu.config.save", msg)) {
		if (ini_write (pc->cfg, val)) {
			return (1);
		}

		return (0);
	}
	else if (msg_is_message ("emu.cpu.model", msg)) {
		if (pc_set_cpu_model (pc, val)) {
			return (1);
		}

		return (0);
	}
	else if (msg_is_message ("emu.cpu.clock", msg)) {
		unsigned long v;

		if (msg_get_ulng (val, &v)) {
			return (1);
		}

		pc_set_cpu_clock (pc, v);

		return (0);
	}
	else if (msg_is_message ("emu.cpu.speed", msg)) {
		unsigned f;

		if (msg_get_uint (val, &f)) {
			return (1);
		}

		pc_set_speed (pc, f);

		return (0);
	}
	else if (msg_is_message ("emu.cpu.speed.step", msg)) {
		int v;

		if (msg_get_sint (val, &v)) {
			return (1);
		}

		v += (int) pc->speed[0];

		if (v <= 0) {
			v = 1;
		}

		pc_set_speed (pc, v);

		return (0);
	}
	else if (msg_is_message ("emu.video.redraw", msg)) {
		pce_video_redraw (pc->video);
		return (0);
	}
	else if (msg_is_message ("emu.video.screenshot", msg)) {
		if (strcmp (val, "") == 0) {
			pc_screenshot (pc, NULL);
		}
		else {
			pc_screenshot (pc, val);
		}
		return (0);
	}
	else if (msg_is_message ("emu.disk.boot", msg)) {
		unsigned v;

		if (msg_get_uint (val, &v)) {
			return (1);
		}

		pc->bootdrive = v;

		return (0);
	}
	else if (msg_is_message ("emu.disk.commit", msg)) {
		if (strcmp (val, "") == 0) {
			if (dsks_commit (pc->dsk)) {
				pce_log (MSG_ERR, "commit failed for at least one disk\n");
				return (1);
			}
		}
		else {
			unsigned d;

			if (msg_get_uint (val, &d)) {
				return (1);
			}

			if (dsks_set_msg (pc->dsk, d, "commit", NULL)) {
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

		dsk = dsks_get_disk (pc->dsk, d);
		if (dsk == NULL) {
			return (1);
		}

		dsks_rmv_disk (pc->dsk, dsk);

		dsk_del (dsk);

		return (0);
	}
	else if (msg_is_message ("emu.disk.insert", msg)) {
		if (dsk_insert (pc->dsk, val, 1)) {
			return (1);
		}

		return (0);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}

static
int pc_copy_msg (char *dst, const char *src, unsigned max)
{
	dst[0] = 0;

	if ((strlen (src) + 1) > max) {
		return (1);
	}

	strcpy (dst, src);

	return (0);
}

int pc_get_msg (ibmpc_t *pc, const char *msg, char *val, unsigned max)
{
	const char    *str;
	char          buf[256];
	unsigned long tmp;

	if ((msg == NULL) || (val == NULL)) {
		return (1);
	}

	if (strcmp (msg, "emu.config.get_string") == 0) {
		ini_get_string (pc->cfg, val, &str, "");
		return (pc_copy_msg (val, str, max));
	}

	if (pc_get_msgul (pc, msg, &tmp) == 0) {
		sprintf (buf, "%lu", tmp);
		return (pc_copy_msg (val, buf, max));
	}

	return (1);
}

int pc_set_msgul (ibmpc_t *pc, const char *msg, unsigned long val)
{
	char buf[256];

	sprintf (buf, "%lu", val);

	return (pc_set_msg (pc, msg, buf));
}

int pc_get_msgul (ibmpc_t *pc, const char *msg, unsigned long *val)
{
	if ((msg == NULL) || (val == NULL)) {
		return (1);
	}

	if (msg_is_message ("emu.cpu.speed", msg)) {
		*val = pc->speed[0];
		return (0);
	}
	if (msg_is_message ("emu.cpu.clock", msg)) {
		*val = pc->cpu_clk[0];
		return (0);
	}
	else if (msg_is_message ("emu.pause", msg) == 0) {
		*val = (pc->pause != 0);
		return (0);
	}

	return (1);
}
