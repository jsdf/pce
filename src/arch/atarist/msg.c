/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/msg.c                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "atarist.h"
#include "msg.h"

#include <string.h>

#include <lib/inidsk.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/msg.h>
#include <lib/string.h>
#include <lib/sysdep.h>


extern monitor_t par_mon;


typedef struct {
	const char *msg;

	int (*set_msg) (atari_st_t *sim, const char *msg, const char *val);
} st_msg_list_t;


static
int st_set_msg_emu_cpu_model (atari_st_t *sim, const char *msg, const char *val)
{
	if (st_set_cpu_model (sim, val)) {
		pce_log (MSG_ERR, "unknown CPU model (%s)\n", val);
		return (1);
	}

	return (0);
}

static
int st_set_msg_emu_cpu_speed (atari_st_t *sim, const char *msg, const char *val)
{
	unsigned f;

	if (msg_get_uint (val, &f)) {
		return (1);
	}

	st_set_speed (sim, f);

	return (0);
}

static
int st_set_msg_emu_cpu_speed_step (atari_st_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v += (int) sim->speed_factor;

	if (v <= 0) {
		v = 1;
	}

	st_set_speed (sim, v);

	return (0);
}

static
int st_set_msg_emu_disk_commit (atari_st_t *sim, const char *msg, const char *val)
{
	int      r;
	unsigned drv;

	if (strcmp (val, "all") == 0) {
		pce_log (MSG_INF, "commiting all drives\n");

		st_fdc_save (&sim->fdc, 0);
		st_fdc_save (&sim->fdc, 1);

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

		if (drv < 2) {
			st_fdc_save (&sim->fdc, drv);
		}

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
int st_set_msg_emu_disk_eject (atari_st_t *sim, const char *msg, const char *val)
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

		if (drv < 2) {
			st_fdc_save (&sim->fdc, drv);
		}

		dsk = dsks_get_disk (sim->dsks, drv);
		dsks_rmv_disk (sim->dsks, dsk);
		dsk_del (dsk);

		if (drv < 2) {
			st_fdc_set_fname (&sim->fdc, drv, NULL);
			st_fdc_load (&sim->fdc, drv);
		}
	}

	return (0);
}

static
int st_set_msg_emu_disk_insert (atari_st_t *sim, const char *msg, const char *val)
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

	st_fdc_save (&sim->fdc, drv);

	if (strcasecmp (ext, ".pri") == 0) {
		st_fdc_set_fname (&sim->fdc, drv, str);
	}
	else {
		st_fdc_set_fname (&sim->fdc, drv, NULL);

		if (dsk_insert (sim->dsks, val, 1)) {
			return (1);
		}
	}

	st_fdc_load (&sim->fdc, drv);

	return (0);
}

static
int st_set_msg_emu_exit (atari_st_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_ABORT;

	mon_set_terminate (&par_mon, 1);

	return (0);
}

static
int st_set_msg_emu_par_driver (atari_st_t *sim, const char *msg, const char *val)
{
	if (sim->parport_drv != NULL) {
		chr_close (sim->parport_drv);
	}

	sim->parport_drv = chr_open (val);

	if (sim->parport_drv == NULL) {
		return (1);
	}

	return (0);
}

static
int st_set_msg_emu_par_file (atari_st_t *sim, const char *msg, const char *val)
{
	int  r;
	char *driver;

	driver = str_cat_alloc ("stdio:file=", val);

	r = st_set_msg_emu_par_driver (sim, msg, driver);

	free (driver);

	return (r);
}

static
int st_set_msg_emu_pause (atari_st_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_bool (val, &v)) {
		return (1);
	}

	st_set_pause (sim, v);

	return (0);
}

static
int st_set_msg_emu_pause_toggle (atari_st_t *sim, const char *msg, const char *val)
{
	st_set_pause (sim, !sim->pause);

	return (0);
}

static
int st_set_msg_emu_realtime (atari_st_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_bool (val, &v)) {
		return (1);
	}

	st_set_speed (sim, v ? 1 : 0);

	return (0);
}

static
int st_set_msg_emu_realtime_toggle (atari_st_t *sim, const char *msg, const char *val)
{
	if (sim->speed_factor > 0) {
		st_set_speed (sim, 0);
	}
	else {
		st_set_speed (sim, 1);
	}

	return (0);
}

static
int st_set_msg_emu_reset (atari_st_t *sim, const char *msg, const char *val)
{
	st_reset (sim);

	return (0);
}

static
int st_set_msg_emu_ser_driver (atari_st_t *sim, const char *msg, const char *val)
{
	if (sim->serport_drv != NULL) {
		chr_close (sim->serport_drv);
	}

	sim->serport_drv = chr_open (val);

	if (sim->serport_drv == NULL) {
		return (1);
	}

	return (0);
}

static
int st_set_msg_emu_ser_file (atari_st_t *sim, const char *msg, const char *val)
{
	int  r;
	char *driver;

	driver = str_cat_alloc ("stdio:file=", val);

	r = st_set_msg_emu_ser_driver (sim, msg, driver);

	free (driver);

	return (r);
}

static
int st_set_msg_emu_stop (atari_st_t *sim, const char *msg, const char *val)
{
	st_set_msg_trm (sim, "term.release", "1");

	sim->brk = PCE_BRK_STOP;

	return (0);
}


static st_msg_list_t set_msg_list[] = {
	{ "emu.cpu.model", st_set_msg_emu_cpu_model },
	{ "emu.cpu.speed", st_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", st_set_msg_emu_cpu_speed_step },
	{ "emu.disk.commit", st_set_msg_emu_disk_commit },
	{ "emu.disk.eject", st_set_msg_emu_disk_eject },
	{ "emu.disk.insert", st_set_msg_emu_disk_insert },
	{ "emu.exit", st_set_msg_emu_exit },
	{ "emu.par.driver", st_set_msg_emu_par_driver },
	{ "emu.par.file", st_set_msg_emu_par_file },
	{ "emu.pause", st_set_msg_emu_pause },
	{ "emu.pause.toggle", st_set_msg_emu_pause_toggle },
	{ "emu.realtime", st_set_msg_emu_realtime },
	{ "emu.realtime.toggle", st_set_msg_emu_realtime_toggle },
	{ "emu.reset", st_set_msg_emu_reset },
	{ "emu.ser.driver", st_set_msg_emu_ser_driver },
	{ "emu.ser.file", st_set_msg_emu_ser_file },
	{ "emu.stop", st_set_msg_emu_stop },
	{ NULL, NULL }
};


int st_set_msg (atari_st_t *sim, const char *msg, const char *val)
{
	int           r;
	st_msg_list_t *lst;

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

	if (msg_is_prefix ("term", msg)) {
		return (1);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
