/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/msg.c                                         *
 * Created:     2004-09-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2024 Hampa Hug <hampa@hampa.ch>                     *
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
#include "ibmpc.h"

#include <string.h>

#include <devices/cassette.h>

#include <drivers/block/block.h>

#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/msg.h>
#include <lib/msgdsk.h>
#include <lib/sysdep.h>


extern monitor_t par_mon;


typedef struct {
	const char *msg;

	int (*set_msg) (ibmpc_t *pc, const char *msg, const char *val);
} pc_msg_list_t;


static
int pc_set_msg_emu_config_save (ibmpc_t *pc, const char *msg, const char *val)
{
	if (ini_write (val, pc->cfg)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_cpu_model (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc_set_cpu_model (pc, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_cpu_speed (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned f;

	if (msg_get_uint (val, &f)) {
		return (1);
	}

	pc_set_speed (pc, f);

	return (0);
}

static
int pc_set_msg_emu_cpu_speed_step (ibmpc_t *pc, const char *msg, const char *val)
{
	int v;

	if (msg_get_sint (val, &v)) {
		return (1);
	}

	v += (int) pc->speed_current;

	if (v <= 0) {
		v = 1;
	}

	pc_set_speed (pc, v);

	return (0);
}

static
int pc_set_msg_emu_disk_boot (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned v;

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	pc->bootdrive = v;

	return (0);
}

static
int pc_set_msg_emu_exit (ibmpc_t *pc, const char *msg, const char *val)
{
	pc->brk = PCE_BRK_ABORT;
	mon_set_terminate (&par_mon, 1);
	return (0);
}

static
int pc_set_msg_emu_fdc_accurate (ibmpc_t *pc, const char *msg, const char *val)
{
	int v;

	if (msg_get_bool (val, &v)) {
		return (1);
	}

	if (pc->fdc != NULL) {
		pce_log (MSG_INF, "%s the FDC accurate mode\n",
			v ? "Enabling" : "Disabling"
		);

		e8272_set_accuracy (&pc->fdc->e8272, v);
	}

	return (0);
}

static
int pc_set_msg_emu_fdc_verbose (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned v;

	if (msg_get_uint (val, &v)) {
		return (1);
	}

	if (pc->fdc != NULL) {
		pce_log (MSG_INF, "FDC verbosity level %u\n", v);

		e8272_set_verbose (&pc->fdc->e8272, v);
	}

	return (0);
}

static
int pc_set_msg_emu_par1_file (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc_set_parport_file (pc, 0, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_par2_file (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc_set_parport_file (pc, 1, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_par3_file (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc_set_parport_file (pc, 2, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_parport_driver (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned idx;

	if (msg_get_prefix_uint (&val, &idx, ":", " \t")) {
		return (1);
	}

	if (pc_set_parport_driver (pc, idx, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_parport_file (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned idx;

	if (msg_get_prefix_uint (&val, &idx, ":", " \t")) {
		return (1);
	}

	if (pc_set_parport_file (pc, idx, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_pause (ibmpc_t *pc, const char *msg, const char *val)
{
	int v;

	if (msg_get_bool (val, &v)) {
		return (1);
	}

	pc->pause = v;

	pc_clock_discontinuity (pc);

	return (0);
}

static
int pc_set_msg_emu_pause_toggle (ibmpc_t *pc, const char *msg, const char *val)
{
	pc->pause = !pc->pause;

	pc_clock_discontinuity (pc);

	return (0);
}

static
int pc_set_msg_emu_reset (ibmpc_t *pc, const char *msg, const char *val)
{
	pc_reset (pc);
	return (0);
}

static
int pc_set_msg_emu_serport_driver (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned idx;

	if (msg_get_prefix_uint (&val, &idx, ":", " \t")) {
		return (1);
	}

	if (pc_set_serport_driver (pc, idx, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_serport_file (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned idx;

	if (msg_get_prefix_uint (&val, &idx, ":", " \t")) {
		return (1);
	}

	if (pc_set_serport_file (pc, idx, val)) {
		return (1);
	}

	return (0);
}

static
int pc_set_msg_emu_stop (ibmpc_t *pc, const char *msg, const char *val)
{
	pc->brk = PCE_BRK_STOP;
	return (0);
}


static pc_msg_list_t set_msg_list[] = {
	{ "emu.config.save", pc_set_msg_emu_config_save },
	{ "emu.cpu.model", pc_set_msg_emu_cpu_model },
	{ "emu.cpu.speed", pc_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", pc_set_msg_emu_cpu_speed_step },
	{ "emu.disk.boot", pc_set_msg_emu_disk_boot },
	{ "emu.exit", pc_set_msg_emu_exit },
	{ "emu.fdc.accurate", pc_set_msg_emu_fdc_accurate },
	{ "emu.fdc.verbose", pc_set_msg_emu_fdc_verbose },
	{ "emu.par1.file", pc_set_msg_emu_par1_file },
	{ "emu.par2.file", pc_set_msg_emu_par2_file },
	{ "emu.par3.file", pc_set_msg_emu_par3_file },
	{ "emu.parport.driver", pc_set_msg_emu_parport_driver },
	{ "emu.parport.file", pc_set_msg_emu_parport_file },
	{ "emu.pause", pc_set_msg_emu_pause },
	{ "emu.pause.toggle", pc_set_msg_emu_pause_toggle },
	{ "emu.reset", pc_set_msg_emu_reset },
	{ "emu.serport.driver", pc_set_msg_emu_serport_driver },
	{ "emu.serport.file", pc_set_msg_emu_serport_file },
	{ "emu.stop", pc_set_msg_emu_stop },
	{ NULL, NULL }
};


int pc_set_msg (ibmpc_t *pc, const char *msg, const char *val)
{
	int           r;
	pc_msg_list_t *lst;

	/* a hack, for debugging only */
	if (pc == NULL) {
		pc = par_pc;
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
			return (lst->set_msg (pc, msg, val));
		}

		lst += 1;
	}

	if (pc->cas != NULL) {
		if ((r = cas_set_msg (pc->cas, msg, val)) >= 0) {
			return (r);
		}
	}

	if ((r = msg_dsk_set_msg (msg, val, pc->dsk, &pc->disk_id)) >= 0) {
		return (r);
	}

	if (pc->trm != NULL) {
		r = trm_set_msg_trm (pc->trm, msg, val);

		if (r >= 0) {
			return (r);
		}
	}

	if (pc->video != NULL) {
		r = pce_video_set_msg (pc->video, msg, val);

		if (r >= 0) {
			return (r);
		}
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
