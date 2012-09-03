/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/msg.c                                         *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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
#include <lib/sysdep.h>


extern monitor_t par_mon;


typedef struct {
	const char *msg;

	int (*set_msg) (rc759_t *sim, const char *msg, const char *val);
} rc759_msg_list_t;


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
	int           v;
	unsigned long clk;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	clk = sim->cpu_clock_frq;

	while (v > 0) {
		clk = (9 * clk + 4) / 8;
		v -= 1;
	}

	while (v < 0) {
		clk = (8 * clk + 4) / 9;
		v += 1;
	}

	rc759_set_cpu_clock (sim, clk);

	return (0);
}

static
int rc759_set_msg_emu_disk_commit (rc759_t *sim, const char *msg, const char *val)
{
	int      r;
	unsigned drv;

	if (strcmp (val, "all") == 0) {
		pce_log (MSG_INF, "commiting all drives\n");

		rc759_fdc_save (&sim->fdc, 0);
		rc759_fdc_save (&sim->fdc, 1);

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

		rc759_fdc_save (&sim->fdc, drv);

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
int rc759_set_msg_emu_disk_eject (rc759_t *sim, const char *msg, const char *val)
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

		pce_log (MSG_INF, "ejecting drive %lu\n", drv);

		rc759_fdc_save (&sim->fdc, drv);
		rc759_fdc_set_fname (&sim->fdc, drv, NULL);

		dsk = dsks_get_disk (sim->dsks, drv);
		dsks_rmv_disk (sim->dsks, dsk);
		dsk_del (dsk);

		rc759_fdc_load (&sim->fdc, drv);
	}

	return (0);
}

static
int rc759_set_msg_emu_disk_insert (rc759_t *sim, const char *msg, const char *val)
{
	unsigned   i;
	unsigned   drv;
	const char *str, *ext;

	str = val;

	if (msg_get_prefix_uint (&str, &drv, ":", " \t")) {
		pce_log (MSG_ERR,
			"*** disk eject error: bad drive (%s)\n",
			val
		);

		return (1);
	}

	i = 0;
	ext = str;

	while (str[i] != 0) {
		if (str[i] == '.') {
			ext = str + i;
		}

		i += 1;
	}

	rc759_fdc_save (&sim->fdc, drv);

	if (strcasecmp (ext, ".pbit") == 0) {
		rc759_fdc_set_fname (&sim->fdc, drv, str);
	}
	else {
		if (dsk_insert (sim->dsks, val, 1)) {
			return (1);
		}
	}

	rc759_fdc_load (&sim->fdc, drv);

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
	{ "emu.config.save", rc759_set_msg_emu_config_save },
	{ "emu.cpu.speed", rc759_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", rc759_set_msg_emu_cpu_speed_step },
	{ "emu.disk.commit", rc759_set_msg_emu_disk_commit },
	{ "emu.disk.eject", rc759_set_msg_emu_disk_eject },
	{ "emu.disk.insert", rc759_set_msg_emu_disk_insert },
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

	if (sim->trm != NULL) {
		r = trm_set_msg_trm (sim->trm, msg, val);

		if (r >= 0) {
			return (r);
		}
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
