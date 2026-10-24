/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/op_fd.c                                        *
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


#define op_fd_cb z80_op_fd_cb


static void op_fd_ud (e8080_t *c)
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

/* OP FD 09: ADD IY, BC */
static void op_fd_09 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_iy (c);
	s2 = e8080_get_bc (c);
	d = s1 + s2;
	e8080_set_iy (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP FD 19: ADD IY, DE */
static void op_fd_19 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_iy (c);
	s2 = e8080_get_de (c);
	d = s1 + s2;
	e8080_set_iy (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP FD 21: LD IY, nnnn */
static void op_fd_21 (e8080_t *c)
{
	e8080_get_inst23 (c);
	e8080_set_iy (c, e8080_uint16 (c->inst[2], c->inst[3]));
	e8080_set_clk (c, 4, 14);
}

/* OP FD 22: LD (nnnn), IY */
static void op_fd_22 (e8080_t *c)
{
	unsigned adr;

	e8080_get_inst23 (c);
	adr = e8080_uint16 (c->inst[2], c->inst[3]);
	e8080_set_mem16 (c, adr, e8080_get_iy (c));
	e8080_set_clk (c, 4, 20);
}

/* OP FD 23: INC IY */
static void op_fd_23 (e8080_t *c)
{
	e8080_set_iy (c, e8080_get_iy (c) + 1);
	e8080_set_clk (c, 2, 10);
}

/* OP FD 24: INC IYH */
static void op_fd_24 (e8080_t *c)
{
	unsigned s;

	s = e8080_get_iyh (c);
	z80_set_psw_inc (c, s & 0xff);
	e8080_set_iyh (c, s + 1);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 25: DEC I&H */
static void op_fd_25 (e8080_t *c)
{
	unsigned s;

	s = e8080_get_iyh (c);
	z80_set_psw_dec (c, s & 0xff);
	e8080_set_iyh (c, s - 1);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 26: LD IYH, nn */
static void op_fd_26 (e8080_t *c)
{
	e8080_get_inst2 (c);
	e8080_set_iyh (c, c->inst[2]);
	e8080_set_clk (c, 3, 11);
}

/* OP FD 29: ADD IY, IY */
static void op_fd_29 (e8080_t *c)
{
	unsigned long d, s;

	s = e8080_get_iy (c);
	d = s + s;
	e8080_set_iy (c, d);
	z80_set_psw_add16 (c, d, s, s);
	e8080_set_clk (c, 2, 15);
}

/* OP FD 2A: LD IY, (nnnn) */
static void op_fd_2a (e8080_t *c)
{
	unsigned short s, p;

	e8080_get_inst23 (c);
	p = e8080_uint16 (c->inst[2], c->inst[3]);
	s = e8080_get_mem16 (c, p);
	e8080_set_iy (c, s);
	e8080_set_clk (c, 4, 20);
}

/* OP FD 2B: DEC IY */
static void op_fd_2b (e8080_t *c)
{
	e8080_set_iy (c, e8080_get_iy (c) - 1);
	e8080_set_clk (c, 2, 10);
}

/* OP FD 2C: INC IYL */
static void op_fd_2c (e8080_t *c)
{
	unsigned s;

	s = e8080_get_iyl (c);
	z80_set_psw_inc (c, s & 0xff);
	e8080_set_iyl (c, s + 1);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 2D: DEC IYL */
static void op_fd_2d (e8080_t *c)
{
	unsigned s;

	s = e8080_get_iyl (c);
	z80_set_psw_dec (c, s & 0xff);
	e8080_set_iyl (c, s - 1);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 2E: LD IYL, nn */
static void op_fd_2e (e8080_t *c)
{
	e8080_get_inst2 (c);
	e8080_set_iyl (c, c->inst[2]);
	e8080_set_clk (c, 3, 11);
}

/* OP FD 34: INC (IY+d) */
static void op_fd_34 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);

	p = e8080_get_iyd (c, 2);
	s = e8080_get_mem8 (c, p);
	e8080_set_mem8 (c, p, s + 1);
	z80_set_psw_inc (c, s);
	e8080_set_clk (c, 3, 23);
}

/* OP FD 35: DEC (IY+d) */
static void op_fd_35 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s = e8080_get_mem8 (c, p);
	e8080_set_mem8 (c, p, s - 1);
	z80_set_psw_dec (c, s);
	e8080_set_clk (c, 3, 23);
}

/* OP FD 36: LD (IY+d), nn */
static void op_fd_36 (e8080_t *c)
{
	unsigned short p;

	e8080_get_inst23 (c);
	p = e8080_get_iyd (c, 2);
	e8080_set_mem8 (c, p, c->inst[3]);
	e8080_set_clk (c, 4, 19);
}

/* OP FD 39: ADD IY, SP */
static void op_fd_39 (e8080_t *c)
{
	unsigned long d, s1, s2;

	s1 = e8080_get_iy (c);
	s2 = e8080_get_sp (c);
	d = s1 + s2;
	e8080_set_iy (c, d);
	z80_set_psw_add16 (c, d, s1, s2);
	e8080_set_clk (c, 2, 15);
}

/* OP FD 44: LD r, IYH */
static void op_fd_44 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[1] >> 3, e8080_get_iyh (c));
	e8080_set_clk (c, 2, 8);
}

/* OP FD 45: LD r, IYL */
static void op_fd_45 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[1] >> 3, e8080_get_iyl (c));
	e8080_set_clk (c, 2, 8);
}

