/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim405/msg.c                                        *
 * Created:     2018-12-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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
#include "msg.h"
#include "sim405.h"

#include <cpu/ppc405/ppc405.h>
#include <devices/memory.h>
#include <lib/msg.h>


static
int s405_msg_emu_disk_commit (sim405_t *sim, const char *msg, const char *val)
{
	int      r;
	unsigned drv;

	if (strcmp (val, "all") == 0) {
		pce_log (MSG_INF, "commiting all drives\n");

		if (dsks_commit (sim->dsks)) {
			pce_log (MSG_ERR,
				"*** commit failed for at least one disk\n"
			);
			return (1);
		}

		return (0);
	}

	r = 0;

	while (*val != 0) {
		if (msg_get_prefix_uint (&val, &drv, ":", " \t")) {
			pce_log (MSG_ERR, "*** commit error: bad drive (%s)\n",
				val
			);

			return (1);
		}

		pce_log (MSG_INF, "commiting drive %u\n", drv);

		if (dsks_set_msg (sim->dsks, drv, "commit", NULL)) {
			pce_log (MSG_ERR, "*** commit error for drive %u\n",
				drv
			);

			r = 1;
		}
	}

	return (r);
}

static
int s405_msg_emu_ser_clock (sim405_t *sim, const char *msg, const char *val)
{
	unsigned p, v;

	if (msg_get_prefix_uint (&val, &p, ":", " \t")) {
		return (1);
	}

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	if (p > 1) {
		pce_log (MSG_ERR, "*** bad serial port (%u)\n", p);
		return (1);
	}

	pce_log (MSG_INF, "setting serial port %u clock multiplier to %u\n", p, v);

	e8250_set_clock_mul (&sim->serport[p]->uart, v);

	return (0);
}

static
int s405_msg_emu_ser_multichar (sim405_t *sim, const char *msg, const char *val)
{
	unsigned p, v;

	if (msg_get_prefix_uint (&val, &p, ":", " \t")) {
		return (1);
	}

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	if (p > 1) {
		pce_log (MSG_ERR, "*** bad serial port (%u)\n", p);
		return (1);
	}

	pce_log (MSG_INF, "setting serial port %u multichar to %u\n", p, v);

	e8250_set_multichar (&sim->serport[p]->uart, v, v);

	return (0);
}

int s405_set_msg (sim405_t *sim, const char *msg, const char *val)
{
	/* a hack, for debugging only */
	if (sim == NULL) {
		sim = par_sim;
	}

	if (msg == NULL) {
		return (1);
	}

	if (val == NULL) {
		val = "";
	}

	if (msg_is_prefix ("term", msg)) {
		return (1);
	}

	if (msg_is_message ("emu.disk.commit", msg)) {
		return (s405_msg_emu_disk_commit (sim, msg, val));
	}
	else if (msg_is_message ("emu.ser.clock", msg)) {
		return (s405_msg_emu_ser_clock (sim, msg, val));
	}
	else if (msg_is_message ("emu.ser.multichar", msg)) {
		return (s405_msg_emu_ser_multichar (sim, msg, val));
	}
	else if (msg_is_message ("emu.stop", msg)) {
		sim->brk = PCE_BRK_STOP;
		return (0);
	}
	else if (msg_is_message ("emu.exit", msg)) {
		sim->brk = PCE_BRK_ABORT;
		return (0);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
