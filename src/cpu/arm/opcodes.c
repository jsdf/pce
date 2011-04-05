/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/opcodes.c                                        *
 * Created:     2004-11-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#include "arm.h"
#include "internal.h"


/*
 * Count the number of 1 bits in v.
 */
static
unsigned arm_bitcnt32 (unsigned long v)
{
	unsigned n;

	n = 0;
	while (v != 0) {
		n += 1;
		v &= v - 1;
	}

	return (n);
}

/*
 * Write a new value to CPSR and adjust the register mapping and execution
 * mode.
 */
int arm_write_cpsr (arm_t *c, uint32_t val, int prvchk)
{
	if (prvchk) {
		if (arm_get_cpsr_m (c) == ARM_MODE_USR) {
			return (1);
		}
	}

	arm_set_reg_map (c, val & 0x1f);
	arm_set_cpsr (c, val);

	c->privileged = ((val & 0x1f) != ARM_MODE_USR);

	arm_tbuf_flush (c);

	return (0);
}

/*
 * Set the N and Z condition codes according to a 32 bit result
 */
static
void arm_set_cc_nz (arm_t *c, uint32_t val)
{
	uint32_t cc;

	val &= 0xffffffff;
	cc = 0;

	if (val == 0) {
		cc |= ARM_PSR_Z;
	}
	else if (val & 0x80000000) {
		cc |= ARM_PSR_N;
	}

	c->cpsr &= ~(ARM_PSR_Z | ARM_PSR_N);
	c->cpsr |= cc;
}

/*
 * Set the condition codes after an addition
 */
static
void arm_set_cc_add (arm_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	uint32_t cc;

	cc = 0;

	if (d == 0) {
		cc |= ARM_PSR_Z;
	}
	else if (d & 0x80000000) {
		cc |= ARM_PSR_N;
	}

	if ((d < s1) || ((d == s1) && (s2 != 0))) {
		cc |= ARM_PSR_C;
	}

	if ((d ^ s1) & (d ^ s2) & 0x80000000) {
		cc |= ARM_PSR_V;
	}

	c->cpsr &= ~(ARM_PSR_Z | ARM_PSR_N | ARM_PSR_C | ARM_PSR_V);
	c->cpsr |= cc;
}

/*
 * Set the condition codes after a subtraction
 */
static
void arm_set_cc_sub (arm_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	uint32_t cc;

	cc = 0;

	if (d == 0) {
		cc |= ARM_PSR_Z;
	}
	else if (d & 0x80000000) {
		cc |= ARM_PSR_N;
	}

	if (!((d > s1) || ((d == s1) && (s2 != 0)))) {
		cc |= ARM_PSR_C;
	}

	if ((d ^ s1) & (s1 ^ s2) & 0x80000000) {
		cc |= ARM_PSR_V;
	}

	c->cpsr &= ~(ARM_PSR_Z | ARM_PSR_N | ARM_PSR_C | ARM_PSR_V);
	c->cpsr |= cc;
}

static
uint32_t arm_set_psr_field (uint32_t psr, uint32_t val, unsigned fld)
{
	if (fld & 0x01) {
		psr &= 0xffffff00;
		psr |= val & 0x000000ff;
	}

	if (fld & 0x02) {
		psr &= 0xffff00ff;
		psr |= val & 0x0000ff00;
	}

	if (fld & 0x04) {
		psr &= 0xff00ffff;
		psr |= val & 0x00ff0000;
	}

	if (fld & 0x08) {
		psr &= 0x00ffffff;
		psr |= val & 0xff000000;
	}

	return (psr);
}

/*
 * Check if condition cond is met
 */
int arm_check_cond (arm_t *c, unsigned cond)
{
	switch (cond & 0x0f) {
	case 0x00: /* eq */
		return (arm_get_cc_z (c));

	case 0x01: /* ne */
		return (!arm_get_cc_z (c));

	case 0x02: /* cs */
		return (arm_get_cc_c (c));

	case 0x03: /* cc */
		return (!arm_get_cc_c (c));

	case 0x04: /* mi */
		return (arm_get_cc_n (c));

	case 0x05: /* pl */
		return (!arm_get_cc_n (c));

	case 0x06: /* vs */
		return (arm_get_cc_v (c));

	case 0x07: /* vc */
		return (!arm_get_cc_v (c));

	case 0x08: /* hi */
		return (arm_get_cc_c (c) && !arm_get_cc_z (c));

	case 0x09: /* ls */
		return (!arm_get_cc_c (c) || arm_get_cc_z (c));

	case 0x0a: /* ge */
		return (arm_get_cc_n (c) == arm_get_cc_v (c));

	case 0x0b: /* lt */
		return (arm_get_cc_n (c) != arm_get_cc_v (c));

	case 0x0c: /* gt */
		return (!arm_get_cc_z (c) && (arm_get_cc_n (c) == arm_get_cc_v (c)));

	case 0x0d: /* le */
		return (arm_get_cc_z (c) || (arm_get_cc_n (c) != arm_get_cc_v (c)));

	case 0x0e: /* al */
		return (1);

	case 0x0f: /* nv */
		return (0);
	}

	return (0);
}

