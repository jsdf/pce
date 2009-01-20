/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/sparc32/opcodes.c                                    *
 * Created:     2004-09-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include "sparc32.h"
#include "internal.h"


int s32_check_privilege (sparc32_t *c)
{
	if (s32_get_psr_s (c) == 0) {
		s32_trap (c, S32_TRAP_PRIVILEGED_INSTRUCTION);
		return (1);
	}

	return (0);
}

void s32_op_undefined (sparc32_t *c)
{
	s32_undefined (c);
	s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
}

void s32_set_icc_log (sparc32_t *c, uint32_t val)
{
	uint32_t icc;

	val &= 0xffffffffUL;
	icc = 0;

	if (val == 0) {
		icc |= S32_PSR_Z;
	}
	else if (val & 0x80000000UL) {
		icc |= S32_PSR_N;
	}

	c->psr &= ~S32_PSR_ICC;
	c->psr |= icc;
}

int s32_check_icc (sparc32_t *c, unsigned cond)
{
	switch (cond) {
	case 0x00: /* n */
		return (0);

	case 0x01: /* e */
		return (s32_get_psr_z (c));

	case 0x02: /* le */
		return (s32_get_psr_z (c) || (s32_get_psr_n (c) != s32_get_psr_v (c)));

	case 0x03: /* l */
		return (s32_get_psr_n (c) != s32_get_psr_v (c));

	case 0x04: /* leu */
		return (s32_get_psr_c (c) || s32_get_psr_z (c));

	case 0x05: /* cs */
		return (s32_get_psr_c (c));

	case 0x06: /* neg */
		return (s32_get_psr_n (c));

	case 0x07: /* vs */
		return (s32_get_psr_v (c));

	case 0x08: /* a */
		return (1);

	case 0x09: /* ne */
		return (!s32_get_psr_z (c));

	case 0x0a: /* g */
		return (!s32_get_psr_z (c) && (s32_get_psr_n (c) == s32_get_psr_v (c)));

	case 0x0b: /* ge */
		return (s32_get_psr_n (c) == s32_get_psr_v (c));

	case 0x0c: /* gu */
		return (!s32_get_psr_c (c) && !s32_get_psr_z (c));

	case 0x0d: /* cc */
		return (!s32_get_psr_c (c));

	case 0x0e: /* pos */
		return (!s32_get_psr_n (c));

	case 0x0f: /* vc */
		return (!s32_get_psr_v (c));
	}

	return (0);
}
/*****************************************************************************/

/* 0 00: unimp const22 */
static
void op000 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
}

/* 0 02: bicc[,a] disp22 */
static
void op002 (sparc32_t *c)
{
	unsigned cond;
	uint32_t d;

	cond = (c->ir >> 25) & 0x0f;

	d = (s32_get_pc (c) + (s32_ir_simm22 (c->ir) << 2)) & 0xffffffffUL;

	s32_set_clk (c, 4, 1);

	if (s32_check_icc (c, cond)) {
		s32_set_npc (c, d);

		if ((cond == 8) && s32_ir_a (c->ir)) {
			s32_set_clk (c, 4, 1);
		}
	}
	else if (s32_ir_a (c->ir)) {
		s32_set_clk (c, 4, 1);
	}
}

/* 0 04: sethi rd, imm22 */
static
void op004 (sparc32_t *c)
{
	s32_set_rd (c, c->ir, s32_ir_uimm22 (c->ir) << 10);

	s32_set_clk (c, 4, 1);
}

/* 0 06: fbcc[,a] disp22 */
static
void op006 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 0 07: cbcc[,a] disp22 */
static
void op007 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 1 00: call disp30 */
static
void op100 (sparc32_t *c)
{
	uint32_t d;

	d = (s32_get_pc (c) + (s32_ir_simm30 (c->ir) << 2)) & 0xfffffffcUL;

	s32_set_gpr (c, 15, s32_get_pc (c));

	s32_set_clk (c, 0, 1);
	s32_set_npc (c, d);
}

