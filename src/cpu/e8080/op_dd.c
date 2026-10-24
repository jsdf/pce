/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/op_dd.c                                        *
 * Created:     2012-12-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "e8080.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


#define op_dd_cb z80_op_dd_cb


static void op_dd_ud (e8080_t *c)
{
	if (e8080_hook_undefined (c)) {
		return;
	}

	e8080_set_clk (c, 1, 4);
}

static void op_noni (e8080_t *c)
{
	e8080_set_clk (c, 1, 4);
}

/* OP DD 09: ADD IX, BC */
static void op_dd_09 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_ix (c);
	s2 = e8080_get_bc (c);
	d = s1 + s2;
	e8080_set_ix (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP DD 19: ADD IX, DE */
static void op_dd_19 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_ix (c);
	s2 = e8080_get_de (c);
	d = s1 + s2;
	e8080_set_ix (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP DD 21: LD IX, nnnn */
static void op_dd_21 (e8080_t *c)
{
	e8080_get_inst23 (c);
	e8080_set_ix (c, e8080_uint16 (c->inst[2], c->inst[3]));
	e8080_set_clk (c, 4, 14);
}

/* OP DD 22: LD (nnnn), IX */
static void op_dd_22 (e8080_t *c)
{
	unsigned adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_mem16 (c, adr, e8080_get_ix (c));
	e8080_set_clk (c, 4, 20);
}

/* OP DD 23: INC IX */
static void op_dd_23 (e8080_t *c)
{
	e8080_set_ix (c, e8080_get_ix (c) + 1);
	e8080_set_clk (c, 2, 10);
}

/* OP DD 24: INC IXH */
static void op_dd_24 (e8080_t *c)
{
	unsigned s;

	s = e8080_get_ixh (c);
	z80_set_psw_inc (c, s & 0xff);
	e8080_set_ixh (c, s + 1);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 25: DEC IXH */
static void op_dd_25 (e8080_t *c)
{
	unsigned s;

	s = e8080_get_ixh (c);
	z80_set_psw_dec (c, s & 0xff);
	e8080_set_ixh (c, s - 1);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 26: LD IXH, nn */
static void op_dd_26 (e8080_t *c)
{
	e8080_get_inst2 (c);
	e8080_set_ixh (c, c->inst[2]);
	e8080_set_clk (c, 3, 11);
}

/* OP DD 29: ADD IX, IX */
static void op_dd_29 (e8080_t *c)
{
	unsigned long s, d;

	s = e8080_get_ix (c);
	d = s + s;
	e8080_set_ix (c, d);
	z80_set_psw_add16 (c, d, s, s);
	e8080_set_clk (c, 2, 15);
}

/* OP DD 2A: LD IX, (nnnn) */
static void op_dd_2a (e8080_t *c)
{
	unsigned adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_ix (c, e8080_get_mem16 (c, adr));
	e8080_set_clk (c, 4, 20);
}

/* OP DD 2B: DEC IX */
static void op_dd_2b (e8080_t *c)
{
	e8080_set_ix (c, e8080_get_ix (c) - 1);
	e8080_set_clk (c, 2, 10);
}

/* OP DD 2C: INC IXL */
static void op_dd_2c (e8080_t *c)
{
	unsigned s;

	s = e8080_get_ixl (c);
	z80_set_psw_inc (c, s & 0xff);
	e8080_set_ixl (c, s + 1);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 2D: DEC IXL */
static void op_dd_2d (e8080_t *c)
{
	unsigned s;

	s = e8080_get_ixl (c);
	z80_set_psw_dec (c, s & 0xff);
	e8080_set_ixl (c, s - 1);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 2E: LD IXL, nn */
static void op_dd_2e (e8080_t *c)
{
	e8080_get_inst2 (c);
	e8080_set_ixl (c, c->inst[2]);
	e8080_set_clk (c, 3, 11);
}

/* OP DD 34: INC (IX+d) */
static void op_dd_34 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	e8080_set_mem8 (c, p, s + 1);
	z80_set_psw_inc (c, s);
	e8080_set_clk (c, 3, 23);
}

/* OP DD 35: DEC (IX+d) */
static void op_dd_35 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	e8080_set_mem8 (c, p, s - 1);
	z80_set_psw_dec (c, s);
	e8080_set_clk (c, 3, 23);
}

/* OP DD 36: LD (IX+d), nn */
static void op_dd_36 (e8080_t *c)
{
	unsigned short p;

	e8080_get_inst23 (c);
	p = e8080_get_ixd (c, 2);
	e8080_set_mem8 (c, p, c->inst[3]);
	e8080_set_clk (c, 4, 19);
}

/* OP DD 39: ADD IX, SP */
static void op_dd_39 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_ix (c);
	s2 = e8080_get_sp (c);
	d = s1 + s2;
	e8080_set_ix (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP DD 44: LD r, IXH */
static void op_dd_44 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[1] >> 3, e8080_get_ixh (c));
	e8080_set_clk (c, 2, 8);
}

/* OP DD 45: LD r, IXL */
static void op_dd_45 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[1] >> 3, e8080_get_ixl (c));
	e8080_set_clk (c, 2, 8);
}

/* OP DD 46: LD r, (IX+d) */
static void op_dd_46 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s = e8080_get_mem8 (c, p);
	e8080_set_reg8 (c, c->inst[1] >> 3, s);
	e8080_set_clk (c, 3, 19);
}

