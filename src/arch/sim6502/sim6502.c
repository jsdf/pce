/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim6502/sim6502.c                                   *
 * Created:     2004-05-25 by Hampa Hug <hampa@hampa.ch>                     *
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


void s6502_break (sim6502_t *sim, unsigned char val);


static
void s6502_setup_cpu (sim6502_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "CPU:", "model=6502\n");

	sim->cpu = e6502_new();

	e6502_set_mem_f (sim->cpu, sim->mem, &mem_get_uint8, &mem_set_uint8);
}

static
void s6502_setup_console (sim6502_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "console");

	con_init (&sim->console, sct);

	mem_add_blk (sim->mem, sim->console.io, 0);

	sim->console.irq_ext = sim->cpu;
	sim->console.irq = (con_set_uint8_f) e6502_set_irq;

	sim->console.msg_ext = sim;
}

sim6502_t *s6502_new (ini_sct_t *ini)
{
	sim6502_t *sim;

	sim = malloc (sizeof (sim6502_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->brk = 0;
	sim->clk_cnt = 0;
	sim->clk_div = 0;

	bps_init (&sim->bps);

	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	s6502_setup_cpu (sim, ini);
	s6502_setup_console (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

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
