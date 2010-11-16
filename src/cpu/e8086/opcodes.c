/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/opcodes.c                                      *
 * Created:     1996-04-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1996-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e8086.h"
#include "internal.h"


void e86_push (e8086_t *c, unsigned short val)
{
	e86_set_sp (c, e86_get_sp (c) - 2);

	e86_set_mem16 (c, e86_get_ss (c), e86_get_sp (c), val);
}

unsigned short e86_pop (e8086_t *c)
{
	unsigned short sp;

	sp = e86_get_sp (c);

	e86_set_sp (c, sp + 2);

	return (e86_get_mem16 (c, e86_get_ss (c), sp));
}


/**************************************************************************
 * Microcode functions
 **************************************************************************/

/* Handle an undefined opcode */
static
unsigned op_ud (e8086_t *c)
{
	e86_set_clk (c, 1);

	return (e86_undefined (c));
}

static
unsigned op_divide_error (e8086_t *c)
{
	e86_set_clk_ea (c, 16, 20);

	e86_set_ip (c, e86_get_ip (c) + c->ea.cnt + 1);
	e86_trap (c, 0);

	return (0);
}

/* OP 00: ADD r/m8, reg8 */
static
unsigned op_00 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 + s2;

	e86_set_ea8 (c, d);
	e86_set_flg_add_8 (c, s1, s2);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 01: ADD r/m16, reg16 */