/* OP FD 46: LD r, (IY+d) */
static void op_fd_46 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s = e8080_get_mem8 (c, p);
	e8080_set_reg8 (c, c->inst[1] >> 3, s);
	e8080_set_clk (c, 3, 19);
}

/* OP FD 60: LD IYH, r */
static void op_fd_60 (e8080_t *c)
{
	e8080_set_iyh (c, e8080_get_reg8 (c, c->inst[1]));
	e8080_set_clk (c, 2, 8);
}

/* OP FD 64: LD IYH, IYH */
static void op_fd_64 (e8080_t *c)
{
	e8080_set_clk (c, 2, 8);
}

/* OP FD 65: LD IYH, IYL */
static void op_fd_65 (e8080_t *c)
{
	e8080_set_iyh (c, e8080_get_iyl (c));
	e8080_set_clk (c, 2, 8);
}

/* OP FD 68: LD IYL, r */
static void op_fd_68 (e8080_t *c)
{
	e8080_set_iyl (c, e8080_get_reg8 (c, c->inst[1]));
	e8080_set_clk (c, 2, 8);
}

/* OP FD 6C: LD IYL, IYH */
static void op_fd_6c (e8080_t *c)
{
	e8080_set_iyl (c, e8080_get_iyh (c));
	e8080_set_clk (c, 2, 8);
}

/* OP FD 6D: LD IXL, IXL */
static void op_fd_6d (e8080_t *c)
{
	e8080_set_clk (c, 2, 8);
}

/* OP FD 70: LD (IY+d), r */
static void op_fd_70 (e8080_t *c)
{
	unsigned char  s;
	unsigned short p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s = e8080_get_reg8 (c, c->inst[1]);
	e8080_set_mem8 (c, p, s);
	e8080_set_clk (c, 3, 19);
}

