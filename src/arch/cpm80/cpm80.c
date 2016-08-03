/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim8080/sim8080.c                                   *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2016 Hampa Hug <hampa@hampa.ch>                     *
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
#include "bios.h"
#include "cpm80.h"
#include "msg.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <cpu/e8080/e8080.h>
#include <devices/memory.h>
#include <drivers/block/block.h>
#include <drivers/char/char.h>
#include <libini/libini.h>
#include <lib/brkpt.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <lib/path.h>
#include <lib/sysdep.h>


static
void c80_set_port8 (cpm80_t *sim, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0x01:
	case 0x11:
		if (val == 0x5f) {
			val = 0x08;
		}
		con_putc (sim, val);
		break;

	case 0x00:
	case 0x02:
		break;

	case 0xfe:
		c80_stop (sim);
		break;

	default:
		sim_log_deb ("OUT %04lX, %02X\n", addr, val);
		break;
	}
}

static
unsigned char c80_get_port8 (cpm80_t *sim, unsigned long addr)
{
	unsigned char val;

	val = 0x00;

	switch (addr) {
	case 0x00:
	case 0x10:
		val = 0x02;
		if (con_ready (sim)) {
			val |= (addr == 0) ? 0x20 : 0x21;
		}
		break;

	case 0x01:
	case 0x11:
		if (con_getc (sim, &val)) {
			val = 0;
		}
		else {
			if ((val == 0x7f) || (val == 0x08)) {
				val = 0x5f;
			}
		}
		break;

	case 0x13:
	case 0x20:
		val = 0;
		break;

	case 0xff:
		val = 0xff;
		break;

	default:
		sim_log_deb ("IN %04lX (%02X)\n", addr, val);
		break;
	}

	return (val);
}

static
void c80_setup_system (cpm80_t *sim, ini_sct_t *ini)
{
	const char *model;
	ini_sct_t  *sct;

	sim->brk = 0;
	sim->clk_cnt = 0;
	sim->clk_div = 0;

	sim->model = CPM80_MODEL_PLAIN;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "model", &model, "cpm");

	pce_log_tag (MSG_INF,
		"SYSTEM:",
		"model=%s\n", model
	);

	if (strcmp (model, "plain") == 0) {
		sim->model = CPM80_MODEL_PLAIN;
	}
	else if (strcmp (model, "cpm") == 0) {
		sim->model = CPM80_MODEL_CPM;
	}
	else {
		pce_log (MSG_ERR, "*** unknown machine model (%s)\n", model);
	}
}

static
void c80_setup_mem (cpm80_t *sim, ini_sct_t *ini)
{
	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);
}

static
void c80_setup_cpu (cpm80_t *sim, ini_sct_t *ini)
{
	const char    *cpu;
	unsigned      speed;
	unsigned long clock;
	ini_sct_t     *sct;

	sct = ini_next_sct (ini, NULL, "system");

	if (ini_get_uint32 (sct, "clock", &clock, 0)) {
		ini_get_uint16 (sct, "speed", &speed, 0);
		clock = 1000000UL * speed;
	}

	ini_get_string (sct, "cpu", &cpu, "8080");

	pce_log_tag (MSG_INF, "CPU:", "model=%s clock=%lu\n", cpu, clock);

	sim->cpu = e8080_new();

	if (sim->cpu == NULL) {
		pce_log (MSG_ERR, "*** failed to create the CPU\n");
		return;
	}

	e8080_set_mem_fct (sim->cpu, sim->mem, &mem_get_uint8, &mem_set_uint8);
	e8080_set_port_fct (sim->cpu, sim, c80_get_port8, c80_set_port8);

	if (sim->ram != NULL) {
		unsigned char *data;
		unsigned long size;

		size = mem_blk_get_size (sim->ram);
		data = mem_blk_get_data (sim->ram);

		e8080_set_mem_map_rd (sim->cpu, 0, size - 1, data);
		e8080_set_mem_map_wr (sim->cpu, 0, size - 1, data);
	}

	sim->clock = clock;

	if (c80_set_cpu_model (sim, cpu)) {
		pce_log (MSG_ERR, "*** failed to set CPU model (%s)\n", cpu);
	}
}

static
void c80_setup_char (cpm80_t *sim, ini_sct_t *ini)
{
	const char *con, *aux, *lst;
	ini_sct_t  *sct;

	sim->con = NULL;
	sim->aux = NULL;
	sim->lst = NULL;

	sim->con_buf = 0;
	sim->con_buf_cnt = 0;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "con", &con, NULL);
	ini_get_string (sct, "aux", &aux, NULL);
	ini_get_string (sct, "lst", &lst, NULL);

	if (con != NULL) {
		pce_log_tag (MSG_INF, "CON:", "driver=%s\n", con);

		if ((sim->con = chr_open (con)) == NULL) {
			pce_log (MSG_ERR, "*** opening con failed\n");
		}
	}

	if (aux != NULL) {
		pce_log_tag (MSG_INF, "AUX:", "driver=%s\n", aux);

		if ((sim->aux = chr_open (aux)) == NULL) {
			pce_log (MSG_ERR, "*** opening aux failed\n");
		}
	}

	if (lst != NULL) {
		pce_log_tag (MSG_INF, "LST:", "driver=%s\n", lst);

		if ((sim->lst = chr_open (lst)) == NULL) {
			pce_log (MSG_ERR, "*** opening lst failed\n");
		}
	}
}