/* OP DD 60: LD IXH, r */
static void op_dd_60 (e8080_t *c)
{
	e8080_set_ixh (c, e8080_get_reg8 (c, c->inst[1]));
	e8080_set_clk (c, 2, 8);
}

/* OP DD 64: LD IXH, IXH */
static void op_dd_64 (e8080_t *c)
{
	e8080_set_clk (c, 2, 8);
}

/* OP DD 65: LD IXH, IXL */
static void op_dd_65 (e8080_t *c)
{
	e8080_set_ixh (c, e8080_get_ixl (c));
	e8080_set_clk (c, 2, 8);
}

/* OP DD 68: LD IXL, r */
static void op_dd_68 (e8080_t *c)
{
	e8080_set_ixl (c, e8080_get_reg8 (c, c->inst[1]));
	e8080_set_clk (c, 2, 8);
}

/* OP DD 6C: LD IXL, IXH */
static void op_dd_6c (e8080_t *c)
{
	e8080_set_ixl (c, e8080_get_ixh (c));
	e8080_set_clk (c, 2, 8);
}

/* OP DD 6D: LD IXL, IXL */
static void op_dd_6d (e8080_t *c)
{
	e8080_set_clk (c, 2, 8);
}

/* OP DD 70: LD (IX+d), r */
static void op_dd_70 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s = e8080_get_reg8 (c, c->inst[1]);
	e8080_set_mem8 (c, p, s);
	e8080_set_clk (c, 3, 19);
}

