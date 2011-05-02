/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e6502/opcodes.c                                      *
 * Created:     2004-05-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e6502.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


unsigned char e6502_pull (e6502_t *c)
{
	unsigned char val;

	e6502_set_s (c, e6502_get_s (c) + 1);
	val = e6502_get_mem8 (c, 0x0100 + e6502_get_s (c));

	return (val);
}

unsigned short e6502_pull16 (e6502_t *c)
{
	unsigned char val1, val2;

	val1 = e6502_get_mem8 (c, 0x0100 + ((e6502_get_s (c) + 1) & 0xff));
	val2 = e6502_get_mem8 (c, 0x0100 + ((e6502_get_s (c) + 2) & 0xff));
	e6502_set_s (c, e6502_get_s (c) + 2);

	return (e6502_mk_uint16 (val1, val2));
}

void e6502_push (e6502_t *c, unsigned char val)
{
	e6502_set_mem8 (c, 0x0100 + e6502_get_s (c), val);
	e6502_set_s (c, e6502_get_s (c) - 1);
}

void e6502_push16 (e6502_t *c, unsigned short val)
{
	e6502_set_mem8 (c, 0x0100 + e6502_get_s (c), val >> 8);
	e6502_set_mem8 (c, 0x0100 + ((e6502_get_s (c) - 1) & 0xff), val & 0xff);
	e6502_set_s (c, e6502_get_s (c) - 2);
}

void e6502_trap (e6502_t *c, unsigned short addr)
{
	e6502_push16 (c, e6502_get_pc (c));
	e6502_push (c, e6502_get_p (c));

	e6502_set_if (c, 1);

	e6502_set_pc (c, e6502_get_mem16 (c, addr));
	e6502_set_clk (c, 0, 7);
}


static
void e6502_op_adc (e6502_t *c, unsigned char s2)
{
	unsigned char  s1, s3;
	unsigned short d;

	s1 = e6502_get_a (c);
	s3 = e6502_get_cf (c);

	if (e6502_get_df (c)) {
		d = (s1 & 0x0f) + (s2 & 0x0f) + s3;
		if (d > 0x09) {
			d = ((d + 0x06) & 0x0f) + 0x10;
		}
		d = d + (s1 & 0xf0) + (s2 & 0xf0);

		e6502_set_nf (c, d & 0x80);
		e6502_set_vf (c, ~(s1 ^ s2) & (s1 ^ d) & 0x80);

		if ((d & 0xfff0) > 0x90) {
			d += 0x60;
		}

		e6502_set_zf (c, ((s1 + s2 + e6502_get_cf (c)) & 0xff) == 0);
	}
	else {
		d = s1 + s2 + s3;

		e6502_set_nf (c, d & 0x80);
		e6502_set_zf (c, (d & 0xff) == 0);
		e6502_set_vf (c, ~(s1 ^ s2) & (s1 ^ d) & 0x80);
	}

	e6502_set_cf (c, d & 0xff00);

	e6502_set_a (c, d);
}

static
void e6502_op_and (e6502_t *c, unsigned char s2)
{
	unsigned char  s1;
	unsigned short d;

	s1 = e6502_get_a (c);
	d = s1 & s2;
	e6502_set_a (c, d);

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, (d & 0xff) == 0);
}

static
unsigned char e6502_op_asl (e6502_t *c, unsigned char s)
{
	unsigned char d;

	d = (s << 1) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, d == 0);
	e6502_set_cf (c, s & 0x80);

	return (d);
}

static
void e6502_op_bcc (e6502_t *c, int cond)
{
	unsigned short addr1, addr2;

	e6502_get_inst1 (c);

	if (cond) {
		addr1 = e6502_get_pc (c);
		addr2 = (addr1 + 2 + e6502_mk_sint16 (c->inst[1])) & 0xffff;
		e6502_set_pc (c, addr2);
		e6502_set_clk (c, 0, ((addr1 ^ addr2) & 0xff00) ? 4 : 3);
	}
	else {
		e6502_set_clk (c, 2, 2);
	}
}