static
void c80_setup_disks (cpm80_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;
	disk_t    *dsk;

	sim->dsks = dsks_new();

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "disk")) != NULL) {
		if (ini_get_disk (sct, &dsk)) {
			pce_log (MSG_ERR, "*** loading drive failed\n");
			continue;
		}

		if (dsk == NULL) {
			continue;
		}

		dsks_add_disk (sim->dsks, dsk);
	}
}

cpm80_t *c80_new (ini_sct_t *ini)
{
	cpm80_t *sim;

	if ((sim = malloc (sizeof (cpm80_t))) == NULL) {
		return (NULL);
	}

	memset (sim, 0, sizeof (cpm80_t));

	bps_init (&sim->bps);

	c80_setup_system (sim, ini);
	c80_setup_mem (sim, ini);
	c80_setup_cpu (sim, ini);
	c80_setup_char (sim, ini);
	c80_setup_disks (sim, ini);
	pce_load_mem_ini (sim->mem, ini);

	if (sim->model == CPM80_MODEL_CPM) {
		c80_bios_init (sim);
	}

	return (sim);
}

static
void c80_del_char (cpm80_t *sim)
{
	if (sim->lst != NULL) {
		chr_close (sim->lst);
	}

	if (sim->aux != NULL) {
		chr_close (sim->aux);
	}

	if (sim->con != NULL) {
		chr_close (sim->con);
	}
}

void c80_del (cpm80_t *sim)
{
	if (sim == NULL) {
		return;
	}

	dsks_del (sim->dsks);
	c80_del_char (sim);
	e8080_del (sim->cpu);
	mem_del (sim->mem);
	bps_free (&sim->bps);

	free (sim);
}

void c80_stop (cpm80_t *sim)
{
	c80_set_msg (sim, "emu.stop", NULL);
}

void c80_reset (cpm80_t *sim)
{
	e8080_reset (sim->cpu);

	if (sim->model == CPM80_MODEL_CPM) {
		c80_bios_init (sim);
	}
}

int c80_set_cpu_model (cpm80_t *sim, const char *str)
{
	if (strcmp (str, "8080") == 0) {
		e8080_set_8080 (sim->cpu);
	}
	else if (strcmp (str, "Z80") == 0) {
		e8080_set_z80 (sim->cpu);
	}
	else {
		return (1);
	}

	return (0);
}

void c80_idle (cpm80_t *sim)
{
	pce_usleep (10000);

	c80_clock_discontinuity (sim);
}

void c80_clock_discontinuity (cpm80_t *sim)
{
	sim->sync_clk = 0;
	sim->sync_us = 0;
	sim->sync_sleep = 0;

	pce_get_interval_us (&sim->sync_us);
}

void c80_set_clock (cpm80_t *sim, unsigned long clock)
{
	sim->clock = clock;

	sim_log_deb ("set clock to %lu\n", clock);

	c80_clock_discontinuity (sim);
}

void c80_set_speed (cpm80_t *sim, unsigned speed)
{
	c80_set_clock (sim, 1000000UL * speed);
}

static
void c80_realtime_sync (cpm80_t *sim, unsigned long n)
{
	unsigned long fct;
	unsigned long us1, us2, sl;

	if (sim->clock == 0) {
		return;
	}

	sim->sync_clk += n;

	fct = sim->clock / CPM80_CPU_SYNC;

	if (sim->sync_clk < fct) {
		return;
	}

	sim->sync_clk -= fct;

	us1 = pce_get_interval_us (&sim->sync_us);
	us2 = (1000000 / CPM80_CPU_SYNC);

	if (us1 < us2) {
		sim->sync_sleep += us2 - us1;
	}
	else {
		sim->sync_sleep -= us1 - us2;
	}

	if (sim->sync_sleep >= (1000000 / CPM80_CPU_SYNC)) {
		pce_usleep (1000000 / CPM80_CPU_SYNC);
		sl = pce_get_interval_us (&sim->sync_us);
		sim->sync_sleep -= sl;
	}

	if (sim->sync_sleep < -1000000) {
		sim_log_deb ("system too slow, skipping 1 second\n");
		sim->sync_sleep += 1000000;
	}
}

void c80_clock (cpm80_t *sim, unsigned n)
{
	sim->clk_div += n;

	if (sim->clk_div >= 16384) {
		sim->clk_div -= 16384;

		c80_realtime_sync (sim, 16384);
	}

	e8080_clock (sim->cpu, n);
}
