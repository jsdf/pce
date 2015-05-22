/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/int10.c                                         *
 * Created:     2013-01-02 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "int10.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * 01: Set cursor size
 */
static
int int10_fct_01 (dos_t *sim)
{
	return (0);
}

/*
 * 0F: Get video mode
 */
static
int int10_fct_0f (dos_t *sim)
{
	e86_set_ah (&sim->cpu, 80);
	e86_set_al (&sim->cpu, 3);
	e86_set_bh (&sim->cpu, 0);

	return (0);
}

int sim_int10 (dos_t *sim)
{
	switch (e86_get_ah (&sim->cpu)) {
	case 0x01:
		return (int10_fct_01 (sim));

	case 0x08:
		e86_set_ax (&sim->cpu, 0x0720);
		return (0);

	case 0x0f:
		return (int10_fct_0f (sim));

	case 0x11:
	case 0x12:
	case 0x1a:
	case 0xfe:
		e86_set_cf (&sim->cpu, 1);
		return (0);
	}

	return (1);
}
