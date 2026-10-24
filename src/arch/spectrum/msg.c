/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/msg.c                                      *
 * Created:     2022-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "spectrum.h"
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

	int (*set_msg) (spectrum_t *sim, const char *msg, const char *val);
} spec_msg_list_t;


extern monitor_t par_mon;
extern spectrum_t   *par_sim;


static
int spec_set_msg_emu_cpu_speed (spectrum_t *sim, const char *msg, const char *val)
{
	unsigned v;

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	spec_set_speed (sim, v);

	return (0);
}

static
int spec_set_msg_emu_cpu_speed_step (spectrum_t *sim, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v += (int) sim->speed;

	if (v <= 0) {
		v = 1;
	}

	spec_set_speed (sim, v);

	return (0);
}

static
int spec_set_msg_emu_exit (spectrum_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_ABORT;

	mon_set_terminate (&par_mon, 1);

	return (0);
}

static
int spec_set_msg_emu_reset (spectrum_t *sim, const char *msg, const char *val)
{
	spec_reset (sim);

	return (0);
}

static
int spec_set_msg_emu_stop (spectrum_t *sim, const char *msg, const char *val)
{
	sim->brk = PCE_BRK_STOP;

	return (0);
}


static spec_msg_list_t set_msg_list[] = {
	{ "emu.cpu.speed", spec_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", spec_set_msg_emu_cpu_speed_step },
	{ "emu.exit", spec_set_msg_emu_exit },
	{ "emu.reset", spec_set_msg_emu_reset },
	{ "emu.stop", spec_set_msg_emu_stop },
	{ NULL, NULL }
};


int spec_set_msg (spectrum_t *sim, const char *msg, const char *val)
{
	int             r;
	spec_msg_list_t *lst;

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

	if (sim->term != NULL) {
		if ((r = trm_set_msg_trm (sim->term, msg, val)) >= 0) {
			return (r);
		}
	}

	if (msg_is_prefix ("term", msg)) {
		return (1);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