/* OP DD 84: ADD A, IXH */
static void op_dd_84 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixh (c);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 85: ADD A, IXL */
static void op_dd_85 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixl (c);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 86: ADD A, (IX+d) */
static void op_dd_86 (e8080_t *c)
{
	unsigned short d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP DD 8C: ADC A, IXH */
static void op_dd_8c (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixh (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 8D: ADC A, IXL */
static void op_dd_8d (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixl (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 8E: ADC A, (IX+d) */
static void op_dd_8e (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP DD 94: SUB A, IXH */
static void op_dd_94 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixh (c);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 95: SUB A, IXL */
static void op_dd_95 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixl (c);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 96: SUB A, (IX+d) */
static void op_dd_96 (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP DD 9C: SBC A, IXH */
static void op_dd_9c (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixh (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 9D: SBC A, IXL */
static void op_dd_9d (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixl (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD 9E: SBC A, (IX+d) */
static void op_dd_9e (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP DD A4: AND A, IXH */
static void op_dd_a4 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) & e8080_get_ixh (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP DD A5: AND A, IXL */
static void op_dd_a5 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) & e8080_get_ixl (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP DD A6: AND A, (IX+d) */
static void op_dd_a6 (e8080_t *c)
{
	unsigned short adr;
	unsigned char  d;

	e8080_get_inst2 (c);
	adr = e8080_get_ixd (c, 2);
	d = e8080_get_a (c) & e8080_get_mem8 (c, adr);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 3, 19);
}

/* OP DD AC: XOR A, IXH */
static void op_dd_ac (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) ^ e8080_get_ixh (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP DD AD: XOR A, IXL */
static void op_dd_ad (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) ^ e8080_get_ixl (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP DD AE: XOR A, (IX+d) */
static void op_dd_ae (e8080_t *c)
{
	unsigned short adr;
	unsigned char  d;

	e8080_get_inst2 (c);
	adr = e8080_get_ixd (c, 2);
	d = e8080_get_a (c) ^ e8080_get_mem8 (c, adr);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 3, 19);
}

/* OP DD B4: OR A, IXH */
static void op_dd_b4 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) | e8080_get_ixh (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP DD B5: OR A, IXL */
static void op_dd_b5 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) | e8080_get_ixl (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP DD B6: OR A, (IX+d) */
static void op_dd_b6 (e8080_t *c)
{
	unsigned short adr;
	unsigned char  d;

	e8080_get_inst2 (c);
	adr = e8080_get_ixd (c, 2);
	d = e8080_get_a (c) | e8080_get_mem8 (c, adr);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 3, 19);
}

/* OP DD BC: CP A, IXH */
static void op_dd_bc (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixh (c);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD BD: CP A, IXL */
static void op_dd_bd (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_ixl (c);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP DD BE: CP A, (IX+d) */
static void op_dd_be (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_ixd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP DD E1: POP IX */
static void op_dd_e1 (e8080_t *c)
{
	e8080_set_ix (c, e8080_get_mem16 (c, e8080_get_sp (c)));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 2, 14);
}

/* OP DD E3: EX (SP), IX */
static void op_dd_e3 (e8080_t *c)
{
	unsigned short sp, v;

	sp = e8080_get_sp (c);

	v = e8080_get_mem16 (c, sp);
	e8080_set_mem16 (c, sp, e8080_get_ix (c));
	e8080_set_ix (c, v);

	e8080_set_clk (c, 2, 23);
}

/* OP DD E5: PUSH IX */
static void op_dd_e5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_ix (c));
	e8080_set_clk (c, 2, 15);
}

/* OP DD E9: JMP (IX) */
static void op_dd_e9 (e8080_t *c)
{
	e8080_set_pc (c, e8080_get_ix (c));
	e8080_set_clk (c, 0, 8);
}

/* OP DD F9: LD SP, IX */
static void op_dd_f9 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_ix (c));
	e8080_set_clk (c, 2, 10);
}

static e8080_opcode_f opcodes_dd[256] = {
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, /* 00 */
	op_dd_ud, op_dd_09, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, /* 10 */
	op_dd_ud, op_dd_19, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud,
	op_dd_ud, op_dd_21, op_dd_22, op_dd_23, op_dd_24, op_dd_25, op_dd_26, op_dd_ud, /* 20 */
	op_dd_ud, op_dd_29, op_dd_2a, op_dd_2b, op_dd_2c, op_dd_2d, op_dd_2e, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_34, op_dd_35, op_dd_36, op_dd_ud, /* 30 */
	op_dd_ud, op_dd_39, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_44, op_dd_45, op_dd_46, op_dd_ud, /* 40 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_44, op_dd_45, op_dd_46, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_44, op_dd_45, op_dd_46, op_dd_ud, /* 50 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_44, op_dd_45, op_dd_46, op_dd_ud,
	op_dd_60, op_dd_60, op_dd_60, op_dd_60, op_dd_64, op_dd_65, op_dd_46, op_dd_60, /* 60 */
	op_dd_68, op_dd_68, op_dd_68, op_dd_68, op_dd_6c, op_dd_6d, op_dd_46, op_dd_68,
	op_dd_70, op_dd_70, op_dd_70, op_dd_70, op_dd_70, op_dd_70, op_dd_ud, op_dd_70, /* 70 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_44, op_dd_45, op_dd_46, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_84, op_dd_85, op_dd_86, op_dd_ud, /* 80 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_8c, op_dd_8d, op_dd_8e, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_94, op_dd_95, op_dd_96, op_dd_ud, /* 90 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_9c, op_dd_9d, op_dd_9e, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_a4, op_dd_a5, op_dd_a6, op_dd_ud, /* A0 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ac, op_dd_ad, op_dd_ae, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_b4, op_dd_b5, op_dd_b6, op_dd_ud, /* B0 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_bc, op_dd_bd, op_dd_be, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, /* C0 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_cb, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, /* D0 */
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_noni,  op_dd_ud, op_dd_ud,
	op_dd_ud, op_dd_e1, op_dd_ud, op_dd_e3, op_dd_ud, op_dd_e5, op_dd_ud, op_dd_ud, /* E0 */
	op_dd_ud, op_dd_e9, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud,
	op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, op_dd_ud, /* F0 */
	op_dd_ud, op_dd_f9, op_dd_ud, op_dd_ud, op_dd_ud, op_noni,  op_dd_ud, op_dd_ud
};

void z80_op_dd (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_inc_r (c);
	opcodes_dd[c->inst[1]] (c);
}
