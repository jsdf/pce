/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/op_cb.c                                        *
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


static void op_cb_ud (e8080_t *c)
{
	if (e8080_hook_undefined (c)) {
		return;
	}

	e8080_set_clk (c, 1, 4);
}

/* OP CB 00: RLC r */
static void op_cb_00 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = (s << 1) | (s >> 7);
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 06: RLC (HL) */
static void op_cb_06 (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = (s << 1) | (s >> 7);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 08: RRC r */
static void op_cb_08 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = (s >> 1) | (s << 7);
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 0E: RRC (HL) */
static void op_cb_0e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = (s >> 1) | (s << 7);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 10: RL r */
static void op_cb_10 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = (s << 1) | e8080_get_cf (c);
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 16: RL (HL) */
static void op_cb_16 (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = (s << 1) | e8080_get_cf (c);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 18: RR r */
static void op_cb_18 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = (s >> 1) | (e8080_get_cf (c) << 7);
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 1E: RR (HL) */
static void op_cb_1e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = (s >> 1) | (e8080_get_cf (c) << 7);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 20: SLA r */
static void op_cb_20 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = s << 1;
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 26: SLA (HL) */
static void op_cb_26 (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = s << 1;
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 0x80);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 28: SRA r */
static void op_cb_28 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = (s >> 1) | (s & 0x80);
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 2E: SRA (HL) */
static void op_cb_2e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = (s >> 1) | (s & 0x80);
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 38: SRL r */
static void op_cb_38 (e8080_t *c)
{
	unsigned      r;
	unsigned char s, d;

	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = s >> 1;
	e8080_set_reg8 (c, r, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 3E: SRL (HL) */
static void op_cb_3e (e8080_t *c)
{
	unsigned      p;
	unsigned char s, d;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = s >> 1;
	e8080_set_mem8 (c, p, d);
	z80_set_psw_rot (c, d, s & 1);
	e8080_set_clk (c, 2, 15);
}

/* OP CB 40: BIT b, r */
static void op_cb_40 (e8080_t *c)
{
	unsigned      r, b;
	unsigned char s, d;

	b = (c->inst[1] >> 3) & 7;
	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = s & (1 << b);
	e8080_set_psw_szp (c, d, E8080_FLG_A | E8080_FLG_3 | E8080_FLG_5, E8080_FLG_N);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 46: BIT b, (HL) */
static void op_cb_46 (e8080_t *c)
{
	unsigned      p, b;
	unsigned char s, d;

	b = (c->inst[1] >> 3) & 7;
	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = s & (1 << b);
	e8080_set_psw_szp (c, d, E8080_FLG_A | E8080_FLG_3 | E8080_FLG_5, E8080_FLG_N);
	e8080_set_clk (c, 2, 12);
}

/* OP CB 80: RES b, r */
static void op_cb_80 (e8080_t *c)
{
	unsigned      r, b;
	unsigned char s, d;

	b = (c->inst[1] >> 3) & 7;
	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = s & ~(1 << b);
	e8080_set_reg8 (c, r, d);
	e8080_set_clk (c, 2, 8);
}

/* OP CB 86: RES b, (HL) */
static void op_cb_86 (e8080_t *c)
{
	unsigned      p, b;
	unsigned char s, d;

	b = (c->inst[1] >> 3) & 7;
	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = s & ~(1 << b);
	e8080_set_mem8 (c, p, d);
	e8080_set_clk (c, 2, 15);
}

/* OP CB C0: SET b, r */
static void op_cb_c0 (e8080_t *c)
{
	unsigned      r, b;
	unsigned char s, d;

	b = (c->inst[1] >> 3) & 7;
	r = c->inst[1] & 7;
	s = e8080_get_reg8 (c, r);
	d = s | (1 << b);
	e8080_set_reg8 (c, r, d);
	e8080_set_clk (c, 2, 8);
}

/* OP CB C6: SET b, (HL) */
static void op_cb_c6 (e8080_t *c)
{
	unsigned      p, b;
	unsigned char s, d;

	b = (c->inst[1] >> 3) & 7;
	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	d = s | (1 << b);
	e8080_set_mem8 (c, p, d);
	e8080_set_clk (c, 2, 15);
}


static e8080_opcode_f opcodes_cb[256] = {
	op_cb_00, op_cb_00, op_cb_00, op_cb_00, op_cb_00, op_cb_00, op_cb_06, op_cb_00, /* 00 */
	op_cb_08, op_cb_08, op_cb_08, op_cb_08, op_cb_08, op_cb_08, op_cb_0e, op_cb_08,
	op_cb_10, op_cb_10, op_cb_10, op_cb_10, op_cb_10, op_cb_10, op_cb_16, op_cb_10, /* 10 */
	op_cb_18, op_cb_18, op_cb_18, op_cb_18, op_cb_18, op_cb_18, op_cb_1e, op_cb_18,
	op_cb_20, op_cb_20, op_cb_20, op_cb_20, op_cb_20, op_cb_20, op_cb_26, op_cb_20, /* 20 */
	op_cb_28, op_cb_28, op_cb_28, op_cb_28, op_cb_28, op_cb_28, op_cb_2e, op_cb_28,
	op_cb_ud, op_cb_ud, op_cb_ud, op_cb_ud, op_cb_ud, op_cb_ud, op_cb_ud, op_cb_ud, /* 30 */
	op_cb_38, op_cb_38, op_cb_38, op_cb_38, op_cb_38, op_cb_38, op_cb_3e, op_cb_38,
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40, /* 40 */
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40,
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40, /* 50 */
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40,
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40, /* 60 */
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40,
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40, /* 70 */
	op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_40, op_cb_46, op_cb_40,
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80, /* 80 */
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80,
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80, /* 90 */
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80,
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80, /* A0 */
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80,
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80, /* B0 */
	op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_80, op_cb_86, op_cb_80,
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0, /* C0 */
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0,
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0, /* D0 */
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0,
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0, /* E0 */
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0,
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0, /* F0 */
	op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c0, op_cb_c6, op_cb_c0
};

void z80_op_cb (e8080_t *c)
{
	e8080_get_inst1 (c);
	opcodes_cb[c->inst[1]] (c);
}
