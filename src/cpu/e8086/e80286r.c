/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e8086/e80286r.c                                    *
 * Created:       2003-10-07 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#include "e8086.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


/* OP 0F 01 00: SGDT mem */
static
unsigned op_0f_01_00 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 2);

	if (!c->ea.is_mem) {
		return (e86_undefined (c));
	}

	e86_set_ea16 (c, 0x0000);
	e86_set_clk_ea (c, 11, 11);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 01: SIDT mem */
static
unsigned op_0f_01_01 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 2);

	if (!c->ea.is_mem) {
		return (e86_undefined (c));
	}

	e86_set_ea16 (c, 0x0000);
	e86_set_clk_ea (c, 11, 11);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 04: SMSW r/m16 */
static
unsigned op_0f_01_04 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 2);
	e86_set_ea16 (c, 0x0000);

	e86_set_clk_ea (c, 2, 3);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 06: LMSW r/m16 */
static
unsigned op_0f_01_06 (e8086_t *c)
{
	unsigned short val;

	e86_get_ea_ptr (c, c->pq + 2);
	val = e86_get_ea16 (c);

	e86_set_clk_ea (c, 3, 6);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 */
static
unsigned op_0f_01 (e8086_t *c)
{
	switch ((c->pq[2] >> 3) & 0x07) {
		case 0x00:
			return (op_0f_01_00 (c));

		case 0x01:
			return (op_0f_01_01 (c));

		case 0x04:
			return (op_0f_01_04 (c));

		case 0x06:
			return (op_0f_01_06 (c));
	}

	return (e86_undefined (c));
}

/* OP 0F */
static
unsigned op_0f (e8086_t *c)
{
	if (c->pq[1] == 0x01) {
		return (op_0f_01 (c));
	}

	return (e86_undefined (c));
}

void e86_enable_286 (e8086_t *c)
{
	e86_enable_186 (c);

	c->cpu |= (E86_CPU_INT6 | E86_CPU_FLAGS286);

	c->op[0x0f] = &op_0f;
}
