/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sims32/sims32.c                                     *
 * Created:     2004-09-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


void ss32_break (sims32_t *sim, unsigned char val);


static
void ss32_setup_cpu (sims32_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *model;
	unsigned   nwindows;

	sct = ini_next_sct (ini, NULL, "sparc32");

	ini_get_string (sct, "model", &model, "sparc32");
	ini_get_uint16 (sct, "nwindows", &nwindows, 4);

	pce_log_tag (MSG_INF, "CPU:", "model=%s nwindows=%u\n",
		model, nwindows
	);

	sim->cpu = s32_new();
	if (sim->cpu == NULL) {
		return;
	}

	s32_set_nwindows (sim->cpu, nwindows);

	s32_set_mem_fct (sim->cpu, sim->mem,
		&mem_get_uint8,
		&mem_get_uint16_be,
		&mem_get_uint32_be,
		&mem_set_uint8,
		&mem_set_uint16_be,
		&mem_set_uint32_be
	);
}

static
void ss32_setup_serport (sims32_t *sim, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long addr;
	unsigned      irq;
	const char    *driver;
	const char    *chip;
	ini_sct_t     *sct;

	static unsigned long defbase[4] = { 0xef600300, 0xef600400 };
	static unsigned      defirq[4] = { 0, 1 };


	sim->serport[0] = NULL;
	sim->serport[1] = NULL;

	i = 0;
	sct = 0;
	while ((sct = ini_next_sct (ini, sct, "serial")) != NULL) {
		if (i >= 2) {
			break;
		}

		if (ini_get_uint32 (sct, "address", &addr, defbase[i])) {
			ini_get_uint32 (sct, "io", &addr, defbase[i]);
		}
		ini_get_uint16 (sct, "irq", &irq, defirq[i]);
		ini_get_string (sct, "uart", &chip, "8250");
		ini_get_string (sct, "driver", &driver, NULL);

		pce_log_tag (MSG_INF, "UART:",
			"n=%u addr=0x%08lx irq=%u uart=%s driver=%s\n",
			i, addr, irq, chip, (driver == NULL) ? "<none>" : driver
		);

		sim->serport[i] = ser_new (addr, 0);
		if (sim->serport[i] == NULL) {
			pce_log (MSG_ERR,
				"*** serial port setup failed [%08lX/%u -> %s]\n",
				addr, irq, (driver == NULL) ? "<none>" : driver
			);
		}
		else {
			if (driver != NULL) {
				if (ser_set_driver (sim->serport[i], driver)) {
					pce_log (MSG_ERR,
						"*** can't open driver (%s)\n",
						driver
					);
				}
			}

			if (e8250_set_chip_str (&sim->serport[i]->uart, chip)) {
				pce_log (MSG_ERR, "*** unknown UART chip (%s)\n", chip);
			}

			mem_add_blk (sim->mem, ser_get_reg (sim->serport[i]), 0);

			i += 1;
		}
	}
}

sims32_t *ss32_new (ini_sct_t *ini)
{
	unsigned i;
	sims32_t *sim;

	sim = malloc (sizeof (sims32_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->brk = 0;
	sim->clk_cnt = 0;

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	bps_init (&sim->bps);

	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	ss32_setup_cpu (sim, ini);
	ss32_setup_serport (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

	return (sim);
}

void ss32_del (sims32_t *sim)
{
	if (sim == NULL) {
		return;
	}

	ser_del (sim->serport[1]);
	ser_del (sim->serport[0]);

	s32_del (sim->cpu);

	mem_del (sim->mem);

	bps_free (&sim->bps);

	free (sim);
}

unsigned long long ss32_get_clkcnt (sims32_t *sim)
{
	return (sim->clk_cnt);
}

void ss32_break (sims32_t *sim, unsigned char val)
{
	if ((val == PCE_BRK_STOP) || (val == PCE_BRK_ABORT)) {
		sim->brk = val;
	}
}

void ss32_set_keycode (sims32_t *sim, unsigned char val)
{
	ser_receive (sim->serport[1], val);
}

void ss32_reset (sims32_t *sim)
{
	s32_reset (sim->cpu);
}

void ss32_clock (sims32_t *sim, unsigned n)
{
	if (sim->clk_div[0] >= 1024) {
		scon_check (sim);

		sim->clk_div[0] &= 1023;
	}

	s32_clock (sim->cpu, n);

	sim->clk_cnt += n;
	sim->clk_div[0] += n;
	/* sim->clk_div[1] += n; */
	/* sim->clk_div[2] += n; */
	/* sim->clk_div[3] += n; */
}

void ss32_set_msg (sims32_t *sim, const char *msg, const char *val)
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
