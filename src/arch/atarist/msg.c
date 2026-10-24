/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/msg.c                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2019 Hampa Hug <hampa@hampa.ch>                     *
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
#include "video.h"
#include "viking.h"

#include <string.h>

#include <lib/inidsk.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/msg.h>
#include <lib/msgdsk.h>
#include <lib/string.h>
#include <lib/sysdep.h>


extern monitor_t par_mon;


typedef struct {
	const char *msg;

	int (*set_msg) (atari_st_t *sim, const char *msg, const char *val);
} st_msg_list_t;


static
int st_set_msg_disk_eject (atari_st_t *sim, const char *msg, const char *val)
{
	if (msg_dsk_get_disk_id (val, &sim->disk_id)) {
		return (1);
	}

	st_fdc_eject_disk (&sim->fdc, sim->disk_id);

	msg_dsk_emu_disk_eject (val, sim->dsks, &sim->disk_id);

	return (0);
}

static
int st_set_msg_disk_insert (atari_st_t *sim, const char *msg, const char *val)
{
	if (dsks_get_disk (sim->dsks, sim->disk_id) != NULL) {
		st_fdc_eject_disk (&sim->fdc, sim->disk_id);
	}

	if (msg_dsk_emu_disk_insert (val, sim->dsks, sim->disk_id)) {
		return (1);
	}

	st_fdc_insert_disk (&sim->fdc, sim->disk_id);

	return (0);
}

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
int st_set_msg_emu_exit (atari_st_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_ABORT;

	mon_set_terminate (&par_mon, 1);

	return (0);
}

static
int set_fdc_ro_rw (atari_st_t *sim, const char *msg, const char *val, int wprot)
{
	unsigned i, drv, msk;

	if ((*val == 0) || (strcmp (val, "all") == 0)) {
		msk = 3;
	}
	else {
		if (msg_get_uint (val, &drv)) {
			return (1);
		}

		if (drv > 1) {
			return (1);
		}

		msk = 1U << drv;
	}

	for (i = 0; i < 2; i++) {
		if (msk & (1U << i)) {
			pce_log (MSG_INF,
				"setting drive %u to %s\n",
				i, wprot ? "RO" : "RW"
			);
			st_fdc_set_wprot (&sim->fdc, i, wprot);
		}
	}

	return (0);
}

static
int st_set_msg_emu_fdc_ro (atari_st_t *sim, const char *msg, const char *val)
{
	return (set_fdc_ro_rw (sim, msg, val, 1));
}

static
int st_set_msg_emu_fdc_rw (atari_st_t *sim, const char *msg, const char *val)
{
	return (set_fdc_ro_rw (sim, msg, val, 0));
}

static
int st_set_msg_emu_midi_file (atari_st_t *sim, const char *msg, const char *val)
{
	return (st_smf_set_file (&sim->smf, val));
}

static
int st_set_msg_emu_par_driver (atari_st_t *sim, const char *msg, const char *val)
{
	char_drv_t *drv;

	if (*val == 0) {
		drv = NULL;
	}
	else {
		if ((drv = chr_open (val)) == NULL) {
			return (1);
		}
	}

	st_set_parport_drv (sim, drv);

	return (0);
}

static
int st_set_msg_emu_par_file (atari_st_t *sim, const char *msg, const char *val)
{
	int  r;
	char *driver;

	if (*val == 0) {
		st_set_parport_drv (sim, NULL);
		return (0);
	}

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
int st_set_msg_emu_psg_aym_file (atari_st_t *sim, const char *msg, const char *val)
{
	if (st_psg_set_aym (&sim->psg, val)) {
		pce_log (MSG_ERR, "*** failed to open AYM file (%s)\n", val);
		return (1);
	}

	return (0);
}

static
int st_set_msg_emu_psg_aym_res (atari_st_t *sim, const char *msg, const char *val)
{
	unsigned long r;

	if (msg_get_ulng (val, &r)) {
		return (1);
	}

	st_psg_set_aym_resolution (&sim->psg, r);

	return (0);
}

static
int st_set_msg_emu_psg_driver (atari_st_t *sim, const char *msg, const char *val)
{
	if (st_psg_set_driver (&sim->psg, val)) {
		st_log_deb (MSG_ERR, "*** failed to open sound driver (%s)\n", val);
		return (1);
	}

	return (0);
}

static
int st_set_msg_emu_psg_lowpass (atari_st_t *sim, const char *msg, const char *val)
{
	unsigned long f;

	if (msg_get_ulng (val, &f)) {
		return (1);
	}

	st_psg_set_lowpass (&sim->psg, f);

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

static
int st_set_msg_emu_viking (atari_st_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_bool (val, &v)) {
		return (1);
	}

	if (v) {
		sim->video_viking = 1;
		st_video_set_terminal (sim->video, NULL);
		st_viking_set_terminal (sim->viking, sim->trm);
	}
	else {
		sim->video_viking = 0;
		st_video_set_terminal (sim->video, sim->trm);
		st_viking_set_terminal (sim->viking, NULL);
	}

	return (0);
}

static
int st_set_msg_emu_viking_toggle (atari_st_t *sim, const char *msg, const char *val)
{
	if (sim->viking == NULL) {
		return (0);
	}

	if (sim->video_viking) {
		sim->video_viking = 0;
		st_video_set_terminal (sim->video, sim->trm);
		st_viking_set_terminal (sim->viking, NULL);
	}
	else {
		sim->video_viking = 1;
		st_video_set_terminal (sim->video, NULL);
		st_viking_set_terminal (sim->viking, sim->trm);
	}

	return (0);
}


static st_msg_list_t set_msg_list[] = {
	{ "disk.eject", st_set_msg_disk_eject },
	{ "disk.insert", st_set_msg_disk_insert },
	{ "emu.cpu.model", st_set_msg_emu_cpu_model },
	{ "emu.cpu.speed", st_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", st_set_msg_emu_cpu_speed_step },
	{ "emu.exit", st_set_msg_emu_exit },
	{ "emu.fdc.ro", st_set_msg_emu_fdc_ro },
	{ "emu.fdc.rw", st_set_msg_emu_fdc_rw },
	{ "emu.midi.file", st_set_msg_emu_midi_file },
	{ "emu.par.driver", st_set_msg_emu_par_driver },
	{ "emu.par.file", st_set_msg_emu_par_file },
	{ "emu.pause", st_set_msg_emu_pause },
	{ "emu.pause.toggle", st_set_msg_emu_pause_toggle },
	{ "emu.psg.aym.file", st_set_msg_emu_psg_aym_file },
	{ "emu.psg.aym.res", st_set_msg_emu_psg_aym_res },
	{ "emu.psg.driver", st_set_msg_emu_psg_driver },
	{ "emu.psg.lowpass", st_set_msg_emu_psg_lowpass },
	{ "emu.realtime", st_set_msg_emu_realtime },
	{ "emu.realtime.toggle", st_set_msg_emu_realtime_toggle },
	{ "emu.reset", st_set_msg_emu_reset },
	{ "emu.ser.driver", st_set_msg_emu_ser_driver },
	{ "emu.ser.file", st_set_msg_emu_ser_file },
	{ "emu.stop", st_set_msg_emu_stop },
	{ "emu.viking", st_set_msg_emu_viking },
	{ "emu.viking.toggle", st_set_msg_emu_viking_toggle },
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

	if ((r = msg_dsk_set_msg (msg, val, sim->dsks, &sim->disk_id)) >= 0) {
		return (r);
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
