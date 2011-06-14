/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/msg.c                                         *
 * Created:     2004-09-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "cassette.h"
#include "ibmpc.h"

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
int pc_set_msg_emu_disk_commit (ibmpc_t *pc, const char *msg, const char *val)
{
	int      r;
	unsigned drv;

	if (strcmp (val, "all") == 0) {
		pce_log (MSG_INF, "commiting all drives\n");

		if (dsks_commit (pc->dsk)) {
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

		if (dsks_set_msg (pc->dsk, drv, "commit", NULL)) {
			pce_log (MSG_ERR, "*** commit error for drive %u\n",
				drv
			);

			r = 1;
		}
	}

	return (r);
}

static
int pc_set_msg_emu_disk_eject (ibmpc_t *pc, const char *msg, const char *val)
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

		dsk = dsks_get_disk (pc->dsk, drv);

		if (dsk == NULL) {
			pce_log (MSG_ERR,
				"*** disk eject error: no such disk (%lu)\n", drv
			);
		}
		else {
			pce_log (MSG_INF, "ejecting drive %lu\n", drv);

			dsks_rmv_disk (pc->dsk, dsk);

			dsk_del (dsk);
		}
	}

	return (0);
}

static
int pc_set_msg_emu_disk_insert (ibmpc_t *pc, const char *msg, const char *val)
{
	if (dsk_insert (pc->dsk, val, 1)) {
		return (1);
	}

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

static
int pc_set_msg_emu_tape_append (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc->cas == NULL) {
		return (1);
	}

	pc_cas_append (pc->cas);
	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_file (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc->cas == NULL) {
		return (1);
	}

	if (*val == 0) {
		val = NULL;
	}

	if (pc_cas_set_fname (pc->cas, val)) {
		return (1);
	}

	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_filter (ibmpc_t *pc, const char *msg, const char *val)
{
	int filter;

	if (pc->cas == NULL) {
		return (1);
	}

	if (msg_get_bool (val, &filter)) {
		return (1);
	}

	pc_cas_set_filter (pc->cas, filter);

	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_load (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned long v;

	if (pc->cas == NULL) {
		return (1);
	}

	pc_cas_set_mode (pc->cas, 0);

	if (*val != 0) {
		if (strcmp (val, "end") == 0) {
			pc_cas_append (pc->cas);
		}
		else {
			if (msg_get_ulng (val, &v)) {
				return (1);
			}

			if (pc_cas_set_position (pc->cas, v)) {
				return (1);
			}
		}
	}

	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_pcm (ibmpc_t *pc, const char *msg, const char *val)
{
	int pcm;

	if (pc->cas == NULL) {
		return (1);
	}

	if (msg_get_bool (val, &pcm)) {
		return (1);
	}

	pc_cas_set_pcm (pc->cas, pcm);

	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_rewind (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc->cas == NULL) {
		return (1);
	}

	pc_cas_rewind (pc->cas);
	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_save (ibmpc_t *pc, const char *msg, const char *val)
{
	unsigned long v;

	if (pc->cas == NULL) {
		return (1);
	}

	pc_cas_set_mode (pc->cas, 1);

	if (*val != 0) {
		if (strcmp (val, "end") == 0) {
			pc_cas_append (pc->cas);
		}
		else {
			if (msg_get_ulng (val, &v)) {
				return (1);
			}

			if (pc_cas_set_position (pc->cas, v)) {
				return (1);
			}
		}
	}

	pc_cas_print_state (pc->cas);

	return (0);
}

static
int pc_set_msg_emu_tape_state (ibmpc_t *pc, const char *msg, const char *val)
{
	if (pc->cas == NULL) {
		return (1);
	}

	pc_cas_print_state (pc->cas);

	return (0);
}


static pc_msg_list_t set_msg_list[] = {
	{ "emu.config.save", pc_set_msg_emu_config_save },
	{ "emu.cpu.model", pc_set_msg_emu_cpu_model },
	{ "emu.cpu.speed", pc_set_msg_emu_cpu_speed },
	{ "emu.cpu.speed.step", pc_set_msg_emu_cpu_speed_step },
	{ "emu.disk.boot", pc_set_msg_emu_disk_boot },
	{ "emu.disk.commit", pc_set_msg_emu_disk_commit },
	{ "emu.disk.eject", pc_set_msg_emu_disk_eject },
	{ "emu.disk.insert", pc_set_msg_emu_disk_insert },
	{ "emu.exit", pc_set_msg_emu_exit },
	{ "emu.parport.driver", pc_set_msg_emu_parport_driver },
	{ "emu.parport.file", pc_set_msg_emu_parport_file },
	{ "emu.pause", pc_set_msg_emu_pause },
	{ "emu.pause.toggle", pc_set_msg_emu_pause_toggle },
	{ "emu.reset", pc_set_msg_emu_reset },
	{ "emu.serport.driver", pc_set_msg_emu_serport_driver },
	{ "emu.serport.file", pc_set_msg_emu_serport_file },
	{ "emu.stop", pc_set_msg_emu_stop },
	{ "emu.tape.append", pc_set_msg_emu_tape_append },
	{ "emu.tape.filter", pc_set_msg_emu_tape_filter },
	{ "emu.tape.file", pc_set_msg_emu_tape_file },
	{ "emu.tape.load", pc_set_msg_emu_tape_load },
	{ "emu.tape.pcm", pc_set_msg_emu_tape_pcm },
	{ "emu.tape.rewind", pc_set_msg_emu_tape_rewind },
	{ "emu.tape.save", pc_set_msg_emu_tape_save },
	{ "emu.tape.state", pc_set_msg_emu_tape_state },
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
