/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/op_ed.c                                        *
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


static void op_ed_ud (e8080_t *c)
{
	if (e8080_hook_undefined (c)) {
		return;
	}

	e8080_set_clk (c, 1, 4);
}


/* OP ED 40: IN r, (C) */
static void op_ed_40 (e8080_t *c)
{
	unsigned      r;
	unsigned char d;

	r = (c->inst[1] >> 3) & 7;
	d = e8080_get_port8 (c, e8080_get_bc (c));
	e8080_set_reg8 (c, r, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N);
	e8080_set_clk (c, 2, 12);
}

/* OP ED 41: OUT (C), r */
static void op_ed_41 (e8080_t *c)
{
	unsigned r;

	r = (c->inst[1] >> 3) & 7;
	e8080_set_port8 (c, e8080_get_bc (c), e8080_get_reg8 (c, r));
	e8080_set_clk (c, 2, 12);
}

/* OP ED 42: SBC HL, BC */
static void op_ed_42 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_bc (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_sub16_2 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 43: LD (nnnn), BC */
static void op_ed_43 (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_mem16 (c, adr, e8080_get_bc (c));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 44: NEG */
static void op_ed_44 (e8080_t *c)
{
	unsigned d, s;

	s = e8080_get_a (c);
	d = 0 - s;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, 0, s);
	e8080_set_clk (c, 2, 8);
}

/* OP ED 45: RETN */
static void op_ed_45 (e8080_t *c)
{
	e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 0, 14);
}

