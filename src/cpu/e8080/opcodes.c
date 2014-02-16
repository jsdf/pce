/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/opcodes.c                                      *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
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


static void op_ud (e8080_t *c)
{
	if (e8080_hook_undefined (c)) {
		return;
	}

	e8080_set_clk (c, 1, 4);
}

/* OP 00: NOP */
static void op_00 (e8080_t *c)
{
	e8080_set_clk (c, 1, 4);
}

/* OP 01: LXI BC */
static void op_01 (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_b (c, c->inst[2]);
	e8080_set_c (c, c->inst[1]);
	e8080_set_clk (c, 3, 10);
}

/* OP 02: STAX BC */
static void op_02 (e8080_t *c)
{
	e8080_set_mem8 (c, e8080_get_bc (c), e8080_get_a (c));
	e8080_set_clk (c, 1, 7);
}

/* OP 03: INX BC */
static void op_03 (e8080_t *c)
{
	e8080_set_bc (c, e8080_get_bc (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 04: INR B */
static void op_04 (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_b (c));
	e8080_set_b (c, e8080_get_b (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 05: DCR B */
static void op_05 (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_b (c));
	e8080_set_b (c, e8080_get_b (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 06: MVI B, IM8 */
static void op_06 (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_b (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 07: RLC */
static void op_07 (e8080_t *c)
{
	unsigned char val;

	val = e8080_get_a (c);
	e8080_set_cf (c, val & 0x80);
	e8080_set_a (c, (val << 1) | (val >> 7));
	e8080_set_clk (c, 1, 4);
}

/* OP 09: DAD BC */
static void op_09 (e8080_t *c)
{
	unsigned short s1, s2, d;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_bc (c);
	d = (s1 + s2) & 0xffff;
	e8080_set_hl (c, d);
	e8080_set_cf (c, d < s1);
	e8080_set_clk (c, 1, 10);
}

/* OP 0A: LDAX BC */
static void op_0a (e8080_t *c)
{
	e8080_set_a (c, e8080_get_mem8 (c, e8080_get_bc (c)));
	e8080_set_clk (c, 1, 7);
}

/* OP 0B: DCX BC */
static void op_0b (e8080_t *c)
{
	e8080_set_bc (c, e8080_get_bc (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 0C: INR C */
static void op_0c (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_c (c));
	e8080_set_c (c, e8080_get_c (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 0D: DCR C */
static void op_0d (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_c (c));
	e8080_set_c (c, e8080_get_c (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 0E: MVI C, IM8 */
static void op_0e (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_c (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 0F: RRC */
static void op_0f (e8080_t *c)
{
	unsigned char val;

	val = e8080_get_a (c);
	e8080_set_cf (c, val & 0x01);
	e8080_set_a (c, (val >> 1) | (val << 7));
	e8080_set_clk (c, 1, 4);
}

/* OP 11: LXI DE */
static void op_11 (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_d (c, c->inst[2]);
	e8080_set_e (c, c->inst[1]);
	e8080_set_clk (c, 3, 10);
}

/* OP 12: STAX DE */
static void op_12 (e8080_t *c)
{
	e8080_set_mem8 (c, e8080_get_de (c), e8080_get_a (c));
	e8080_set_clk (c, 1, 7);
}

/* OP 13: INX DE */
static void op_13 (e8080_t *c)
{
	e8080_set_de (c, e8080_get_de (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 14: INR D */
static void op_14 (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_d (c));
	e8080_set_d (c, e8080_get_d (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 15: DCR D */
static void op_15 (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_d (c));
	e8080_set_d (c, e8080_get_d (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 16: MVI D, IM8 */
static void op_16 (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_d (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 17: RAL */
static void op_17 (e8080_t *c)
{
	unsigned char val;

	val = e8080_get_a (c);
	e8080_set_a (c, (val << 1) | e8080_get_cf (c));
	e8080_set_cf (c, val & 0x80);
	e8080_set_clk (c, 1, 4);
}

/* OP 19: DAD DE */
static void op_19 (e8080_t *c)
{
	unsigned short s1, s2, d;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_de (c);
	d = (s1 + s2) & 0xffff;
	e8080_set_hl (c, d);
	e8080_set_cf (c, d < s1);
	e8080_set_clk (c, 1, 10);
}

/* OP 1A: LDAX DE */
static void op_1a (e8080_t *c)
{
	e8080_set_a (c, e8080_get_mem8 (c, e8080_get_de (c)));
	e8080_set_clk (c, 1, 7);
}

/* OP 1B: DCX DE */
static void op_1b (e8080_t *c)
{
	e8080_set_de (c, e8080_get_de (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 1C: INR E */
static void op_1c (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_e (c));
	e8080_set_e (c, e8080_get_e (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 1D: DCR E */
static void op_1d (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_e (c));
	e8080_set_e (c, e8080_get_e (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 1E: MVI E, IM8 */
static void op_1e (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_e (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 1F: RAR */
static void op_1f (e8080_t *c)
{
	unsigned char val;

	val = e8080_get_a (c);
	e8080_set_a (c, (val >> 1) | (e8080_get_cf (c) << 7));
	e8080_set_cf (c, val & 0x01);
	e8080_set_clk (c, 1, 4);
}

/* OP 21: LXI HL */
static void op_21 (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_h (c, c->inst[2]);
	e8080_set_l (c, c->inst[1]);
	e8080_set_clk (c, 3, 10);
}

/* OP 22: SHLD IM16 */
static void op_22 (e8080_t *c)
{
	unsigned adr;

	e8080_get_inst12 (c);
	adr = e8080_uint16 (c->inst[1], c->inst[2]);
	e8080_set_mem8 (c, adr + 0, e8080_get_l (c));
	e8080_set_mem8 (c, adr + 1, e8080_get_h (c));
	e8080_set_clk (c, 3, 16);
}

/* OP 23: INX HL */
static void op_23 (e8080_t *c)
{
	e8080_set_hl (c, e8080_get_hl (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 24: INR H */
static void op_24 (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_h (c));
	e8080_set_h (c, e8080_get_h (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 25: DCR H */
static void op_25 (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_h (c));
	e8080_set_h (c, e8080_get_h (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 26: MVI H, IM8 */
static void op_26 (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_h (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 27: DAA */
static void op_27 (e8080_t *c)
{
	unsigned s, d;

	s = e8080_get_a (c);
	d = s;

	if (((s & 0x0f) >= 0x0a) || e8080_get_af (c)) {
		d += 0x06;
	}

	if ((d >= 0xa0) || e8080_get_cf (c)) {
		d += 0x60;
	}

	e8080_set_psw_szp (c, d, 0, 0);
	e8080_set_a (c, d);
	e8080_set_af (c, (s ^ d) & 0x10);
	e8080_set_cf (c, d > 255);
	e8080_set_clk (c, 1, 4);
}

/* OP 29: DAD HL */
static void op_29 (e8080_t *c)
{
	unsigned short s, d;

	s = e8080_get_hl (c);
	d = (s + s) & 0xffff;
	e8080_set_hl (c, d);
	e8080_set_cf (c, d < s);
	e8080_set_clk (c, 1, 10);
}

/* OP 2A: LHLD IM16 */
static void op_2a (e8080_t *c)
{
	unsigned adr;

	e8080_get_inst12 (c);
	adr = e8080_uint16 (c->inst[1], c->inst[2]);
	e8080_set_l (c, e8080_get_mem8 (c, adr + 0));
	e8080_set_h (c, e8080_get_mem8 (c, adr + 1));
	e8080_set_clk (c, 3, 16);
}

/* OP 2B: DCX HL */
static void op_2b (e8080_t *c)
{
	e8080_set_hl (c, e8080_get_hl (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 2C: INR L */
static void op_2c (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_l (c));
	e8080_set_l (c, e8080_get_l (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 2D: DCR L */
static void op_2d (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_l (c));
	e8080_set_l (c, e8080_get_l (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 2E: MVI L, IM8 */
static void op_2e (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_l (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 2F: CMA */
static void op_2f (e8080_t *c)
{
	e8080_set_a (c, ~e8080_get_a (c));
	e8080_set_clk (c, 1, 4);
}

/* OP 31: LXI SP */
static void op_31 (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_sp (c, e8080_uint16 (c->inst[1], c->inst[2]));
	e8080_set_clk (c, 3, 10);
}

/* OP 32: STA IM16 */
static void op_32 (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_mem8 (c, e8080_uint16 (c->inst[1], c->inst[2]), e8080_get_a (c));
	e8080_set_clk (c, 3, 13);
}

/* OP 33: INX SP */
static void op_33 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 34: INR M */
static void op_34 (e8080_t *c)
{
	unsigned char  val;
	unsigned short adr;

	adr = e8080_get_hl (c);
	val = e8080_get_mem8 (c, adr);
	e8080_set_psw_inc (c, val);
	e8080_set_mem8 (c, adr, val + 1);
	e8080_set_clk (c, 1, 10);
}

/* OP 35: DCR M */
static void op_35 (e8080_t *c)
{
	unsigned char  val;
	unsigned short adr;

	adr = e8080_get_hl (c);
	val = e8080_get_mem8 (c, adr);
	e8080_set_psw_dec (c, val);
	e8080_set_mem8 (c, adr, val - 1);
	e8080_set_clk (c, 1, 10);
}

/* OP 36: MVI M, IM8 */
static void op_36 (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_mem8 (c, e8080_get_hl (c), c->inst[1]);
	e8080_set_clk (c, 2, 10);
}

/* OP 37: STC */
static void op_37 (e8080_t *c)
{
	e8080_set_cf (c, 1);
	e8080_set_clk (c, 1, 4);
}

/* OP 39: DAD SP */
static void op_39 (e8080_t *c)
{
	unsigned short s1, s2, d;

	s1 = e8080_get_hl (c);
	s2 = e8080_get_sp (c);
	d = (s1 + s2) & 0xffff;
	e8080_set_hl (c, d);
	e8080_set_cf (c, d < s1);
	e8080_set_clk (c, 1, 10);
}

/* OP 3A: LDA IM16 */
static void op_3a (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_a (c, e8080_get_mem8 (c, e8080_uint16 (c->inst[1], c->inst[2])));
	e8080_set_clk (c, 3, 13);
}

/* OP 3B: DCX SP */
static void op_3b (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 3C: INR A */
static void op_3c (e8080_t *c)
{
	e8080_set_psw_inc (c, e8080_get_a (c));
	e8080_set_a (c, e8080_get_a (c) + 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 3D: DCR A */
static void op_3d (e8080_t *c)
{
	e8080_set_psw_dec (c, e8080_get_a (c));
	e8080_set_a (c, e8080_get_a (c) - 1);
	e8080_set_clk (c, 1, 5);
}

/* OP 3E: MVI A, IM8 */
static void op_3e (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_a (c, c->inst[1]);
	e8080_set_clk (c, 2, 7);
}

/* OP 3F: CMC */
static void op_3f (e8080_t *c)
{
	e8080_set_cf (c, e8080_get_cf (c) == 0);
	e8080_set_clk (c, 1, 4);
}

/* OP 40: MOV R, R */
static void op_40 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[0] >> 3, e8080_get_reg8 (c, c->inst[0]));
	e8080_set_clk (c, 1, 5);
}

/* OP 46: MOV R, M */
static void op_46 (e8080_t *c)
{
	e8080_set_reg8 (c, c->inst[0] >> 3, e8080_get_mem8 (c, e8080_get_hl (c)));
	e8080_set_clk (c, 1, 7);
}

/* OP 70: MOV M, R */
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

/* OP 80: ADD R */
static void op_80 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	e8080_set_a (c, s1 + s2);
	e8080_set_psw_add (c, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP 86: ADD M */
static void op_86 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_a (c, s1 + s2);
	e8080_set_psw_add (c, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP 88: ADC R */
static void op_88 (e8080_t *c)
{
	unsigned char s1, s2, s3;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	s3 = e8080_get_cf (c);
	e8080_set_a (c, s1 + s2 + s3);
	e8080_set_psw_adc (c, s1, s2, s3);
	e8080_set_clk (c, 1, 4);
}

/* OP 8E: ADC M */
static void op_8e (e8080_t *c)
{
	unsigned char s1, s2, s3;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	s3 = e8080_get_cf (c);
	e8080_set_a (c, s1 + s2 + s3);
	e8080_set_psw_adc (c, s1, s2, s3);
	e8080_set_clk (c, 1, 7);
}

/* OP 90: SUB R */
static void op_90 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	e8080_set_a (c, s1 - s2);
	e8080_set_psw_sub (c, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP 96: SUB M */
static void op_96 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_a (c, s1 - s2);
	e8080_set_psw_sub (c, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP 98: SBB R */
static void op_98 (e8080_t *c)
{
	unsigned char s1, s2, s3;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	s3 = e8080_get_cf (c);
	e8080_set_a (c, s1 - s2 - s3);
	e8080_set_psw_sbb (c, s1, s2, s3);
	e8080_set_clk (c, 1, 4);
}

/* OP 9E: SBB M */
static void op_9e (e8080_t *c)
{
	unsigned char s1, s2, s3;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	s3 = e8080_get_cf (c);
	e8080_set_a (c, s1 - s2 - s3);
	e8080_set_psw_sbb (c, s1, s2, s3);
	e8080_set_clk (c, 1, 7);
}

/* OP A0: ANA R */
static void op_a0 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	e8080_set_a (c, s1 & s2);
	e8080_set_psw_log (c, s1 & s2);
	e8080_set_clk (c, 1, 4);
}

/* OP A6: ANA M */
static void op_a6 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_a (c, s1 & s2);
	e8080_set_psw_log (c, s1 & s2);
	e8080_set_clk (c, 1, 7);
}

/* OP A8: XRA R */
static void op_a8 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	e8080_set_a (c, s1 ^ s2);
	e8080_set_psw_log (c, s1 ^ s2);
	e8080_set_clk (c, 1, 4);
}

/* OP AE: XRA M */
static void op_ae (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_a (c, s1 ^ s2);
	e8080_set_psw_log (c, s1 ^ s2);
	e8080_set_clk (c, 1, 7);
}

/* OP B0: ORA R */
static void op_b0 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	e8080_set_a (c, s1 | s2);
	e8080_set_psw_log (c, s1 | s2);
	e8080_set_clk (c, 1, 4);
}

/* OP B6: ORA M */
static void op_b6 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_a (c, s1 | s2);
	e8080_set_psw_log (c, s1 | s2);
	e8080_set_clk (c, 1, 7);
}

/* OP B8: CMP R */
static void op_b8 (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_reg8 (c, c->inst[0]);
	e8080_set_psw_sub (c, s1, s2);
	e8080_set_clk (c, 1, 4);
}

/* OP BE: CMP M */
static void op_be (e8080_t *c)
{
	unsigned char s1, s2;

	s1 = e8080_get_a (c);
	s2 = e8080_get_mem8 (c, e8080_get_hl (c));
	e8080_set_psw_sub (c, s1, s2);
	e8080_set_clk (c, 1, 7);
}

/* OP C0: RNZ */
static void op_c0 (e8080_t *c)
{
	if (e8080_get_zf (c) == 0) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP C1: POP BC */
static void op_c1 (e8080_t *c)
{
	e8080_set_c (c, e8080_get_mem8 (c, e8080_get_sp (c) + 0));
	e8080_set_b (c, e8080_get_mem8 (c, e8080_get_sp (c) + 1));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 1, 10);
}

/* OP C2: JNZ IM16 */
static void op_c2 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_zf (c) == 0) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP C3: JMP IM16 */
static void op_c3 (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
	e8080_set_clk (c, 0, 10);
}

/* OP C4: CNZ IM16 */
static void op_c4 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_zf (c) == 0) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP C5: PUSH BC */
static void op_c5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem8 (c, e8080_get_sp (c) + 1, e8080_get_b (c));
	e8080_set_mem8 (c, e8080_get_sp (c) + 0, e8080_get_c (c));
	e8080_set_clk (c, 1, 11);
}

/* OP C6: ADI IM8 */
static void op_c6 (e8080_t *c)
{
	unsigned char s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	e8080_set_a (c, s1 + s2);
	e8080_set_psw_add (c, s1, s2);
	e8080_set_clk (c, 2, 7);
}

/* OP C7: RST IM3 */
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

/* OP C8: RZ */
static void op_c8 (e8080_t *c)
{
	if (e8080_get_zf (c)) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP C9: RET */
static void op_c9 (e8080_t *c)
{
	e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 0, 10);
}

/* OP CA: JZ IM16 */
static void op_ca (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_zf (c)) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP CC: CZ IM16 */
static void op_cc (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_zf (c)) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP CD: CALL IM16 */
static void op_cd (e8080_t *c)
{
	e8080_get_inst12 (c);
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
	e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
	e8080_set_clk (c, 0, 17);
}

/* OP CE: ACI IM8 */
static void op_ce (e8080_t *c)
{
	unsigned char s1, s2, s3;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	s3 = e8080_get_cf (c);
	e8080_set_a (c, s1 + s2 + s3);
	e8080_set_psw_adc (c, s1, s2, s3);
	e8080_set_clk (c, 2, 7);
}

/* OP D0: RNC */
static void op_d0 (e8080_t *c)
{
	if (e8080_get_cf (c) == 0) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP D1: POP DE */
static void op_d1 (e8080_t *c)
{
	e8080_set_e (c, e8080_get_mem8 (c, e8080_get_sp (c) + 0));
	e8080_set_d (c, e8080_get_mem8 (c, e8080_get_sp (c) + 1));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 1, 10);
}

/* OP D2: JNC IM16 */
static void op_d2 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_cf (c) == 0) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP D3: OUT nn */
static void op_d3 (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_port8 (c, c->inst[1], e8080_get_a (c));
	e8080_set_clk (c, 2, 10);
}

/* OP D4: CNC IM16 */
static void op_d4 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_cf (c) == 0) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP D5: PUSH DE */
static void op_d5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem8 (c, e8080_get_sp (c) + 1, e8080_get_d (c));
	e8080_set_mem8 (c, e8080_get_sp (c) + 0, e8080_get_e (c));
	e8080_set_clk (c, 1, 11);
}

/* OP D6: SUI IM8 */
static void op_d6 (e8080_t *c)
{
	unsigned char s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	e8080_set_a (c, s1 - s2);
	e8080_set_psw_sub (c, s1, s2);
	e8080_set_clk (c, 2, 7);
}

/* OP D8: RC */
static void op_d8 (e8080_t *c)
{
	if (e8080_get_cf (c)) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP DA: JC IM16 */
static void op_da (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_cf (c)) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP DB: IN nn */
static void op_db (e8080_t *c)
{
	e8080_get_inst1 (c);
	e8080_set_a (c, e8080_get_port8 (c, c->inst[1]));
	e8080_set_clk (c, 2, 10);
}

/* OP DC: CC IM16 */
static void op_dc (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_cf (c)) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP DE: SBI IM8 */
static void op_de (e8080_t *c)
{
	unsigned char s1, s2, s3;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	s3 = e8080_get_cf (c);
	e8080_set_a (c, s1 - s2 - s3);
	e8080_set_psw_sbb (c, s1, s2, s3);
	e8080_set_clk (c, 2, 7);
}

/* OP E0: RPO */
static void op_e0 (e8080_t *c)
{
	if (e8080_get_pf (c) == 0) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP E1: POP HL */
static void op_e1 (e8080_t *c)
{
	e8080_set_l (c, e8080_get_mem8 (c, e8080_get_sp (c) + 0));
	e8080_set_h (c, e8080_get_mem8 (c, e8080_get_sp (c) + 1));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 1, 10);
}

/* OP E2: JPO IM16 */
static void op_e2 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_pf (c) == 0) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP E3: XTHL */
static void op_e3 (e8080_t *c)
{
	unsigned      sp;
	unsigned char v;

	sp = e8080_get_sp (c);

	v = e8080_get_mem8 (c, sp);
	e8080_set_mem8 (c, sp, e8080_get_l (c));
	e8080_set_l (c, v);

	v = e8080_get_mem8 (c, sp + 1);
	e8080_set_mem8 (c, sp + 1, e8080_get_h (c));
	e8080_set_h (c, v);

	e8080_set_clk (c, 1, 18);
}

/* OP E4: CPO IM16 */
static void op_e4 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_pf (c) == 0) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP E5: PUSH HL */
static void op_e5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem8 (c, e8080_get_sp (c) + 1, e8080_get_h (c));
	e8080_set_mem8 (c, e8080_get_sp (c) + 0, e8080_get_l (c));
	e8080_set_clk (c, 1, 11);
}

/* OP E6: ANI IM8 */
static void op_e6 (e8080_t *c)
{
	unsigned char s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	e8080_set_a (c, s1 & s2);
	e8080_set_psw_log (c, s1 & s2);
	e8080_set_clk (c, 2, 7);
}

/* OP E8: RPE */
static void op_e8 (e8080_t *c)
{
	if (e8080_get_pf (c)) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP E9: PCHL */
static void op_e9 (e8080_t *c)
{
	e8080_set_pc (c, e8080_get_hl (c));
	e8080_set_clk (c, 0, 5);
}

/* OP EA: JPE IM16 */
static void op_ea (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_pf (c)) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP EB: XCHG */
static void op_eb (e8080_t *c)
{
	unsigned short v;

	v = e8080_get_hl (c);
	e8080_set_hl (c, e8080_get_de (c));
	e8080_set_de (c, v);

	e8080_set_clk (c, 1, 4);
}

/* OP EC: CPE IM16 */
static void op_ec (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_pf (c)) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP EE: XRI IM8 */
static void op_ee (e8080_t *c)
{
	unsigned char s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	e8080_set_a (c, s1 ^ s2);
	e8080_set_psw_log (c, s1 ^ s2);
	e8080_set_clk (c, 2, 7);
}

/* OP F0: RP */
static void op_f0 (e8080_t *c)
{
	if (e8080_get_sf (c) == 0) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP F1: POP AF */
static void op_f1 (e8080_t *c)
{
	e8080_set_psw (c, (e8080_get_mem8 (c, e8080_get_sp (c) + 0) & 0xd7) | 0x02);
	e8080_set_a (c, e8080_get_mem8 (c, e8080_get_sp (c) + 1));
	e8080_set_sp (c, e8080_get_sp (c) + 2);
	e8080_set_clk (c, 1, 10);
}

/* OP F2: JP IM16 */
static void op_f2 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_sf (c) == 0) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP F3: DI */
static void op_f3 (e8080_t *c)
{
	c->iff = 0;
	c->iff2 = 0;
	e8080_set_clk (c, 1, 4);
}

/* OP F4: CP IM16 */
static void op_f4 (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_sf (c) == 0) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP F5: PUSH AF */
static void op_f5 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem8 (c, e8080_get_sp (c) + 1, e8080_get_a (c));
	e8080_set_mem8 (c, e8080_get_sp (c) + 0, (e8080_get_psw (c) & 0xd7) | 0x02);
	e8080_set_clk (c, 1, 11);
}

/* OP F6: ORI IM8 */
static void op_f6 (e8080_t *c)
{
	unsigned char s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	e8080_set_a (c, s1 | s2);
	e8080_set_psw_log (c, s1 | s2);
	e8080_set_clk (c, 2, 7);
}

/* OP F8: RM */
static void op_f8 (e8080_t *c)
{
	if (e8080_get_sf (c)) {
		e8080_set_pc (c, e8080_get_mem16 (c, e8080_get_sp (c)));
		e8080_set_sp (c, e8080_get_sp (c) + 2);
		e8080_set_clk (c, 0, 11);
	}
	else {
		e8080_set_clk (c, 1, 5);
	}
}

/* OP F9: SPHL */
static void op_f9 (e8080_t *c)
{
	e8080_set_sp (c, e8080_get_hl (c));
	e8080_set_clk (c, 1, 5);
}

/* OP FA: JM IM16 */
static void op_fa (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_sf (c)) {
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 10);
	}
	else {
		e8080_set_clk (c, 3, 10);
	}
}

/* OP FB: EI */
static void op_fb (e8080_t *c)
{
	c->iff = 1;
	c->iff2 = 1;
	e8080_set_clk (c, 1, 4);
}

/* OP FC: CM IM16 */
static void op_fc (e8080_t *c)
{
	e8080_get_inst12 (c);

	if (e8080_get_sf (c)) {
		e8080_set_sp (c, e8080_get_sp (c) - 2);
		e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c) + 3);
		e8080_set_pc (c, e8080_uint16 (c->inst[1], c->inst[2]));
		e8080_set_clk (c, 0, 17);
	}
	else {
		e8080_set_clk (c, 3, 11);
	}
}

/* OP FE: CPI IM8 */
static void op_fe (e8080_t *c)
{
	unsigned char s1, s2;

	e8080_get_inst1 (c);
	s1 = e8080_get_a (c);
	s2 = c->inst[1];
	e8080_set_psw_sub (c, s1, s2);
	e8080_set_clk (c, 2, 7);
}


e8080_opcode_f e8080_opcodes[256] = {
	op_00, op_01, op_02, op_03, op_04, op_05, op_06, op_07, /* 00 */
	 NULL, op_09, op_0a, op_0b, op_0c, op_0d, op_0e, op_0f,
	 NULL, op_11, op_12, op_13, op_14, op_15, op_16, op_17, /* 10 */
	 NULL, op_19, op_1a, op_1b, op_1c, op_1d, op_1e, op_1f,
	 NULL, op_21, op_22, op_23, op_24, op_25, op_26, op_27, /* 20 */
	 NULL, op_29, op_2a, op_2b, op_2c, op_2d, op_2e, op_2f,
	 NULL, op_31, op_32, op_33, op_34, op_35, op_36, op_37, /* 30 */
	 NULL, op_39, op_3a, op_3b, op_3c, op_3d, op_3e, op_3f,
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
	op_c0, op_c1, op_c2, op_c3, op_c4, op_c5, op_c6, op_c7, /* C0 */
	op_c8, op_c9, op_ca,  NULL, op_cc, op_cd, op_ce, op_c7,
	op_d0, op_d1, op_d2, op_d3, op_d4, op_d5, op_d6, op_c7, /* D0 */
	op_d8,  NULL, op_da, op_db, op_dc,  NULL, op_de, op_c7,
	op_e0, op_e1, op_e2, op_e3, op_e4, op_e5, op_e6, op_c7, /* E0 */
	op_e8, op_e9, op_ea, op_eb, op_ec,  NULL, op_ee, op_c7,
	op_f0, op_f1, op_f2, op_f3, op_f4, op_f5, op_f6, op_c7, /* F0 */
	op_f8, op_f9, op_fa, op_fb, op_fc,  NULL, op_fe, op_c7
};

void e8080_set_opcodes (e8080_t *c)
{
	unsigned i;

	for (i = 0; i < 256; i++) {
		if (e8080_opcodes[i] == NULL) {
			c->op[i] = op_ud;
		}
		else {
			c->op[i] = e8080_opcodes[i];
		}
	}
}
