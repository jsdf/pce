/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/msg.c                                         *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "rc759.h"

#include <string.h>

#include <drivers/block/block.h>

#include <lib/inidsk.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/msg.h>
#include <lib/msgdsk.h>
#include <lib/sysdep.h>


extern monitor_t par_mon;


typedef struct {
	const char *msg;

	int (*set_msg) (rc759_t *sim, const char *msg, const char *val);
} rc759_msg_list_t;


static
int rc759_set_msg_disk_eject (rc759_t *sim, const char *msg, const char *val)
{
	if (msg_dsk_get_disk_id (val, &sim->disk_id)) {
		return (1);
	}

	rc759_fdc_eject_disk (&sim->fdc, sim->disk_id);

	msg_dsk_emu_disk_eject (val, sim->dsks, &sim->disk_id);

	return (0);
}

static
int rc759_set_msg_disk_insert (rc759_t *sim, const char *msg, const char *val)
{
	if (dsks_get_disk (sim->dsks, sim->disk_id) != NULL) {
		rc759_fdc_eject_disk (&sim->fdc, sim->disk_id);
	}

	if (msg_dsk_emu_disk_insert (val, sim->dsks, sim->disk_id)) {
		return (1);
	}

	rc759_fdc_insert_disk (&sim->fdc, sim->disk_id);

	return (0);
}

static
int rc759_set_msg_emu_config_save (rc759_t *sim, const char *msg, const char *val)
{
	if (ini_write (val, sim->cfg)) {
		return (1);
	}

	return (0);
}

static
int rc759_set_msg_emu_cpu_speed (rc759_t *sim, const char *msg, const char *val)
{
	unsigned f;

	if (msg_get_uint (val, &f)) {
		return (1);
	}

	rc759_set_speed (sim, f);

	return (0);
}

static
int rc759_set_msg_emu_cpu_speed_step (rc759_t *sim, const char *msg, const char *val)
{
	int      v;
	unsigned speed;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	speed = sim->speed;

	if (v > 0) {
		speed = sim->speed + v;
	}
	else {
		speed = -v;
		speed = (-v < sim->speed) ? (sim->speed - -v) : 1;
	}

	rc759_set_speed (sim, speed);

	return (0);
}

static
int rc759_set_msg_emu_exit (rc759_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_ABORT;
	mon_set_terminate (&par_mon, 1);
	return (0);
}

static
int rc759_set_msg_emu_parport1_driver (rc759_t *sim, const char *msg, const char *val)
{
	if (rc759_set_parport_driver (sim, 0, val)) {
		return (1);
	}

	return (0);
}

static
int rc759_set_msg_emu_parport1_file (rc759_t *sim, const char *msg, const char *val)
{
	if (rc759_set_parport_file (sim, 0, val)) {
		return (1);
	}

	return (0);
}

static
int rc759_set_msg_emu_parport2_driver (rc759_t *sim, const char *msg, const char *val)
{
	if (rc759_set_parport_driver (sim, 1, val)) {
		return (1);
	}

	return (0);
}

static
int rc759_set_msg_emu_parport2_file (rc759_t *sim, const char *msg, const char *val)
{
	if (rc759_set_parport_file (sim, 1, val)) {
		return (1);
	}

	return (0);
}

static
int rc759_set_msg_emu_pause (rc759_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_bool (val, &v)) {
		return (1);
	}

	sim->pause = v;

	rc759_clock_discontinuity (sim);

	return (0);
}

static
int rc759_set_msg_emu_pause_toggle (rc759_t *sim, const char *msg, const char *val)
{
	sim->pause = !sim->pause;

	rc759_clock_discontinuity (sim);

	return (0);
}

static
int rc759_set_msg_emu_reset (rc759_t *sim, const char *msg, const char *val)
{
	rc759_reset (sim);
	return (0);
}

static
int rc759_set_msg_emu_stop (rc759_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_STOP;
	return (0);
}


static rc759_msg_list_t set_msg_list[] = {
	{ "disk.eject", rc759_set_msg_disk_eject },
	{ "disk.insert", rc759_set_msg_disk_insert },
	{ "emu.config.save", rc759_set_msg_emu_config_save },
	{ "emu.cpu.speed", rc759_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", rc759_set_msg_emu_cpu_speed_step },
	{ "emu.exit", rc759_set_msg_emu_exit },
	{ "emu.parport1.driver", rc759_set_msg_emu_parport1_driver },
	{ "emu.parport1.file", rc759_set_msg_emu_parport1_file },
	{ "emu.parport2.driver", rc759_set_msg_emu_parport2_driver },
	{ "emu.parport2.file", rc759_set_msg_emu_parport2_file },
	{ "emu.pause", rc759_set_msg_emu_pause },
	{ "emu.pause.toggle", rc759_set_msg_emu_pause_toggle },
	{ "emu.reset", rc759_set_msg_emu_reset },
	{ "emu.stop", rc759_set_msg_emu_stop },
	{ NULL, NULL }
};


int rc759_set_msg (rc759_t *sim, const char *msg, const char *val)
{
	int              r;
	rc759_msg_list_t *lst;

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

	if ((r = msg_dsk_set_msg (msg, val, sim->dsks, &sim->disk_id)) >= 0) {
		return (r);
	}

	if (sim->trm != NULL) {
		r = trm_set_msg_trm (sim->trm, msg, val);

		if (r >= 0) {
			return (r);
		}
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
