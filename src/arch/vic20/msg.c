/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/msg.c                                         *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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
#include "vic20.h"

#include <string.h>

#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/msg.h>
#include <lib/string.h>
#include <lib/sysdep.h>


extern monitor_t par_mon;


typedef struct {
	const char *msg;

	int (*set_msg) (vic20_t *sim, const char *msg, const char *val);
} v20_msg_list_t;


static
int v20_set_msg_emu_config_save (vic20_t *sim, const char *msg, const char *val)
{
	if (ini_write (val, sim->cfg)) {
		return (1);
	}

	return (0);
}

static
int v20_set_msg_emu_cpu_speed (vic20_t *sim, const char *msg, const char *val)
{
	unsigned v;

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	v20_set_speed (sim, v);

	return (0);
}

static
int v20_set_msg_emu_cpu_speed_step (vic20_t *sim, const char *msg, const char *val)
{
	int      t;
	unsigned v;

	if (msg_get_sint (val, &t)) {
		return (1);
	}

	if (t >= 0) {
		v = sim->speed + t;
	}
	else {
		if (-t >= sim->speed) {
			v = 1;
		}
		else {
			v = sim->speed + t;
		}
	}

	v20_set_speed (sim, v);

	return (0);
}

static
int v20_set_msg_emu_exit (vic20_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_ABORT;

	mon_set_terminate (&par_mon, 1);

	return (0);
}

static
int v20_set_msg_emu_reset (vic20_t *sim, const char *msg, const char *val)
{
	v20_reset (sim);

	return (0);
}

static
int v20_set_msg_emu_stop (vic20_t *sim, const char *msg, const char *val)
{
	if (sim->trm != NULL) {
		trm_set_msg_trm (sim->trm, "term.release", "1");
	}

	sim->brk = PCE_BRK_STOP;

	return (0);
}

static
int v20_set_msg_emu_video_brightness (vic20_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v20_video_set_brightness (&sim->video, v / 100.0);

	return (0);
}

static
int v20_set_msg_emu_video_framedrop (vic20_t *sim, const char *msg, const char *val)
{
	unsigned v;

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	v20_video_set_framedrop (&sim->video, v);

	return (0);
}

static
int v20_set_msg_emu_video_hue (vic20_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v20_video_set_hue (&sim->video, (double) v);

	return (0);
}

static
int v20_set_msg_emu_video_saturation (vic20_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v20_video_set_saturation (&sim->video, v / 100.0);

	return (0);
}

static v20_msg_list_t set_msg_list[] = {
	{ "emu.config.save", v20_set_msg_emu_config_save },
	{ "emu.cpu.speed", v20_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", v20_set_msg_emu_cpu_speed_step },
	{ "emu.exit", v20_set_msg_emu_exit },
	{ "emu.reset", v20_set_msg_emu_reset },
	{ "emu.stop", v20_set_msg_emu_stop },
	{ "emu.video.brightness", v20_set_msg_emu_video_brightness },
	{ "emu.video.framedrop", v20_set_msg_emu_video_framedrop },
	{ "emu.video.hue", v20_set_msg_emu_video_hue },
	{ "emu.video.saturation", v20_set_msg_emu_video_saturation },
	{ NULL, NULL }
};


int v20_set_msg (vic20_t *sim, const char *msg, const char *val)
{
	int            r;
	v20_msg_list_t *lst;

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

	if ((r = cas_set_msg (&sim->cas, msg, val)) >= 0) {
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