static
void e6502_op_bit (e6502_t *c, unsigned char s)
{
	e6502_set_nf (c, s & 0x80);
	e6502_set_vf (c, s & 0x40);
	e6502_set_zf (c, (e6502_get_a (c) & s) == 0);
}

static
void e6502_op_cmp (e6502_t *c, unsigned char s)
{
	unsigned char d, a;

	a = e6502_get_a (c);
	d = (a - s) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_cf (c, s <= a);
	e6502_set_zf (c, d == 0);
}

static
void e6502_op_cpx (e6502_t *c, unsigned char s)
{
	unsigned char d, x;

	x = e6502_get_x (c);
	d = (x - s) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_cf (c, s <= x);
	e6502_set_zf (c, d == 0);
}

static
void e6502_op_cpy (e6502_t *c, unsigned char s)
{
	unsigned char d, y;

	y = e6502_get_y (c);
	d = (y - s) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_cf (c, s <= y);
	e6502_set_zf (c, d == 0);
}

static
unsigned char e6502_op_dec (e6502_t *c, unsigned char s)
{
	unsigned char d;

	d = (s - 1) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, d == 0);

	return (d);
}

static
void e6502_op_eor (e6502_t *c, unsigned char s2)
{
	unsigned char  s1;
	unsigned short d;

	s1 = e6502_get_a (c);
	d = s1 ^ s2;
	e6502_set_a (c, d);

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, (d & 0xff) == 0);
}

static
unsigned char e6502_op_inc (e6502_t *c, unsigned char s)
{
	unsigned char d;

	d = (s + 1) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, d == 0);

	return (d);
}

static
void e6502_op_lda (e6502_t *c, unsigned char s)
{
	e6502_set_a (c, s);
	e6502_set_nf (c, s & 0x80);
	e6502_set_zf (c, s == 0);
}

static
void e6502_op_ldx (e6502_t *c, unsigned char s)
{
	e6502_set_x (c, s);
	e6502_set_nf (c, s & 0x80);
	e6502_set_zf (c, s == 0);
}

static
void e6502_op_ldy (e6502_t *c, unsigned char s)
{
	e6502_set_y (c, s);
	e6502_set_nf (c, s & 0x80);
	e6502_set_zf (c, s == 0);
}

static
unsigned char e6502_op_lsr (e6502_t *c, unsigned char s)
{
	unsigned char d;

	d = (s >> 1) & 0x7f;

	e6502_set_nf (c, 0);
	e6502_set_zf (c, d == 0);
	e6502_set_cf (c, s & 0x01);

	return (d);
}

static
void e6502_op_ora (e6502_t *c, unsigned char s2)
{
	unsigned char  s1;
	unsigned short d;

	s1 = e6502_get_a (c);
	d = s1 | s2;
	e6502_set_a (c, d);

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, (d & 0xff) == 0);
}

static
unsigned char e6502_op_rol (e6502_t *c, unsigned char s)
{
	unsigned char d;

	d = ((s << 1) | e6502_get_cf (c)) & 0xff;

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, d == 0);
	e6502_set_cf (c, s & 0x80);

	return (d);
}

static
unsigned char e6502_op_ror (e6502_t *c, unsigned char s)
{
	unsigned char d;

	d = ((s >> 1) & 0x7f) | (e6502_get_cf (c) ? 0x80 : 0x00);

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, d == 0);
	e6502_set_cf (c, s & 0x01);

	return (d);
}

static
void e6502_op_sbc (e6502_t *c, unsigned char s2)
{
	unsigned char  s1, s3;
	unsigned short d;

	s1 = e6502_get_a (c);
	s3 = (e6502_get_cf (c) == 0);

	if (e6502_get_df (c)) {
		d = (s1 & 0x0f) - (s2 & 0x0f) - s3;
		if (d & 0x10) {
			d = ((d - 0x06) & 0x0f) | ((s1 & 0xf0) - (s2 & 0xf0) - 0x10);
		}
		else {
			d = (d & 0x0f) | ((s1 & 0xf0) - (s2 & 0xf0));
		}
		if (d & 0x100) {
			d -= 0x60;
		}
	}
	else {
		d = s1 - s2 - s3;
	}

	e6502_set_nf (c, d & 0x80);
	e6502_set_zf (c, (d & 0xff) == 0);
	e6502_set_vf (c, (s1 ^ s2) & (s1 ^ d) & 0x80);
	e6502_set_cf (c, (d & 0xff00) == 0);

	e6502_set_a (c, d);
}

