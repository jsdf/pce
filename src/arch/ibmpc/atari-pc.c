/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/atari-pc.c                                    *
 * Created:     2018-09-01 by Hampa Hug <hampa@hampa.ch>                     *
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

/* Atari PC specific code */


#include "main.h"
#include "ibmpc.h"
#include "atari-pc.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <chipset/clock/mc146818a.h>

#include <lib/log.h>

#include <libini/libini.h>


#ifndef DEBUG_ATARI_PC
#define DEBUG_ATARI_PC 0
#endif


static
void atari_pc_set_turbo (ibmpc_t *pc, int val)
{
	if (val) {
		pc_set_speed (pc, pc->atari_pc_turbo);
	}
	else {
		pc_set_speed (pc, 1);
	}
}

static
void atari_pc_set_port_34 (ibmpc_t *pc, unsigned char val)
{
	if ((pc->atari_pc_port34 ^ val) & 1) {
		atari_pc_set_turbo (pc, val & 1);
	}

#if DEBUG_ATARI_PC >= 1
	pc_log_deb ("ATARI: 0034 <- %02X\n", val);
#endif

	pc->atari_pc_port34 = val;
}

int atari_pc_get_port8 (ibmpc_t *pc, unsigned long addr, unsigned char *val)
{
	if ((pc->model & PCE_IBMPC_ATARI) == 0) {
		return (1);
	}

	switch (addr) {
	case 0x34:
		*val = pc->atari_pc_port34;
		return (0);

	case 0x170:
		*val = pc->atari_pc_rtc_port;
		return (0);

	case 0x171:
		if (pc->atari_pc_rtc != NULL) {
			*val = mc146818a_get_uint8 (pc->atari_pc_rtc, pc->atari_pc_rtc_port & 0x3f);
		}
		else {
			*val = 0;
		}

		return (0);

	case 0x3c2:
		*val = pc->atari_pc_switches;

#if DEBUG_ATARI_PC >= 1
		fprintf (stderr, "ATARI: 03C2 <- %02X\n", *val);
#endif

		return (0);
	}

	return (1);
}

int atari_pc_set_port8 (ibmpc_t *pc, unsigned long addr, unsigned char val)
{
	if ((pc->model & PCE_IBMPC_ATARI) == 0) {
		return (1);
	}

	switch (addr) {
	case 0x34:
		atari_pc_set_port_34 (pc, val);
		return (0);

	case 0x170:
		pc->atari_pc_rtc_port = val;
		return (0);

	case 0x171:
		if (pc->atari_pc_rtc != NULL) {
			mc146818a_set_uint8 (pc->atari_pc_rtc, pc->atari_pc_rtc_port & 0x3f, val);
		}
		return (0);
	}

	return (1);
}

void pc_setup_atari_pc (ibmpc_t *pc, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	unsigned      switches, turbo;
	const char    *rtc, *start;

	pc->atari_pc_turbo = 2;
	pc->atari_pc_port34 = 0;
	pc->atari_pc_switches = 0;
	pc->atari_pc_rtc_port = 0;
	pc->atari_pc_rtc = NULL;

	if ((pc->model & PCE_IBMPC_ATARI) == 0) {
		return;
	}

	sct = ini_next_sct (ini, NULL, "atari_pc");

	ini_get_uint16 (sct, "switches", &switches, 0);
	ini_get_uint16 (sct, "turbo", &turbo, 2);
	ini_get_string (sct, "rtc", &rtc, NULL);
	ini_get_string (sct, "rtc_start", &start, NULL);

	pce_log_tag (MSG_INF, "ATARI:",
		"rtc=%s rtc start=%s switches=%02X turbo=%u\n",
		(rtc == NULL) ? "<none>" : rtc,
		(start == NULL) ? "now" : start,
		switches, turbo
	);

	pc->atari_pc_turbo = turbo;
	pc->atari_pc_switches = switches;

	if ((pc->atari_pc_rtc = mc146818a_new()) == NULL) {
		return;
	}

	mc146818a_init (pc->atari_pc_rtc);
	mc146818a_set_input_clock (pc->atari_pc_rtc, PCE_IBMPC_CLK2);
	mc146818a_set_century_offset (pc->atari_pc_rtc, 0x32);
	mc146818a_set_fname (pc->atari_pc_rtc, rtc);
	mc146818a_load (pc->atari_pc_rtc);
	mc146818a_set_time_string (pc->atari_pc_rtc, start);
}

void atari_pc_del (ibmpc_t *pc)
{
	if (pc->atari_pc_rtc != NULL) {
		mc146818a_save (pc->atari_pc_rtc);
		mc146818a_free (pc->atari_pc_rtc);
	}
}
