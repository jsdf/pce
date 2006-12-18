/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim6502/sim6502.c                                 *
 * Created:       2004-05-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include "main.h"


void s6502_break (sim6502_t *sim, unsigned char val);


static
int s6502_load_ram (mem_blk_t *blk, const char *fname)
{
	FILE *fp;

	fp = fopen (fname, "rb");
	if (fp == NULL) {
		return (1);
	}

	fread (blk->data, 1, blk->size, fp);

	fclose (fp);

	return (0);
}

static
void s6502_setup_ram (sim6502_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *ram;
	const char    *fname;
	unsigned long base, size;

	sim->ram = NULL;

	sct = ini_sct_find_sct (ini, "ram");

	while (sct != NULL) {
		fname = ini_get_str (sct, "file");
		base = ini_get_lng_def (sct, "base", 0);
		size = ini_get_lng_def (sct, "size", 65536UL);

		pce_log (MSG_INF, "RAM:\tbase=0x%04lx size=%lu file=%s\n",
			base, size, (fname == NULL) ? "<none>" : fname
		);

		ram = mem_blk_new (base, size, 1);
		mem_blk_clear (ram, 0x00);
		mem_blk_set_readonly (ram, 0);
		mem_add_blk (sim->mem, ram, 1);

		if (base == 0) {
			sim->ram = ram;
		}

		if (fname != NULL) {
			if (s6502_load_ram (ram, fname)) {
				pce_log (MSG_ERR, "*** loading ram failed (%s)\n", fname);
			}
		}

		sct = ini_sct_find_next (sct, "ram");
	}
}

static
void s6502_setup_rom (sim6502_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *rom;
	const char    *fname;
	unsigned long base, size;

	sct = ini_sct_find_sct (ini, "rom");

	while (sct != NULL) {
		fname = ini_get_str_def (sct, "file", "default.rom");
		base = ini_get_lng_def (sct, "base", 0);
		size = ini_get_lng_def (sct, "size", 4096);

		pce_log (MSG_INF, "ROM:\tbase=0x%04lx size=%lu file=%s\n", base, size, fname);

		rom = mem_blk_new (base, size, 1);
		mem_blk_clear (rom, 0x00);
		mem_blk_set_readonly (rom, 1);
		mem_add_blk (sim->mem, rom, 1);

		if (s6502_load_ram (rom, fname)) {
			pce_log (MSG_ERR, "*** loading rom failed (%s)\n", fname);
		}

		sct = ini_sct_find_next (sct, "rom");
	}
}

static
void s6502_setup_cpu (sim6502_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;

	sct = ini_sct_find_sct (ini, "cpu");

	pce_log (MSG_INF, "CPU:\tmodel=6502\n");

	sim->cpu = e6502_new();

	e6502_set_mem_f (sim->cpu, sim->mem, &mem_get_uint8, &mem_set_uint8);

	if (sim->ram != NULL) {
		e6502_set_ram (sim->cpu, mem_blk_get_data (sim->ram), 0, mem_blk_get_size (sim->ram) - 1);
	}
}

static
void s6502_setup_console (sim6502_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;

	sct = ini_sct_find_sct (ini, "console");

	con_init (&sim->console, sct);

	mem_add_blk (sim->mem, sim->console.io, 0);

	sim->console.irq_ext = sim->cpu;
	sim->console.irq = (con_set_uint8_f) e6502_set_irq;

	sim->console.msg_ext = sim;
}

static
void s6502_load_mem (sim6502_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *fmt;
	const char    *fname;
	unsigned long addr;

	sct = ini_sct_find_sct (ini, "load");

	while (sct != NULL) {
		fmt = ini_get_str_def (sct, "format", "binary");
		fname = ini_get_str (sct, "file");
		addr = ini_get_lng_def (sct, "base", 0);

		if (fname != NULL) {
			pce_log (MSG_INF, "Load:\tformat=%s file=%s\n",
				fmt, (fname != NULL) ? fname : "<none>"
			);

			if (pce_load_mem (sim->mem, fname, fmt, addr)) {
				pce_log (MSG_ERR, "*** loading failed (%s)\n", fname);
			}
		}

		sct = ini_sct_find_next (sct, "load");
	}
}

sim6502_t *s6502_new (ini_sct_t *ini)
{
	sim6502_t *sim;

	sim = (sim6502_t *) malloc (sizeof (sim6502_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->brk = 0;
	sim->clk_cnt = 0;
	sim->clk_div = 0;

	bps_init (&sim->bps);

	sim->mem = mem_new();

	s6502_setup_ram (sim, ini);
	s6502_setup_rom (sim, ini);
	s6502_setup_cpu (sim, ini);
	s6502_setup_console (sim, ini);

	s6502_load_mem (sim, ini);

	return (sim);
}

void s6502_del (sim6502_t *sim)
{
	if (sim == NULL) {
		return;
	}

	con_free (&sim->console);

	e6502_del (sim->cpu);

	mem_del (sim->mem);

	bps_free (&sim->bps);

	free (sim);
}

unsigned long long s6502_get_clkcnt (sim6502_t *sim)
{
	return (sim->clk_cnt);
}

void s6502_break (sim6502_t *sim, unsigned char val)
{
	if ((val == PCE_BRK_STOP) || (val == PCE_BRK_ABORT)) {
		sim->brk = val;
	}
}

void s6502_reset (sim6502_t *sim)
{
	e6502_reset (sim->cpu);
}

void s6502_clock (sim6502_t *sim, unsigned n)
{
	sim->clk_div += n;

	if (sim->clk_div >= 4096) {
		sim->clk_div -= 4096;
		con_check (&sim->console);
	}

	e6502_clock (sim->cpu, n);
}

void s6502_set_msg (sim6502_t *sim, const char *msg, const char *val)
{
	if (strcmp (msg, "break") == 0) {
		if (strcmp (val, "stop") == 0) {
			sim->brk = PCE_BRK_STOP;
			return;
		}
		else if (strcmp (val, "abort") == 0) {
			sim->brk = PCE_BRK_ABORT;
			return;
		}
	}

	pce_log (MSG_DEB, "msg (\"%s\", \"%s\")\n", msg, val);

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);
}