static
void e6502_op_setnz (e6502_t *c, unsigned char s)
{
	e6502_set_nf (c, s & 0x80);
	e6502_set_zf (c, (s & 0xff) == 0);
}


/*****************************************************************************
 * opcodes
 *****************************************************************************/

/* Handle an undefined opcode */
static void op_ud (e6502_t *c)
{
	if (e6502_hook_undefined (c)) {
		return;
	}

	e6502_set_clk (c, 1, 1);
}

/* OP 00: BRK */
static void op_00 (e6502_t *c)
{
	if (e6502_hook_brk (c)) {
		return;
	}

	e6502_push16 (c, e6502_get_pc (c) + 2);
	e6502_push (c, e6502_get_p (c) | E6502_FLG_B);

	e6502_set_if (c, 1);
	e6502_set_pc (c, e6502_get_mem16 (c, 0xfffe));

	e6502_set_clk (c, 0, 7);
}

/* OP 01: ORA [[xx + X]] */
static void op_01 (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP 05: ORA [xx] */
static void op_05 (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 06: ASL [xx] */
static void op_06 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_asl (c, e6502_get_zpg (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 5);
}

/* OP 08: PHP */
static void op_08 (e6502_t *c)
{
	e6502_push (c, e6502_get_p (c));
	e6502_set_clk (c, 1, 3);
}

/* OP 09: ORA #xx */
static void op_09 (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP 0A: ASL */
static void op_0a (e6502_t *c)
{
	e6502_set_a (c, e6502_op_asl (c, e6502_get_a (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP 0D: ORA [xxxx] */
static void op_0d (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 0E: ASL [xxxx] */
static void op_0e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_asl (c, e6502_get_abs (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 6);
}

/* OP 10: BPL */
static void op_10 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_nf (c) == 0);
}

/* OP 11: ORA [[xx] + Y] */
static void op_11 (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP 15: ORA [xx + X] */
static void op_15 (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 16: ASL [xx + X] */
static void op_16 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_asl (c, e6502_get_zpg_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 6);
}

/* OP 18: CLC */
static void op_18 (e6502_t *c)
{
	e6502_set_cf (c, 0);
	e6502_set_clk (c, 1, 2);
}

/* OP 19: ORA [xxxx + Y] */
static void op_19 (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 1D: ORA [xxxx + X] */
static void op_1d (e6502_t *c)
{
	e6502_op_ora (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 1E: ASL [xxxx + X] */
static void op_1e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_asl (c, e6502_get_abs_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 7);
}

/* OP 20: JSR xxxx */
static void op_20 (e6502_t *c)
{
	e6502_get_inst2 (c);

	e6502_push16 (c, e6502_get_pc (c) + 2);
	e6502_set_pc (c, e6502_mk_uint16 (c->inst[1], c->inst[2]));

	e6502_set_clk (c, 0, 6);
}

/* OP 21: AND [[xx + X]] */
static void op_21 (e6502_t *c)
{
	e6502_op_and (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP 24: BIT [xx] */
static void op_24 (e6502_t *c)
{
	e6502_op_bit (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 25: AND [xx] */
static void op_25 (e6502_t *c)
{
	e6502_op_and (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 26: ROL [xx] */
static void op_26 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_rol (c, e6502_get_zpg (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 5);
}

/* OP 28: PLP */
static void op_28 (e6502_t *c)
{
	e6502_set_p (c, e6502_pull (c));
	e6502_set_bf (c, 0);
	e6502_set_rf (c, 1);
	e6502_set_clk (c, 1, 4);
}

/* OP 29: AND #xx */
static void op_29 (e6502_t *c)
{
	e6502_op_and (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP 2A: ROL */
static void op_2a (e6502_t *c)
{
	e6502_set_a (c, e6502_op_rol (c, e6502_get_a (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP 2C: BIT [xxxx] */
static void op_2c (e6502_t *c)
{
	e6502_op_bit (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 2D: AND [xxxx] */
static void op_2d (e6502_t *c)
{
	e6502_op_and (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 2E: ROL [xxxx] */
static void op_2e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_rol (c, e6502_get_abs (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 6);
}

/* OP 30: BMI */
static void op_30 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_nf (c) != 0);
}

/* OP 31: AND [[xx] + Y] */
static void op_31 (e6502_t *c)
{
	e6502_op_and (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP 35: AND [xx + X] */
static void op_35 (e6502_t *c)
{
	e6502_op_and (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 36: ROL [xx + X] */
static void op_36 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_rol (c, e6502_get_zpg_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 6);
}

/* OP 38: SEC */
static void op_38 (e6502_t *c)
{
	e6502_set_cf (c, 1);
	e6502_set_clk (c, 1, 2);
}

/* OP 39: AND [xxxx + Y] */
static void op_39 (e6502_t *c)
{
	e6502_op_and (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 3D: AND [xxxx + X] */
static void op_3d (e6502_t *c)
{
	e6502_op_and (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 3E: ROL [xxxx + X] */
static void op_3e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_rol (c, e6502_get_abs_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 7);
}

/* OP 40: RTI */
static void op_40 (e6502_t *c)
{
	e6502_set_p (c, e6502_pull (c));
	e6502_set_pc (c, e6502_pull16 (c));
	e6502_set_rf (c, 1);
	e6502_set_bf (c, 0);
	e6502_set_clk (c, 0, 6);

	c->check_irq = (e6502_get_if (c) == 0);
}

/* OP 41: EOR [[xx + X]] */
static void op_41 (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP 45: EOR [xx] */
static void op_45 (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 46: LSR [xx] */
static void op_46 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_lsr (c, e6502_get_zpg (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 5);
}

/* OP 48: PHA */
static void op_48 (e6502_t *c)
{
	e6502_push (c, e6502_get_a (c));
	e6502_set_clk (c, 1, 3);
}

/* OP 49: EOR #xx */
static void op_49 (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP 4A: LSR */
static void op_4a (e6502_t *c)
{
	e6502_set_a (c, e6502_op_lsr (c, e6502_get_a (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP 4C: JMP xxxx */
static void op_4c (e6502_t *c)
{
	e6502_get_inst2 (c);
	e6502_set_pc (c, e6502_mk_uint16 (c->inst[1], c->inst[2]));
	e6502_set_clk (c, 0, 3);
}

/* OP 4D: EOR [xxxx] */
static void op_4d (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 4E: LSR [xxxx] */
static void op_4e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_lsr (c, e6502_get_abs (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 6);
}

/* OP 51: EOR [[xx] + Y] */
static void op_51 (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP 55: EOR [xx + X] */
static void op_55 (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 56: LSR [xx + X] */
static void op_56 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_lsr (c, e6502_get_zpg_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 6);
}

/* OP 59: EOR [xxxx + Y] */
static void op_59 (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 5D: EOR [xxxx + X] */
static void op_5d (e6502_t *c)
{
	e6502_op_eor (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 5E: LSR [xxxx + X] */
static void op_5e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_lsr (c, e6502_get_abs_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 7);
}

/* OP 50: BVC */
static void op_50 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_vf (c) == 0);
}

/* OP 58: CLI */
static void op_58 (e6502_t *c)
{
	e6502_set_if (c, 0);
	e6502_set_clk (c, 1, 2);
}

/* OP 60: RTS */
static void op_60 (e6502_t *c)
{
	e6502_set_pc (c, e6502_pull16 (c) + 1);

	e6502_set_clk (c, 0, 6);
}

/* OP 61: ADC [[xx + X]] */
static void op_61 (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP 65: ADC [xx] */
static void op_65 (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 66: ROR [xx] */
static void op_66 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_ror (c, e6502_get_zpg (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 5);
}

/* OP 68: PLA */
static void op_68 (e6502_t *c)
{
	unsigned char val;

	val = e6502_pull (c);

	e6502_set_a (c, val);
	e6502_set_nf (c, val & 0x80);
	e6502_set_zf (c, (val & 0xff) == 0);

	e6502_set_clk (c, 1, 4);
}

/* OP 69: ADC #xx */
static void op_69 (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP 6A: ROR */
static void op_6a (e6502_t *c)
{
	e6502_set_a (c, e6502_op_ror (c, e6502_get_a (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP 6C: JMP [xxxx] */
static void op_6c (e6502_t *c)
{
	unsigned ial, iah, adl, adh;

	e6502_get_inst2 (c);

	ial = c->inst[1];
	iah = (unsigned) c->inst[2] << 8;

	adl = e6502_get_mem8 (c, iah | ial);
	adh = e6502_get_mem8 (c, iah | ((ial + 1) & 0xff));

	e6502_set_pc (c, (adh << 8) | adl);
	e6502_set_clk (c, 0, 5);
}

/* OP 6D: ADC [xxxx] */
static void op_6d (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 6E: ROR [xxxx] */
static void op_6e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_ror (c, e6502_get_abs (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 6);
}

/* OP 70: BVS */
static void op_70 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_vf (c) != 0);
}

/* OP 71: ADC [[xx] + Y] */
static void op_71 (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP 75: ADC [xx + X] */
static void op_75 (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 76: ROR [xx + X] */
static void op_76 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_ror (c, e6502_get_zpg_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 6);
}

/* OP 78: SEI */
static void op_78 (e6502_t *c)
{
	e6502_set_if (c, 1);
	e6502_set_clk (c, 1, 2);
}

/* OP 79: ADC [xxxx + Y] */
static void op_79 (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 7D: ADC [xxxx + X] */
static void op_7d (e6502_t *c)
{
	e6502_op_adc (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP 7E: ROR [xxxx + X] */
static void op_7e (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_ror (c, e6502_get_abs_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 7);
}

/* OP 81: STA [[xx + X]] */
static void op_81 (e6502_t *c)
{
	e6502_get_ea_idx_ind_x (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 2, 6);
}

/* OP 84: STY [xx] */
static void op_84 (e6502_t *c)
{
	e6502_get_ea_zpg (c);
	e6502_set_ea (c, e6502_get_y (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 85: STA [xx] */
static void op_85 (e6502_t *c)
{
	e6502_get_ea_zpg (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 86: STX [xx] */
static void op_86 (e6502_t *c)
{
	e6502_get_ea_zpg (c);
	e6502_set_ea (c, e6502_get_x (c));
	e6502_set_clk (c, 2, 3);
}

/* OP 88: DEY */
static void op_88 (e6502_t *c)
{
	e6502_set_y (c, e6502_op_dec (c, e6502_get_y (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP 8A: TXA */
static void op_8a (e6502_t *c)
{
	e6502_op_setnz (c, e6502_get_x (c));
	e6502_set_a (c, e6502_get_x (c));
	e6502_set_clk (c, 1, 2);
}

/* OP 8C: STY [xxxx] */
static void op_8c (e6502_t *c)
{
	e6502_get_ea_abs (c);
	e6502_set_ea (c, e6502_get_y (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 8D: STA [xxxx] */
static void op_8d (e6502_t *c)
{
	e6502_get_ea_abs (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 8E: STX [xxxx] */
static void op_8e (e6502_t *c)
{
	e6502_get_ea_abs (c);
	e6502_set_ea (c, e6502_get_x (c));
	e6502_set_clk (c, 3, 4);
}

/* OP 90: BCC */
static void op_90 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_cf (c) == 0);
}

/* OP 91: STA [[xx] + Y] */
static void op_91 (e6502_t *c)
{
	e6502_get_ea_ind_idx_y (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 2, 6);
}

/* OP 94: STY [xx + X] */
static void op_94 (e6502_t *c)
{
	e6502_get_ea_zpg_x (c);
	e6502_set_ea (c, e6502_get_y (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 95: STA [xx + X] */
static void op_95 (e6502_t *c)
{
	e6502_get_ea_zpg_x (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 96: STX [xx + Y] */
static void op_96 (e6502_t *c)
{
	e6502_get_ea_zpg_y (c);
	e6502_set_ea (c, e6502_get_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP 98: TYA */
static void op_98 (e6502_t *c)
{
	e6502_op_setnz (c, e6502_get_y (c));
	e6502_set_a (c, e6502_get_y (c));
	e6502_set_clk (c, 1, 2);
}

/* OP 99: STA [xxxx + Y] */
static void op_99 (e6502_t *c)
{
	e6502_get_ea_abs_y (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 3, 5);
}

/* OP 89: TXS */
static void op_9a (e6502_t *c)
{
	e6502_set_s (c, e6502_get_x (c));
	e6502_set_clk (c, 1, 2);
}

/* OP 9D: STA [xxxx + X] */
static void op_9d (e6502_t *c)
{
	e6502_get_ea_abs_x (c);
	e6502_set_ea (c, e6502_get_a (c));
	e6502_set_clk (c, 3, 5);
}

/* OP A0: LDY #xx */
static void op_a0 (e6502_t *c)
{
	e6502_op_ldy (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP A1: LDA [[xx + X]] */
static void op_a1 (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP A2: LDX #xx */
static void op_a2 (e6502_t *c)
{
	e6502_op_ldx (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP A4: LDY [xx] */
static void op_a4 (e6502_t *c)
{
	e6502_op_ldy (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP A5: LDA [xx] */
static void op_a5 (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP A6: LDX [xx] */
static void op_a6 (e6502_t *c)
{
	e6502_op_ldx (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP A8: TAY */
static void op_a8 (e6502_t *c)
{
	e6502_op_setnz (c, e6502_get_a (c));
	e6502_set_y (c, e6502_get_a (c));
	e6502_set_clk (c, 1, 2);
}

/* OP A9: LDA #xx */
static void op_a9 (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP AA: TAX */
static void op_aa (e6502_t *c)
{
	e6502_op_setnz (c, e6502_get_a (c));
	e6502_set_x (c, e6502_get_a (c));
	e6502_set_clk (c, 1, 2);
}

/* OP AC: LDY [xxxx] */
static void op_ac (e6502_t *c)
{
	e6502_op_ldy (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP AD: LDA [xxxx] */
static void op_ad (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP AE: LDX [xxxx] */
static void op_ae (e6502_t *c)
{
	e6502_op_ldx (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP B0: BCS */
static void op_b0 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_cf (c) != 0);
}

/* OP B1: LDA [[xx] + Y] */
static void op_b1 (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP B4: LDY [xx + X] */
static void op_b4 (e6502_t *c)
{
	e6502_op_ldy (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP B5: LDA [xx + X] */
static void op_b5 (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP B6: LDX [xx + Y] */
static void op_b6 (e6502_t *c)
{
	e6502_op_ldx (c, e6502_get_zpg_y (c));
	e6502_set_clk (c, 2, 4);
}

/* OP B8: CLV */
static void op_b8 (e6502_t *c)
{
	e6502_set_vf (c, 0);
	e6502_set_clk (c, 1, 2);
}

/* OP B9: LDA [xxxx + Y] */
static void op_b9 (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP BA: TSX */
static void op_ba (e6502_t *c)
{
	e6502_op_setnz (c, e6502_get_s (c));
	e6502_set_x (c, e6502_get_s (c));
	e6502_set_clk (c, 1, 2);
}

/* OP BC: LDY [xxxx + X] */
static void op_bc (e6502_t *c)
{
	e6502_op_ldy (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP BD: LDA [xxxx + X] */
static void op_bd (e6502_t *c)
{
	e6502_op_lda (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP BD: LDX [xxxx + Y] */
static void op_be (e6502_t *c)
{
	e6502_op_ldx (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP C0: CPY #xx */
static void op_c0 (e6502_t *c)
{
	e6502_op_cpy (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP C1: CMP [[xx + X]] */
static void op_c1 (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP C4: CPY [xx] */
static void op_c4 (e6502_t *c)
{
	e6502_op_cpy (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP C5: CMP [xx] */
static void op_c5 (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP C6: DEC [xx] */
static void op_c6 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_dec (c, e6502_get_zpg (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 5);
}

/* OP C8: INY */
static void op_c8 (e6502_t *c)
{
	e6502_set_y (c, e6502_op_inc (c, e6502_get_y (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP C9: CMP #xx */
static void op_c9 (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP CA: DEX */
static void op_ca (e6502_t *c)
{
	e6502_set_x (c, e6502_op_dec (c, e6502_get_x (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP CC: CPY [xxxx] */
static void op_cc (e6502_t *c)
{
	e6502_op_cpy (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP CD: CMP [xxxx] */
static void op_cd (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP CE: DEC [xxxx] */
static void op_ce (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_dec (c, e6502_get_abs (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 6);
}

/* OP D1: CMP [[xx] + Y] */
static void op_d1 (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP D5: CMP [xx + X] */
static void op_d5 (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP D6: DEC [xx + X] */
static void op_d6 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_dec (c, e6502_get_zpg_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 6);
}

/* OP D9: CMP [xxxx + Y] */
static void op_d9 (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP DD: CMP [xxxx + X] */
static void op_dd (e6502_t *c)
{
	e6502_op_cmp (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP DE: DEC [xxxx + X] */
static void op_de (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_dec (c, e6502_get_abs_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 7);
}

/* OP D0: BNE */
static void op_d0 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_zf (c) == 0);
}

/* OP D8: CLD */
static void op_d8 (e6502_t *c)
{
	e6502_set_df (c, 0);
	e6502_set_clk (c, 1, 2);
}

/* OP E0: CPX #xx */
static void op_e0 (e6502_t *c)
{
	e6502_op_cpx (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP E1: SBC [[xx + X]] */
static void op_e1 (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_idx_ind_x (c));
	e6502_set_clk (c, 2, 6);
}

/* OP E4: CPX [xx] */
static void op_e4 (e6502_t *c)
{
	e6502_op_cpx (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP E5: SBC [xx] */
static void op_e5 (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_zpg (c));
	e6502_set_clk (c, 2, 3);
}

/* OP E6: INC [xx] */
static void op_e6 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_inc (c, e6502_get_zpg (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 5);
}

/* OP E8: INX */
static void op_e8 (e6502_t *c)
{
	e6502_set_x (c, e6502_op_inc (c, e6502_get_x (c)));
	e6502_set_clk (c, 1, 2);
}

/* OP E9: SBC #xx */
static void op_e9 (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_imm (c));
	e6502_set_clk (c, 2, 2);
}

/* OP EA: NOP */
static void op_ea (e6502_t *c)
{
	e6502_set_clk (c, 1, 2);
}

/* OP EC: CPX [xxxx] */
static void op_ec (e6502_t *c)
{
	e6502_op_cpx (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP ED: SBC [xxxx] */
static void op_ed (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_abs (c));
	e6502_set_clk (c, 3, 4);
}

/* OP EE: INC [xxxx] */
static void op_ee (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_inc (c, e6502_get_abs (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 6);
}

/* OP F0: BEQ */
static void op_f0 (e6502_t *c)
{
	e6502_op_bcc (c, e6502_get_zf (c) != 0);
}

/* OP F1: SBC [[xx] + Y] */
static void op_f1 (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_ind_idx_y (c));
	e6502_set_clk (c, 2, 5 + c->ea_page);
}

/* OP F5: SBC [xx + X] */
static void op_f5 (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_zpg_x (c));
	e6502_set_clk (c, 2, 4);
}

/* OP F6: INC [xx + X] */
static void op_f6 (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_inc (c, e6502_get_zpg_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 2, 6);
}

/* OP F8: SED */
static void op_f8 (e6502_t *c)
{
	e6502_set_df (c, 1);
	e6502_set_clk (c, 1, 2);
}

/* OP F9: SBC [xxxx + Y] */
static void op_f9 (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_abs_y (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP FD: SBC [xxxx + X] */
static void op_fd (e6502_t *c)
{
	e6502_op_sbc (c, e6502_get_abs_x (c));
	e6502_set_clk (c, 3, 4 + c->ea_page);
}

/* OP FE: INC [xxxx + X] */
static void op_fe (e6502_t *c)
{
	unsigned char tmp;

	tmp = e6502_op_inc (c, e6502_get_abs_x (c));
	e6502_set_ea (c, tmp);

	e6502_set_clk (c, 3, 7);
}

e6502_opcode_f e6502_opcodes[256] = {
	&op_00, &op_01, &op_ud, &op_ud, &op_ud, &op_05, &op_06, &op_ud, /* 00 */
	&op_08, &op_09, &op_0a, &op_ud, &op_ud, &op_0d, &op_0e, &op_ud,
	&op_10, &op_11, &op_ud, &op_ud, &op_ud, &op_15, &op_16, &op_ud, /* 10 */
	&op_18, &op_19, &op_ud, &op_ud, &op_ud, &op_1d, &op_1e, &op_ud,
	&op_20, &op_21, &op_ud, &op_ud, &op_24, &op_25, &op_26, &op_ud, /* 20 */
	&op_28, &op_29, &op_2a, &op_ud, &op_2c, &op_2d, &op_2e, &op_ud,
	&op_30, &op_31, &op_ud, &op_ud, &op_ud, &op_35, &op_36, &op_ud, /* 30 */
	&op_38, &op_39, &op_ud, &op_ud, &op_ud, &op_3d, &op_3e, &op_ud,
	&op_40, &op_41, &op_ud, &op_ud, &op_ud, &op_45, &op_46, &op_ud, /* 40 */
	&op_48, &op_49, &op_4a, &op_ud, &op_4c, &op_4d, &op_4e, &op_ud,
	&op_50, &op_51, &op_ud, &op_ud, &op_ud, &op_55, &op_56, &op_ud, /* 50 */
	&op_58, &op_59, &op_ud, &op_ud, &op_ud, &op_5d, &op_5e, &op_ud,
	&op_60, &op_61, &op_ud, &op_ud, &op_ud, &op_65, &op_66, &op_ud, /* 60 */
	&op_68, &op_69, &op_6a, &op_ud, &op_6c, &op_6d, &op_6e, &op_ud,
	&op_70, &op_71, &op_ud, &op_ud, &op_ud, &op_75, &op_76, &op_ud, /* 70 */
	&op_78, &op_79, &op_ud, &op_ud, &op_ud, &op_7d, &op_7e, &op_ud,
	&op_ud, &op_81, &op_ud, &op_ud, &op_84, &op_85, &op_86, &op_ud, /* 80 */
	&op_88, &op_ud, &op_8a, &op_ud, &op_8c, &op_8d, &op_8e, &op_ud,
	&op_90, &op_91, &op_ud, &op_ud, &op_94, &op_95, &op_96, &op_ud, /* 90 */
	&op_98, &op_99, &op_9a, &op_ud, &op_ud, &op_9d, &op_ud, &op_ud,
	&op_a0, &op_a1, &op_a2, &op_ud, &op_a4, &op_a5, &op_a6, &op_ud, /* A0 */
	&op_a8, &op_a9, &op_aa, &op_ud, &op_ac, &op_ad, &op_ae, &op_ud,
	&op_b0, &op_b1, &op_ud, &op_ud, &op_b4, &op_b5, &op_b6, &op_ud, /* B0 */
	&op_b8, &op_b9, &op_ba, &op_ud, &op_bc, &op_bd, &op_be, &op_ud,
	&op_c0, &op_c1, &op_ud, &op_ud, &op_c4, &op_c5, &op_c6, &op_ud, /* C0 */
	&op_c8, &op_c9, &op_ca, &op_ud, &op_cc, &op_cd, &op_ce, &op_ud,
	&op_d0, &op_d1, &op_ud, &op_ud, &op_ud, &op_d5, &op_d6, &op_ud, /* D0 */
	&op_d8, &op_d9, &op_ud, &op_ud, &op_ud, &op_dd, &op_de, &op_ud,
	&op_e0, &op_e1, &op_ud, &op_ud, &op_e4, &op_e5, &op_e6, &op_ud, /* E0 */
	&op_e8, &op_e9, &op_ea, &op_ud, &op_ec, &op_ed, &op_ee, &op_ud,
	&op_f0, &op_f1, &op_ud, &op_ud, &op_ud, &op_f5, &op_f6, &op_ud, /* F0 */
	&op_f8, &op_f9, &op_ud, &op_ud, &op_ud, &op_fd, &op_fe, &op_ud
};