static
uint32_t arm_get_sh (arm_t *c, uint32_t ir, uint32_t *cry)
{
	unsigned n;
	uint32_t v, tmp;

	if (cry == NULL) {
		cry = &tmp;
	}

	if (arm_get_bit (ir, 25)) {
		/* 32 bit immediate */

		v = (ir & 0xff);
		n = (ir >> 7) & 0x1e;

		if (n == 0) {
			*cry = arm_get_cc_c (c);
		}
		else {
			v = ((v >> n) | (v << (32 - n))) & 0xffffffff;
			*cry = ((v & 0x80000000) != 0);
		}

		return (v);
	}

	v = arm_get_rm (c, c->ir);

	if (arm_get_bit (ir, 4)) {
		/* register shifts */

		if (arm_get_bit (ir, 7)) {
			/* extension */
			*cry = 0;
			return (0);
		}

		n = arm_get_rs (c, c->ir) & 0xff;

		switch (arm_get_bits (ir, 5, 2)) {
		case 0x00: /* lsl */
			if (n == 0) {
				*cry = arm_get_cc_c (c);
			}
			else if (n <= 32) {
				*cry = (v >> (32 - n)) & 0x01;
				v = (n < 32) ? (v << n) : 0;
			}
			else {
				*cry = 0;
				v = 0;
			}
			return (v);

		case 0x01: /* lsr */
			if (n == 0) {
				*cry = arm_get_cc_c (c);
			}
			else if (n <= 32) {
				*cry = (v >> (n - 1)) & 0x01;
				v = (n < 32) ? (v >> n) : 0;
			}
			else {
				*cry = 0;
				v = 0;
			}
			return (v);

		case 0x02: /* asr */
			if (n == 0) {
				*cry = arm_get_cc_c (c);
			}
			else if (n < 32) {
				*cry = (v >> (n - 1)) & 0x01;
				if (v & 0x80000000) {
					v = v >> n;
					v |= (0xffffffff << (32 - n)) & 0xffffffff;
				}
				else {
					v = v >> n;
				}
			}
			else {
				*cry = (v >> 31) & 0x01;
				v = *cry ? 0xffffffffUL : 0x00000000UL;
			}
			return (v);

		case 0x03: /* ror */
			if (n == 0) {
				*cry = arm_get_cc_c (c);
			}
			else if ((n & 0x1f) == 0) {
				*cry = (v >> 31) & 0x01;
			}
			else {
				n &= 0x1f;
				*cry = (v >> (n - 1)) & 0x01;
				v = (v >> n) | (v << (32 - n));
			}
			return (v);
		}
	}
	else {
		/* immediate shifts */

		n = arm_get_bits (ir, 7, 5);

		switch (arm_get_bits (ir, 5, 2)) {
		case 0x00: /* lsl */
			if (n == 0) {
				*cry = arm_get_cc_c (c);
			}
			else {
				*cry = (v >> (32 - n)) & 0x01;
				v = (v << n) & 0xffffffff;
			}
			return (v);

		case 0x01: /* lsr */
			if (n == 0) {
				*cry = (v >> 31) & 0x01;
				v = 0;
			}
			else {
				*cry = (v >> (n - 1)) & 0x01;
				v = v >> n;
			}
			return (v);

		case 0x02: /* asr */
			if (n == 0) {
				*cry = (v >> 31) & 0x01;
				v = *cry ? 0xffffffffUL : 0x00000000UL;
			}
			else {
				*cry = (v >> (n - 1)) & 0x01;
				v = arm_asr32 (v, n);
			}
			return (v);

		case 0x03: /* ror */
			if (n == 0) {
				*cry = v & 0x01;
				v = (v >> 1) | (arm_get_cc_c (c) << 31);
			}
			else {
				*cry = (v >> (n - 1)) & 0x01;
				v = (v >> n) | (v << (32 - n));
			}
			return (v);
		}
	}

	*cry = 0;

	return (0);
}


void op_undefined (arm_t *c)
{
	if (c->log_undef != NULL) {
		c->log_undef (c->log_ext, c->ir);
	}

	arm_set_clk (c, 0, 1);

	arm_exception_undefined (c);
}

