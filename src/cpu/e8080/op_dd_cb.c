/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/op_dd_cb.c                                     *
 * Created:     2012-12-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2014 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e8080.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


static void op_dd_cb_ud (e8080_t *c)
{
	if (e8080_hook_undefined (c)) {
		return;
	}

	e8080_set_clk (c, 1, 4);
}

/* OP DD CB xx 06: RLC (IX+d) */
static void op_dd_cb_06 (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = (s << 1) | (s >> 7);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 0E: RRC (IX+d) */
static void op_dd_cb_0e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = (s >> 1) | (s << 7);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 16: RL (IX+d) */
static void op_dd_cb_16 (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = (s << 1) | e8080_get_cf (c);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 1E: RR (IX+d) */
static void op_dd_cb_1e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = (s >> 1) | (e8080_get_cf (c) << 7);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 26: SLA (IX+d) */
static void op_dd_cb_26 (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = s << 1;
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 2E: SRA (IX+d) */
static void op_dd_cb_2e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = (s >> 1) | (s & 0x80);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 3E: SRL (IX+d) */
static void op_dd_cb_3e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = s >> 1;
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx 46: BIT b, (IX+d) */
static void op_dd_cb_46 (e8080_t *c)
{
	unsigned      p, b;
	unsigned char s, d;

	b = (c->inst[3] >> 3) & 7;
	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = s & (1 << b);
	e8080_set_psw_szp (c, d, E8080_FLG_A | E8080_FLG_3 | E8080_FLG_5, E8080_FLG_N);
	e8080_set_clk (c, 4, 20);
}

/* OP DD CB xx 86: RES b, (IX+d) */
static void op_dd_cb_86 (e8080_t *c)
{
	unsigned      p, b;
	unsigned char s, d;

	b = (c->inst[3] >> 3) & 7;
	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = s & ~(1 << b);
	e8080_set_mem8 (c, p, d);
	e8080_set_clk (c, 4, 23);
}

/* OP DD CB xx C6: SET b, (IX+d) */
static void op_dd_cb_c6 (e8080_t *c)
{
	unsigned      p, b;
	unsigned char s, d;

	b = (c->inst[3] >> 3) & 7;
	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	d = s | (1 << b);
	e8080_set_mem8 (c, p, d);
	e8080_set_clk (c, 4, 23);
}


static e8080_opcode_f opcodes_dd_cb[256] = {
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_06, op_dd_cb_ud, /* 00 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_0e, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_16, op_dd_cb_ud, /* 10 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_1e, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_26, op_dd_cb_ud, /* 20 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_2e, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, /* 30 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_3e, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud, /* 40 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud, /* 50 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud, /* 60 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud, /* 70 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_46, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud, /* 80 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud, /* 90 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud, /* A0 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud, /* B0 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_86, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud, /* C0 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud, /* D0 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud, /* E0 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud,
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud, /* F0 */
	op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_ud, op_dd_cb_c6, op_dd_cb_ud
};

void z80_op_dd_cb (e8080_t *c)
{
	e8080_get_inst23 (c);
	opcodes_dd_cb[c->inst[3]] (c);
}
