/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/int.c                                           *
 * Created:     2013-01-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include "dos.h"
#include "int.h"
#include "int10.h"
#include "int21.h"

#include <cpu/e8086/e8086.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Int 11
 */
static
int sim_int11 (dos_t *sim)
{
	e86_set_ax (&sim->cpu, 0x0061);

	return (0);
}

/*
 * Int 12
 */
static
int sim_int12 (dos_t *sim)
{
	e86_set_ax (&sim->cpu, sim_get_uint16 (sim, 0x40, 0x13));

	return (0);
}


/*
 * Int 16
 */
static
int sim_int16 (dos_t *sim)
{
	switch (e86_get_ah (&sim->cpu)) {
	case 0x01:
	case 0x11:
		e86_set_zf (&sim->cpu, 1);
		e86_set_ax (&sim->cpu, 0);
		break;

	case 0x02:
	case 0x12:
		e86_set_al (&sim->cpu, 0);
		break;

	case 5:
		break;

	default:
		return (1);
	}

	return (0);
}


/*
 * 1A / 00: Get time
 */
static
int int1a_fct_00 (dos_t *sim)
{
	e86_set_cx (&sim->cpu, 0);
	e86_set_dx (&sim->cpu, 0);

	return (0);
}

/*
 * Int 1A
 */
static
int sim_int1a (dos_t *sim)
{
	switch (e86_get_ah (&sim->cpu)) {
	case 0x00:
		return (int1a_fct_00 (sim));
	}

	return (1);
}


void sim_int (dos_t *sim, unsigned char val)
{
	int ret;

	if (sim->log_int) {
		fprintf (stderr, "%02X: ", val);
		sim_print_state_cpu (sim, stderr);
	}

	ret = 0;

	if ((val < 0x10) || (val > 0x28)) {
		ret = 0;
	}
	else if (val == 0x10) {
		ret = sim_int10 (sim);
	}
	else if (val == 0x11) {
		ret = sim_int11 (sim);
	}
	else if (val == 0x12) {
		ret = sim_int12 (sim);
	}
	else if (val == 0x16) {
		ret = sim_int16 (sim);
	}
	else if (val == 0x1a) {
		ret = sim_int1a (sim);
	}
	else if (val == 0x21) {
		ret = sim_int21 (sim);
	}
	else if (val == 0x28) {
		;
	}
	else {
		ret = 1;
	}

	if (sim->log_int) {
		fprintf (stderr, "%02X: ", val);
		sim_print_state_cpu (sim, stderr);
		fputc ('\n', stderr);
		fflush (stderr);
	}

	if (ret) {
		sim_print_state_cpu (sim, stderr);
		fprintf (stderr, "unknown int: %02X / %04X\n", val, e86_get_ax (&sim->cpu));
		exit (1);
	}
}
