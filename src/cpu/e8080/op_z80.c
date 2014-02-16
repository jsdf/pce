/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/op_z80.c                                       *
 * Created:     2012-12-07 by Hampa Hug <hampa@hampa.ch>                     *
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


/* OP 03: INC BC */
static void op_03 (e8080_t *c)
{
	e8080_set_bc (c, e8080_get_bc (c) + 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 04: INC B */
static void op_04 (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_b (c));
	e8080_set_b (c, e8080_get_b (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 05: DEC B */
static void op_05 (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_b (c));
	e8080_set_b (c, e8080_get_b (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 07: RLCA */
static void op_07 (e8080_t *c)
{
	unsigned char d, s;

	s = e8080_get_a (c);
	d = (s << 1) | (s >> 7);
	e8080_set_a (c, d);
	e8080_set_af (c, 0);
	e8080_set_nf (c, 0);
	e8080_set_cf (c, s & 0x80);
	e8080_set_clk (c, 1, 4);
}

/* OP 08: EX AF, AF' */
static void op_08 (e8080_t *c)
{
	unsigned char t;

	t = c->psw;
	c->psw = c->psw2;
	c->psw2 = t;

	t = c->reg[7];
	c->reg[7] = c->reg2[7];
	c->reg2[7] = t;

	e8080_set_clk (c, 1, 4);
}

/* OP 09: ADD HL, BC */
static void op_09 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_bc (c);
	d = s1 + s2;
	e8080_set_hl (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 1, 11);
}

/* OP 0B: DEC BC */
static void op_0b (e8080_t *c)
{
	e8080_set_bc (c, e8080_get_bc (c) - 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 0C: INC C */
static void op_0c (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_c (c));
	e8080_set_c (c, e8080_get_c (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 0D: DEC C */
static void op_0d (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_c (c));
	e8080_set_c (c, e8080_get_c (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 0F: RRCA */
static void op_0f (e8080_t *c)
{
	unsigned char d, s;

	s = e8080_get_a (c);
	d = (s >> 1) | (s << 7);
	e8080_set_a (c, d);
	e8080_set_af (c, 0);
	e8080_set_nf (c, 0);
	e8080_set_cf (c, s & 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 10: DJNZ imm8 */
static void op_10 (e8080_t *c)
{
	unsigned short dist;

	e8080_get_inst1 (c);
	dist = e8080_sext8 (c->inst[1]);

	e8080_set_b (c, e8080_get_b (c) - 1);

	if (e8080_get_b (c) != 0) {
		e8080_set_clk (c, 2 + dist, 13);
	}
	else {
		e8080_set_clk (c, 2, 8);
	}
}

/* OP 13: INC DE */
static void op_13 (e8080_t *c)
{
	e8080_set_de (c, e8080_get_de (c) + 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 14: INC D */
static void op_14 (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_d (c));
	e8080_set_d (c, e8080_get_d (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 15: DEC D */
static void op_15 (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_d (c));
	e8080_set_d (c, e8080_get_d (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 17: RLA */
static void op_17 (e8080_t *c)
{
	unsigned char d, s;

	s = e8080_get_a (c);
	d = (s << 1) | e8080_get_cf (c);
	e8080_set_a (c, d);
	e8080_set_cf (c, s & 0x80);
	e8080_set_af (c, 0);
	e8080_set_nf (c, 0);
	e8080_set_clk (c, 1, 4);
}

/* OP 18: JR imm8 */
static void op_18 (e8080_t *c)
{
	unsigned short dist;

	e8080_get_inst1 (c);
	dist = e8080_sext8 (c->inst[1]);
	e8080_set_clk (c, dist + 2, 12);
}

/* OP 19: ADD HL, DE */
static void op_19 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_de (c);
	d = s1 + s2;
	e8080_set_hl (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 1, 11);
}

/* OP 1B: DEC DE */
static void op_1b (e8080_t *c)
{
	e8080_set_de (c, e8080_get_de (c) - 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 1C: INC E */
static void op_1c (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_e (c));
	e8080_set_e (c, e8080_get_e (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 1D: DEC E */
static void op_1d (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_e (c));
	e8080_set_e (c, e8080_get_e (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 1F: RRA */
static void op_1f (e8080_t *c)
{
	unsigned char d, s;

	s = e8080_get_a (c);
	d = (s >> 1) | (e8080_get_cf (c) << 7);
	e8080_set_a (c, d);
	e8080_set_cf (c, s & 1);
	e8080_set_af (c, 0);
	e8080_set_nf (c, 0);
	e8080_set_clk (c, 1, 4);
}

/* OP 20: JR NZ, imm8 */
static void op_20 (e8080_t *c)
{
	unsigned short dist;

	e8080_get_inst1 (c);
	dist = e8080_sext8 (c->inst[1]);

	if (e8080_get_zf (c) == 0) {
		e8080_set_clk (c, 2 + dist, 12);
	}
	else {
		e8080_set_clk (c, 2, 7);
	}
}

/* OP 23: INC HL */
static void op_23 (e8080_t *c)
{
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 24: INC H */
static void op_24 (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_h (c));
	e8080_set_h (c, e8080_get_h (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 25: DEC H */
static void op_25 (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_h (c));
	e8080_set_h (c, e8080_get_h (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 27: DAA */
static void op_27 (e8080_t *c)
{
	unsigned s, d;

	s = e8080_get_a (c);
	d = s;

	if (e8080_get_nf (c)) {
		if (((s & 0x0f) >= 0x0a) || e8080_get_af (c)) {
			if ((s & 0x0f) >= 0x06) {
				e8080_set_af (c, 0);
			}

			d = (d - 0x06) & 0xff;
		}

		if ((s > 0x99) || e8080_get_cf (c)) {
			d -= 0x160;
		}
	}
	else {
		if (((s & 0x0f) >= 0x0a) || e8080_get_af (c)) {
			e8080_set_af (c, (s & 0x0f) >= 0x0a);
			d += 0x06;
		}

		if ((d >= 0xa0) || e8080_get_cf (c)) {
			d += 0x60;
		}
	}

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, 0);
	e8080_set_cf (c, d > 255);
	e8080_set_clk (c, 1, 4);
}

/* OP 28: JR Z, imm8 */
static void op_28 (e8080_t *c)
{
	unsigned short dist;

	e8080_get_inst1 (c);
	dist = e8080_sext8 (c->inst[1]);

	if (e8080_get_zf (c)) {
		e8080_set_clk (c, 2 + dist, 12);
	}
	else {
		e8080_set_clk (c, 2, 7);
	}
}

/* OP 29: ADD HL, HL */
static void op_29 (e8080_t *c)
{
	unsigned long d, s;

	s = e8080_get_hl (c);
	d = s + s;
	e8080_set_hl (c, d);
	z80_set_psw_add16 (c, d, s, s);
	e8080_set_clk (c, 1, 11);
}

/* OP 2B: DEC HL */
static void op_2b (e8080_t *c)
{
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 2C: INC L */
static void op_2c (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_l (c));
	e8080_set_l (c, e8080_get_l (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 2D: DEC L */
static void op_2d (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_l (c));
	e8080_set_l (c, e8080_get_l (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 2F: CPL */
static void op_2f (e8080_t *c)
{
	e8080_set_a (c, ~e8080_get_a (c));
	e8080_set_af (c, 1);
	e8080_set_nf (c, 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 30: JR NC, imm8 */
static void op_30 (e8080_t *c)
{
	unsigned short dist;

	e8080_get_inst1 (c);
	dist = e8080_sext8 (c->inst[1]);

	if (e8080_get_cf (c) == 0) {
		e8080_set_clk (c, 2 + dist, 12);
	}
	else {
		e8080_set_clk (c, 2, 7);
	}
}

/* OP 33: INC SP */
static void op_33 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) + 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 34: INC (HL) */
static void op_34 (e8080_t *c)
{
	unsigned      p;
	unsigned char s;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	e8080_set_mem8 (c, p, s + 1);
	z80_set_psw_inc (c, s);
	e8080_set_clk (c, 1, 11);
}

/* OP 35: DEC (HL) */
static void op_35 (e8080_t *c)
{
	unsigned      p;
	unsigned char s;

	p = e8080_get_hl (c);
	s = e8080_get_mem8 (c, p);
	e8080_set_mem8 (c, p, s - 1);
	z80_set_psw_dec (c, s);
	e8080_set_clk (c, 1, 11);
}

/* OP 37: SCF */
static void op_37 (e8080_t *c)
{
	e8080_set_cf (c, 1);
	e8080_set_af (c, 0);
	e8080_set_nf (c, 0);
	e8080_set_clk (c, 1, 4);
}

/* OP 38: JR C, imm8 */
static void op_38 (e8080_t *c)
{
	unsigned short dist;

	e8080_get_inst1 (c);
	dist = e8080_sext8 (c->inst[1]);

	if (e8080_get_cf (c)) {
		e8080_set_clk (c, 2 + dist, 12);
	}
	else {
		e8080_set_clk (c, 2, 7);
	}
}

/* OP 39: ADD HL, SP */
static void op_39 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_sp (c);
	d = s1 + s2;
	e8080_set_hl (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 1, 11);
}

/* OP 3B: DEC SP */
static void op_3b (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 1);
	e8080_set_clk (c, 1, 6);
}

/* OP 3C: INC A */
static void op_3c (e8080_t *c)
{
	z80_set_psw_inc (c, e8080_get_a (c));
	e8080_set_a (c, e8080_get_a (c) + 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 3D: DEC A */
static void op_3d (e8080_t *c)
{
	z80_set_psw_dec (c, e8080_get_a (c));
	e8080_set_a (c, e8080_get_a (c) - 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 3F: CCF */
static void op_3f (e8080_t *c)
{
	e8080_set_af (c, e8080_get_cf (c));
	e8080_set_nf (c, 0);
	e8080_set_cf (c, e8080_get_cf (c) == 0);
	e8080_set_clk (c, 1, 4);
}

/* OP 40: LD r, r */
static void op_40 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[0] >> 3, e8080_get_reg8 (c, c->inst[0]));
	e8080_set_clk (c, 1, 4);
}

/* OP 46: LD r, (HL) */
static void op_46 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[0] >> 3, e8080_get_mem8 (c, e8080_get_hl (c)));
	e8080_set_clk (c, 1, 7);
}

/* OP 70: LD (HL), r */
static void op_70 (e8080_t *c)
{
	e8080_set_mem8 (c, e8080_get_hl (c), e8080_get_reg8 (c, c->inst[0]));
	e8080_set_clk (c, 1, 7);
}

/* OP 76: HLT */
static void op_76 (e8080_t *c)
{
	c->halt = 1;
	e8080_set_clk (c, 1, 7);
}

/* OP 80: ADD A, r */
static void op_80 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP 86: ADD A, (HL) */
static void op_86 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP 88: ADC A, r */
static void op_88 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP 8E: ADC A, (HL) */
static void op_8e (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP 90: SUB A, r */
static void op_90 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP 96: SUB A, (HL) */
static void op_96 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP 98: SBC A, r */
static void op_98 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP 9E: SBC A, (HL) */
static void op_9e (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP A0: AND A, r */
static void op_a0 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) & e8080_get_reg8 (c, c->inst[0]);

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 1, 4);
}

/* OP A6: AND A, (HL) */
static void op_a6 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) & e8080_get_mem8 (c, e8080_get_hl (c));

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 1, 7);
}

/* OP A8: XOR A, r */
static void op_a8 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) ^ e8080_get_reg8 (c, c->inst[0]);

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 1, 4);
}

/* OP AE: XOR A, (HL) */
static void op_ae (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) ^ e8080_get_mem8 (c, e8080_get_hl (c));

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 1, 7);
}

/* OP B0: OR A, r */
static void op_b0 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) | e8080_get_reg8 (c, c->inst[0]);

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 1, 4);
}

/* OP B6: OR A, (HL) */
static void op_b6 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) | e8080_get_mem8 (c, e8080_get_hl (c));

	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 1, 7);
}

/* OP B8: CP A, r */
static void op_b8 (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP BE: CP A, (HL) */
static void op_be (e8080_t *c)
{
	unsigned d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP C6: ADD A, im8 */
static void op_c6 (e8080_t *c)
{
	unsigned d, s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 7);
}

/* OP C7: RST im3 */
static void op_c7 (e8080_t *c)
{
	e8080_set_clk (c, 1, 11);

	if (e8080_hook_rst (c)) {
		return;
	}

	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c));
	e8080_set_pc (c, c->inst[0] & 0x38);
}

/* OP CE: ADC A, im8 */
static void op_ce (e8080_t *c)
{
	unsigned d, s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 7);
}

/* OP D6: SUB A, im8 */
static void op_d6 (e8080_t *c)
{
	unsigned d, s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 7);
}

/* OP D9: EXX */
static void op_d9 (e8080_t *c)
{
	unsigned      i;
	unsigned char t;

	for (i = 0; i < 6; i++) {
		t = c->reg[i];
		c->reg[i] = c->reg2[i];
		c->reg2[i] = t;
	}

	e8080_set_clk (c, 1, 4);
}

/* OP DE: SBC A, im8 */
static void op_de (e8080_t *c)
{
	unsigned d, s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 7);
}

/* OP E6: AND A, im8 */
static void op_e6 (e8080_t *c)
{
	unsigned char d;

	e8080_get_inst1 (c);
	d = e8080_get_a (c) & c->inst[1];
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 7);
}

/* OP EE: XOR A, im8 */
static void op_ee (e8080_t *c)
{
	unsigned char d;

	e8080_get_inst1 (c);
	d = e8080_get_a (c) ^ c->inst[1];
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 7);
}

/* OP F1: POP AF */
static void op_f1 (e8080_t *c)
{
	e8080_set_psw (c, e8080_get_mem8 (c, e8080_get_sp (c) + 0));
	e8080_set_a (c, e8080_get_mem8 (c, e8080_get_sp (c) + 1));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 1, 10);
}

/* OP F5: PUSH AF */
static void op_f5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem8 (c, e8080_get_sp (c) + 1, e8080_get_a (c));
	e8080_set_mem8 (c, e8080_get_sp (c) + 0, e8080_get_psw (c));
	e8080_set_clk (c, 1, 11);
}

/* OP F6: OR A, im8 */
static void op_f6 (e8080_t *c)
{
	unsigned char d;

	e8080_get_inst1 (c);
	d = e8080_get_a (c) | c->inst[1];
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 7);
}

/* OP FE: CP A, im8 */
static void op_fe (e8080_t *c)
{
	unsigned d, s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 7);
}


static e8080_opcode_f opcodes_z80[256] = {
	 NULL,  NULL,  NULL, op_03, op_04, op_05,  NULL, op_07, /* 00 */
	op_08, op_09,  NULL, op_0b, op_0c, op_0d,  NULL, op_0f,
	op_10,  NULL,  NULL, op_13, op_14, op_15,  NULL, op_17, /* 10 */
	op_18, op_19,  NULL, op_1b, op_1c, op_1d,  NULL, op_1f,
	op_20,  NULL,  NULL, op_23, op_24, op_25,  NULL, op_27, /* 20 */
	op_28, op_29,  NULL, op_2b, op_2c, op_2d,  NULL, op_2f,
	op_30,  NULL,  NULL, op_33, op_34, op_35,  NULL, op_37, /* 30 */
	op_38, op_39,  NULL, op_3b, op_3c, op_3d,  NULL, op_3f,
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40, /* 40 */
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40,
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40, /* 50 */
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40,
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40, /* 60 */
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40,
	op_70, op_70, op_70, op_70, op_70, op_70, op_76, op_70, /* 70 */
	op_40, op_40, op_40, op_40, op_40, op_40, op_46, op_40,
	op_80, op_80, op_80, op_80, op_80, op_80, op_86, op_80, /* 80 */
	op_88, op_88, op_88, op_88, op_88, op_88, op_8e, op_88,
	op_90, op_90, op_90, op_90, op_90, op_90, op_96, op_90, /* 90 */
	op_98, op_98, op_98, op_98, op_98, op_98, op_9e, op_98,
	op_a0, op_a0, op_a0, op_a0, op_a0, op_a0, op_a6, op_a0, /* 90 */
	op_a8, op_a8, op_a8, op_a8, op_a8, op_a8, op_ae, op_a8,
	op_b0, op_b0, op_b0, op_b0, op_b0, op_b0, op_b6, op_b0, /* 90 */
	op_b8, op_b8, op_b8, op_b8, op_b8, op_b8, op_be, op_b8,
	 NULL,  NULL,  NULL,  NULL,  NULL,  NULL, op_c6, op_c7, /* C0 */
	 NULL,  NULL,  NULL,  NULL,  NULL,  NULL, op_ce, op_c7,
	 NULL,  NULL,  NULL,  NULL,  NULL,  NULL, op_d6, op_c7, /* D0 */
	 NULL, op_d9,  NULL,  NULL,  NULL,  NULL, op_de, op_c7,
	 NULL,  NULL,  NULL,  NULL,  NULL,  NULL, op_e6, op_c7, /* E0 */
	 NULL,  NULL,  NULL,  NULL,  NULL,  NULL, op_ee, op_c7,
	 NULL, op_f1,  NULL,  NULL,  NULL, op_f5, op_f6, op_c7, /* F0 */
	 NULL,  NULL,  NULL,  NULL,  NULL,  NULL, op_fe, op_c7
};

void z80_set_opcodes (e8080_t *c)
{
	unsigned i;

	e8080_set_opcodes (c);

	for (i = 0; i < 256; i++) {
		if (opcodes_z80[i] != NULL) {
			c->op[i] = opcodes_z80[i];
		}
	}

	c->op[0xcb] = z80_op_cb;
	c->op[0xdd] = z80_op_dd;
	c->op[0xed] = z80_op_ed;
	c->op[0xfd] = z80_op_fd;
}