static
unsigned op_01 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 + s2;

	e86_set_ea16 (c, d);
	e86_set_flg_add_16 (c, s1, s2);
	e86_set_clk_ea (clk, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 02: ADD reg8, r/m8 */
static
unsigned op_02 (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);

	d = s1 + s2;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_add_8 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 03: ADD reg16, r/m16 */
static
unsigned op_03 (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);

	d = s1 + s2;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_add_16 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 04: ADD AL, data8 */
static
unsigned op_04 (e8086_t *c)
{
	unsigned short s1, s2, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	d = s1 + s2;

	e86_set_al (c, d);
	e86_set_flg_add_8 (c, s1, s2);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 05: ADD AX, data16 */
static
unsigned op_05 (e8086_t *c)
{
	unsigned long s1, s2, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 + s2;

	e86_set_ax (c, d);
	e86_set_flg_add_16 (c, s1, s2);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 06: PUSH ES */
static
unsigned op_06 (e8086_t *c)
{
	e86_push (c, e86_get_es (c));
	e86_set_clk (c, 10);

	return (1);
}

/* OP 07: POP ES */
static
unsigned op_07 (e8086_t *c)
{
	e86_set_es (c, e86_pop (c));
	e86_set_clk (c, 8);

	return (1);
}

/* OP 08: OR r/m8, reg8 */
static
unsigned op_08 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 | s2;

	e86_set_ea8 (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 09: OR r/m16, reg16 */
static
unsigned op_09 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 | s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 0A: OR reg8, r/m8 */
static
unsigned op_0a (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);

	d = s1 | s2;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 0B: OR reg16, r/m16 */
static
unsigned op_0b (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);

	d = s1 | s2;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 0C: OR AL, data8 */
static
unsigned op_0c (e8086_t *c)
{
	unsigned short s1, s2, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	d = s1 | s2;

	e86_set_al (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 0D: OR AX, data16 */
static
unsigned op_0d (e8086_t *c)
{
	unsigned long s1, s2, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 | s2;

	e86_set_ax (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 0E: PUSH CS */
static
unsigned op_0e (e8086_t *c)
{
	e86_push (c, e86_get_cs (c));
	e86_set_clk (c, 10);

	return (1);
}

/* OP 0F: POP CS */
static
unsigned op_0f (e8086_t *c)
{
	e86_set_cs (c, e86_pop (c));
	e86_pq_init (c);

	return (1);
}

/* OP 10: ADC r/m8, reg8 */
static
unsigned op_10 (e8086_t *c)
{
	unsigned short s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_ea8 (c, d);
	e86_set_flg_adc_8 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 11: ADC r/m16, reg16 */
static
unsigned op_11 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_ea16 (c, d);
	e86_set_flg_adc_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 12: ADC reg8, r/m8 */
static
unsigned op_12 (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_adc_8 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 13: ADC reg16, r/m16 */
static
unsigned op_13 (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_adc_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 14: ADC AL, data8 */
static
unsigned op_14 (e8086_t *c)
{
	unsigned short s1, s2, s3, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_al (c, d);
	e86_set_flg_adc_8 (c, s1, s2, s3);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 15: ADC AX, data16 */
static
unsigned op_15 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_ax (c, d);
	e86_set_flg_adc_16 (c, s1, s2, s3);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 16: PUSH SS */
static
unsigned op_16 (e8086_t *c)
{
	e86_push (c, e86_get_ss (c));
	e86_set_clk (c, 10);

	return (1);
}

/* OP 17: POP SS */
static
unsigned op_17 (e8086_t *c)
{
	e86_set_ss (c, e86_pop (c));
	e86_set_clk (c, 8);

	return (1);
}

/* OP 18: SBB r/m8, reg8 */
static
unsigned op_18 (e8086_t *c)
{
	unsigned short s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_ea8 (c, d);
	e86_set_flg_sbb_8 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 19: SBB r/m16, reg16 "" */
static
unsigned op_19 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_ea16 (c, d);
	e86_set_flg_sbb_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 1A: SBB reg8, r/m8 */
static
unsigned op_1a (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_sbb_8 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 1B: SBB reg16, r/m16 */
static
unsigned op_1b (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_sbb_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 1C: SBB AL, data8 */
static
unsigned op_1c (e8086_t *c)
{
	unsigned short s1, s2, s3, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_al (c, d);
	e86_set_flg_sbb_8 (c, s1, s2, s3);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 1D: SBB AX, data16 */
static
unsigned op_1d (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_ax (c, d);
	e86_set_flg_sbb_16 (c, s1, s2, s3);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 1E: PUSH DS */
static
unsigned op_1e (e8086_t *c)
{
	e86_push (c, e86_get_ds (c));
	e86_set_clk (c, 10);

	return (1);
}

/* OP 1F: POP DS */
static
unsigned op_1f (e8086_t *c)
{
	e86_set_ds (c, e86_pop (c));
	e86_set_clk (c, 8);

	return (1);
}

/* OP 20: AND r/m8, reg8 */
static
unsigned op_20 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 & s2;

	e86_set_ea8 (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 21: AND r/m16, reg16 */
static
unsigned op_21 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 & s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 22: AND reg8, r/m8 */
static unsigned op_22 (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);

	d = s1 & s2;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 23: AND reg16, r/m16 */
static
unsigned op_23 (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);

	d = s1 & s2;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 24: AND AL, data8 */
static
unsigned op_24 (e8086_t *c)
{
	unsigned short s1, s2, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	d = s1 & s2;

	e86_set_al (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 25: AND AX, data16 */
static
unsigned op_25 (e8086_t *c)
{
	unsigned long s1, s2, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 & s2;

	e86_set_ax (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 26: ES: */
static
unsigned op_26 (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_SEG);

	c->seg_override = c->sreg[E86_REG_ES];

	return (1);
}

/* OP 27: DAA */
static
unsigned op_27 (e8086_t *c)
{
	unsigned al, cf;

	al = e86_get_al (c);
	cf = e86_get_cf (c);

	if (((al & 0x0f) > 9) || e86_get_af (c)) {
		al += 6;
		cf |= ((al & 0xff00) != 0);
		e86_set_af (c, 1);
	}
	else {
		e86_set_af (c, 0);
	}

	if (((al & 0xf0) > 0x90) || cf) {
		al += 0x60;
		e86_set_cf (c, 1);
	}
	else {
		e86_set_cf (c, 0);
	}

	e86_set_al (c, al);
	e86_set_flg_szp_8 (c, al);
	e86_set_clk (c, 4);

	return (1);
}

/* OP 28: SUB r/m8, reg8 */
static
unsigned op_28 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 - s2;

	e86_set_ea8 (c, d);
	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 29: SUB r/m16, reg16 */
static
unsigned op_29 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 - s2;

	e86_set_ea16 (c, d);
	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 2A: SUB reg8, r/m8 */
static
unsigned op_2a (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);

	d = s1 - s2;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 2B: SUB reg16, r/m16 */
static
unsigned op_2b (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);

	d = s1 - s2;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 2C: SUB AL, data8 */
static
unsigned op_2c (e8086_t *c)
{
	unsigned short s1, s2, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	d = s1 - s2;

	e86_set_al (c, d);
	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 2D: SUB AX, data16 */
static
unsigned op_2d (e8086_t *c)
{
	unsigned long s1, s2, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 - s2;

	e86_set_ax (c, d);
	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 2E: CS: */
static
unsigned op_2e (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_SEG);

	c->seg_override = c->sreg[E86_REG_CS];

	return (1);
}

/* OP 2F: DAS */
static
unsigned op_2f (e8086_t *c)
{
	unsigned al, cf;

	al = e86_get_al (c);
	cf = e86_get_cf (c);

	if (((al & 0x0f) > 9) || e86_get_af (c)) {
		al -= 6;
		cf |= ((al & 0xff00) != 0);
		e86_set_af (c, 1);
	}
	else {
		e86_set_af (c, 0);
	}

	if (((al & 0xf0) > 0x90) || cf) {
		al -= 0x60;
		e86_set_cf (c, 1);
	}
	else {
		e86_set_cf (c, 0);
	}

	e86_set_al (c, al);
	e86_set_flg_szp_8 (c, al);
	e86_set_clk (c, 4);

	return (1);
}

/* OP 30: XOR r/m8, reg8 */
static
unsigned op_30 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 ^ s2;

	e86_set_ea8 (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 31: XOR r/m16, reg16 */
static
unsigned op_31 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 ^ s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 32: XOR reg8, r/m8 */
static
unsigned op_32 (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, reg);
	s2 = e86_get_ea8 (c);

	d = s1 ^ s2;

	e86_set_reg8 (c, reg, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 33: XOR reg16, r/m16 */
static
unsigned op_33 (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, reg);
	s2 = e86_get_ea16 (c);

	d = s1 ^ s2;

	e86_set_reg16 (c, reg, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 34: XOR AL, data8 */
static
unsigned op_34 (e8086_t *c)
{
	unsigned short s1, s2, d;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	d = s1 ^ s2;

	e86_set_al (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 35: XOR AX, data16 */
static
unsigned op_35 (e8086_t *c)
{
	unsigned long s1, s2, d;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 ^ s2;

	e86_set_ax (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 36: SS: */
static
unsigned op_36 (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_SEG);

	c->seg_override = c->sreg[E86_REG_SS];

	return (1);
}

/* OP 37: AAA */
static
unsigned op_37 (e8086_t *c)
{
	unsigned ah, al;

	al = e86_get_al (c);
	ah = e86_get_ah (c);

	if (((al & 0x0f) > 9) || e86_get_af (c)) {
		al += 6;
		ah += 1;
		c->flg |= (E86_FLG_A | E86_FLG_C);
	}
	else {
		c->flg &= ~(E86_FLG_A | E86_FLG_C);
	}

	e86_set_ax (c, ((ah & 0xff) << 8) | (al & 0x0f));

	e86_set_clk (c, 8);

	return (1);
}

/* OP 38: CMP r/m8, reg8 */
static
unsigned op_38 (e8086_t *c)
{
	unsigned short s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 39: CMP r/m16, reg16 */
static
unsigned op_39 (e8086_t *c)
{
	unsigned long s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 3A: CMP reg8, r/m8 */
static unsigned op_3a (e8086_t *c)
{
	unsigned short s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);
	s2 = e86_get_ea8 (c);

	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 3B: CMP reg16, r/m16 */
static
unsigned op_3b (e8086_t *c)
{
	unsigned long s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);
	s2 = e86_get_ea16 (c);

	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 3C: CMP AL, data8 */
static
unsigned op_3c (e8086_t *c)
{
	unsigned short s1, s2;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk (c, 4);

	return (2);
}

/* OP 3D: CMP AX, data16 */
static
unsigned op_3d (e8086_t *c)
{
	unsigned long s1, s2;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk (c, 4);

	return (3);
}

/* OP 3E: DS: */
static
unsigned op_3e (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_SEG);

	c->seg_override = c->sreg[E86_REG_DS];

	return (1);
}

/* OP 3F: AAS */
static
unsigned op_3f (e8086_t *c)
{
	unsigned ah, al;

	al = e86_get_al (c);
	ah = e86_get_ah (c);

	if (((al & 0x0f) > 9) || e86_get_af (c)) {
		al -= 6;
		ah -= 1;
		c->flg |= (E86_FLG_A | E86_FLG_C);
	}
	else {
		c->flg &= ~(E86_FLG_A | E86_FLG_C);
	}

	e86_set_ax (c, ((ah & 0xff) << 8) | (al & 0x0f));

	e86_set_clk (c, 8);

	return (1);
}

/* OP 4x: INC reg16 */
static
unsigned op_40 (e8086_t *c)
{
	unsigned       r;
	unsigned short cf;
	unsigned long  s;

	r = c->pq[0] & 7;
	s = c->dreg[r];
	c->dreg[r] = (s + 1) & 0xffff;

	cf = c->flg;
	e86_set_flg_add_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E86_FLG_C;

	e86_set_clk (c, 3);

	return (1);
}

/* OP 4x: DEC reg16 */
static
unsigned op_48 (e8086_t *c)
{
	unsigned       r;
	unsigned short cf;
	unsigned long  s;

	r = c->pq[0] & 7;
	s = c->dreg[r];
	c->dreg[r] = (s - 1) & 0xffff;

	cf = c->flg;
	e86_set_flg_sub_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E86_FLG_C;

	e86_set_clk (c, 3);

	return (1);
}

/* OP 5x: PUSH reg16 */
static
unsigned op_50 (e8086_t *c)
{
	unsigned reg;

	reg = c->pq[0] & 7;

	if ((reg == E86_REG_SP) && ((c->cpu & E86_CPU_PUSH_FIRST) == 0)) {
		e86_push (c, e86_get_sp (c) - 2);
	}
	else {
		e86_push (c, e86_get_reg16 (c, reg));
	}

	e86_set_clk (c, 10);

	return (1);
}

/* OP 5x: POP reg16 */
static
unsigned op_58 (e8086_t *c)
{
	unsigned short val;

	val = e86_pop (c);
	e86_set_reg16 (c, c->pq[0] & 7, val);
	e86_set_clk (c, 8);

	return (1);
}

/* OP 66: hook */
static
unsigned op_66 (e8086_t *c)
{
	unsigned short cs, ip;

	if (c->pq[1] != 0x66) {
		return (op_ud (c));
	}

	cs = e86_get_cs (c);
	ip = e86_get_ip (c);

	if (c->op_hook != NULL) {
		c->op_hook (c->op_ext, c->pq[2], c->pq[3]);
	}

	e86_set_clk (c, 16);

	if ((e86_get_cs (c) != cs) || (e86_get_ip (c) != ip)) {
		return (0);
	}

	return (4);
}

/* OP 70: JO imm8 */
static
unsigned op_70 (e8086_t *c)
{
	if (e86_get_of (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 71: JNO imm8 */
static
unsigned op_71 (e8086_t *c)
{
	if (!e86_get_of (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 72: JB imm8 */
static
unsigned op_72 (e8086_t *c)
{
	if (e86_get_cf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 73: JAE imm8 */
static
unsigned op_73 (e8086_t *c)
{
	if (!e86_get_cf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 74: JE imm8 */
static
unsigned op_74 (e8086_t *c)
{
	if (e86_get_zf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 75: JNE imm8 */
static
unsigned op_75 (e8086_t *c)
{
	if (!e86_get_zf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 76: JBE imm8 */
static
unsigned op_76 (e8086_t *c)
{
	if (e86_get_zf (c) || e86_get_cf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 77: JA imm8 */
static
unsigned op_77 (e8086_t *c)
{
	if (!(e86_get_zf (c) || e86_get_cf (c))) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 78: JS imm8 */
static
unsigned op_78 (e8086_t *c)
{
	if (e86_get_sf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 79: JNS imm8 */
static
unsigned op_79 (e8086_t *c)
{
	if (!e86_get_sf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 7A: JPE imm8 */
static
unsigned op_7a (e8086_t *c)
{
	if (e86_get_pf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 7B: JPO imm8 */
static
unsigned op_7b (e8086_t *c)
{
	if (!e86_get_pf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 7C: JL imm8 */
static
unsigned op_7c (e8086_t *c)
{
	if (e86_get_sf (c) != e86_get_of (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 7D: JGE imm8 */
static
unsigned op_7d (e8086_t *c)
{
	if (e86_get_sf (c) == e86_get_of (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 7E: JLE imm8 */
static
unsigned op_7e (e8086_t *c)
{
	if ((e86_get_sf (c) != e86_get_of (c)) || e86_get_zf (c)) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 7F: JG imm8 */
static
unsigned op_7f (e8086_t *c)
{
	if ((e86_get_sf (c) == e86_get_of (c)) && (!e86_get_zf (c))) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 16);
		return (0);
	}

	e86_set_clk (c, 4);

	return (2);
}

/* OP 80 00: ADD r/m8, imm8 */
static
unsigned op_80_00 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 + s2;

	e86_set_ea8 (c, d & 0xff);
	e86_set_flg_add_8 (c, s1, s2);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 01: OR r/m8, imm8 */
static
unsigned op_80_01 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 | s2;

	e86_set_ea8 (c, d & 0xff);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 02: ADC r/m8, imm8 */
static
unsigned op_80_02 (e8086_t *c)
{
	unsigned short s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_ea8 (c, d & 0xff);
	e86_set_flg_adc_8 (c, s1, s2, s3);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 03: SBB r/m8, imm8 */
static
unsigned op_80_03 (e8086_t *c)
{
	unsigned short s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_ea8 (c, d & 0xff);
	e86_set_flg_sbb_8 (c, s1, s2, s3);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 04: AND r/m8, imm8 */
static
unsigned op_80_04 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 & s2;

	e86_set_ea8 (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 05: SUB r/m8, imm8 */
static
unsigned op_80_05 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 - s2;

	e86_set_ea8 (c, d & 0xff);
	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 06: XOR r/m8, imm8 */
static
unsigned op_80_06 (e8086_t *c)
{
	unsigned short s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 ^ s2;

	e86_set_ea8 (c, d);
	e86_set_flg_log_8 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 07: CMP r/m8, imm8 */
static
unsigned op_80_07 (e8086_t *c)
{
	unsigned short s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	e86_set_flg_sub_8 (c, s1, s2);
	e86_set_clk_ea (c, 4, 10);

	return (2 + c->ea.cnt);
}

static
e86_opcode_f e86_opcodes_80[8] = {
	&op_80_00, &op_80_01, &op_80_02, &op_80_03,
	&op_80_04, &op_80_05, &op_80_06, &op_80_07,
};

/* OP 80: xxx r/m8, imm8 */
static
unsigned op_80 (e8086_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e86_opcodes_80[xop] (c));
}

/* OP 81 00: ADD r/m16, imm16 */
static
unsigned op_81_00 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 + s2;

	e86_set_ea16 (c, d & 0xffff);
	e86_set_flg_add_16 (c, s1, s2);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 01: OR r/m16, imm16 */
static
unsigned op_81_01 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 | s2;

	e86_set_ea16 (c, d & 0xffff);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 02: ADC r/m16, imm16 */
static
unsigned op_81_02 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_ea16 (c, d & 0xffff);
	e86_set_flg_adc_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 03: SBB r/m16, imm16 */
static
unsigned op_81_03 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_ea16 (c, d & 0xffff);
	e86_set_flg_sbb_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 04: AND r/m16, imm16 */
static
unsigned op_81_04 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 & s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 05: SUB r/m16, imm16 */
static
unsigned op_81_05 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 - s2;

	e86_set_ea16 (c, d & 0xffff);
	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 06: XOR r/m16, imm16 */
static
unsigned op_81_06 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 ^ s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 07: CMP r/m16, imm16 */
static
unsigned op_81_07 (e8086_t *c)
{
	unsigned long s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 4, 10);

	return (3 + c->ea.cnt);
}

static
e86_opcode_f e86_opcodes_81[8] = {
	&op_81_00, &op_81_01, &op_81_02, &op_81_03,
	&op_81_04, &op_81_05, &op_81_06, &op_81_07,
};

/* OP 81: xxx r/m16, imm16 */
static
unsigned op_81 (e8086_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e86_opcodes_81[xop] (c));
}

/* OP 83 00: ADD r/m16, imm8 */
static
unsigned op_83_00 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 + s2;

	e86_set_ea16 (c, d);
	e86_set_flg_add_16 (c, s1, s2);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 01: OR r/m16, imm8 */
static
unsigned op_83_01 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 | s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 02: ADC r/m16, imm8 */
static
unsigned op_83_02 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);
	s3 = e86_get_cf (c);

	d = s1 + s2 + s3;

	e86_set_ea16 (c, d);
	e86_set_flg_adc_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 03: SBB r/m16, imm8 */
static
unsigned op_83_03 (e8086_t *c)
{
	unsigned long s1, s2, s3, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);
	s3 = e86_get_cf (c);

	d = s1 - s2 - s3;

	e86_set_ea16 (c, d);
	e86_set_flg_sbb_16 (c, s1, s2, s3);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 04: AND r/m16, imm8 */
static
unsigned op_83_04 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 & s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 05: SUB r/m16, imm8 */
static
unsigned op_83_05 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 - s2;

	e86_set_ea16 (c, d);
	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 06: XOR r/m16, imm8 */
static
unsigned op_83_06 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 ^ s2;

	e86_set_ea16 (c, d);
	e86_set_flg_log_16 (c, d);
	e86_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 07: CMP r/m16, imm8 */
static
unsigned op_83_07 (e8086_t *c)
{
	unsigned long s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	e86_set_flg_sub_16 (c, s1, s2);
	e86_set_clk_ea (c, 4, 10);

	return (2 + c->ea.cnt);
}

static
e86_opcode_f e86_opcodes_83[8] = {
	&op_83_00, &op_83_01, &op_83_02, &op_83_03,
	&op_83_04, &op_83_05, &op_83_06, &op_83_07,
};

/* OP 83: xxx r/m16, imm8 */
static
unsigned op_83 (e8086_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e86_opcodes_83[xop] (c));
}

/* OP 84: TEST r/m8, reg8 */
static
unsigned op_84 (e8086_t *c)
{
	unsigned short s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, (c->pq[1] >> 3) & 7);

	e86_set_flg_log_8 (c, s1 & s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 85: TEST r/m16, reg16 */
static
unsigned op_85 (e8086_t *c)
{
	unsigned long s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	e86_set_flg_log_16 (c, s1 & s2);
	e86_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 86: XCHG r/m8, reg8 */
static
unsigned op_86 (e8086_t *c)
{
	unsigned       reg;
	unsigned short s1, s2;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = e86_get_reg8 (c, reg);

	e86_set_ea8 (c, s2);
	e86_set_reg8 (c, reg, s1);
	e86_set_clk_ea (c, 4, 17);

	return (c->ea.cnt + 1);
}

/* OP 87: XCHG r/m16, reg16 */
static
unsigned op_87 (e8086_t *c)
{
	unsigned      reg;
	unsigned long s1, s2;

	reg = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_get_reg16 (c, reg);

	e86_set_ea16 (c, s2);
	e86_set_reg16 (c, reg, s1);
	e86_set_clk_ea (c, 4, 17);

	return (c->ea.cnt + 1);
}

/* OP 88: MOV r/m8, reg8 */
static
unsigned op_88 (e8086_t *c)
{
	unsigned char val;
	unsigned      reg;

	e86_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e86_get_reg8 (c, reg);

	e86_set_ea8 (c, val);
	e86_set_clk_ea (c, 2, 9);

	return (c->ea.cnt + 1);
}

/* OP 89: MOV r/m16, reg16 */
static
unsigned op_89 (e8086_t *c)
{
	unsigned short val;
	unsigned       reg;

	e86_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e86_get_reg16 (c, reg);

	e86_set_ea16 (c, val);
	e86_set_clk_ea (c, 2, 9);

	return (c->ea.cnt + 1);
}

/* OP 8A: MOV reg8, r/m8 */
static
unsigned op_8a (e8086_t *c)
{
	unsigned char val;
	unsigned      reg;

	e86_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e86_get_ea8 (c);

	e86_set_reg8 (c, reg, val);
	e86_set_clk_ea (c, 2, 8);

	return (c->ea.cnt + 1);
}

/* OP 8B: MOV reg16, r/m16 */
static
unsigned op_8b (e8086_t *c)
{
	unsigned short val;
	unsigned       reg;

	e86_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e86_get_ea16 (c);

	e86_set_reg16 (c, reg, val);
	e86_set_clk_ea (c, 2, 8);

	return (c->ea.cnt + 1);
}

/* OP 8C: MOV r/m16, sreg */
static
unsigned op_8c (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_ea16 (c, c->sreg[(c->pq[1] >> 3) & 3]);
	e86_set_clk_ea (c, 2, 9);

	return (c->ea.cnt + 1);
}

/* OP 8D: LEA reg16, r/m16 */
static
unsigned op_8d (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);

	if (c->ea.is_mem) {
		e86_set_reg16 (c, (c->pq[1] >> 3) & 7, c->ea.ofs);
		e86_set_clk (c, 2);
	}
	else {
		if (e86_undefined (c) == 0) {
			return (0);
		}
	}

	return (c->ea.cnt + 1);
}

/* OP 8E: MOV sreg, r/m16 */
static
unsigned op_8e (e8086_t *c)
{
	unsigned reg;

	reg = (c->pq[1] >> 3) & 3;

	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_sreg (c, reg, e86_get_ea16 (c));
	e86_set_clk_ea (c, 2, 8);

	if (reg == E86_REG_CS) {
		e86_pq_init (c);
	}

	return (c->ea.cnt + 1);
}

/* OP 8F: POP r/m16 */
static
unsigned op_8f (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_ea16 (c, e86_pop (c));
	e86_set_clk_ea (c, 8, 17);

	return (c->ea.cnt + 1);
}

/* OP 9x: XCHG AX, reg16 */
static
unsigned op_90 (e8086_t *c)
{
	unsigned       reg;
	unsigned short val;

	reg = c->pq[0] & 7;

	val = e86_get_ax (c);
	e86_set_ax (c, e86_get_reg16 (c, reg));
	e86_set_reg16 (c, reg, val);
	e86_set_clk (c, 3);

	return (1);
}

/* OP 98: CBW */
static
unsigned op_98 (e8086_t *c)
{
	e86_set_ax (c, e86_mk_sint16 (e86_get_al (c)));
	e86_set_clk (c, 2);

	return (1);
}

/* OP 99: CWD */
static
unsigned op_99 (e8086_t *c)
{
	e86_set_dx (c, (e86_get_ax (c) & 0x8000) ? 0xffff : 0x0000);
	e86_set_clk (c, 5);

	return (1);
}

/* OP 9A: CALL seg:ofs */
static
unsigned op_9a (e8086_t *c)
{
	e86_push (c, e86_get_cs (c));
	e86_push (c, e86_get_ip (c) + 5);

	e86_set_ip (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_cs (c, e86_mk_uint16 (c->pq[3], c->pq[4]));

	e86_pq_init (c);

	e86_set_clk (c, 28);

	return (0);
}

/* OP 9B: WAIT */
static
unsigned op_9b (e8086_t *c)
{
	e86_set_clk (c, 4);

	return (1);
}

/* OP 9C: PUSHF */
static
unsigned op_9c (e8086_t *c)
{
	if (c->cpu & E86_CPU_FLAGS286) {
		e86_push (c, c->flg & 0x0fd5);
	}
	else {
		e86_push (c, (c->flg & 0x0fd5) | 0xf002);
	}

	e86_set_clk (c, 10);

	return (1);
}

/* OP 9D: POPF */
static
unsigned op_9d (e8086_t *c)
{
	c->flg = (e86_pop (c) & 0x0fd5) | 0xf002;
	e86_set_clk (c, 8);

	return (1);
}

/* OP 9E: SAHF */
static
unsigned op_9e (e8086_t *c)
{
	c->flg &= 0xff00;
	c->flg |= e86_get_ah (c) & 0xd5;
	c->flg |= 0x02;

	e86_set_clk (c, 4);

	return (1);
}

/* OP 9F: LAHF */
static
unsigned op_9f (e8086_t *c)
{
	e86_set_ah (c, (c->flg & 0xd5) | 0x02);
	e86_set_clk (c, 4);

	return (1);
}

/* OP A0: MOV AL, [data16] */
static
unsigned op_a0 (e8086_t *c)
{
	unsigned short seg, ofs;
	unsigned char  val;

	seg = e86_get_seg (c, E86_REG_DS);
	ofs = e86_mk_uint16 (c->pq[1], c->pq[2]);
	val = e86_get_mem8 (c, seg, ofs);

	e86_set_al (c, val);
	e86_set_clk (c, 10);

	return (3);
}

/* OP A1: MOV AX, [data16] */
static
unsigned op_a1 (e8086_t *c)
{
	unsigned short seg, ofs;

	seg = e86_get_seg (c, E86_REG_DS);
	ofs = e86_mk_uint16 (c->pq[1], c->pq[2]);

	e86_set_ax (c, e86_get_mem16 (c, seg, ofs));
	e86_set_clk (c, 10);

	return (3);
}

/* OP A2: MOV [data16], AL */
static
unsigned op_a2 (e8086_t *c)
{
	unsigned short seg, ofs;

	seg = e86_get_seg (c, E86_REG_DS);
	ofs = e86_mk_uint16 (c->pq[1], c->pq[2]);

	e86_set_mem8 (c, seg, ofs, c->dreg[E86_REG_AX] & 0xff);
	e86_set_clk (c, 10);

	return (3);
}

/* OP A3: MOV [data16], AX */
static
unsigned op_a3 (e8086_t *c)
{
	unsigned short seg, ofs;

	seg = e86_get_seg (c, E86_REG_DS);
	ofs = e86_mk_uint16 (c->pq[1], c->pq[2]);

	e86_set_mem16 (c, seg, ofs, c->dreg[E86_REG_AX]);
	e86_set_clk (c, 10);

	return (3);
}

/* OP A4: MOVSB */
static
unsigned op_a4 (e8086_t *c)
{
	unsigned char  val;
	unsigned short seg1, seg2;
	unsigned short inc;

	seg1 = e86_get_seg (c, E86_REG_DS);
	seg2 = e86_get_es (c);
	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		unsigned end = 0;

		if ((c->cpu & E86_CPU_REP_BUG) && e86_get_if (c) && (c->prefix & E86_PREFIX_SEG)) {
			end = 1;
		}

		while (e86_get_cx (c) > end) {
			val = e86_get_mem8 (c, seg1, e86_get_si (c));
			e86_set_mem8 (c, seg2, e86_get_di (c), val);

			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			c->instructions += 1;
			e86_set_clk (c, 18);
		}
	}
	else {
		val = e86_get_mem8 (c, seg1, e86_get_si (c));
		e86_set_mem8 (c, seg2, e86_get_di (c), val);

		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_clk (c, 18);
	}

	return (1);
}

/* OP A5: MOVSW */
static
unsigned op_a5 (e8086_t *c)
{
	unsigned short val;
	unsigned short seg1, seg2;
	unsigned short inc;

	seg1 = e86_get_seg (c, E86_REG_DS);
	seg2 = e86_get_es (c);
	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		unsigned end = 0;

		if ((c->cpu & E86_CPU_REP_BUG) && e86_get_if (c) && (c->prefix & E86_PREFIX_SEG)) {
			end = 1;
		}

		while (e86_get_cx (c) > end) {
			val = e86_get_mem16 (c, seg1, e86_get_si (c));
			e86_set_mem16 (c, seg2, e86_get_di (c), val);

			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			c->instructions += 1;
			e86_set_clk (c, 18);
		}
	}
	else {
		val = e86_get_mem16 (c, seg1, e86_get_si (c));
		e86_set_mem16 (c, seg2, e86_get_di (c), val);

		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_clk (c, 18);
	}

	return (1);
}

/* Opcode A6: CMPSB */
static
unsigned op_a6 (e8086_t *c)
{
	unsigned short s1, s2;
	unsigned short seg1, seg2;
	unsigned short inc;
	int            z;

	seg1 = e86_get_seg (c, E86_REG_DS);
	seg2 = e86_get_es (c);

	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		unsigned end = 0;

		if ((c->cpu & E86_CPU_REP_BUG) && e86_get_if (c) && (c->prefix & E86_PREFIX_SEG)) {
			end = 1;
		}

		z = (c->prefix & E86_PREFIX_REP) ? 1 : 0;

		while (e86_get_cx (c) > end) {
			s1 = e86_get_mem8 (c, seg1, e86_get_si (c));
			s2 = e86_get_mem8 (c, seg2, e86_get_di (c));

			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			e86_set_flg_sub_8 (c, s1, s2);

			c->instructions += 1;
			e86_set_clk (c, 22);

			if (e86_get_zf (c) != z) {
				break;
			}
		}
	}
	else {
		s1 = e86_get_mem8 (c, seg1, e86_get_si (c));
		s2 = e86_get_mem8 (c, seg2, e86_get_di (c));

		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_flg_sub_8 (c, s1, s2);

		e86_set_clk (c, 22);
	}

	return (1);
}

/* Opcode A7: CMPSW */
static
unsigned op_a7 (e8086_t *c)
{
	unsigned long  s1, s2;
	unsigned short seg1, seg2;
	unsigned short inc;
	int            z;

	seg1 = e86_get_seg (c, E86_REG_DS);
	seg2 = e86_get_es (c);

	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		unsigned end = 0;

		if ((c->cpu & E86_CPU_REP_BUG) && e86_get_if (c) && (c->prefix & E86_PREFIX_SEG)) {
			end = 1;
		}

		z = (c->prefix & E86_PREFIX_REP) ? 1 : 0;

		while (e86_get_cx (c) > end) {
			s1 = e86_get_mem16 (c, seg1, e86_get_si (c));
			s2 = e86_get_mem16 (c, seg2, e86_get_di (c));

			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			e86_set_flg_sub_16 (c, s1, s2);

			c->instructions += 1;
			e86_set_clk (c, 22);

			if (e86_get_zf (c) != z) {
				break;
			}
		}
	}
	else {
		s1 = e86_get_mem16 (c, seg1, e86_get_si (c));
		s2 = e86_get_mem16 (c, seg2, e86_get_di (c));

		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_flg_sub_16 (c, s1, s2);

		e86_set_clk (c, 22);
	}

	return (1);
}

/* Opcode A8: TEST AL, data8 */
static
unsigned op_a8 (e8086_t *c)
{
	unsigned short s1, s2;

	s1 = e86_get_al (c);
	s2 = c->pq[1];

	e86_set_flg_log_8 (c, s1 & s2);
	e86_set_clk (c, 4);

	return (2);
}

/* Opcode A9: TEST AX, data16 */
static
unsigned op_a9 (e8086_t *c)
{
	unsigned long s1, s2;

	s1 = e86_get_ax (c);
	s2 = e86_mk_uint16 (c->pq[1], c->pq[2]);

	e86_set_flg_log_16 (c, s1 & s2);
	e86_set_clk (c, 4);

	return (3);
}

/* Opcode AA: STOSB */
static
unsigned op_aa (e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e86_get_es (c);
	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx (c) > 0) {
			e86_set_mem8 (c, seg, e86_get_di (c), e86_get_al (c));

			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			c->instructions += 1;
			e86_set_clk (c, 11);
		}
	}
	else {
		e86_set_mem8 (c, seg, e86_get_di (c), e86_get_al (c));
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_clk (c, 11);
	}

	return (1);
}

/* Opcode AB: STOSW */
static
unsigned op_ab (e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e86_get_es (c);
	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx (c) > 0) {
			e86_set_mem16 (c, seg, e86_get_di (c), e86_get_ax (c));

			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			c->instructions += 1;
			e86_set_clk (c, 11);
		}
	}
	else {
		e86_set_mem16 (c, seg, e86_get_di (c), e86_get_ax (c));
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_clk (c, 11);
	}

	return (1);
}

/* Opcode AC: LODSB */
static
unsigned op_ac (e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e86_get_seg (c, E86_REG_DS);
	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		unsigned end = 0;

		if ((c->cpu & E86_CPU_REP_BUG) && e86_get_if (c) && (c->prefix & E86_PREFIX_SEG)) {
			end = 1;
		}

		while (e86_get_cx (c) > end) {
			e86_set_al (c, e86_get_mem8 (c, seg, e86_get_si (c)));
			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);
			c->instructions += 1;
			e86_set_clk (c, 12);
		}
	}
	else {
		e86_set_al (c, e86_get_mem8 (c, seg, e86_get_si (c)));
		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_clk (c, 12);
	}

	return (1);
}

/* Opcode AD: LODSW */
static
unsigned op_ad (e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e86_get_seg (c, E86_REG_DS);
	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		unsigned end = 0;

		if ((c->cpu & E86_CPU_REP_BUG) && e86_get_if (c) && (c->prefix & E86_PREFIX_SEG)) {
			end = 1;
		}

		while (e86_get_cx (c) > end) {
			e86_set_ax (c, e86_get_mem16 (c, seg, e86_get_si (c)));
			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);
			c->instructions += 1;
			e86_set_clk (c, 12);
		}
	}
	else {
		e86_set_ax (c, e86_get_mem16 (c, seg, e86_get_si (c)));
		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_clk (c, 12);
	}

	return (1);
}

/* Opcode AE: SCASB */
static
unsigned op_ae (e8086_t *c)
{
	unsigned short s1, s2;
	unsigned short seg;
	unsigned short inc;
	int            z;

	seg = e86_get_es (c);
	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		z = (c->prefix & E86_PREFIX_REP) ? 1 : 0;

		while (e86_get_cx (c) > 0) {
			s1 = e86_get_al (c);
			s2 = e86_get_mem8 (c, seg, e86_get_di (c));

			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			e86_set_flg_sub_8 (c, s1, s2);

			c->instructions += 1;
			e86_set_clk (c, 15);

			if (e86_get_zf (c) != z) {
				break;
			}
		}
	}
	else {
		s1 = e86_get_al (c);
		s2 = e86_get_mem8 (c, seg, e86_get_di (c));

		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_flg_sub_8 (c, s1, s2);
		e86_set_clk (c, 15);
	}

	return (1);
}

/* Opcode AF: SCASW */
static
unsigned op_af (e8086_t *c)
{
	unsigned long  s1, s2;
	unsigned short seg;
	unsigned short inc;
	int            z;

	seg = e86_get_es (c);
	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		z = (c->prefix & E86_PREFIX_REP) ? 1 : 0;

		while (e86_get_cx (c) > 0) {
			s1 = e86_get_ax (c);
			s2 = e86_get_mem16 (c, seg, e86_get_di (c));

			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			e86_set_flg_sub_16 (c, s1, s2);

			c->instructions += 1;
			e86_set_clk (c, 15);

			if (e86_get_zf (c) != z) {
				break;
			}
		}
	}
	else {
		s1 = e86_get_ax (c);
		s2 = e86_get_mem16 (c, seg, e86_get_di (c));

		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_flg_sub_16 (c, s1, s2);
		e86_set_clk (c, 15);
	}

	return (1);
}

/* OP B0: MOV AL, imm8 */
static
unsigned op_b0 (e8086_t *c)
{
	e86_set_al (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B1: MOV CL, imm8 */
static
unsigned op_b1 (e8086_t *c)
{
	e86_set_cl (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B2: MOV DL, imm8 */
static
unsigned op_b2 (e8086_t *c)
{
	e86_set_dl (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B3: MOV BL, imm8 */
static
unsigned op_b3 (e8086_t *c)
{
	e86_set_bl (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B4: MOV AH, imm8 */
static
unsigned op_b4 (e8086_t *c)
{
	e86_set_ah (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B5: MOV CH, imm8 */
static
unsigned op_b5 (e8086_t *c)
{
	e86_set_ch (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B6: MOV DH, imm8 */
static
unsigned op_b6 (e8086_t *c)
{
	e86_set_dh (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B7: MOV BH, imm8 */
static
unsigned op_b7 (e8086_t *c)
{
	e86_set_bh (c, c->pq[1]);
	e86_set_clk (c, 4);

	return (2);
}

/* OP B8: MOV AX, imm16 */
static
unsigned op_b8 (e8086_t *c)
{
	e86_set_ax (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP B9: MOV CX, imm16 */
static
unsigned op_b9 (e8086_t *c)
{
	e86_set_cx (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP BA: MOV DX, imm16 */
static
unsigned op_ba (e8086_t *c)
{
	e86_set_dx (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP BB: MOV BX, imm16 */
static
unsigned op_bb (e8086_t *c)
{
	e86_set_bx (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP BC: MOV SP, imm16 */
static
unsigned op_bc (e8086_t *c)
{
	e86_set_sp (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP BD: MOV BP, imm16 */
static
unsigned op_bd (e8086_t *c)
{
	e86_set_bp (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP BE: MOV SI, imm16 */
static
unsigned op_be (e8086_t *c)
{
	e86_set_si (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP BF: MOV DI, imm16 */
static
unsigned op_bf (e8086_t *c)
{
	e86_set_di (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 4);

	return (3);
}

/* OP C2: RET imm16 */
static
unsigned op_c2 (e8086_t *c)
{
	e86_set_ip (c, e86_pop (c));
	e86_set_sp (c, e86_get_sp (c) + e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_pq_init (c);
	e86_set_clk (c, 20);

	return (0);
}

/* OP C3: RET */
static
unsigned op_c3 (e8086_t *c)
{
	e86_set_ip (c, e86_pop (c));
	e86_pq_init (c);
	e86_set_clk (c, 16);

	return (0);
}

/* OP C4: LES reg16, r/m16 */
static
unsigned op_c4 (e8086_t *c)
{
	unsigned short val1, val2;

	e86_get_ea_ptr (c, c->pq + 1);

	if (c->ea.is_mem) {
		val1 = e86_get_mem16 (c, c->ea.seg, c->ea.ofs);
		val2 = e86_get_mem16 (c, c->ea.seg, c->ea.ofs + 2);

		e86_set_reg16 (c, (c->pq[1] >> 3) & 7, val1);
		e86_set_es (c, val2);

		e86_set_clk (c, 16);
	}

	return (c->ea.cnt + 1);
}

/* OP C5: LDS reg16, r/m16 */
static
unsigned op_c5 (e8086_t *c)
{
	unsigned short val1, val2;

	e86_get_ea_ptr (c, c->pq + 1);

	if (c->ea.is_mem) {
		val1 = e86_get_mem16 (c, c->ea.seg, c->ea.ofs);
		val2 = e86_get_mem16 (c, c->ea.seg, c->ea.ofs + 2);

		e86_set_reg16 (c, (c->pq[1] >> 3) & 7, val1);
		e86_set_ds (c, val2);

		e86_set_clk (c, 16);
	}

	return (c->ea.cnt + 1);
}

/* OP C6: MOV r/m8, data8 */
static
unsigned op_c6 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_ea8 (c, c->pq[c->ea.cnt + 1]);
	e86_set_clk_ea (c, 4, 10);

	return (c->ea.cnt + 2);
}

/* OP C7: MOV r/m16, data16 */
static
unsigned op_c7 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_ea16 (c, e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]));
	e86_set_clk_ea (c, 4, 10);

	return (c->ea.cnt + 3);
}

/* OP CA: RETF data16 */
static
unsigned op_ca (e8086_t *c)
{
	e86_set_ip (c, e86_pop (c));
	e86_set_cs (c, e86_pop (c));
	e86_set_sp (c, e86_get_sp (c) + e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_pq_init (c);
	e86_set_clk (c, 25);

	return (0);
}

/* OP CB: RETF */
static
unsigned op_cb (e8086_t *c)
{
	e86_set_ip (c, e86_pop (c));
	e86_set_cs (c, e86_pop (c));
	e86_pq_init (c);
	e86_set_clk (c, 26);

	return (0);
}

/* OP CC: INT 3 */
static
unsigned op_cc (e8086_t *c)
{
	e86_set_ip (c, e86_get_ip (c) + 1);
	e86_trap (c, 3);

	e86_pq_init (c);

	e86_set_clk (c, 52);

	return (0);
}

/* OP CD: INT imm8 */
static
unsigned op_cd (e8086_t *c)
{
	e86_set_ip (c, e86_get_ip (c) + 2);
	e86_trap (c, c->pq[1]);

	e86_pq_init (c);

	e86_set_clk (c, 51);

	return (0);
}

/* OP CE: INTO */
static
unsigned op_ce (e8086_t *c)
{
	if (e86_get_of (c)) {
		e86_set_ip (c, e86_get_ip (c) + 1);
		e86_trap (c, 4);

		e86_pq_init (c);

		e86_set_clk (c, 53);

		return (0);
	}

	e86_set_clk (c, 4);

	return (1);
}

/* OP CF: IRET */
static
unsigned op_cf (e8086_t *c)
{
	e86_set_ip (c, e86_pop (c));
	e86_set_cs (c, e86_pop (c));
	c->flg = e86_pop (c);

	e86_pq_init (c);

	e86_set_clk (c, 32);

	return (0);
}

/* OP D0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, 1 */
static
unsigned op_d0 (e8086_t *c)
{
	unsigned       xop;
	unsigned short d, s;

	xop = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);
	s = e86_get_ea8 (c);

	switch (xop) {
	case 0: /* ROL r/m8, 1 */
		d = (s << 1) | (s >> 7);
		e86_set_cf (c, s & 0x80);
		break;

	case 1: /* ROR r/m8, 1 */
		d = (s >> 1) | (s << 7);
		e86_set_cf (c, s & 1);
		break;

	case 2: /* RCL r/m8, 1 */
		d = (s << 1) | e86_get_cf (c);
		e86_set_cf (c, s & 0x80);
		break;

	case 3: /* RCR r/m8, 1 */
		d = (s >> 1) | (e86_get_cf (c) << 7);
		e86_set_cf (c, s & 1);
		break;

	case 4: /* SHL r/m8, 1 */
		d = s << 1;
		e86_set_flg_szp_8 (c, d);
		e86_set_cf (c, s & 0x80);
		break;

	case 5: /* SHR r/m8, 1 */
		d = s >> 1;
		e86_set_flg_szp_8 (c, d);
		e86_set_cf (c, s & 1);
		break;

	case 7: /* SAR r/m8, 1 */
		d = (s >> 1) | (s & 0x80);
		e86_set_flg_szp_8 (c, d);
		e86_set_cf (c, s & 1);
		break;

	default:
		return (op_ud (c));
	}

	e86_set_of (c, (d ^ s) & 0x80);

	e86_set_ea8 (c, d & 0xff);
	e86_set_clk_ea (c, 2, 15);

	return (c->ea.cnt + 1);
}

/* OP D1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, 1 */
static
unsigned op_d1 (e8086_t *c)
{
	unsigned      xop;
	unsigned long d, s;

	xop = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);
	s = e86_get_ea16 (c);

	switch (xop) {
	case 0: /* ROL r/m16, 1 */
		d = (s << 1) | (s >> 15);
		e86_set_cf (c, s & 0x8000);
		break;

	case 1: /* ROR r/m16, 1 */
		d = (s >> 1) | (s << 15);
		e86_set_cf (c, s & 1);
		break;

	case 2: /* RCL r/m16, 1 */
		d = (s << 1) | e86_get_cf (c);
		e86_set_cf (c, s & 0x8000);
		break;

	case 3: /* RCR r/m16, 1 */
		d = (s >> 1) | (e86_get_cf (c) << 15);
		e86_set_cf (c, s & 1);
		break;

	case 4: /* SHL r/m16, 1 */
		d = s << 1;
		e86_set_flg_szp_16 (c, d);
		e86_set_cf (c, s & 0x8000);
		break;

	case 5: /* SHR r/m16, 1 */
		d = s >> 1;
		e86_set_flg_szp_16 (c, d);
		e86_set_cf (c, s & 1);
		break;

	case 7: /* SAR r/m16, 1 */
		d = (s >> 1) | (s & 0x8000);
		e86_set_flg_szp_16 (c, d);
		e86_set_cf (c, s & 1);
		break;

	default:
		return (op_ud (c));
	}

	e86_set_of (c, (d ^ s) & 0x8000);

	e86_set_ea16 (c, d & 0xffff);
	e86_set_clk_ea (c, 2, 15);

	return (c->ea.cnt + 1);
}

/* OP D2: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, CL */
static
unsigned op_d2 (e8086_t *c)
{
	unsigned       xop;
	unsigned       cnt;
	unsigned short d, s;

	xop = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);
	s = e86_get_ea8 (c);

	cnt = e86_get_cl (c);

	if (c->cpu & E86_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}

	if (cnt == 0) {
		e86_set_clk_ea (c, 8, 20);
		return (c->ea.cnt + 1);
	}

	switch (xop) {
	case 0: /* ROL r/m8, CL */
		d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
		e86_set_cf (c, d & 1);
		break;

	case 1: /* ROR r/m8, CL */
		d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
		e86_set_cf (c, d & 0x80);
		break;

	case 2: /* RCL r/m8, CL */
		s |= e86_get_cf (c) << 8;
		d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
		e86_set_cf (c, d & 0x100);
		break;

	case 3: /* RCR r/m8, CL */
		s |= e86_get_cf (c) << 8;
		d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
		e86_set_cf (c, d & 0x100);
		break;

	case 4: /* SHL r/m8, CL */
		d = (cnt > 8) ? 0 : (s << cnt);
		e86_set_flg_szp_8 (c, d);
		e86_set_cf (c, d & 0x100);
		break;

	case 5: /* SHR r/m8, CL */
		d = (cnt > 8) ? 0 : (s >> (cnt - 1));
		e86_set_cf (c, d & 1);
		d = d >> 1;
		e86_set_flg_szp_8 (c, d);
		break;

	case 7: /* SAR r/m8, CL */
		s |= (s & 0x80) ? 0xff00 : 0x0000;
		d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
		e86_set_cf (c, d & 1);
		d = (d >> 1) & 0xff;
		e86_set_flg_szp_8 (c, d);
		break;

	default:
		return (op_ud (c));
	}

	e86_set_of (c, (d ^ s) & 0x80);

	e86_set_ea8 (c, d & 0xff);
	e86_set_clk_ea (c, 8 + 4 * cnt, 20 + 4 * cnt);

	return (c->ea.cnt + 1);
}

/* OP D3: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, CL */
static
unsigned op_d3 (e8086_t *c)
{
	unsigned      xop;
	unsigned      cnt;
	unsigned long d, s;

	xop = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);
	s = e86_get_ea16 (c);

	cnt = e86_get_cl (c);

	if (c->cpu & E86_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}

	if (cnt == 0) {
		e86_set_clk_ea (c, 8, 20);
		return (c->ea.cnt + 1);
	}

	switch (xop) {
	case 0: /* ROL r/m16, CL */
		d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
		e86_set_cf (c, d & 1);
		break;

	case 1: /* ROR r/m16, CL */
		d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
		e86_set_cf (c, d & 0x8000);
		break;

	case 2: /* RCL r/m16, CL */
		s |= (unsigned long) e86_get_cf (c) << 16;
		d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
		e86_set_cf (c, d & 0x10000);
		break;

	case 3: /* RCR r/m16, CL */
		s |= (unsigned long) e86_get_cf (c) << 16;
		d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
		e86_set_cf (c, d & 0x10000);
		break;

	case 4: /* SHL r/m16, CL */
		d = (cnt > 16) ? 0 : (s << cnt);
		e86_set_flg_szp_16 (c, d);
		e86_set_cf (c, d & 0x10000);
		break;

	case 5: /* SHR r/m16, CL */
		d = (cnt > 16) ? 0 : (s >> (cnt - 1));
		e86_set_cf (c, d & 1);
		d = d >> 1;
		e86_set_flg_szp_16 (c, d);
		break;

	case 7: /* SAR r/m16, CL */
		s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
		d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
		e86_set_cf (c, d & 1);
		d = (d >> 1) & 0xffff;
		e86_set_flg_szp_16 (c, d);
		break;

	default:
		return (op_ud (c));
	}

	e86_set_of (c, (d ^ s) & 0x8000);

	e86_set_ea16 (c, d);
	e86_set_clk_ea (c, 8 + 4 * cnt, 20 + 4 * cnt);

	return (c->ea.cnt + 1);
}

/* OP D4: AAM imm8 */
static
unsigned op_d4 (e8086_t *c)
{
	unsigned short s, d;
	unsigned short div;

	div = c->pq[1];

	/* check for division by zero */
	if (div == 0) {
		e86_trap (c, 0);
		return (0);
	}

	s = e86_get_al (c);
	d = (((s / div) & 0xff) << 8) | ((s % div) & 0xff);

	e86_set_ax (c, d);

	e86_set_flg_szp_16 (c, d);
	e86_set_clk (c, 83);

	return (2);
}

/* OP D5: AAD imm8 */
static
unsigned op_d5 (e8086_t *c)
{
	unsigned short s1, s2, d;
	unsigned short mul;

	mul = c->pq[1];

	s1 = e86_get_ah (c);
	s2 = e86_get_al (c);

	d = mul * s1 + s2;

	e86_set_ax (c, d & 0xff);
	e86_set_flg_szp_16 (c, d);
	e86_set_clk (c, 60);

	return (2);
}

/* OP D7: XLAT */
static
unsigned op_d7 (e8086_t *c)
{
	unsigned short seg, ofs;

	seg = e86_get_seg (c, E86_REG_DS);
	ofs = e86_get_bx (c) + e86_get_al (c);

	e86_set_al (c, e86_get_mem8 (c, seg, ofs));
	e86_set_clk (c, 11);

	return (1);
}

/* OP D8: ESC */
static
unsigned op_d8 (e8086_t *c)
{
	if (c->cpu & E86_CPU_INT7) {
		e86_trap (c, 7);
		e86_set_clk (c, 50);
		return (0);
	}

	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_clk (c, 2);

	return (1 + c->ea.cnt);
}

/* OP E0: LOOPNZ imm8 */
static
unsigned op_e0 (e8086_t *c)
{
	e86_set_cx (c, e86_get_cx (c) - 1);

	if ((e86_get_cx (c) != 0) && (e86_get_zf (c) == 0)) {
		e86_set_ip (c, e86_add_sint8 (c->ip, c->pq[1]) + 2);
		e86_pq_init (c);
		e86_set_clk (c, 19);

		return (0);
	}

	e86_set_clk (c, 5);

	return (2);
}

/* OP E1: LOOPZ imm8 */
static
unsigned op_e1 (e8086_t *c)
{
	e86_set_cx (c, e86_get_cx (c) - 1);

	if ((e86_get_cx (c) != 0) && (e86_get_zf (c) != 0)) {
		e86_set_ip (c, e86_add_sint8 (c->ip, c->pq[1]) + 2);
		e86_pq_init (c);
		e86_set_clk (c, 18);

		return (0);
	}

	e86_set_clk (c, 5);

	return (2);
}

/* OP E2: LOOP imm8 */
static
unsigned op_e2 (e8086_t *c)
{
	e86_set_cx (c, e86_get_cx (c) - 1);

	if (e86_get_cx (c) != 0) {
		e86_set_ip (c, e86_add_sint8 (c->ip, c->pq[1]) + 2);
		e86_pq_init (c);
		e86_set_clk (c, 18);

		return (0);
	}

	e86_set_clk (c, 5);

	return (2);
}

/* OP E3: JCXZ imm8 */
static
unsigned op_e3 (e8086_t *c)
{
	if (e86_get_cx (c) == 0) {
		e86_set_ip (c, e86_add_sint8 (c->ip + 2, c->pq[1]));
		e86_pq_init (c);
		e86_set_clk (c, 18);
		return (0);
	}

	e86_set_clk (c, 6);

	return (2);
}

/* OP E4: IN AL, imm8 */
static
unsigned op_e4 (e8086_t *c)
{
	e86_set_al (c, e86_get_prt8 (c, c->pq[1]));
	e86_set_clk (c, 10);

	return (2);
}

/* OP E5: IN AX, imm8 */
static
unsigned op_e5 (e8086_t *c)
{
	e86_set_ax (c, e86_get_prt16 (c, c->pq[1]));
	e86_set_clk (c, 14);

	return (2);
}

/* OP E6: OUT imm8, AL */
static
unsigned op_e6 (e8086_t *c)
{
	e86_set_prt8 (c, c->pq[1], e86_get_al (c));
	e86_set_clk (c, 10);

	return (2);
}

/* OP E7: OUT imm8, AX */
static
unsigned op_e7 (e8086_t *c)
{
	e86_set_prt16 (c, c->pq[1], e86_get_ax (c));
	e86_set_clk (c, 10);

	return (2);
}

/* OP E8: CALL imm16 */
static
unsigned op_e8 (e8086_t *c)
{
	e86_push (c, e86_get_ip (c) + 3);
	e86_set_ip (c, e86_get_ip (c) + 3 + e86_mk_uint16 (c->pq[1], c->pq[2]));

	e86_pq_init (c);

	e86_set_clk (c, 19);

	return (0);
}

/* OP E9: JMP imm16 */
static
unsigned op_e9 (e8086_t *c)
{
	e86_set_ip (c, c->ip + e86_mk_uint16 (c->pq[1], c->pq[2]) + 3);
	e86_pq_init (c);
	e86_set_clk (c, 15);

	return (0);
}

/* OP EA: JMP imm32 */
static
unsigned op_ea (e8086_t *c)
{
	e86_set_ip (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_cs (c, e86_mk_uint16 (c->pq[3], c->pq[4]));
	e86_pq_init (c);
	e86_set_clk (c, 15);

	return (0);
}

/* OP EB: JMP imm8 */
static
unsigned op_eb (e8086_t *c)
{
	e86_set_ip (c, e86_add_sint8 (c->ip, c->pq[1]) + 2);
	e86_pq_init (c);
	e86_set_clk (c, 15);

	return (0);
}

/* OP EC: IN AL, DX */
static
unsigned op_ec (e8086_t *c)
{
	e86_set_al (c, e86_get_prt8 (c, e86_get_dx (c)));
	e86_set_clk (c, 8);

	return (1);
}

/* OP ED: IN AX, DX */
static
unsigned op_ed (e8086_t *c)
{
	e86_set_ax (c, e86_get_prt16 (c, e86_get_dx (c)));
	e86_set_clk (c, 12);

	return (1);
}

/* OP EE: OUT DX, AL */
static
unsigned op_ee (e8086_t *c)
{
	e86_set_prt8 (c, e86_get_dx (c), e86_get_al (c));
	e86_set_clk (c, 8);

	return (1);
}

/* OP EF: OUT DX, AX */
static
unsigned op_ef (e8086_t *c)
{
	e86_set_prt16 (c, e86_get_dx (c), e86_get_ax (c));
	e86_set_clk (c, 8);

	return (1);
}

/* OP F0: LOCK */
static
unsigned op_f0 (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_LOCK);

	e86_set_clk (c, 2);

	return (1);
}

/* OP F2: REPNE */
static
unsigned op_f2 (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_REPN);
	e86_set_clk (c, 2);

	return (1);
}

/* OP F3: REP */
static
unsigned op_f3 (e8086_t *c)
{
	c->prefix |= (E86_PREFIX_NEW | E86_PREFIX_REP);
	e86_set_clk (c, 2);

	return (1);
}

/* OP F4: HLT */
static
unsigned op_f4 (e8086_t *c)
{
	c->halt = 1;

	e86_set_clk (c, 2);

	return (1);
}

/* OP F5: CMC */
static
unsigned op_f5 (e8086_t *c)
{
	c->flg ^= E86_FLG_C;
	e86_set_clk (c, 2);

	return (1);
}

/* OP F6 00: TEST r/m8, imm8 */
static
unsigned op_f6_00 (e8086_t *c)
{
	unsigned short s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	e86_set_flg_log_8 (c, s1 & s2);
	e86_set_clk_ea (c, 5, 11);

	return (c->ea.cnt + 2);
}

/* OP F6 02: NOT r/m8 */
static
unsigned op_f6_02 (e8086_t *c)
{
	unsigned short d, s;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea8 (c);
	d = ~s & 0xff;

	e86_set_ea8 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F6 03: NEG r/m8 */
static
unsigned op_f6_03 (e8086_t *c)
{
	unsigned short d, s;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea8 (c);
	d = (~s + 1) & 0xff;

	e86_set_ea8 (c, d);
	e86_set_flg_sub_8 (c, 0, s);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F6 04: MUL r/m8 */
static
unsigned op_f6_04 (e8086_t *c)
{
	unsigned short d;

	e86_get_ea_ptr (c, c->pq + 1);

	d = e86_get_ea8 (c) * e86_get_al (c);

	e86_set_ax (c, d);
	e86_set_f (c, E86_FLG_O | E86_FLG_C, d & 0xff00);
	e86_set_f (c, E86_FLG_Z, d == 0);
	e86_set_clk_ea (c, (70 + 77) / 2, (76 + 83) / 2);

	return (c->ea.cnt + 1);
}

/* OP F6 05: IMUL r/m8 */
static
unsigned op_f6_05 (e8086_t *c)
{
	unsigned short s;
	unsigned long  d;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea8 (c);

	d = (unsigned long) e86_mk_sint16 (s);
	d *= (unsigned long) e86_mk_sint16 (e86_get_al (c));
	d &= 0xffff;

	e86_set_ax (c, d);

	d &= 0xff00;

	e86_set_f (c, E86_FLG_C | E86_FLG_O, (d != 0xff00) && (d != 0x0000));

	e86_set_clk_ea (c, (80 + 98) / 2, (86 + 104) / 2);

	return (c->ea.cnt + 1);
}

/* OP F6 06: DIV r/m8 */
static
unsigned op_f6_06 (e8086_t *c)
{
	unsigned short s, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea8 (c);

	/* check for division by zero */
	if (s == 0) {
		return (op_divide_error (c));
	}

	d = e86_get_ax (c) / s;

	/* check for overflow */
	if (d & 0xff00) {
		return (op_divide_error (c));
	}

	e86_set_ax (c, ((e86_get_ax (c) % s) << 8) | d);

	e86_set_clk_ea (c, (80 + 90) / 2, (86 + 96) / 2);

	return (c->ea.cnt + 1);
}

/* OP F6 07: IDIV r/m8 */
static
unsigned op_f6_07 (e8086_t *c)
{
	unsigned short s1, s2, d1, d2;
	int            sign1, sign2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ax (c);
	s2 = e86_mk_sint16 (e86_get_ea8 (c));

	/* check for division by zero */
	if (s2 == 0) {
		return (op_divide_error (c));
	}

	sign1 = (s1 & 0x8000) != 0;
	sign2 = (s2 & 0x8000) != 0;

	s1 = sign1 ? ((~s1 + 1) & 0xffff) : s1;
	s2 = sign2 ? ((~s2 + 1) & 0xffff) : s2;

	d1 = s1 / s2;
	d2 = s1 % s2;

	if (sign1 != sign2) {
		if (d1 > 0x80) {
			return (op_divide_error (c));
		}

		d1 = (~d1 + 1) & 0xff;
	}
	else {
		if (d1 > 0x7f) {
			return (op_divide_error (c));
		}
	}

	if (sign1) {
		d2 = (~d2 + 1) & 0xff;
	}

	e86_set_ax (c, (d2 << 8) | d1);

	e86_set_clk_ea (c, (101 + 112) / 2, (107 + 118) / 2);

	return (c->ea.cnt + 1);
}

static
e86_opcode_f e86_opcodes_f6[8] = {
	&op_f6_00, &op_ud,    &op_f6_02, &op_f6_03,
	&op_f6_04, &op_f6_05, &op_f6_06, &op_f6_07,
};

/* OP F6: xxx r/m8 */
static
unsigned op_f6 (e8086_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e86_opcodes_f6[xop] (c));
}

/* OP F7 00: TEST r/m16, imm16 */
static
unsigned op_f7_00 (e8086_t *c)
{
	unsigned long s1, s2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	e86_set_flg_log_16 (c, s1 & s2);
	e86_set_clk_ea (c, 5, 11);

	return (c->ea.cnt + 3);
}

/* OP F7 02: NOT r/m16 */
static
unsigned op_f7_02 (e8086_t *c)
{
	unsigned long d, s;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea16 (c);
	d = ~s & 0xffff;

	e86_set_ea16 (c, d);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F7 03: NEG r/m16 */
static
unsigned op_f7_03 (e8086_t *c)
{
	unsigned long d, s;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea16 (c);
	d = (~s + 1) & 0xffff;

	e86_set_ea16 (c, d);
	e86_set_flg_sub_16 (c, 0, s);
	e86_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F7 04: MUL r/m16 */
static
unsigned op_f7_04 (e8086_t *c)
{
	unsigned long d;

	e86_get_ea_ptr (c, c->pq + 1);

	d = e86_get_ea16 (c) * e86_get_ax (c);

	e86_set_ax (c, d & 0xffff);
	e86_set_dx (c, d >> 16);
	e86_set_f (c, E86_FLG_C | E86_FLG_O, d & 0xffff0000);
	e86_set_f (c, E86_FLG_Z, d == 0);
	e86_set_clk_ea (c, (118 + 113) / 2, (124 + 139) / 2);

	return (c->ea.cnt + 1);
}

/* OP F7 05: IMUL r/m16 */
static
unsigned op_f7_05 (e8086_t *c)
{
	unsigned long  s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ax (c);
	s2 = e86_get_ea16 (c);

	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;

	d = (s1 * s2) & 0xffffffff;

	e86_set_ax (c, d & 0xffff);
	e86_set_dx (c, d >> 16);

	d &= 0xffff0000;

	e86_set_f (c, E86_FLG_C | E86_FLG_O, (d != 0xffff0000) && (d != 0x00000000));

	e86_set_clk_ea (c, (128 + 154) / 2, (134 + 160) / 2);

	return (c->ea.cnt + 1);
}

/* OP F7 06: DIV r/m16 */
static
unsigned op_f7_06 (e8086_t *c)
{
	unsigned long s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s2 = e86_get_ea16 (c);

	/* check for division by zero */
	if (s2 == 0) {
		return (op_divide_error (c));
	}

	s1 = ((unsigned long) e86_get_dx (c) << 16) | e86_get_ax (c);

	d = s1 / s2;

	/* check for overflow */
	if (d & 0xffff0000) {
		return (op_divide_error (c));
	}

	e86_set_ax (c, d);
	e86_set_dx (c, s1 % s2);

	e86_set_clk_ea (c, (144 + 162) / 2, (150 + 168) / 2);

	return (c->ea.cnt + 1);
}

/* OP F7 07: IDIV r/m16 */
static
unsigned op_f7_07 (e8086_t *c)
{
	unsigned long s1, s2, d1, d2;
	int           sign1, sign2;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = ((unsigned long) e86_get_dx (c) << 16) | e86_get_ax (c);
	s2 = e86_get_ea16 (c);
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;

	if (s2 == 0) {
		return (op_divide_error (c));
	}

	sign1 = (s1 & 0x80000000) != 0;
	sign2 = (s2 & 0x80000000) != 0;

	s1 = sign1 ? ((~s1 + 1) & 0xffffffff) : s1;
	s2 = sign2 ? ((~s2 + 1) & 0xffffffff) : s2;

	d1 = s1 / s2;
	d2 = s1 % s2;

	if (sign1 != sign2) {
		if (d1 > 0x8000) {
			return (op_divide_error (c));
		}

		d1 = (~d1 + 1) & 0xffff;
	}
	else {
		if (d1 > 0x7fff) {
			return (op_divide_error (c));
		}
	}

	if (sign1) {
		d2 = (~d2 + 1) & 0xffff;
	}

	e86_set_ax (c, d1);
	e86_set_dx (c, d2);

	e86_set_clk_ea (c, (165 + 184) / 2, (171 + 190) / 2);

	return (c->ea.cnt + 1);
}

static
e86_opcode_f e86_opcodes_f7[8] = {
	&op_f7_00, &op_ud,    &op_f7_02, &op_f7_03,
	&op_f7_04, &op_f7_05, &op_f7_06, &op_f7_07,
};

/* OP F7: xxx r/m16 */
static
unsigned op_f7 (e8086_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e86_opcodes_f7[xop] (c));
}

/* OP F8: CLC */
static
unsigned op_f8 (e8086_t *c)
{
	e86_set_cf (c, 0);
	e86_set_clk (c, 2);

	return (1);
}

/* OP F9: STC */
static
unsigned op_f9 (e8086_t *c)
{
	e86_set_cf (c, 1);
	e86_set_clk (c, 2);

	return (1);
}

/* OP FA: CLI */
static
unsigned op_fa (e8086_t *c)
{
	e86_set_if (c, 0);
	e86_set_clk (c, 2);

	return (1);
}

/* OP FB: STI */
static
unsigned op_fb (e8086_t *c)
{
	e86_set_if (c, 1);
	e86_set_clk (c, 2);

	return (1);
}

/* OP FC: CLD */
static
unsigned op_fc (e8086_t *c)
{
	e86_set_df (c, 0);
	e86_set_clk (c, 2);

	return (1);
}

/* OP FD: STD */
static
unsigned op_fd (e8086_t *c)
{
	e86_set_df (c, 1);
	e86_set_clk (c, 2);

	return (1);
}

/* OP FE: INC, DEC r/m8 */
static
unsigned op_fe (e8086_t *c)
{
	unsigned       xop;
	unsigned short d, s;
	unsigned short cf;

	xop = (c->pq[1] >> 3) & 7;

	switch (xop) {
		case 0: /* INC r/m8 */
			e86_get_ea_ptr (c, c->pq + 1);
			s = e86_get_ea8 (c);

			d = s + 1;

			e86_set_ea8 (c, d);

			cf = c->flg;
			e86_set_flg_add_8 (c, s, 1);
			c->flg ^= (c->flg ^ cf) & E86_FLG_C;

			e86_set_clk_ea (c, 3, 15);

			return (c->ea.cnt + 1);

		case 1: /* DEC r/m8 */
			e86_get_ea_ptr (c, c->pq + 1);
			s = e86_get_ea8 (c);

			d = s - 1;

			e86_set_ea8 (c, d);

			cf = c->flg;
			e86_set_flg_sub_8 (c, s, 1);
			c->flg ^= (c->flg ^ cf) & E86_FLG_C;

			e86_set_clk_ea (c, 3, 15);

			return (c->ea.cnt + 1);

		default:
			return (op_ud (c));
	}

	return (0);
}

/* OP FF 00: INC r/m16 */
static
unsigned op_ff_00 (e8086_t *c)
{
	unsigned long  s, d;
	unsigned short cf;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea16 (c);
	d = s + 1;

	e86_set_ea16 (c, d);

	cf = c->flg;
	e86_set_flg_add_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E86_FLG_C;

	e86_set_clk_ea (c, 3, 15);

	return (c->ea.cnt + 1);
}

/* OP FF 01: DEC r/m16 */
static
unsigned op_ff_01 (e8086_t *c)
{
	unsigned long  s, d;
	unsigned short cf;

	e86_get_ea_ptr (c, c->pq + 1);

	s = e86_get_ea16 (c);
	d = s - 1;

	e86_set_ea16 (c, d);

	cf = c->flg;
	e86_set_flg_sub_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E86_FLG_C;

	e86_set_clk_ea (c, 3, 15);

	return (c->ea.cnt + 1);
}

/* OP FF 02: CALL r/m16 */
static
unsigned op_ff_02 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);

	e86_push (c, e86_get_ip (c) + c->ea.cnt + 1);
	e86_set_ip (c, e86_get_ea16 (c));

	e86_pq_init (c);

	e86_set_clk_ea (c, 16, 21);

	return (0);
}

/* OP FF 03: CALL m32 */
static
unsigned op_ff_03 (e8086_t *c)
{
	unsigned short seg, ofs;

	e86_get_ea_ptr (c, c->pq + 1);

	if (!c->ea.is_mem) {
		return (c->ea.cnt + 1);
	}

	e86_push (c, e86_get_cs (c));
	e86_push (c, e86_get_ip (c) + c->ea.cnt + 1);

	seg = e86_get_sego (c, c->ea.seg);
	ofs = c->ea.ofs;

	e86_set_ip (c, e86_get_mem16 (c, seg, ofs));
	e86_set_cs (c, e86_get_mem16 (c, seg, ofs + 2));

	e86_pq_init (c);

	e86_set_clk (c, 37);

	return (0);
}

/* OP FF 04: JMP r/m16 */
static
unsigned op_ff_04 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);
	e86_set_ip (c, e86_get_ea16 (c));
	e86_pq_init (c);
	e86_set_clk_ea (c, 11, 18);

	return (0);
}

/* OP FF 05: JMP m32 */
static
unsigned op_ff_05 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);

	if (!c->ea.is_mem) {
		return (c->ea.cnt + 1);
	}

	e86_set_ip (c, e86_get_mem16 (c, c->ea.seg, c->ea.ofs));
	e86_set_cs (c, e86_get_mem16 (c, c->ea.seg, c->ea.ofs + 2));

	e86_pq_init (c);

	e86_set_clk (c, 24);

	return (0);
}

/* OP FF 06: PUSH r/m16 */
static
unsigned op_ff_06 (e8086_t *c)
{
	e86_get_ea_ptr (c, c->pq + 1);

	if ((c->ea.is_mem == 0) && (c->ea.ofs == E86_REG_SP) && ((c->cpu & E86_CPU_PUSH_FIRST) == 0)) {
		e86_push (c, e86_get_sp (c) - 2);
	}
	else {
		e86_push (c, e86_get_ea16 (c));
	}

	e86_set_clk_ea (c, 10, 16);

	return (c->ea.cnt + 1);
}

static
e86_opcode_f e86_opcodes_ff[8] = {
	&op_ff_00, &op_ff_01, &op_ff_02, &op_ff_03,
	&op_ff_04, &op_ff_05, &op_ff_06, &op_ud
};

/* OP FF: xxx r/m16 */
static
unsigned op_ff (e8086_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e86_opcodes_ff[xop] (c));
}

e86_opcode_f e86_opcodes[256] = {
	&op_00, &op_01, &op_02, &op_03, &op_04, &op_05, &op_06, &op_07,
	&op_08, &op_09, &op_0a, &op_0b, &op_0c, &op_0d, &op_0e, &op_0f,
	&op_10, &op_11, &op_12, &op_13, &op_14, &op_15, &op_16, &op_17,
	&op_18, &op_19, &op_1a, &op_1b, &op_1c, &op_1d, &op_1e, &op_1f,
	&op_20, &op_21, &op_22, &op_23, &op_24, &op_25, &op_26, &op_27, /* 20 */
	&op_28, &op_29, &op_2a, &op_2b, &op_2c, &op_2d, &op_2e, &op_2f,
	&op_30, &op_31, &op_32, &op_33, &op_34, &op_35, &op_36, &op_37, /* 30 */
	&op_38, &op_39, &op_3a, &op_3b, &op_3c, &op_3d, &op_3e, &op_3f,
	&op_40, &op_40, &op_40, &op_40, &op_40, &op_40, &op_40, &op_40, /* 40 */
	&op_48, &op_48, &op_48, &op_48, &op_48, &op_48, &op_48, &op_48,
	&op_50, &op_50, &op_50, &op_50, &op_50, &op_50, &op_50, &op_50, /* 50 */
	&op_58, &op_58, &op_58, &op_58, &op_58, &op_58, &op_58, &op_58,
	&op_ud, &op_ud, &op_ud, &op_ud, &op_ud, &op_ud, &op_66, &op_ud, /* 60 */
	&op_ud, &op_ud, &op_ud, &op_ud, &op_ud, &op_ud, &op_ud, &op_ud,
	&op_70, &op_71, &op_72, &op_73, &op_74, &op_75, &op_76, &op_77, /* 70 */
	&op_78, &op_79, &op_7a, &op_7b, &op_7c, &op_7d, &op_7e, &op_7f,
	&op_80, &op_81, &op_80, &op_83, &op_84, &op_85, &op_86, &op_87, /* 80 */
	&op_88, &op_89, &op_8a, &op_8b, &op_8c, &op_8d, &op_8e, &op_8f,
	&op_90, &op_90, &op_90, &op_90, &op_90, &op_90, &op_90, &op_90, /* 90 */
	&op_98, &op_99, &op_9a, &op_9b, &op_9c, &op_9d, &op_9e, &op_9f,
	&op_a0, &op_a1, &op_a2, &op_a3, &op_a4, &op_a5, &op_a6, &op_a7, /* A0 */
	&op_a8, &op_a9, &op_aa, &op_ab, &op_ac, &op_ad, &op_ae, &op_af,
	&op_b0, &op_b1, &op_b2, &op_b3, &op_b4, &op_b5, &op_b6, &op_b7, /* B0 */
	&op_b8, &op_b9, &op_ba, &op_bb, &op_bc, &op_bd, &op_be, &op_bf,
	&op_ud, &op_ud, &op_c2, &op_c3, &op_c4, &op_c5, &op_c6, &op_c7, /* C0 */
	&op_ud, &op_ud, &op_ca, &op_cb, &op_cc, &op_cd, &op_ce, &op_cf,
	&op_d0, &op_d1, &op_d2, &op_d3, &op_d4, &op_d5, &op_ud, &op_d7, /* D0 */
	&op_d8, &op_d8, &op_d8, &op_d8, &op_d8, &op_d8, &op_d8, &op_d8,
	&op_e0, &op_e1, &op_e2, &op_e3, &op_e4, &op_e5, &op_e6, &op_e7, /* E0 */
	&op_e8, &op_e9, &op_ea, &op_eb, &op_ec, &op_ed, &op_ee, &op_ef,
	&op_f0, &op_ud, &op_f2, &op_f3, &op_f4, &op_f5, &op_f6, &op_f7, /* F0 */
	&op_f8, &op_f9, &op_fa, &op_fb, &op_fc, &op_fd, &op_fe, &op_ff
};