/* 2 00: add rs1, reg_or_imm, rd */
static
void op200 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 + s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 01: and rs1, reg_or_imm, rd */
static
void op201 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 & s2;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 02: or rs1, reg_or_imm, rd */
static
void op202 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 | s2;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 03: xor rs1, reg_or_imm, rd */
static
void op203 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 ^ s2;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 04: sub rs1, reg_or_imm, rd */
static
void op204 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 - s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 05: andn rs1, reg_or_imm, rd */
static
void op205 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 & ~s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 06: orn rs1, reg_or_imm, rd */
static
void op206 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 | ~s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 07: xnor rs1, reg_or_imm, rd */
static
void op207 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 ^ ~s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 08: addx rs1, reg_or_imm, rd */
static
void op208 (sparc32_t *c)
{
	uint32_t d, s1, s2, s3;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	s3 = s32_get_psr_c (c);

	d = (s1 + s2 + s3) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 0A: umul rs1, reg_or_imm, rd */
static
void op20a (sparc32_t *c)
{
	uint32_t s1, s2;
	uint64_t d;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	d = (uint64_t) s1 * (uint64_t) s2;

	s32_set_rd (c, c->ir, d & 0xffffffffUL);
	s32_set_y (c, (d >> 32) & 0xffffffffUL);

	s32_set_clk (c, 4, 19);
}

/* 2 0B: smul rs1, reg_or_imm, rd */
static
void op20b (sparc32_t *c)
{
	int      s;
	uint32_t s1, s2;
	uint64_t d;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	s = ((s1 ^ s2) & 0x80000000UL) != 0;

	if (s1 & 0x80000000UL) {
		s1 = (~s1 + 1) & 0xffffffff;
	}

	if (s2 & 0x80000000UL) {
		s2 = (~s2 + 1) & 0xffffffff;
	}

	d = (uint64_t) s1 * (uint64_t) s2;

	if (s) {
		d = ~d + 1;
	}

	s32_set_rd (c, c->ir, d & 0xffffffffUL);
	s32_set_y (c, (d >> 32) & 0xffffffffUL);

	s32_set_clk (c, 4, 19);
}

/* 2 0C: subx rs1, reg_or_imm, rd */
static
void op20c (sparc32_t *c)
{
	uint32_t d, s1, s2, s3;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	s3 = s32_get_psr_c (c);

	d = (s1 - s2 - s3) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 0E: udiv rs1, reg_or_imm, rd */
static
void op20e (sparc32_t *c)
{
	uint32_t s2;
	uint64_t d, s1;

	s1 = s32_get_y (c);
	s1 = (s1 << 32) | s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	if (s2 == 0) {
		s32_trap (c, S32_TRAP_DIVISION_BY_ZERO);
		return;
	}

	d = s1 / s2;

	if ((d >> 32) & 0xffffffffUL) {
		d = 0xffffffffUL;
	}

	s32_set_rd (c, c->ir, d & 0xffffffffUL);

	s32_set_clk (c, 4, 39);
}

/* 2 0F: sdiv rs1, reg_or_imm, rd */
static
void op20f (sparc32_t *c)
{
	int      s;
	uint32_t s2;
	uint64_t d, s1;

	s1 = s32_get_y (c);
	s1 = (s1 << 32) | s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	if (s2 == 0) {
		s32_trap (c, S32_TRAP_DIVISION_BY_ZERO);
		return;
	}

	s = ((s1 >> 63) ^ (s2 >> 31)) != 0;

	if (s1 >> 63) {
		s1 = (~s1 + 1) & 0xffffffffffffffffULL;
	}

	if (s2 & 0x80000000UL) {
		s2 = (~s2 + 1) & 0xffffffffUL;
	}

	d = s1 / s2;

	if (s) {
		if (d > 0x80000000UL) {
			d = 0x80000000UL;
		}
		else {
			d = (~d + 1) & 0xffffffffUL;
		}
	}
	else {
		if (d >= 0x80000000UL) {
			d = 0x7fffffffUL;
		}
	}

	s32_set_rd (c, c->ir, d & 0xffffffffUL);

	s32_set_clk (c, 4, 39);
}

/* 2 10: addcc rs1, reg_or_imm, rd */
static
void op210 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 + s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if (d < s1) {
		s32_set_psr_c (c, 1);
	}

	if (~(s1 ^ s2) & (s1 ^ d) & 0x80000000UL) {
		s32_set_psr_v (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 11: andcc rs1, reg_or_imm, rd */
static
void op211 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 & s2;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 12: orcc rs1, reg_or_imm, rd */
static
void op212 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 | s2;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 13: xorcc rs1, reg_or_imm, rd */
static
void op213 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 ^ s2;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 14: subcc rs1, reg_or_imm, rd */
static
void op214 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 - s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if (d > s1) {
		s32_set_psr_c (c, 1);
	}

	if ((s1 ^ s2) & (s1 ^ d) & 0x80000000UL) {
		s32_set_psr_v (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 15: andncc rs1, reg_or_imm, rd */
static
void op215 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 & ~s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 16: orncc rs1, reg_or_imm, rd */
static
void op216 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 | ~s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 17: xnorcc rs1, reg_or_imm, rd */
static
void op217 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 ^ ~s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 18: addxcc rs1, reg_or_imm, rd */
static
void op218 (sparc32_t *c)
{
	uint32_t d, t, s1, s2, s3;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	s3 = s32_get_psr_c (c);

	t = (s1 + s2) & 0xffffffffUL;
	d = (t + s3) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if ((d < t) || (t < s1)) {
		s32_set_psr_c (c, 1);
	}

	if (~(s1 ^ s2) & (s1 ^ d) & 0x80000000UL) {
		s32_set_psr_v (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 1A: umulcc rs1, reg_or_imm, rd */
static
void op21a (sparc32_t *c)
{
	uint32_t s1, s2;
	uint64_t d;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	d = (uint64_t) s1 * (uint64_t) s2;

	s32_set_rd (c, c->ir, d & 0xffffffffUL);
	s32_set_y (c, (d >> 32) & 0xffffffffUL);

	s32_set_icc_log (c, d & 0xffffffffUL);

	s32_set_clk (c, 4, 19);
}

/* 2 1B: smulcc rs1, reg_or_imm, rd */
static
void op21b (sparc32_t *c)
{
	int      s;
	uint32_t s1, s2;
	uint64_t d;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	s = ((s1 ^ s2) & 0x80000000UL) != 0;

	if (s1 & 0x80000000UL) {
		s1 = (~s1 + 1) & 0xffffffff;
	}

	if (s2 & 0x80000000UL) {
		s2 = (~s2 + 1) & 0xffffffff;
	}

	d = (uint64_t) s1 * (uint64_t) s2;

	if (s) {
		d = ~d + 1;
	}

	s32_set_rd (c, c->ir, d & 0xffffffffUL);
	s32_set_y (c, (d >> 32) & 0xffffffffUL);

	s32_set_icc_log (c, d & 0xffffffffUL);

	s32_set_clk (c, 4, 19);
}

/* 2 1C: subxcc rs1, reg_or_imm, rd */
static
void op21c (sparc32_t *c)
{
	uint32_t d, t, s1, s2, s3;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	s3 = s32_get_psr_c (c);

	t = (s1 - s2) & 0xffffffffUL;
	d = (t - s3) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if ((t > s1) || (d > t)) {
		s32_set_psr_c (c, 1);
	}

	if ((s1 ^ s2) & (s1 ^ d) & 0x80000000UL) {
		s32_set_psr_v (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 1E: udivcc rs1, reg_or_imm, rd */
static
void op21e (sparc32_t *c)
{
	uint32_t s2;
	uint64_t d, s1;

	s1 = s32_get_y (c);
	s1 = (s1 << 32) | s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	if (s2 == 0) {
		s32_trap (c, S32_TRAP_DIVISION_BY_ZERO);
		return;
	}

	d = s1 / s2;

	if ((d >> 32) & 0xffffffffUL) {
		d = 0xffffffffUL;
		s32_set_icc_log (c, d & 0xffffffffUL);
		s32_set_psr_v (c, 1);
	}
	else {
		s32_set_icc_log (c, d & 0xffffffffUL);
	}

	s32_set_rd (c, c->ir, d & 0xffffffffUL);

	s32_set_clk (c, 4, 39);
}

/* 2 1F: sdivcc rs1, reg_or_imm, rd */
static
void op21f (sparc32_t *c)
{
	int      s, of;
	uint32_t s2;
	uint64_t d, s1;

	s1 = s32_get_y (c);
	s1 = (s1 << 32) | s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	if (s2 == 0) {
		s32_trap (c, S32_TRAP_DIVISION_BY_ZERO);
		return;
	}

	s = ((s1 >> 63) ^ (s2 >> 31)) != 0;

	if (s1 >> 63) {
		s1 = (~s1 + 1) & 0xffffffffffffffffULL;
	}

	if (s2 & 0x80000000UL) {
		s2 = (~s2 + 1) & 0xffffffffUL;
	}

	d = s1 / s2;

	of = 0;

	if (s) {
		if (d > 0x80000000UL) {
			d = 0x80000000UL;
			of = 1;
		}
		else {
			d = (~d + 1) & 0xffffffffUL;
		}
	}
	else {
		if (d >= 0x80000000UL) {
			d = 0x7fffffffUL;
			of = 1;
		}
	}

	s32_set_rd (c, c->ir, d & 0xffffffffUL);

	s32_set_icc_log (c, d & 0xffffffffUL);
	s32_set_psr_v (c, of);

	s32_set_clk (c, 4, 39);
}

/* 2 20: taddcc rs1, reg_or_imm, rd */
static
void op220 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 + s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if (d < s1) {
		s32_set_psr_c (c, 1);
	}

	if ((~(s1 ^ s2) & (s1 ^ d) & 0x80000000UL) || ((s1 | s2) & 3)) {
		s32_set_psr_v (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 21: tsubcc rs1, reg_or_imm, rd */
static
void op221 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 - s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if (d > s1) {
		s32_set_psr_c (c, 1);
	}

	if (((s1 ^ s2) & (s1 ^ d) & 0x80000000UL) || ((s1 | s2) & 3)) {
		s32_set_psr_v (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 22: taddcctv rs1, reg_or_imm, rd */
static
void op222 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 + s2) & 0xffffffffUL;

	if ((~(s1 ^ s2) & (s1 ^ d) & 0x80000000UL) || ((s1 | s2) & 3)) {
		s32_trap (c, S32_TRAP_TAG_OVERFLOW);
		return;
	}

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if (d < s1) {
		s32_set_psr_c (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 23: tsubcctv rs1, reg_or_imm, rd */
static
void op223 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 - s2) & 0xffffffffUL;

	if (((s1 ^ s2) & (s1 ^ d) & 0x80000000UL) || ((s1 | s2) & 3)) {
		s32_trap (c, S32_TRAP_TAG_OVERFLOW);
		return;
	}

	s32_set_rd (c, c->ir, d);

	s32_set_icc_log (c, d);

	if (d > s1) {
		s32_set_psr_c (c, 1);
	}

	s32_set_clk (c, 4, 1);
}

/* 2 24: mulscc rs1, reg_or_imm, rd */
static
void op224 (sparc32_t *c)
{
	uint32_t d, s1, s2, t1, t2, y;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir) & 0x1f;
	y = s32_get_y (c);

	if (s32_get_psr_n (c) != s32_get_psr_v (c)) {
		t1 = (s1 >> 1) | 0x80000000UL;
	}
	else {
		t1 = (s1 >> 1);
	}

	t2 = (y & 1) ? s2 : 0;

	d = (t1 + t2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);
	s32_set_icc_log (c, d);

	if (d < t1) {
		s32_set_psr_c (c, 1);
	}

	if (~(t1 ^ t2) & (t1 ^ d) & 0x80000000UL) {
		s32_set_psr_v (c, 1);
	}

	y = ((y >> 1) | (s1 << 31)) & 0xffffffffUL;

	s32_set_y (c, y);

	s32_set_clk (c, 4, 1);
}

/* 2 25: sll rs1, reg_or_imm, rd */
static
void op225 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir) & 0x1f;
	d = (s1 << s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 26: srl rs1, reg_or_imm, rd */
static
void op226 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir) & 0x1f;
	d = s1 >> s2;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 27: sra rs1, reg_or_imm, rd */
static
void op227 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir) & 0x1f;

	if (s1 & 0x80000000UL) {
		d = ((s1 >> s2) | (0xffffffffUL << (32 - s2))) & 0xffffffffUL;
	}
	else {
		d = s1 >> s2;
	}

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 28: rd asr, rd */
static
void op228 (sparc32_t *c)
{
	unsigned rs1;

	rs1 = s32_ir_rs1 (c->ir);

	if (rs1 == 0) {
		s32_set_rd (c, c->ir, s32_get_y (c));
	}
	else if (rs1 < 15) {
		/* rdasr 1-14 */
	}
	else if (rs1 == 15) {
		if (s32_ir_rd (c->ir) == 0) {
			/* stbar */
		}
	}
	else {
		/* rdasr 16-31 */
	}

	s32_set_clk (c, 4, 1);
}

/* 2 29: rd %psr, rd */
static
void op229 (sparc32_t *c)
{
	if (s32_check_privilege (c)) {
		return;
	}

	s32_set_rd (c, c->ir, s32_get_psr (c));

	s32_set_clk (c, 4, 1);
}

/* 2 2A: rd %wim, rd */
static
void op22a (sparc32_t *c)
{
	uint32_t d;

	if (s32_check_privilege (c)) {
		return;
	}

	d = s32_get_wim (c);
	if (c->nwindows < 32) {
		d &= (1UL << c->nwindows) - 1;
	}

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 2B: rd %tbr, rd */
static
void op22b (sparc32_t *c)
{
	if (s32_check_privilege (c)) {
		return;
	}

	s32_set_rd (c, c->ir, s32_get_tbr (c) & 0xfffffff0UL);

	s32_set_clk (c, 4, 1);
}

/* 2 30: wr rs1, reg_or_imm, %asr */
static
void op230 (sparc32_t *c)
{
	unsigned rd;
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 ^ s2;

	rd = s32_ir_rd (c->ir);

	if (rd == 0) {
		s32_set_y (c, d);
	}
	else if (rd < 16) {
		/* wrasr 1-14 */
	}
	else {
		/* wrasr 16-31 */
	}

	s32_set_clk (c, 4, 1);
}

/* 2 31: wr rs1, reg_or_imm, %psr */
static
void op231 (sparc32_t *c)
{
	uint32_t d, s1, s2;
	unsigned cwp1, cwp2;

	if (s32_check_privilege (c)) {
		return;
	}

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 ^ s2;

	cwp1 = s32_get_cwp (c);
	cwp2 = d & S32_PSR_CWP;

	if (cwp1 != cwp2) {
		if (cwp2 >= c->nwindows) {
			s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
			return;
		}

		s32_regstk_save (c, cwp1);
		s32_regstk_load (c, cwp2);
	}

	if (cwp2 & S32_PSR_S) {
		c->asi_text = S32_ASI_STEXT;
		c->asi_text = S32_ASI_SDATA;
	}
	else {
		c->asi_text = S32_ASI_UTEXT;
		c->asi_text = S32_ASI_UDATA;
	}

	s32_set_psr (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 32: wr rs1, reg_or_imm, %wim */
static
void op232 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	if (s32_check_privilege (c)) {
		return;
	}

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 ^ s2;

	if (c->nwindows < 32) {
		d &= (1UL << c->nwindows) - 1;
	}

	s32_set_wim (c, d);

	s32_set_clk (c, 4, 1);
}

/* 2 33: wr rs1, reg_or_imm, %tbr */
static
void op233 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	if (s32_check_privilege (c)) {
		return;
	}

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = s1 ^ s2;

	s32_set_tbr (c, d & 0xfffffff0UL);

	s32_set_clk (c, 4, 1);
}

/* 2 34: fpop1 rs1, rs2, rd */
static
void op234 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 2 35: fpop2 rs1, rs2, rd */
static
void op235 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 2 36: cpop1 rs1, rs2, rd */
static
void op236 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 2 37: cpop2 rs1, rs2, rd */
static
void op237 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 2 38: jmpl rs1, reg_or_imm, rd */
static
void op238 (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 + s2) & 0xffffffffUL;

	if (d & 0x03) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	s32_set_rd (c, c->ir, s32_get_pc (c));

	s32_set_clk (c, 0, 2);
	s32_set_npc (c, d);
}

/* 2 39: rett rs1, reg_or_imm */
static
void op239 (sparc32_t *c)
{
	uint32_t d, s1, s2, psr;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);
	d = (s1 + s2) & 0xffffffffUL;

	if (s32_get_psr_et (c)) {
		if (s32_get_psr_s (c)) {
			s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		}
		else {
			s32_trap (c, S32_TRAP_PRIVILEGED_INSTRUCTION);
		}
		return;
	}

	if (s32_get_psr_s (c) == 0) {
		s32_trap (c, S32_TRAP_PRIVILEGED_INSTRUCTION);
		/* error_mode */
		return;
	}

	if (d & 0x03) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		/* error_mode */
		return;
	}

	if (s32_restore (c, 0)) {
		s32_trap (c, S32_TRAP_WINDOW_UNDERFLOW);
		/* error mode */
		return;
	}

	s32_set_clk (c, 0, 1);
	s32_set_npc (c, d);

	psr = s32_get_psr (c);

	if (psr & S32_PSR_PS) {
		psr |= S32_PSR_S;
	}
	else {
		psr &= ~S32_PSR_S;
	}

	psr |= S32_PSR_ET;

	s32_set_psr (c, psr);
}

/* 2 3A: ticc rs1, reg_or_imm */
static
void op23a (sparc32_t *c)
{
	unsigned cond;
	uint32_t s1, s2;

	cond = (c->ir >> 25) & 0x0f;

	if (s32_check_icc (c, cond)) {
		s1 = s32_get_rs1 (c, c->ir);
		s2 = s32_get_rs2_or_simm13 (c, c->ir);

		s32_trap (c, 128 + ((s1 + s2) & 0x7f));

		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 2 3C: save rs1, reg_or_imm, rd */
static
void op23c (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	if (s32_save (c, 1)) {
		s32_trap (c, S32_TRAP_WINDOW_OVERFLOW);
		return;
	}

	d = (s1 + s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 2 3D: restore rs1, reg_or_imm, rd */
static
void op23d (sparc32_t *c)
{
	uint32_t d, s1, s2;

	s1 = s32_get_rs1 (c, c->ir);
	s2 = s32_get_rs2_or_simm13 (c, c->ir);

	if (s32_restore (c, 1)) {
		s32_trap (c, S32_TRAP_WINDOW_UNDERFLOW);
		return;
	}

	d = (s1 + s2) & 0xffffffffUL;

	s32_set_rd (c, c->ir, d);

	s32_set_clk (c, 4, 1);
}

/* 3 00: ld [rs1 + reg_or_imm], rd */
static
void op300 (sparc32_t *c)
{
	uint32_t addr;
	uint32_t val;

	addr = s32_get_addr (c, c->ir);

	if (addr & 3) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload32 (c, addr, c->asi_data, &val)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xffffffffUL);

	s32_set_clk (c, 4, 1);
}

/* 3 01: ldub [rs1 + reg_or_imm], rd */
static
void op301 (sparc32_t *c)
{
	uint32_t addr;
	uint8_t  val;

	addr = s32_get_addr (c, c->ir);

	if (s32_dload8 (c, addr, c->asi_data, &val)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xff);

	s32_set_clk (c, 4, 1);
}

/* 3 02: lduh [rs1 + reg_or_imm], rd */
static
void op302 (sparc32_t *c)
{
	uint32_t addr;
	uint16_t val;

	addr = s32_get_addr (c, c->ir);

	if (addr & 1) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload16 (c, addr, c->asi_data, &val)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xffffU);

	s32_set_clk (c, 4, 1);
}

/* 3 03: ldd [rs1 + reg_or_imm], rd */
static
void op303 (sparc32_t *c)
{
	unsigned rd;
	uint32_t addr;
	uint32_t val1, val2;

	addr = s32_get_addr (c, c->ir);

	if (addr & 7) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	rd = s32_ir_rd (c->ir);

	if (rd & 1) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	if (s32_dload32 (c, addr, c->asi_data, &val1)) {
		return;
	}

	if (s32_dload32 (c, addr + 4, c->asi_data, &val2)) {
		return;
	}

	s32_set_gpr (c, rd + 0, val1 & 0xffffffff);
	s32_set_gpr (c, rd + 1, val2 & 0xffffffff);

	s32_set_clk (c, 4, 2);
}

/* 3 04: st rd, [rs1 + reg_or_imm] */
static
void op304 (sparc32_t *c)
{
	uint32_t addr;

	addr = s32_get_addr (c, c->ir);
	if (addr & 3) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dstore32 (c, addr, c->asi_data, s32_get_rd (c, c->ir))) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 05: stb rd, [rs1 + reg_or_imm] */
static
void op305 (sparc32_t *c)
{
	uint32_t addr;

	addr = s32_get_addr (c, c->ir);

	if (s32_dstore8 (c, addr, c->asi_data, s32_get_rd (c, c->ir) & 0xff)) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 06: sth rd, [rs1 + reg_or_imm] */
static
void op306 (sparc32_t *c)
{
	uint32_t addr;

	addr = s32_get_addr (c, c->ir);
	if (addr & 1) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dstore16 (c, addr, c->asi_data, s32_get_rd (c, c->ir) & 0xffffU)) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 07: std rd, [rs1 + reg_or_imm] */
static
void op307 (sparc32_t *c)
{
	unsigned rd;
	uint32_t addr;

	addr = s32_get_addr (c, c->ir);
	if (addr & 7) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	rd = s32_ir_rd (c->ir);
	if (rd & 1) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	if (s32_dstore32 (c, addr, c->asi_data, s32_get_gpr (c, rd))) {
		return;
	}

	if (s32_dstore32 (c, addr + 4, c->asi_data, s32_get_gpr (c, rd + 1))) {
		return;
	}

	s32_set_clk (c, 4, 2);
}

/* 3 09: ldsb [rs1 + reg_or_imm], rd */
static
void op309 (sparc32_t *c)
{
	uint32_t addr;
	uint8_t  val;

	addr = s32_get_addr (c, c->ir);

	if (s32_dload8 (c, addr, c->asi_data, &val)) {
		return;
	}

	s32_set_rd (c, c->ir, s32_exts (val, 8));

	s32_set_clk (c, 4, 1);
}

/* 3 0A: ldsh [rs1 + reg_or_imm], rd */
static
void op30a (sparc32_t *c)
{
	uint32_t addr;
	uint16_t val;

	addr = s32_get_addr (c, c->ir);

	if (addr & 1) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload16 (c, addr, c->asi_data, &val)) {
		return;
	}

	s32_set_rd (c, c->ir, s32_exts (val, 16));

	s32_set_clk (c, 4, 1);
}

/* 3 0D: ldstub [rs1 + reg_or_imm], rd */
static
void op30d (sparc32_t *c)
{
	uint32_t addr;
	uint8_t  val;

	addr = s32_get_addr (c, c->ir);

	if (s32_dload8 (c, addr, c->asi_data, &val)) {
		return;
	}

	if (s32_dstore8 (c, addr, c->asi_data, 0xff)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xff);

	s32_set_clk (c, 4, 1);
}

/* 3 0F: swap [rs1 + reg_or_imm], rd */
static
void op30f (sparc32_t *c)
{
	uint32_t addr;
	uint32_t val;

	addr = s32_get_addr (c, c->ir);
	if (addr & 3) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload32 (c, addr, c->asi_data, &val)) {
		return;
	}

	if (s32_dstore32 (c, addr, c->asi_data, s32_get_rd (c, c->ir))) {
		return;
	}

	s32_set_rd (c, c->ir, val);

	s32_set_clk (c, 4, 2);
}

/* 3 10: lda [rs1 + rs2]asi, rd */
static
void op310 (sparc32_t *c)
{
	uint32_t addr;
	uint32_t val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (addr & 3) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload32 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xffffffffUL);

	s32_set_clk (c, 4, 1);
}

/* 3 11: lduba [rs1 + rs2]asi, rd */
static
void op311 (sparc32_t *c)
{
	uint32_t addr;
	uint8_t  val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (s32_dload8 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xff);

	s32_set_clk (c, 4, 1);
}

/* 3 12: lduha [rs1 + rs2]asi, rd */
static
void op312 (sparc32_t *c)
{
	uint32_t addr;
	uint16_t val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (addr & 1) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload16 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xffffU);

	s32_set_clk (c, 4, 1);
}

/* 3 13: ldda [rs1 + rs2]asi, rd */
static
void op313 (sparc32_t *c)
{
	unsigned rd;
	uint32_t addr;
	uint32_t val1, val2;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);
	if (addr & 7) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	rd = s32_ir_rd (c->ir);
	if (rd & 1) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	if (s32_dload32 (c, addr, s32_ir_asi (c->ir), &val1)) {
		return;
	}

	if (s32_dload32 (c, addr + 4, s32_ir_asi (c->ir), &val2)) {
		return;
	}

	s32_set_gpr (c, rd + 0, val1 & 0xffffffff);
	s32_set_gpr (c, rd + 1, val2 & 0xffffffff);

	s32_set_clk (c, 4, 1);
}

/* 3 14: sta rd, [rs1 + rs2]asi */
static
void op314 (sparc32_t *c)
{
	uint32_t addr;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);
	if (addr & 3) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dstore32 (c, addr, s32_ir_asi (c->ir), s32_get_rd (c, c->ir))) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 15: stba rd, [rs1 + rs2]asi */
static
void op315 (sparc32_t *c)
{
	uint32_t addr;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (s32_dstore8 (c, addr, s32_ir_asi (c->ir), s32_get_rd (c, c->ir) & 0xff)) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 16: stha rd, [rs1 + rs2]asi */
static
void op316 (sparc32_t *c)
{
	uint32_t addr;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);
	if (addr & 1) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dstore16 (c, addr, s32_ir_asi (c->ir), s32_get_rd (c, c->ir) & 0xffffU)) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 17: stda rd, [rs1 + rs2]asi */
static
void op317 (sparc32_t *c)
{
	unsigned rd;
	uint32_t addr;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);
	if (addr & 7) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	rd = s32_ir_rd (c->ir);
	if (rd & 1) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	if (s32_dstore32 (c, addr, s32_ir_asi (c->ir), s32_get_gpr (c, rd))) {
		return;
	}

	if (s32_dstore32 (c, addr + 4, s32_ir_asi (c->ir), s32_get_gpr (c, rd + 1))) {
		return;
	}

	s32_set_clk (c, 4, 1);
}

/* 3 19: ldsba [rs1 + rs2]asi, rd */
static
void op319 (sparc32_t *c)
{
	uint32_t addr;
	uint8_t  val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (s32_dload8 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	s32_set_rd (c, c->ir, s32_exts (val, 8));

	s32_set_clk (c, 4, 1);
}

/* 3 1A: ldsha [rs1 + rs2]asi, rd */
static
void op31a (sparc32_t *c)
{
	uint32_t addr;
	uint16_t val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (addr & 1) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload16 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	s32_set_rd (c, c->ir, s32_exts (val, 16));

	s32_set_clk (c, 4, 1);
}

/* 3 1D: ldstuba [rs1 + rs2]asi, rd */
static
void op31d (sparc32_t *c)
{
	uint32_t addr;
	uint8_t  val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);

	if (s32_dload8 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	if (s32_dstore8 (c, addr, s32_ir_asi (c->ir), 0xff)) {
		return;
	}

	s32_set_rd (c, c->ir, val & 0xff);

	s32_set_clk (c, 4, 1);
}

/* 3 1F: swapa [rs1 + rs2]asi, rd */
static
void op31f (sparc32_t *c)
{
	uint32_t addr;
	uint32_t val;

	if (s32_check_privilege (c)) {
		return;
	}

	if (s32_ir_i (c->ir)) {
		s32_trap (c, S32_TRAP_ILLEGAL_INSTRUCTION);
		return;
	}

	addr = s32_get_addr (c, c->ir);
	if (addr & 3) {
		s32_trap (c, S32_TRAP_MEM_ADDRESS_NOT_ALIGNED);
		return;
	}

	if (s32_dload32 (c, addr, s32_ir_asi (c->ir), &val)) {
		return;
	}

	if (s32_dstore32 (c, addr, s32_ir_asi (c->ir), s32_get_rd (c, c->ir))) {
		return;
	}

	s32_set_rd (c, c->ir, val);

	s32_set_clk (c, 4, 1);
}

/* 3 20: ldf [rs1 + reg_or_imm], rd */
static
void op320 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 3 21: ldf [rs1 + reg_or_imm], %fsr */
static
void op321 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 3 23: lddf [rs1 + reg_or_imm], rd */
static
void op323 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 3 24: stf rd, [rs1 + reg_or_imm] */
static
void op324 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 3 25: stf %fsr, [rs1 + reg_or_imm] */
static
void op325 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 3 27: stdf rd, [rs1 + reg_or_imm] */
static
void op327 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_FP_DISABLED);
}

/* 3 30: ldc [rs1 + reg_or_imm], rd */
static
void op330 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 3 31: ldcsr [rs1 + reg_or_imm], %csr */
static
void op331 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 3 33: lddc [rs1 + reg_or_imm], rd */
static
void op333 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 3 34: stc rd, [rs1 + reg_or_imm] */
static
void op334 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 3 35: stc %csr, [rs1 + reg_or_imm] */
static
void op335 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}

/* 3 37: stdc rd, [rs1 + reg_or_imm] */
static
void op337 (sparc32_t *c)
{
	s32_trap (c, S32_TRAP_CP_DISABLED);
}


static
s32_opcode_f s32_opcodes[4][64] = {
	{
		op000, NULL,  op002, NULL,  op004, NULL,  op006, op007,  /* 0 00 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 0 10 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 0 20 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 0 30 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL
	},
	{
		op100, NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 1 00 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 1 10 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 1 20 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,   /* 1 30 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL
	},
	{
		op200, op201, op202, op203, op204, op205, op206, op207,  /* 2 00 */
		op208, NULL,  op20a, op20b, op20c, NULL,  op20e, op20f,
		op210, op211, op212, op213, op214, op215, op216, op217,  /* 2 10 */
		op218, NULL,  op21a, op21b, op21c, NULL,  op21e, op21f,
		op220, op221, op222, op223, op224, op225, op226, op227,  /* 2 20 */
		op228, op229, op22a, op22b, NULL,  NULL,  NULL,  NULL,
		op230, op231, op232, op233, op234, op235, op236, op237,  /* 2 30 */
		op238, op239, op23a, NULL,  op23c, op23d, NULL,  NULL
	},
	{
		op300, op301, op302, op303, op304, op305, op306, op307,  /* 3 00 */
		NULL,  op309, op30a, NULL,  NULL,  op30d, NULL,  op30f,
		op310, op311, op312, op313, op314, op315, op316, op317,  /* 3 10 */
		NULL,  op319, op31a, NULL,  NULL,  op31d, NULL,  op31f,
		op320, op321, NULL,  op323, op324, op325, NULL,  op327,  /* 3 20 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
		op330, op331, NULL,  op333, op334, op335, NULL,  op337,  /* 3 30 */
		NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL
	}
};


void s32_set_opcodes (sparc32_t *c)
{
	unsigned i, j;

	for (j = 0; j < 4; j++) {
		for (i = 0; i < 64; i++) {
			if (s32_opcodes[j][i] != NULL) {
				c->opcodes[j][i] = s32_opcodes[j][i];
			}
			else {
				c->opcodes[j][i] = &s32_op_undefined;
			}
		}
	}
}