/* OP ED 4A: ADC HL, BC */
static void op_ed_4a (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_bc (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_add16_2 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 4B: LD BC, (nnnn) */
static void op_ed_4b (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_bc (c, e8080_get_mem16 (c, adr));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 4D: RETI */
static void op_ed_4d (e8080_t *c)
{
	e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 0, 14);
}

/* OP ED 52: SBC HL, DE */
static void op_ed_52 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_de (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_sub16_2 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 53: LD (nnnn), DE */
static void op_ed_53 (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_mem16 (c, adr, e8080_get_de (c));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 57: LD A, I */
static void op_ed_57 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_i (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N);
	e8080_set_pf (c, c->iff2);
	e8080_set_clk (c, 2, 9);
}

/* OP ED 5A: ADC HL, DE */
static void op_ed_5a (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_de (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_add16_2 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 5B: LD DE, (nnnn) */
static void op_ed_5b (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_de (c, e8080_get_mem16 (c, adr));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 62: SBC HL, HL */
static void op_ed_62 (e8080_t *c)
{
	unsigned long d, s;

	s = e8080_get_hl (c);
	d = s - s - e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_sub16_2 (c, d, s, s);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 63: LD (nnnn), HL */
static void op_ed_63 (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_mem16 (c, adr, e8080_get_hl (c));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 67: RRD */
static void op_ed_67 (e8080_t *c)
{
	unsigned char s, d, a;

	s = e8080_get_mem8 (c, e8080_get_hl (c));
	a = e8080_get_a (c);

	d = (s >> 4) | (a << 4);
	a = (a & 0xf0) | (s & 0x0f);

	e8080_set_a (c, a);
	e8080_set_mem8 (c, e8080_get_hl (c), d);
	e8080_set_psw_szp (c, a, 0, E8080_FLG_A | E8080_FLG_N);
	e8080_set_clk (c, 2, 18);
}

/* OP ED 6A: ADC HL, HL */
static void op_ed_6a (e8080_t *c)
{
	unsigned long d, s;

	s = e8080_get_hl (c);
	d = s + s + e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_add16_2 (c, d, s, s);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 6B: LD HL, (nnnn) */
static void op_ed_6b (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_hl (c, e8080_get_mem16 (c, adr));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 6F: RLD */
static void op_ed_6f (e8080_t *c)
{
	unsigned char s, d, a;

	s = e8080_get_mem8 (c, e8080_get_hl (c));
	a = e8080_get_a (c);

	d = (s << 4) | (a & 0x0f);
	a = (a & 0xf0) | (s >> 4);

	e8080_set_a (c, a);
	e8080_set_mem8 (c, e8080_get_hl (c), d);
	e8080_set_psw_szp (c, a, 0, E8080_FLG_A | E8080_FLG_N);
	e8080_set_clk (c, 2, 18);
}

/* OP ED 70: IN (C) */
static void op_ed_70 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_port8 (c, e8080_get_bc (c));
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N);
	e8080_set_clk (c, 2, 12);
}

/* OP ED 71: OUT (C), 0 */
static void op_ed_71 (e8080_t *c)
{
	e8080_set_port8 (c, e8080_get_bc (c), 0);
	e8080_set_clk (c, 2, 12);
}

/* OP ED 72: SBC HL, SP */
static void op_ed_72 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_sp (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_sub16_2 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 73: LD (nnnn), SP */
static void op_ed_73 (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_mem16 (c, adr, e8080_get_sp (c));
	e8080_set_clk (c, 4, 20);
}

/* OP ED 7A: ADC HL, SP */
static void op_ed_7a (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_sp (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_hl (c, d);
	z80_set_psw_add16_2 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP ED 7B: LD SP, (nnnn) */
static void op_ed_7b (e8080_t *c)
{
	unsigned short adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_sp (c, e8080_get_mem16 (c, adr));
	e8080_set_clk (c, 4, 20);
}

/* OP ED A0: LDI */
static void op_ed_a0 (e8080_t *c)
{
	unsigned char  s;
	unsigned short n;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_mem8 (c, e8080_get_de (c), s);
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_de (c, e8080_get_de (c) + 1);
	e8080_set_bc (c, n);
	e8080_set_flag (c, E8080_FLG_A | E8080_FLG_P | E8080_FLG_N, 0);
	e8080_set_pf (c, n != 0);
	e8080_set_clk (c, 2, 16);
}

/* OP ED A1: CPI */
static void op_ed_a1 (e8080_t *c)
{
	unsigned short n;
	unsigned char  d, s1, s2;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2;
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_bc (c, n);
	e8080_set_psw_szp (c, d, E8080_FLG_N, 0);
	e8080_set_pf (c, n != 0);
	e8080_set_af (c, (d ^ s1 ^ s2) & 0x10);
	e8080_set_clk (c, 2, 16);
}

/* OP ED A2: INI */
static void op_ed_a2 (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_port8 (c, e8080_get_bc (c));
	e8080_set_mem8 (c, e8080_get_hl (c), s);
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, n == 0);
	e8080_set_nf (c, 1);
	e8080_set_clk (c, 2, 16);
}

/* OP ED A3: OUTI */
static void op_ed_a3 (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_port8 (c, e8080_get_bc (c), s);
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, n == 0);
	e8080_set_nf (c, 1);
	e8080_set_clk (c, 2, 16);
}

/* OP ED A8: LDD */
static void op_ed_a8 (e8080_t *c)
{
	unsigned char  s;
	unsigned short n;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_mem8 (c, e8080_get_de (c), s);
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_de (c, e8080_get_de (c) - 1);
	e8080_set_bc (c, n);
	e8080_set_flag (c, E8080_FLG_A | E8080_FLG_P | E8080_FLG_N, 0);
	e8080_set_pf (c, n != 0);
	e8080_set_clk (c, 2, 16);
}

/* OP ED A9: CPD */
static void op_ed_a9 (e8080_t *c)
{
	unsigned short n;
	unsigned char  d, s1, s2;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2;
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_bc (c, n);
	e8080_set_psw_szp (c, d, E8080_FLG_N, 0);
	e8080_set_pf (c, n != 0);
	e8080_set_af (c, (d ^ s1 ^ s2) & 0x10);
	e8080_set_clk (c, 2, 16);
}

/* OP ED AA: IND */
static void op_ed_aa (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_port8 (c, e8080_get_bc (c));
	e8080_set_mem8 (c, e8080_get_hl (c), s);
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, n == 0);
	e8080_set_nf (c, 1);
	e8080_set_clk (c, 2, 16);
}

/* OP ED AB: OUTD */
static void op_ed_ab (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_port8 (c, e8080_get_bc (c), s);
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, n == 0);
	e8080_set_nf (c, 1);
	e8080_set_clk (c, 2, 16);
}

/* OP ED B0: LDIR */
static void op_ed_b0 (e8080_t *c)
{
	unsigned char  s;
	unsigned short n;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_mem8 (c, e8080_get_de (c), s);
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_de (c, e8080_get_de (c) + 1);
	e8080_set_bc (c, n);
	e8080_set_flag (c, E8080_FLG_A | E8080_FLG_P | E8080_FLG_N, 0);

	if (n == 0) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED B1: CPIR */
static void op_ed_b1 (e8080_t *c)
{
	unsigned short n;
	unsigned char  d, s1, s2;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2;
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_bc (c, n);
	e8080_set_psw_szp (c, d, E8080_FLG_N, 0);
	e8080_set_pf (c, n != 0);
	e8080_set_af (c, (d ^ s1 ^ s2) & 0x10);

	if ((n == 0) || (s1 == s2)) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED B2: INIR */
static void op_ed_b2 (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_port8 (c, e8080_get_bc (c));
	e8080_set_mem8 (c, e8080_get_hl (c), s);
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, 1);
	e8080_set_nf (c, 1);

	if (n == 0) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED B3: OTIR */
static void op_ed_b3 (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_port8 (c, e8080_get_bc (c), s);
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, 1);
	e8080_set_nf (c, 1);

	if (n == 0) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED B8: LDDR */
static void op_ed_b8 (e8080_t *c)
{
	unsigned char  s;
	unsigned short n;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_mem8 (c, e8080_get_de (c), s);
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_de (c, e8080_get_de (c) - 1);
	e8080_set_bc (c, n);
	e8080_set_flag (c, E8080_FLG_A | E8080_FLG_P | E8080_FLG_N, 0);

	if (n == 0) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED B9: CPDR */
static void op_ed_b9 (e8080_t *c)
{
	unsigned short n;
	unsigned char  d, s1, s2;

	n = (e8080_get_bc (c) - 1) & 0xffff;
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	d = s1 - s2;
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_bc (c, n);
	e8080_set_psw_szp (c, d, E8080_FLG_N, 0);
	e8080_set_pf (c, n != 0);
	e8080_set_af (c, (d ^ s1 ^ s2) & 0x10);

	if ((n == 0) || (s1 == s2)) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED BA: INDR */
static void op_ed_ba (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_port8 (c, e8080_get_bc (c));
	e8080_set_mem8 (c, e8080_get_hl (c), s);
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, 1);
	e8080_set_nf (c, 1);

	if (n == 0) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

/* OP ED BB: OTDR */
static void op_ed_bb (e8080_t *c)
{
	unsigned char s, n;

	n = (e8080_get_b (c) - 1) & 0xff;
	s = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_port8 (c, e8080_get_bc (c), s);
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_b (c, n);
	e8080_set_zf (c, 1);
	e8080_set_nf (c, 1);

	if (n == 0) {
		e8080_set_clk (c, 2, 16);
	}
	else {
		e8080_set_clk (c, 0, 21);
	}
}

static e8080_opcode_f opcodes_ed[256] = {
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 00 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 10 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 20 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 30 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_40, op_ed_41, op_ed_42, op_ed_43, op_ed_44, op_ed_45, op_ed_ud, op_ed_ud, /* 40 */
	op_ed_40, op_ed_41, op_ed_4a, op_ed_4b, op_ed_ud, op_ed_4d, op_ed_ud, op_ed_ud,
	op_ed_40, op_ed_41, op_ed_52, op_ed_53, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_57, /* 50 */
	op_ed_40, op_ed_41, op_ed_5a, op_ed_5b, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_40, op_ed_41, op_ed_62, op_ed_63, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_67, /* 60 */
	op_ed_40, op_ed_41, op_ed_6a, op_ed_6b, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_6f,
	op_ed_70, op_ed_71, op_ed_72, op_ed_73, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 70 */
	op_ed_40, op_ed_41, op_ed_7a, op_ed_7b, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 80 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* 90 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_a0, op_ed_a1, op_ed_a2, op_ed_a3, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* A0 */
	op_ed_a8, op_ed_a9, op_ed_aa, op_ed_ab, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_b0, op_ed_b1, op_ed_b2, op_ed_b3, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* B0 */
	op_ed_b8, op_ed_b9, op_ed_ba, op_ed_bb, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* C0 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* D0 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* E0 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, /* F0 */
	op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud, op_ed_ud,
};

void z80_op_ed (e8080_t *c)
{
	e8080_get_inst1 (c);
	opcodes_ed[c->inst[1]] (c);
}
