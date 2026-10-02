/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/msg.c                                         *
 * Created:     2012-11-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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
#include "cpm80.h"
#include "msg.h"

#include <string.h>

#include <drivers/block/block.h>

#include <lib/console.h>
#include <lib/inidsk.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/msg.h>


typedef struct {
	const char *msg;

	int (*set_msg) (cpm80_t *sim, const char *msg, const char *val);
} c80_msg_list_t;


extern monitor_t par_mon;
extern cpm80_t   *par_sim;


static
int c80_set_msg_emu_aux_read (cpm80_t *sim, const char *msg, const char *val)
{
	if (c80_set_aux_read (sim, (*val == 0) ? NULL : val)) {
		pce_log (MSG_ERR, "*** can't open file (%s)\n", val);
	}

	return (0);
}

static
int c80_set_msg_emu_aux_write (cpm80_t *sim, const char *msg, const char *val)
{
	if (c80_set_aux_write (sim, (*val == 0) ? NULL : val, 1)) {
		pce_log (MSG_ERR, "*** can't open file (%s)\n", val);
	}

	return (0);
}

static
int c80_set_msg_emu_con_read (cpm80_t *sim, const char *msg, const char *val)
{
	if (c80_set_con_read (sim, (*val == 0) ? NULL : val)) {
		pce_log (MSG_ERR, "*** can't open file (%s)\n", val);
	}

	return (0);
}

static
int c80_set_msg_emu_con_write (cpm80_t *sim, const char *msg, const char *val)
{
	if (c80_set_con_write (sim, (*val == 0) ? NULL : val, 1)) {
		pce_log (MSG_ERR, "*** can't open file (%s)\n", val);
	}

	return (0);
}

static
int c80_set_msg_emu_cpu_model (cpm80_t *sim, const char *msg, const char *val)
{
	if (c80_set_cpu_model (sim, val)) {
		pce_log (MSG_ERR, "*** failed to set CPU model (%s)\n", val);
	}

	return (0);
}

static
int c80_set_msg_emu_cpu_speed (cpm80_t *sim, const char *msg, const char *val)
{
	unsigned v;

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	if (v > 4) {
		v = 8 * (v - 4);
	}

	c80_set_speed (sim, v);

	return (0);
}

static
int c80_set_msg_emu_cpu_speed_double (cpm80_t *sim, const char *msg, const char *val)
{
	c80_set_speed (sim, 2 * sim->speed);

	return (0);
}

static
int c80_set_msg_emu_cpu_speed_half (cpm80_t *sim, const char *msg, const char *val)
{
	if (sim->speed > 1) {
		c80_set_speed (sim, sim->speed / 2);
	}

	return (0);
}

static
int c80_set_msg_emu_cpu_speed_step (cpm80_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v += (int) sim->speed;

	if (v <= 0) {
		v = 1;
	}

	c80_set_speed (sim, v);

	return (0);
}

static
int c80_set_msg_emu_disk_commit (cpm80_t *sim, const char *msg, const char *val)
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
int c80_set_msg_emu_disk_eject (cpm80_t *sim, const char *msg, const char *val)
{
	unsigned drv;
	disk_t   *dsk;

	while (*val != 0) {
		if (msg_get_prefix_uint (&val, &drv, ":", " \t")) {
			pce_log (MSG_ERR,
				"*** disk eject error: bad drive (%s)\n",
				val
			);

			return (1);
		}

		dsk = dsks_get_disk (sim->dsks, drv);

		if (dsk == NULL) {
			pce_log (MSG_ERR,
				"*** disk eject error: no such disk (%lu)\n", drv
			);
		}
		else {
			pce_log (MSG_INF, "ejecting drive %lu\n", drv);

			dsks_rmv_disk (sim->dsks, dsk);

			dsk_del (dsk);
		}
	}

	return (0);
}

static
int c80_set_msg_emu_disk_insert (cpm80_t *sim, const char *msg, const char *val)
{
	if (dsk_insert (sim->dsks, val, 1)) {
		return (1);
	}

	return (0);
}

static
int c80_set_msg_emu_exit (cpm80_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_ABORT;

	mon_set_terminate (&par_mon, 1);

	return (0);
}

static
int c80_set_msg_emu_reset (cpm80_t *sim, const char *msg, const char *val)
{
	c80_reset (sim);

	return (0);
}

static
int c80_set_msg_emu_stop (cpm80_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_STOP;

	return (0);
}

static c80_msg_list_t set_msg_list[] = {
	{ "emu.aux.read", c80_set_msg_emu_aux_read },
	{ "emu.aux.write", c80_set_msg_emu_aux_write },
	{ "emu.con.read", c80_set_msg_emu_con_read },
	{ "emu.con.write", c80_set_msg_emu_con_write },
	{ "emu.cpu.model", c80_set_msg_emu_cpu_model },
	{ "emu.cpu.speed", c80_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.double", c80_set_msg_emu_cpu_speed_double },
	{ "emu.cpu.speed.half", c80_set_msg_emu_cpu_speed_half },
	{ "emu.cpu.speed.step", c80_set_msg_emu_cpu_speed_step },
	{ "emu.disk.commit", c80_set_msg_emu_disk_commit },
	{ "emu.disk.eject", c80_set_msg_emu_disk_eject },
	{ "emu.disk.insert", c80_set_msg_emu_disk_insert },
	{ "emu.exit", c80_set_msg_emu_exit },
	{ "emu.reset", c80_set_msg_emu_reset },
	{ "emu.stop", c80_set_msg_emu_stop },
	{ NULL, NULL }
};


int c80_set_msg (cpm80_t *sim, const char *msg, const char *val)
{
	c80_msg_list_t *lst;

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

	lst = set_msg_list;

	while (lst->msg != NULL) {
		if (msg_is_message (lst->msg, msg)) {
			return (lst->set_msg (sim, msg, val));
		}

		lst += 1;
	}

	if (msg_is_prefix ("term", msg)) {
		return (1);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