/* 00 09: mul[cond] rn, rm, rs */
static
void op00_09 (arm_t *c)
{
	uint32_t d, s1, s2;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	d = (s1 * s2) & 0xffffffff;

	arm_set_rn (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 00 0b: str[cond][h|sh|sb|d] rd, addressing_mode */
static
void op00_0b (arm_t *c)
{
	int      p, u, w, s, h;
	uint32_t val, base, index;

	p = arm_get_bit (c->ir, 24);
	u = arm_get_bit (c->ir, 23);
	w = arm_get_bit (c->ir, 21);
	s = arm_get_bit (c->ir, 6);
	h = arm_get_bit (c->ir, 5);

	base = arm_get_rn (c, c->ir);

	/* get index */
	if (arm_get_bit (c->ir, 22)) {
		index = arm_get_bits (c->ir, 8, 4) << 4;
		index |= arm_get_bits (c->ir, 0, 4);
	}
	else {
		index = arm_get_rm (c, c->ir);
	}

	/* pre-index */
	if (p) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	val = arm_get_rd (c, c->ir);

	if (s) {
		/* ldrd/strd */

		if (arm_ir_rd (c->ir) & 1) {
			op_undefined (c);
			return;
		}

		base &= 0xfffffff8;

		if (h) {
			if (arm_dstore32 (c, base, val)) {
				return;
			}

			val = arm_get_gpr (c, (arm_ir_rd (c->ir) + 1) & 0x0f);

			if (arm_dstore32 (c, base + 4, val)) {
				return;
			}
		}
		else {
			uint32_t val2;

			if (arm_dload32 (c, base, &val)) {
				return;
			}

			if (arm_dload32 (c, base + 4, &val2)) {
				return;
			}

			arm_set_rd (c, c->ir, val);
			arm_set_gpr (c, (arm_ir_rd (c->ir) + 1) & 0xff, val2);
		}
	}
	else if (h) {
		if (arm_dstore16 (c, base & 0xfffffffe, val)) {
			return;
		}
	}
	else {
		if (arm_dstore8 (c, base, val)) {
			return;
		}
	}

	/* post-index */
	if (p == 0) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	/* base register writeback */
	if ((p == 0) || (w == 1)) {
		arm_set_rn (c, c->ir, base);
	}

	arm_set_clk (c, 4, 1);
}

/* 00 ext */
static
void op00_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op00_09 (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 00: and[cond] rd, rn, shifter_operand */
static
void op00 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op00_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = s1 & s2;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 01 09: mul[cond]s rn, rm, rs */
static
void op01_09 (arm_t *c)
{
	uint32_t d, s1, s2;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	d = (s1 * s2) & 0xffffffff;

	arm_set_rn (c, c->ir, d);
	arm_set_cc_nz (c, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 01 0b: ldr[cond][h|sh|sb] rd, addressing_mode */
static
void op01_0b (arm_t *c)
{
	int      p, u, w, s, h;
	uint32_t val, base, index;

	p = arm_get_bit (c->ir, 24);
	u = arm_get_bit (c->ir, 23);
	w = arm_get_bit (c->ir, 21);
	s = arm_get_bit (c->ir, 6);
	h = arm_get_bit (c->ir, 5);

	base = arm_get_rn (c, c->ir);

	/* get index */
	if (arm_get_bit (c->ir, 22)) {
		index = arm_get_bits (c->ir, 8, 4) << 4;
		index |= arm_get_bits (c->ir, 0, 4);
	}
	else {
		index = arm_get_rm (c, c->ir);
	}

	/* pre-index */
	if (p) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	if (h) {
		uint16_t tmp;

		if (arm_dload16 (c, base & 0xfffffffe, &tmp)) {
			return;
		}

		if (s) {
			val = arm_exts (tmp, 16);
		}
		else {
			val = arm_extu (tmp, 16);
		}
	}
	else {
		uint8_t tmp;

		if (arm_dload8 (c, base, &tmp)) {
			return;
		}

		if (s) {
			val = arm_exts (tmp, 8);
		}
		else {
			val = arm_extu (tmp, 8);
		}
	}

	arm_set_rd (c, c->ir, val);

	/* post-index */
	if (p == 0) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	/* base register writeback */
	if ((p == 0) || (w == 1)) {
		arm_set_rn (c, c->ir, base);
	}

	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 01 ext */
static
void op01_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op01_09 (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 01: and[cond]s rd, rn, shifter_operand */
static
void op01 (arm_t *c)
{
	uint32_t s1, s2, d, shc;

	if (arm_is_shext (c->ir)) {
		op01_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, &shc);

	d = s1 & s2;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_nz (c, d);
		arm_set_cc_c (c, shc);
		arm_set_clk (c, 4, 1);
	}
}

/* 02 09: mla[cond] rn, rm, rs, rd */
static
void op02_09 (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);
	s3 = arm_get_rd (c, c->ir);

	d = (s1 * s2 + s3) & 0xffffffff;

	arm_set_rn (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 02 ext */
static
void op02_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op02_09 (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 02: eor[cond] rd, rn, shifter_operand */
static
void op02 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op02_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = s1 ^ s2;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 03 09: mla[cond]s rn, rm, rs, rd */
static
void op03_09 (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);
	s3 = arm_get_rd (c, c->ir);

	d = (s1 * s2 + s3) & 0xffffffff;

	arm_set_rn (c, c->ir, d);
	arm_set_cc_nz (c, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 03 ext */
static
void op03_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op03_09 (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 03: eor[cond]s rd, rn, shifter_operand */
static
void op03 (arm_t *c)
{
	uint32_t s1, s2, d, shc;

	if (arm_is_shext (c->ir)) {
		op03_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, &shc);

	d = s1 ^ s2;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_nz (c, d);
		arm_set_cc_c (c, shc);
		arm_set_clk (c, 4, 1);
	}
}

/* 04 ext */
static
void op04_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 04: sub[cond] rd, rn, shifter_operand */
static
void op04 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op04_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = (s1 - s2) & 0xffffffff;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 05 ext */
static
void op05_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 05: sub[cond]s rd, rn, shifter_operand */
static
void op05 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op05_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = (s1 - s2) & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_sub (c, d, s1, s2);
		arm_set_clk (c, 4, 1);
	}
}

/* 06 ext */
static
void op06_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 06: rsb[cond] rd, rn, shifter_operand */
static
void op06 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op06_ext (c);
		return;
	}

	s2 = arm_get_rn (c, c->ir);
	s1 = arm_get_sh (c, c->ir, NULL);

	d = (s1 - s2) & 0xffffffff;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 07 ext */
static
void op07_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 07: rsb[cond]s rd, rn, shifter_operand */
static
void op07 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op07_ext (c);
		return;
	}

	s2 = arm_get_rn (c, c->ir);
	s1 = arm_get_sh (c, c->ir, NULL);

	d = (s1 - s2) & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_sub (c, d, s1, s2);
		arm_set_clk (c, 4, 1);
	}
}

/* 08 09: umull[cond] rdlo, rdhi, rm, rs */
static
void op08_09 (arm_t *c)
{
	uint32_t d1, d2, s1, s2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	d = (uint64_t) s1 * (uint64_t) s2;
	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rn (c, c->ir, d2);
	arm_set_rd (c, c->ir, d1);

	arm_set_clk (c, 4, 1);
}

/* 08 ext */
static
void op08_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op08_09 (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 08: add[cond] rd, rn, shifter_operand */
static
void op08 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op08_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = (s1 + s2) & 0xffffffffUL;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 09 09: umull[cond]s rdlo, rdhi, rm, rs */
static
void op09_09 (arm_t *c)
{
	uint32_t d1, d2, s1, s2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	d = (uint64_t) s1 * (uint64_t) s2;
	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rn (c, c->ir, d2);
	arm_set_rd (c, c->ir, d1);
	arm_set_cc_nz (c, d2);

	arm_set_clk (c, 4, 1);
}

/* 09 ext */
static
void op09_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op09_09 (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 09: add[cond]s rd, rn, shifter_operand */
static
void op09 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op09_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = (s1 + s2) & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_add (c, d, s1, s2);
		arm_set_clk (c, 4, 1);
	}
}

/* 0A 09: umlal[cond] rdlo, rdhi, rm, rs */
static
void op0a_09 (arm_t *c)
{
	uint32_t s1, s2, d1, d2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);
	d1 = arm_get_rd (c, c->ir);
	d2 = arm_get_rn (c, c->ir);

	d = (uint64_t) s1 * (uint64_t) s2;

	d += d1;
	d += (uint64_t) d2 << 32;

	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rd (c, c->ir, d1);
	arm_set_rn (c, c->ir, d2);

	arm_set_clk (c, 4, 1);
}

/* 0A ext */
static
void op0a_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op0a_09 (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 0A: adc[cond] rd, rn, shifter_operand */
static
void op0a (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	if (arm_is_shext (c->ir)) {
		op0a_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);
	s3 = arm_get_cc_c (c);

	d = (s1 + s2 + s3) & 0xffffffffUL;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 0B 09: umlal[cond]s rdlo, rdhi, rm, rs */
static
void op0b_09 (arm_t *c)
{
	uint32_t s1, s2, d1, d2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);
	d1 = arm_get_rd (c, c->ir);
	d2 = arm_get_rn (c, c->ir);

	d = (uint64_t) s1 * (uint64_t) s2;

	d += d1;
	d += (uint64_t) d2 << 32;

	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rd (c, c->ir, d1);
	arm_set_rn (c, c->ir, d2);
	arm_set_cc_nz (c, d2);

	arm_set_clk (c, 4, 1);
}

/* 0B ext */
static
void op0b_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op0b_09 (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 0B: adc[cond]s rd, rn, shifter_operand */
static
void op0b (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	if (arm_is_shext (c->ir)) {
		op0b_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);
	s3 = arm_get_cc_c (c);

	d = (s1 + s2 + s3) & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_add (c, d, s1, s2);
		arm_set_clk (c, 4, 1);
	}
}

/* 0C 09: smull[cond] rdlo, rdhi, rm, rs */
static
void op0c_09 (arm_t *c)
{
	int      sign;
	uint32_t d1, d2, s1, s2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	sign = ((s1 ^ s2) & 0x80000000) != 0;

	s1 = ((s1 & 0x80000000) ? (~s1 + 1) : s1) & 0xffffffff;
	s2 = ((s2 & 0x80000000) ? (~s2 + 1) : s2) & 0xffffffff;

	d = (uint64_t) s1 * (uint64_t) s2;

	if (sign) {
		d = ~d + 1;
	}

	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rd (c, c->ir, d1);
	arm_set_rn (c, c->ir, d2);

	arm_set_clk (c, 4, 1);
}

/* 0C ext */
static
void op0c_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op0c_09 (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 0C: sbc[cond] rd, rn, shifter_operand */
static
void op0c (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	if (arm_is_shext (c->ir)) {
		op0c_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);
	s3 = !arm_get_cc_c (c);

	d = (s1 - s2 - s3) & 0xffffffff;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 0D 09: smull[cond]s rdlo, rdhi, rm, rs */
static
void op0d_09 (arm_t *c)
{
	int      sign;
	uint32_t d1, d2, s1, s2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	sign = ((s1 ^ s2) & 0x80000000) != 0;

	s1 = ((s1 & 0x80000000) ? (~s1 + 1) : s1) & 0xffffffff;
	s2 = ((s2 & 0x80000000) ? (~s2 + 1) : s2) & 0xffffffff;

	d = (uint64_t) s1 * (uint64_t) s2;

	if (sign) {
		d = ~d + 1;
	}

	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rd (c, c->ir, d1);
	arm_set_rn (c, c->ir, d2);
	arm_set_cc_nz (c, d2);

	arm_set_clk (c, 4, 1);
}

/* 0D ext */
static
void op0d_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op0d_09 (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 0D: sbc[cond]s rd, rn, shifter_operand */
static
void op0d (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	if (arm_is_shext (c->ir)) {
		op0d_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);
	s3 = !arm_get_cc_c (c);

	d = (s1 - s2 - s3) & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_sub (c, d, s1, s2);
		arm_set_clk (c, 4, 1);
	}
}

/* 0E 09: smlal[cond] rdlo, rdhi, rm, rs */
static
void op0e_09 (arm_t *c)
{
	int      sign;
	uint32_t s1, s2, d1, d2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);
	d1 = arm_get_rd (c, c->ir);
	d2 = arm_get_rn (c, c->ir);

	sign = ((s1 ^ s2) & 0x80000000) != 0;

	s1 = ((s1 & 0x80000000) ? (~s1 + 1) : s1) & 0xffffffff;
	s2 = ((s2 & 0x80000000) ? (~s2 + 1) : s2) & 0xffffffff;

	d = (uint64_t) s1 * (uint64_t) s2;

	if (sign) {
		d = ~d + 1;
	}

	d += d1;
	d += (uint64_t) d2 << 32;

	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rd (c, c->ir, d1);
	arm_set_rn (c, c->ir, d2);

	arm_set_clk (c, 4, 1);
}

/* 0E ext */
static
void op0e_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op0e_09 (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 0E: rsc[cond] rd, rn, shifter_operand */
static
void op0e (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	if (arm_is_shext (c->ir)) {
		op0e_ext (c);
		return;
	}

	s2 = arm_get_rn (c, c->ir);
	s1 = arm_get_sh (c, c->ir, NULL);
	s3 = !arm_get_cc_c (c);

	d = (s1 - s2 - s3) & 0xffffffffUL;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 0F 09: smlal[cond]s rdlo, rdhi, rm, rs */
static
void op0f_09 (arm_t *c)
{
	int      sign;
	uint32_t s1, s2, d1, d2;
	uint64_t d;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);
	d1 = arm_get_rd (c, c->ir);
	d2 = arm_get_rn (c, c->ir);

	sign = ((s1 ^ s2) & 0x80000000) != 0;

	s1 = ((s1 & 0x80000000) ? (~s1 + 1) : s1) & 0xffffffff;
	s2 = ((s2 & 0x80000000) ? (~s2 + 1) : s2) & 0xffffffff;

	d = (uint64_t) s1 * (uint64_t) s2;

	if (sign) {
		d = ~d + 1;
	}

	d += d1;
	d += (uint64_t) d2 << 32;

	d1 = d & 0xffffffff;
	d2 = (d >> 32) & 0xffffffff;

	arm_set_rd (c, c->ir, d1);
	arm_set_rn (c, c->ir, d2);
	arm_set_cc_nz (c, d2);

	arm_set_clk (c, 4, 1);
}

/* 0F ext */
static
void op0f_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op0f_09 (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 0F: rsc[cond]s rd, rn, shifter_operand */
static
void op0f (arm_t *c)
{
	uint32_t s1, s2, s3, d;

	if (arm_is_shext (c->ir)) {
		op0f_ext (c);
		return;
	}

	s2 = arm_get_rn (c, c->ir);
	s1 = arm_get_sh (c, c->ir, NULL);
	s3 = !arm_get_cc_c (c);

	d = (s1 - s2 - s3) & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_sub (c, d, s1, s2);
		arm_set_clk (c, 4, 1);
	}
}

/* 10 00: mrs[cond] rd, cpsr */
static
void op10_00 (arm_t *c)
{
	arm_set_rd (c, c->ir, arm_get_cpsr (c));
	arm_set_clk (c, 4, 1);
}

/* 10 09: swp[cond] rd, rm, rn */
static
void op10_09 (arm_t *c)
{
	uint32_t addr, d, s;

	addr = arm_get_rn (c, c->ir);
	s = arm_get_rm (c, c->ir);

	if (arm_dload32 (c, addr & 0xfffffffc, &d)) {
		return;
	}

	if (addr & 0x03) {
		d = arm_ror32 (d, (addr & 0x03) << 3);
	}

	if (arm_dstore32 (c, addr & 0xfffffffc, s)) {
		return;
	}

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 10 */
static
void op10 (arm_t *c)
{
	switch (arm_get_bits (c->ir, 4, 4)) {
	case 0x00:
		op10_00 (c);
		break;

	case 0x09:
		op10_09 (c);
		break;

	case 0x0b:
	case 0x0d:
	case 0x0f:
		op00_0b (c);
		break;

	default:
		op_undefined (c);
		break;
	}
}

/* 11 ext */
static
void op11_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 11: tst[cond] rn, shifter_operand */
static
void op11 (arm_t *c)
{
	uint32_t s1, s2, d, shc;

	if (arm_is_shext (c->ir)) {
		op11_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, &shc);

	d = s1 & s2;

	arm_set_cc_nz (c, d);
	arm_set_cc_c (c, shc);
	arm_set_clk (c, 4, 1);
}

/* 12 00: msr[cond] fields, rm */
static
void op12_00 (arm_t *c)
{
	unsigned fld;
	uint32_t val;

	fld = arm_get_bits (c->ir, 16, 4);

	if (arm_is_privileged (c) == 0) {
		fld &= 0x08;
	}

	val = arm_get_rm (c, c->ir);

	if (arm_get_bit (c->ir, 22)) {
		val = arm_set_psr_field (arm_get_spsr (c), val, fld);
		arm_set_spsr (c, val);
	}
	else {
		val = arm_set_psr_field (arm_get_cpsr (c), val, fld);
		arm_write_cpsr (c, val, 0);
	}

	arm_set_clk (c, 4, 1);
}

/* 12 01: bx rm */
static
void op12_01 (arm_t *c)
{
	uint32_t d;

	d = arm_get_rm (c, c->ir);

	if (d & 1) {
		/* branch to thumb instruction set */
		op_undefined (c);
		return;
	}

	arm_set_pc (c, d & 0xfffffffc);

	arm_set_clk (c, 0, 1);
}

/* 12 03: blx rm */
static
void op12_03 (arm_t *c)
{
	uint32_t d;

	d = arm_get_rm (c, c->ir);

	if (d & 1) {
		/* branch to thumb instruction set */
		op_undefined (c);
		return;
	}

	arm_set_lr (c, (arm_get_pc (c) + 4) & 0xffffffff);
	arm_set_pc (c, d & 0xfffffffc);

	arm_set_clk (c, 0, 1);
}

/* 12 07: bkpt uimm16 */
static
void op12_07 (arm_t *c)
{
	arm_set_clk (c, 0, 1);
	arm_exception_prefetch_abort (c);
}

/* 12 */
static
void op12 (arm_t *c)
{
	switch (c->ir & 0x0ff000f0) {
	case 0x01200000:
		op12_00 (c);
		break;

	case 0x01200010:
		op12_01 (c);
		break;

	case 0x01200030:
		op12_03 (c);
		break;

	case 0x01200070:
		op12_07 (c);
		break;

	case 0x012000b0:
	case 0x012000d0:
	case 0x012000f0:
		op00_0b (c);
		break;

	default:
		op_undefined (c);
		break;
	}
}

/* 13 ext */
static
void op13_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 13: teq[cond] rn, shifter_operand */
static
void op13 (arm_t *c)
{
	uint32_t s1, s2, d, shc;

	if (arm_is_shext (c->ir)) {
		op13_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, &shc);

	d = s1 ^ s2;

	arm_set_cc_nz (c, d);
	arm_set_cc_c (c, shc);
	arm_set_clk (c, 4, 1);
}

/* 14 00: mrs[cond] rd, spsr */
static
void op14_00 (arm_t *c)
{
	arm_set_rd (c, c->ir, arm_get_spsr (c));
	arm_set_clk (c, 4, 1);
}

/* 14 09: swp[cond]b rd, rm, rn */
static
void op14_09 (arm_t *c)
{
	uint32_t addr, s;
	uint8_t  d;

	addr = arm_get_rn (c, c->ir);
	s = arm_get_rm (c, c->ir);

	if (arm_dload8 (c, addr, &d)) {
		return;
	}

	if (arm_dstore8 (c, addr, s & 0xff)) {
		return;
	}

	arm_set_rd (c, c->ir, d & 0xff);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 14 */
static
void op14 (arm_t *c)
{
	switch (arm_get_bits (c->ir, 4, 4)) {
	case 0x00:
		op14_00 (c);
		break;

	case 0x09:
		op14_09 (c);
		break;

	case 0x0b:
	case 0x0d:
	case 0x0f:
		op00_0b (c);
		break;

	default:
		op_undefined (c);
		break;
	}
}

/* 15 ext */
static
void op15_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 15: cmp[cond] rn, shifter_operand */
static
void op15 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op15_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = (s1 - s2) & 0xffffffff;

	arm_set_cc_sub (c, d, s1, s2);
	arm_set_clk (c, 4, 1);
}

/* 16 01: clz rd, rm */
static
void op16_01 (arm_t *c)
{
	if (arm_get_bits (c->ir, 4, 4) == 0x01) {
		uint32_t d, s;

		s = arm_get_rm (c, c->ir);

		if (s == 0) {
			d = 32;
		}
		else {
			d = 0;
			while ((s & 0x80000000) == 0) {
				d += 1;
				s <<= 1;
			}
		}

		arm_set_rd (c, c->ir, d);
		arm_set_clk (c, 4, 1);
	}
	else {
		op_undefined (c);
	}
}

/* 16 08: smulxy[cond] rd, rm, rs */
static
void op16_08 (arm_t *c)
{
	int      sign;
	uint32_t d, s1, s2;

	s1 = arm_get_rm (c, c->ir);
	s2 = arm_get_rs (c, c->ir);

	if (arm_get_bit (c->ir, 5)) {
		s1 = s1 >> 16;
	}

	if (arm_get_bit (c->ir, 6)) {
		s2 = s2 >> 16;
	}

	sign = ((s1 ^ s2) & 0x8000) != 0;

	s1 = ((s1 & 0x8000) ? (~s1 + 1) : s1) & 0xffff;
	s2 = ((s2 & 0x8000) ? (~s2 + 1) : s2) & 0xffff;

	d = s1 * s2;

	if (sign) {
		d = (~d + 1) & 0xffffffff;
	}

	arm_set_rn (c, c->ir, d);
	arm_set_clk (c, 4, 1);
}

/* 16 */
static
void op16 (arm_t *c)
{
	switch (c->ir & 0x0ff000f0) {
	case 0x01600000:
		op12_00 (c);
		break;

	case 0x01600010:
		op16_01 (c);
		break;

	case 0x01600080:
	case 0x016000a0:
	case 0x016000c0:
	case 0x016000e0:
		op16_08 (c);
		break;

	case 0x016000b0:
	case 0x016000d0:
	case 0x016000f0:
		op00_0b (c);
		break;

	default:
		op_undefined (c);
		break;
	}
}

/* 17 ext */
static
void op17_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 17: cmn[cond] rn, shifter_operand */
static
void op17 (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op17_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = (s1 + s2) & 0xffffffff;

	arm_set_cc_add (c, d, s1, s2);
	arm_set_clk (c, 4, 1);
}

/* 18 ext */
static
void op18_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 18: orr[cond] rd, rn, shifter_operand */
static
void op18 (arm_t *c)
{
	uint32_t d, s1, s2;

	if (arm_is_shext (c->ir)) {
		op18_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = s1 | s2;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 19 ext */
static
void op19_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 19: orr[cond]s rd, rn, shifter_operand */
static
void op19 (arm_t *c)
{
	uint32_t s1, s2, d, shc;

	if (arm_is_shext (c->ir)) {
		op19_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, &shc);

	d = s1 | s2;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_nz (c, d);
		arm_set_cc_c (c, shc);
		arm_set_clk (c, 4, 1);
	}
}

/* 1A ext */
static
void op1a_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 1A: mov[cond] rd, shifter_operand */
static
void op1a (arm_t *c)
{
	uint32_t d;

	if (arm_is_shext (c->ir)) {
		op1a_ext (c);
		return;
	}

	d = arm_get_sh (c, c->ir, NULL);

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 1B ext */
static
void op1b_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 1B: mov[cond]s rd, shifter_operand */
static
void op1b (arm_t *c)
{
	uint32_t d, shc;

	if (arm_is_shext (c->ir)) {
		op1b_ext (c);
		return;
	}

	d = arm_get_sh (c, c->ir, &shc);

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_nz (c, d);
		arm_set_cc_c (c, shc);
		arm_set_clk (c, 4, 1);
	}
}

/* 1C ext */
static
void op1c_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 1C: bic[cond] rd, rn, shifter_operand */
static
void op1c (arm_t *c)
{
	uint32_t s1, s2, d;

	if (arm_is_shext (c->ir)) {
		op1c_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, NULL);

	d = s1 & ~s2;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 1D ext */
static
void op1d_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 1d: bic[cond]s rd, rn, shifter_operand */
static
void op1d (arm_t *c)
{
	uint32_t s1, s2, d, shc;

	if (arm_is_shext (c->ir)) {
		op1d_ext (c);
		return;
	}

	s1 = arm_get_rn (c, c->ir);
	s2 = arm_get_sh (c, c->ir, &shc);

	d = s1 & ~s2;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_nz (c, d);
		arm_set_cc_c (c, shc);
		arm_set_clk (c, 4, 1);
	}
}

/* 1E ext */
static
void op1e_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op00_0b (c);
		break;
	}
}

/* 1E: mvn[cond] rd, shifter_operand */
static
void op1e (arm_t *c)
{
	uint32_t d;

	if (arm_is_shext (c->ir)) {
		op1e_ext (c);
		return;
	}

	d = arm_get_sh (c, c->ir, NULL);

	d = ~d & 0xffffffff;

	arm_set_rd (c, c->ir, d);
	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 1F ext */
static
void op1f_ext (arm_t *c)
{
	switch (arm_get_shext (c->ir)) {
	case 0x09:
		op_undefined (c);
		break;

	default:
		op01_0b (c);
		break;
	}
}

/* 1F: mvn[cond]s rd, shifter_operand */
static
void op1f (arm_t *c)
{
	uint32_t d, shc;

	if (arm_is_shext (c->ir)) {
		op1f_ext (c);
		return;
	}

	d = arm_get_sh (c, c->ir, &shc);

	d = ~d & 0xffffffff;

	arm_set_rd (c, c->ir, d);

	if (arm_rd_is_pc (c->ir)) {
		arm_write_cpsr (c, arm_get_spsr (c), 1);
		arm_set_clk (c, 0, 1);
	}
	else {
		arm_set_cc_nz (c, d);
		arm_set_cc_c (c, shc);
		arm_set_clk (c, 4, 1);
	}
}

/* 32: msr[cond] fields, #imm */
static
void op32 (arm_t *c)
{
	unsigned fld, rot;
	uint32_t val;

	fld = arm_get_bits (c->ir, 16, 4);
	rot = 2 * arm_get_bits (c->ir, 8, 4);

	if (arm_is_privileged (c) == 0) {
		fld &= 0x08;
	}

	val = arm_get_bits (c->ir, 0, 8);
	val = arm_ror32 (val, rot);

	if (arm_get_bit (c->ir, 22)) {
		val = arm_set_psr_field (arm_get_spsr (c), val, fld);
		arm_set_spsr (c, val);
	}
	else {
		val = arm_set_psr_field (arm_get_cpsr (c), val, fld);
		arm_write_cpsr (c, val, 0);
	}

	arm_set_clk (c, 4, 1);
}

/* 40: str[cond][b][t] rd, [rn], address */
static
void op40 (arm_t *c)
{
	int      p, u, b, w, t;
	uint32_t val, base, index;

	p = arm_get_bit (c->ir, 24);
	u = arm_get_bit (c->ir, 23);
	b = arm_get_bit (c->ir, 22);
	w = arm_get_bit (c->ir, 21);
	t = (p == 0) && (w == 1);

	base = arm_get_rn (c, c->ir);

	/* get index */
	if (arm_get_bit (c->ir, 25)) {
		unsigned n;

		index = arm_get_rm (c, c->ir);
		n = arm_get_bits (c->ir, 7, 5);

		switch (arm_get_bits (c->ir, 5, 2)) {
		case 0x00: /* lsl */
			index = index << n;
			break;

		case 0x01: /* lsr */
			index = (n == 0) ? 0 : (index >> n);
			break;

		case 0x02: /* asr */
			if (n == 0) {
				if (index & 0x80000000) {
					index = 0xffffffff;
				}
				else {
					index = 0x00000000;
				}
			}
			else {
				index = arm_asr32 (index, n);
			}
			break;

		case 0x03: /* ror / rrx */
			if (n == 0) {
				index = (index >> 1) | (arm_get_cc_c (c) << 31);
			}
			else {
				index = (index >> n) | (index << (32 - n));
			}
		}
	}
	else {
		index = arm_extu (c->ir, 12);
	}

	/* pre-index */
	if (p) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	val = arm_get_rd (c, c->ir);

	if (b) {
		if (t) {
			if (arm_dstore8_t (c, base, val)) {
				return;
			}
		}
		else {
			if (arm_dstore8 (c, base, val)) {
				return;
			}
		}
	}
	else {
		if (t) {
			if (arm_dstore32_t (c, base, val)) {
				return;
			}
		}
		else {
			if (arm_dstore32 (c, base, val)) {
				return;
			}
		}
	}

	/* post-index */
	if (p == 0) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	/* base register writeback */
	if ((p == 0) || (w == 1)) {
		arm_set_rn (c, c->ir, base);
	}

	arm_set_clk (c, 4, 1);
}

/* 41: ldr[cond][b][t] rd, [rn], address */
static
void op41 (arm_t *c)
{
	int      p, u, b, w, t;
	uint32_t val, base, index;

	p = arm_get_bit (c->ir, 24);
	u = arm_get_bit (c->ir, 23);
	b = arm_get_bit (c->ir, 22);
	w = arm_get_bit (c->ir, 21);
	t = (p == 0) && (w == 1);

	base = arm_get_rn (c, c->ir);

	/* get index */
	if (arm_get_bit (c->ir, 25)) {
		unsigned n;

		index = arm_get_rm (c, c->ir);
		n = arm_get_bits (c->ir, 7, 5);

		switch (arm_get_bits (c->ir, 5, 2)) {
		case 0x00: /* lsl */
			index = index << n;
			break;

		case 0x01: /* lsr */
			index = (n == 0) ? 0 : (index >> n);
			break;

		case 0x02: /* asr */
			if (n == 0) {
				if (index & 0x80000000) {
					index = 0xffffffff;
				}
				else {
					index = 0x00000000;
				}
			}
			else {
				index = arm_asr32 (index, n);
			}
			break;

		case 0x03: /* ror / rrx */
			if (n == 0) {
				index = (index >> 1) | (arm_get_cc_c (c) << 31);
			}
			else {
				index = (index >> n) | (index << (32 - n));
			}
		}
	}
	else {
		index = arm_extu (c->ir, 12);
	}

	/* pre-index */
	if (p) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	if (b) {
		uint8_t tmp;

		if (t) {
			if (arm_dload8_t (c, base, &tmp)) {
				return;
			}
		}
		else {
			if (arm_dload8 (c, base, &tmp)) {
				return;
			}
		}

		val = tmp & 0xff;
	}
	else {
		if (t) {
			if (arm_dload32_t (c, base & 0xfffffffc, &val)) {
				return;
			}
		}
		else {
			if (arm_dload32 (c, base & 0xfffffffc, &val)) {
				return;
			}
		}

		if (base & 0x03) {
			val = arm_ror32 (val, (base & 0x03) << 3);
		}
	}

	arm_set_rd (c, c->ir, val);

	/* post-index */
	if (p == 0) {
		base += u ? index : (~index + 1);
		base &= 0xffffffff;
	}

	/* base register writeback */
	if ((p == 0) || (w == 1)) {
		arm_set_rn (c, c->ir, base);
	}

	arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 80: stm[cond][mode] rn[!], registers[^] */
static
void op80 (arm_t *c)
{
	int      p, u, s, w;
	unsigned i;
	unsigned regs, regn;
	unsigned mode;
	uint32_t addr, base, writeback;
	uint32_t val;

	p = arm_get_bit (c->ir, 24);
	u = arm_get_bit (c->ir, 23);
	s = arm_get_bit (c->ir, 22);
	w = arm_get_bit (c->ir, 21);

	regs = arm_get_bits (c->ir, 0, 16);
	regn = arm_bitcnt32 (regs);
	mode = arm_get_cpsr_m (c);
	base = arm_get_rn (c, c->ir);

	if (u) {
		writeback = base + 4 * regn;

		if (p) {
			/* ib */
			addr = base + 4;
		}
		else {
			/* ia */
			addr = base;
		}
	}
	else {
		writeback = base - 4 * regn;

		if (p) {
			/* db */
			addr = base - 4 * regn;
		}
		else {
			/* da */
			addr = base - 4 * regn + 4;
		}
	}

	if (s) {
		/* access user mode registers */
		arm_set_reg_map (c, ARM_MODE_USR);
	}

	for (i = 0; i < 16; i++) {
		if (regs & (1U << i)) {
			val = arm_get_reg_pc (c, i, 8);

			if (arm_dstore32 (c, addr & 0xfffffffc, val)) {
				return;
			}

			addr += 4;
		}
	}

	if (s) {
		arm_set_reg_map (c, mode);
	}

	if (w) {
		arm_set_rn (c, c->ir, writeback);
	}

	arm_set_clk (c, 4, 1);
}

/* 81: ldm[cond][mode] rn[!], registers[^] */
static
void op81 (arm_t *c)
{
	int      p, u, s, w;
	unsigned i;
	unsigned regs, regn;
	unsigned mode;
	uint32_t addr, base, writeback;
	uint32_t val;

	p = arm_get_bit (c->ir, 24);
	u = arm_get_bit (c->ir, 23);
	s = arm_get_bit (c->ir, 22);
	w = arm_get_bit (c->ir, 21);

	regs = arm_get_bits (c->ir, 0, 16);
	regn = arm_bitcnt32 (regs);
	mode = arm_get_cpsr_m (c);
	base = arm_get_rn (c, c->ir);

	if (u) {
		writeback = base + 4 * regn;

		if (p) {
			/* ib */
			addr = base + 4;
		}
		else {
			/* ia */
			addr = base;
		}
	}
	else {
		writeback = base - 4 * regn;

		if (p) {
			/* db */
			addr = base - 4 * regn;
		}
		else {
			/* da */
			addr = base - 4 * regn + 4;
		}
	}

	if (s && !(regs & 0x8000)) {
		/* access user mode registers */
		arm_set_reg_map (c, ARM_MODE_USR);
	}

	for (i = 0; i < 16; i++) {
		if (regs & (1U << i)) {
			if (arm_dload32 (c, addr & 0xfffffffc, &val)) {
				return;
			}

			if (i == arm_ir_rn (c->ir)) {
				/* defer modification of base register */
				base = val;
			}
			else {
				arm_set_gpr (c, i, val);
			}

			addr += 4;
		}
	}

	arm_set_rn (c, c->ir, base);

	if (s) {
		if (regs & 0x8000) {
			arm_write_cpsr (c, arm_get_spsr (c), 1);
		}
		else {
			arm_set_reg_map (c, mode);
		}
	}

	if (w) {
		arm_set_rn (c, c->ir, writeback);
	}

	arm_set_clk (c, (regs & 0x8000) ? 0 : 4, 1);
}

/* A0: b[cond] target */
static
void opa0 (arm_t *c)
{
	uint32_t d;

	d = arm_exts (c->ir, 24);
	d = (arm_get_pc (c) + (d << 2) + 8) & 0xffffffff;

	arm_set_pc (c, d);

	arm_set_clk (c, 0, 1);
}

/* B0: bl[cond] target */
static
void opb0 (arm_t *c)
{
	uint32_t d;

	d = arm_exts (c->ir, 24);
	d = (arm_get_pc (c) + (d << 2) + 8) & 0xffffffff;

	arm_set_lr (c, (arm_get_pc (c) + 4) & 0xffffffff);
	arm_set_pc (c, d);

	arm_set_clk (c, 0, 1);
}

/* C4: mcrr[cond] coproc, opcode, rd, rn, crm */
static
void opc4 (arm_t *c)
{
	int      r;
	unsigned cop;

	cop = arm_get_bits (c->ir, 8, 4);

	if (arm_copr_check (c, cop) == 0) {
		r = c->copr[cop]->exec (c, c->copr[cop]);
	}
	else {
		r = 1;
	}

	if (cop == 0) {
		/* hack */
		arm_set_clk (c, 4, 1);
		return;
	}

	if (r) {
		arm_set_clk (c, 0, 1);
		arm_exception_undefined (c);
		return;
	}
}

/* C5: mrrc[cond] coproc, opcode, rd, rn, crm */
static
void opc5 (arm_t *c)
{
	int      r;
	unsigned cop;

	cop = arm_get_bits (c->ir, 8, 4);

	if (arm_copr_check (c, cop) == 0) {
		r = c->copr[cop]->exec (c, c->copr[cop]);
	}
	else {
		r = 1;
	}

	if (cop == 0) {
		/* hack */
		arm_set_clk (c, 4, 1);
		return;
	}

	if (r) {
		arm_set_clk (c, 0, 1);
		arm_exception_undefined (c);
		return;
	}
}

/* E0 00: cdp[cond] coproc, opcode1, crd, crn, crm, opcode2 */
static
void ope0_00 (arm_t *c)
{
	int      r;
	unsigned cop;

	cop = arm_get_bits (c->ir, 8, 4);

	if (arm_copr_check (c, cop) == 0) {
		r = c->copr[cop]->exec (c, c->copr[cop]);
	}
	else {
		r = 1;
	}

	if (r) {
		arm_set_clk (c, 0, 1);
		arm_exception_undefined (c);
		return;
	}

	arm_set_clk (c, 4, 1);
}

/* E0 01: mcr[cond] coproc, opcode1, rd, crn, crm, opcode2 */
static
void ope0_01 (arm_t *c)
{
	int      r;
	unsigned cop;

	cop = arm_get_bits (c->ir, 8, 4);

	if (arm_copr_check (c, cop) == 0) {
		r = c->copr[cop]->exec (c, c->copr[cop]);
	}
	else {
		r = 1;
	}

	if (r) {
		arm_set_clk (c, 0, 1);
		arm_exception_undefined (c);
		return;
	}
}

/* E0 11: mrc[cond] coproc, opcode1, rd, crn, crm, opcode2 */
static
void ope0_11 (arm_t *c)
{
	int      r;
	unsigned cop;

	cop = arm_get_bits (c->ir, 8, 4);

	if (arm_copr_check (c, cop) == 0) {
		r = c->copr[cop]->exec (c, c->copr[cop]);
	}
	else {
		r = 1;
	}

	if (cop == 0) {
		/* hack */
		arm_set_clk (c, 4, 1);
		return;
	}

	if (r) {
		arm_set_clk (c, 0, 1);
		arm_exception_undefined (c);
		return;
	}
}

/* E0 */
static
void ope0 (arm_t *c)
{
	switch (c->ir & 0x00100010) {
	case 0x00000000:
	case 0x00100000:
		ope0_00 (c);
		break;

	case 0x00000010:
		ope0_01 (c);
		break;

	case 0x00100010:
		ope0_11 (c);
		break;

	default:
		op_undefined (c);
		break;
	}
}

/* F0: swi[cond] immediate */
static
void opf0 (arm_t *c)
{
	arm_set_clk (c, 0, 1);

	arm_exception_swi (c);
}


static
arm_opcode_f arm_opcodes[256] = {
	op00, op01, op02, op03, op04, op05, op06, op07,  /* 00 */
	op08, op09, op0a, op0b, op0c, op0d, op0e, op0f,
	op10, op11, op12, op13, op14, op15, op16, op17,  /* 10 */
	op18, op19, op1a, op1b, op1c, op1d, op1e, op1f,
	op00, op01, op02, op03, op04, op05, op06, op07,  /* 20 */
	op08, op09, op0a, op0b, op0c, op0d, op0e, op0f,
	NULL, op11, op32, op13, NULL, op15, NULL, op17,  /* 30 */
	op18, op19, op1a, op1b, op1c, op1d, op1e, op1f,
	op40, op41, op40, op41, op40, op41, op40, op41,  /* 40 */
	op40, op41, op40, op41, op40, op41, op40, op41,
	op40, op41, op40, op41, op40, op41, op40, op41,  /* 50 */
	op40, op41, op40, op41, op40, op41, op40, op41,
	op40, op41, op40, op41, op40, op41, op40, op41,  /* 60 */
	op40, op41, op40, op41, op40, op41, op40, op41,
	op40, op41, op40, op41, op40, op41, op40, op41,  /* 70 */
	op40, op41, op40, op41, op40, op41, op40, op41,
	op80, op81, op80, op81, op80, op81, op80, op81,  /* 80 */
	op80, op81, op80, op81, op80, op81, op80, op81,
	op80, op81, op80, op81, op80, op81, op80, op81,  /* 90 */
	op80, op81, op80, op81, op80, op81, op80, op81,
	opa0, opa0, opa0, opa0, opa0, opa0, opa0, opa0,  /* a0 */
	opa0, opa0, opa0, opa0, opa0, opa0, opa0, opa0,
	opb0, opb0, opb0, opb0, opb0, opb0, opb0, opb0,  /* b0 */
	opb0, opb0, opb0, opb0, opb0, opb0, opb0, opb0,
	NULL, NULL, NULL, NULL, opc4, opc5, NULL, NULL,  /* c0 */
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  /* d0 */
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	ope0, ope0, ope0, ope0, ope0, ope0, ope0, ope0,  /* e0 */
	ope0, ope0, ope0, ope0, ope0, ope0, ope0, ope0,
	opf0, opf0, opf0, opf0, opf0, opf0, opf0, opf0,  /* f0 */
	opf0, opf0, opf0, opf0, opf0, opf0, opf0, opf0
};


void arm_set_opcodes (arm_t *c)
{
	unsigned i;

	for (i = 0; i < 256; i++) {
		if (arm_opcodes[i] != NULL) {
			c->opcodes[i] = arm_opcodes[i];
		}
		else {
			c->opcodes[i] = op_undefined;
		}
	}
}