/* OP FD 84: ADD A, IYH */
static void op_fd_84 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyh (c);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 85: ADD A, IYL */
static void op_fd_85 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyl (c);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 86: ADD A, (IY+d) */
static void op_fd_86 (e8080_t *c)
{
	unsigned short d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 + s2;
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP FD 8C: ADC A, IYH */
static void op_fd_8c (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyh (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 8D: ADC A, IYL */
static void op_fd_8d (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyl (c);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 8E: ADC A, (IY+d) */
static void op_fd_8e (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 + s2 + e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_add (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP FD 94: SUB A, IYH */
static void op_fd_94 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyh (c);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 95: SUB A, IYL */
static void op_fd_95 (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyl (c);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 96: SUB A, (IY+d) */
static void op_fd_96 (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 - s2;
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP FD 9C: SBC A, IYH */
static void op_fd_9c (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyh (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 9D: SBC A, IYL */
static void op_fd_9d (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyl (c);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD 9E: SBC A, (IY+d) */
static void op_fd_9e (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 - s2 - e8080_get_cf (c);
	e8080_set_a (c, d);
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP FD A4: AND A, IYH */
static void op_fd_a4 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) & e8080_get_iyh (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP FD A5: AND A, IYL */
static void op_fd_a5 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) & e8080_get_iyl (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP FD A6: AND A, (IY+d) */
static void op_fd_a6 (e8080_t *c)
{
	unsigned short adr;
	unsigned char  d;

	e8080_get_inst2 (c);
	adr = e8080_get_iyd (c, 2);
	d = e8080_get_a (c) & e8080_get_mem8 (c, adr);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, E8080_FLG_A, E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 3, 19);
}

/* OP FD AC: XOR A, IYH */
static void op_fd_ac (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) ^ e8080_get_iyh (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP FD AD: XOR A, IYL */
static void op_fd_ad (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) ^ e8080_get_iyl (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP FD AE: XOR A, (IY+d) */
static void op_fd_ae (e8080_t *c)
{
	unsigned short adr;
	unsigned char  d;

	e8080_get_inst2 (c);
	adr = e8080_get_iyd (c, 2);
	d = e8080_get_a (c) ^ e8080_get_mem8 (c, adr);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 3, 19);
}

/* OP FD B4: OR A, IYH */
static void op_fd_b4 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) | e8080_get_iyh (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP FD B5: OR A, IYL */
static void op_fd_b5 (e8080_t *c)
{
	unsigned char d;

	d = e8080_get_a (c) | e8080_get_iyl (c);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 2, 8);
}

/* OP FD B6: OR A, (IY+d) */
static void op_fd_b6 (e8080_t *c)
{
	unsigned short adr;
	unsigned char  d;

	e8080_get_inst2 (c);
	adr = e8080_get_iyd (c, 2);
	d = e8080_get_a (c) | e8080_get_mem8 (c, adr);
	e8080_set_a (c, d);
	e8080_set_psw_szp (c, d, 0, E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);
	e8080_set_clk (c, 3, 19);
}

/* OP FD BC: CP A, IYH */
static void op_fd_bc (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyh (c);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD BD: CP A, IYL */
static void op_fd_bd (e8080_t *c)
{
	unsigned char d, s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_iyl (c);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 2, 8);
}

/* OP FD BE: CP A, (IY+d) */
static void op_fd_be (e8080_t *c)
{
	unsigned d, s1, s2, p;

	e8080_get_inst2 (c);
	p = e8080_get_iyd (c, 2);
	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, p);
	d = s1 - s2;
	z80_set_psw_sub (c, d, s1, s2);
	e8080_set_clk (c, 3, 19);
}

/* OP FD E1: POP IY */
static void op_fd_e1 (e8080_t *c)
{
	e8080_set_iy (c, e8080_get_mem16 (c, e8080_get_sp (c)));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 2, 14);
}

/* OP FD E3: EX (SP), IY */
static void op_fd_e3 (e8080_t *c)
{
	unsigned short sp, v;

	sp = e8080_get_sp (c);

	v = e8080_get_mem16 (c, sp);
	e8080_set_mem16 (c, sp, e8080_get_iy (c));
	e8080_set_iy (c, v);

	e8080_set_clk (c, 2, 23);
}

/* OP FD E5: PUSH IY */
static void op_fd_e5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_iy (c));
	e8080_set_clk (c, 2, 15);
}

/* OP FD E9: JMP (IY) */
static void op_fd_e9 (e8080_t *c)
{
	e8080_set_pc (c, e8080_get_iy (c));
	e8080_set_clk (c, 0, 8);
}

/* OP FD F9: LD SP, IY */
static void op_fd_f9 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_iy (c));
	e8080_set_clk (c, 2, 10);
}


static e8080_opcode_f opcodes_fd[256] = {
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, /* 00 */
	op_fd_ud, op_fd_09, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, /* 10 */
	op_fd_ud, op_fd_19, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud,
	op_fd_ud, op_fd_21, op_fd_22, op_fd_23, op_fd_24, op_fd_25, op_fd_26, op_fd_ud, /* 20 */
	op_fd_ud, op_fd_29, op_fd_2a, op_fd_2b, op_fd_2c, op_fd_2d, op_fd_2e, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_34, op_fd_35, op_fd_36, op_fd_ud, /* 30 */
	op_fd_ud, op_fd_39, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_44, op_fd_45, op_fd_46, op_fd_ud, /* 40 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_44, op_fd_45, op_fd_46, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_44, op_fd_45, op_fd_46, op_fd_ud, /* 50 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_44, op_fd_45, op_fd_46, op_fd_ud,
	op_fd_60, op_fd_60, op_fd_60, op_fd_60, op_fd_64, op_fd_65, op_fd_46, op_fd_60, /* 60 */
	op_fd_68, op_fd_68, op_fd_68, op_fd_68, op_fd_6c, op_fd_6d, op_fd_46, op_fd_68,
	op_fd_70, op_fd_70, op_fd_70, op_fd_70, op_fd_70, op_fd_70, op_fd_ud, op_fd_70, /* 70 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_44, op_fd_45, op_fd_46, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_84, op_fd_85, op_fd_86, op_fd_ud, /* 80 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_8c, op_fd_8d, op_fd_8e, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_94, op_fd_95, op_fd_96, op_fd_ud, /* 90 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_9c, op_fd_9d, op_fd_9e, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_a4, op_fd_a5, op_fd_a6, op_fd_ud, /* A0 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ac, op_fd_ad, op_fd_ae, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_b4, op_fd_b5, op_fd_b6, op_fd_ud, /* B0 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_bc, op_fd_bd, op_fd_be, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, /* C0 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_cb, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, /* D0 */
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_noni,  op_fd_ud, op_fd_ud,
	op_fd_ud, op_fd_e1, op_fd_ud, op_fd_e3, op_fd_ud, op_fd_e5, op_fd_ud, op_fd_ud, /* E0 */
	op_fd_ud, op_fd_e9, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud,
	op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, op_fd_ud, /* F0 */
	op_fd_ud, op_fd_f9, op_fd_ud, op_fd_ud, op_fd_ud, op_noni,  op_fd_ud, op_fd_ud
};

void z80_op_fd (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_inc_r (c);
	opcodes_fd[c->inst[1]] (c);
}
