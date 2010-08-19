/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/e8086/e80186.c                                           *
 * Created:     2003-08-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>


/* OP 60: PUSHA */
static
unsigned op_60 (e8086_t *c)
{
	unsigned short tmp;

	tmp = e86_get_sp (c);

	e86_push (c, e86_get_ax (c));
	e86_push (c, e86_get_cx (c));
	e86_push (c, e86_get_dx (c));
	e86_push (c, e86_get_bx (c));
	e86_push (c, tmp);
	e86_push (c, e86_get_bp (c));
	e86_push (c, e86_get_si (c));
	e86_push (c, e86_get_di (c));

	e86_set_clk (c, 19);

	return (1);
}

/* OP 61: POPA */
static
unsigned op_61 (e8086_t *c)
{
	e86_set_di (c, e86_pop (c));
	e86_set_si (c, e86_pop (c));
	e86_set_bp (c, e86_pop (c));
	e86_pop (c);
	e86_set_bx (c, e86_pop (c));
	e86_set_dx (c, e86_pop (c));
	e86_set_cx (c, e86_pop (c));
	e86_set_ax (c, e86_pop (c));

	e86_set_clk (c, 19);

	return (1);
}

/* OP 62: BOUND reg16, r/m16 */
static
unsigned op_62 (e8086_t *c)
{
	long           s1, lo, hi;
	unsigned short seg, ofs;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

	if (c->ea.is_mem == 0) {
		fprintf (stderr, "BOUND EA is register\n");
		return (c->ea.cnt + 1);
	}

	seg = c->ea.seg;
	ofs = c->ea.ofs;

	lo = e86_get_mem16 (c, seg, ofs);
	hi = e86_get_mem16 (c, seg, ofs + 2);

	s1 = (s1 > 0x7fff) ? (s1 - 0x10000) : s1;
	lo = (lo > 0x7fff) ? (lo - 0x10000) : lo;
	hi = (hi > 0x7fff) ? (hi - 0x10000) : hi;

	if ((s1 < lo) || (s1 > (hi + 2))) {
		e86_trap (c, 5);
		return (0);
	}

	e86_set_clk (c, (33 + 35) / 2);

	return (c->ea.cnt + 1);
}

/* OP 68: PUSH imm16 */
static
unsigned op_68 (e8086_t *c)
{
	e86_push (c, e86_mk_uint16 (c->pq[1], c->pq[2]));
	e86_set_clk (c, 3);

	return (3);
}

/* OP 69: IMUL r16, r/m16, imm16 */
static
unsigned op_69 (e8086_t *c)
{
	unsigned long  s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;

	d = (s1 * s2) & 0xffffffff;

	e86_set_reg16 (c, (c->pq[1] >> 3) & 7, d & 0xffff);

	d &= 0xffff8000;
	e86_set_f (c, E86_FLG_C | E86_FLG_O, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f (c, E86_FLG_Z, d == 0);

	e86_set_clk_ea (c, (22 + 25) / 2, (29 + 32) / 2);

	return (c->ea.cnt + 3);
}

/* OP 6A: PUSH imm8 */
static
unsigned op_6a (e8086_t *c)
{
	e86_push (c, e86_mk_sint16 (c->pq[1]));
	e86_set_clk (c, 3);

	return (2);
}

/* OP 6B: IMUL r16, r/m16, imm8 */
static
unsigned op_6b (e8086_t *c)
{
	unsigned long  s1, s2, d;

	e86_get_ea_ptr (c, c->pq + 1);

	s1 = e86_get_ea16 (c);
	s2 = e86_mk_sint16 (c->pq[c->ea.cnt + 1]);

	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;

	d = (s1 * s2) & 0xffffffff;

	e86_set_reg16 (c, (c->pq[1] >> 3) & 7, d & 0xffff);

	d &= 0xffff8000;
	e86_set_f (c, E86_FLG_C | E86_FLG_O, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f (c, E86_FLG_Z, d == 0);

	e86_set_clk_ea (c, (22 + 25) / 2, (29 + 32) / 2);

	return (c->ea.cnt + 2);
}

/* OP 6C: INSB */
static
unsigned op_6c (e8086_t *c)
{
	unsigned short inc;

	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx (c) > 0) {
			e86_set_mem8 (c, e86_get_es (c), e86_get_di (c), e86_get_prt8 (c, e86_get_dx (c)));

			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			c->instructions += 1;
			e86_set_clk (c, 8);
		}
	}
	else {
		e86_set_mem8 (c, e86_get_es (c), e86_get_di (c), e86_get_prt8 (c, e86_get_dx (c)));
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_clk (c, 8);
	}

	return (1);
}

/* OP 6D: INSW */
static
unsigned op_6d (e8086_t *c)
{
	unsigned short inc;

	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx (c) > 0) {
			e86_set_mem16 (c, e86_get_es (c), e86_get_di (c), e86_get_prt16 (c, e86_get_dx (c)));

			e86_set_di (c, e86_get_di (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);

			c->instructions += 1;
			e86_set_clk (c, 8);
		}
	}
	else {
		e86_set_mem16 (c, e86_get_es (c), e86_get_di (c), e86_get_prt16 (c, e86_get_dx (c)));
		e86_set_di (c, e86_get_di (c) + inc);

		e86_set_clk (c, 8);
	}

	return (1);
}

/* OP 6E: OUTSB */
static
unsigned op_6e (e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e86_get_seg (c, E86_REG_DS);
	inc = e86_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx (c) > 0) {
			e86_set_prt8 (c, e86_get_dx (c), e86_get_mem8 (c, seg, e86_get_si (c)));
			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);
			c->instructions += 1;
			e86_set_clk (c, 8);
		}
	}
	else {
		e86_set_prt8 (c, e86_get_dx (c), e86_get_mem8 (c, seg, e86_get_si (c)));
		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_clk (c, 8);
	}

	return (1);
}

/* Opcode 6F: OUTSW */
static
unsigned op_6f (e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e86_get_seg (c, E86_REG_DS);
	inc = e86_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx (c) > 0) {
			e86_set_prt16 (c, e86_get_dx (c), e86_get_mem16 (c, seg, e86_get_si (c)));
			e86_set_si (c, e86_get_si (c) + inc);
			e86_set_cx (c, e86_get_cx (c) - 1);
			c->instructions += 1;
			e86_set_clk (c, 8);
		}
	}
	else {
		e86_set_prt16 (c, e86_get_dx (c), e86_get_mem16 (c, seg, e86_get_si (c)));
		e86_set_si (c, e86_get_si (c) + inc);
		e86_set_clk (c, 8);
	}

	return (1);
}

/* OP C0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, imm8 */
static
unsigned op_c0 (e8086_t *c)
{
	unsigned       xop;
	unsigned       cnt;
	unsigned short d, s;

	xop = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);
	s = e86_get_ea8 (c);

	cnt = c->pq[c->ea.cnt + 1];

	if (c->cpu & E86_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}

	if (cnt == 0) {
		return (c->ea.cnt + 1);
	}

	switch (xop) {
		case 0: /* ROL r/m8, imm8 */
			d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
			e86_set_cf (c, d & 1);
			e86_set_of (c, ((s << 1) ^ s) & 0x80);
			break;

		case 1: /* ROR r/m8, imm8 */
			d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
			e86_set_cf (c, d & 0x80);
			e86_set_of (c, ((d << 1) ^ s) & 0x80);
			break;

		case 2: /* RCL r/m8, imm8 */
			s |= e86_get_cf (c) << 8;
			d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
			e86_set_cf (c, d & 0x100);
			e86_set_of (c, ((s << 1) ^ s) & 0x80);
			break;

		case 3: /* RCR r/m8, imm8 */
			s |= e86_get_cf (c) << 8;
			d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
			e86_set_cf (c, d & 0x100);
			e86_set_of (c, ((d << 1) ^ s) & 0x80);
			break;

		case 4: /* SHL r/m8, imm8 */
			d = (cnt > 8) ? 0 : (s << cnt);
			e86_set_flg_szp_8 (c, d);
			e86_set_cf (c, d & 0x100);
			e86_set_of (c, ((s << 1) ^ s) & 0x80);
			break;

		case 5: /* SHR r/m8, imm8 */
			/* c > 0 */
			d = (cnt > 8) ? 0 : (s >> (cnt - 1));
			e86_set_cf (c, d & 1);
			d = d >> 1;
			e86_set_flg_szp_8 (c, d);
			e86_set_of (c, s & 0x80);
			break;

		case 7: /* SAR r/m8, imm8 */
			s |= (s & 0x80) ? 0xff00 : 0x0000;
			d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
			e86_set_cf (c, d & 1);
			d = (d >> 1) & 0xff;
			e86_set_flg_szp_8 (c, d);
			e86_set_of (c, 0);
			break;

		default:
			d = 0; /* To avoid compiler warning */
			break;
	}

	e86_set_ea8 (c, d & 0xff);
	e86_set_clk_ea (c, 5 + cnt, 17 + cnt);

	return (c->ea.cnt + 2);
}

/* OP C1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, imm8 */
static
unsigned op_c1 (e8086_t *c)
{
	unsigned      xop;
	unsigned      cnt;
	unsigned long d, s;

	xop = (c->pq[1] >> 3) & 7;

	e86_get_ea_ptr (c, c->pq + 1);
	s = e86_get_ea16 (c);

	cnt = c->pq[c->ea.cnt + 1];

	if (c->cpu & E86_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}

	if (cnt == 0) {
		return (c->ea.cnt + 1);
	}

	switch (xop) {
		case 0: /* ROL r/m16, imm8 */
			d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
			e86_set_cf (c, d & 1);
			e86_set_of (c, ((s << 1) ^ s) & 0x8000);
			break;

		case 1: /* ROR r/m16, imm8 */
			d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
			e86_set_cf (c, d & 0x8000);
			e86_set_of (c, ((d << 1) ^ s) & 0x8000);
			break;

		case 2: /* RCL r/m16, imm8 */
			s |= e86_get_cf (c) << 16;
			d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
			e86_set_cf (c, d & 0x10000);
			e86_set_of (c, ((s << 1) ^ s) & 0x8000);
			break;

		case 3: /* RCR r/m16, imm8 */
			s |= e86_get_cf (c) << 16;
			d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
			e86_set_cf (c, d & 0x10000);
			e86_set_of (c, ((d << 1) ^ s) & 0x8000);
			break;

		case 4: /* SHL r/m16, imm8 */
			d = (cnt > 16) ? 0 : (s << cnt);
			e86_set_flg_szp_16 (c, d);
			e86_set_cf (c, d & 0x10000);
			e86_set_of (c, ((s << 1) ^ s) & 0x8000);
			break;

		case 5: /* SHR r/m16, imm8 */
			/* c > 0 */
			d = (cnt > 16) ? 0 : (s >> (cnt - 1));
			e86_set_cf (c, d & 1);
			d = d >> 1;
			e86_set_flg_szp_16 (c, d);
			e86_set_of (c, s & 0x8000);
			break;

		case 7: /* SAR r/m16, imm8 */
			s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
			d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
			e86_set_cf (c, d & 1);
			d = (d >> 1) & 0xffff;
			e86_set_flg_szp_16 (c, d);
			e86_set_of (c, 0);
			break;

		default:
			d = 0; /* To avoid compiler warning */
			break;
	}

	e86_set_ea16 (c, d);
	e86_set_clk_ea (c, 5 + cnt, 17 + cnt);

	return (c->ea.cnt + 2);
}

/* OP C8: ENTER imm16, imm8 */
static
unsigned op_c8 (e8086_t *c)
{
	unsigned char  level;
	unsigned short tmp;

	e86_push (c, e86_get_bp (c));

	tmp = e86_get_sp (c);

	level = c->pq[3] & 0x1f;

	if (level > 0) {
		unsigned       i;
		unsigned short sp, bp;

		sp = e86_get_sp (c);
		bp = e86_get_bp (c);

		for (i = 1; i < level; i++) {
			bp -= 2;
			e86_push (c, e86_get_mem16 (c, sp, bp));
		}

		e86_push (c, tmp);
	}

	e86_set_bp (c, tmp);
	e86_set_sp (c, tmp - e86_mk_uint16 (c->pq[1], c->pq[2]));

	switch (level) {
		case 0:
			e86_set_clk (c, 15);
			break;

		case 1:
			e86_set_clk (c, 25);
			break;

		default:
			e86_set_clk (c, 22 + 16 * (level - 1));
			break;
	}

	return (4);
}

/* OP C9: LEAVE */
static
unsigned op_c9 (e8086_t *c)
{
	e86_set_sp (c, e86_get_bp (c));
	e86_set_bp (c, e86_pop (c));
	e86_set_clk (c, 8);

	return (1);
}

void e86_set_80186 (e8086_t *c)
{
	e86_set_8086 (c);

	c->cpu &= ~E86_CPU_REP_BUG;
	c->cpu |= E86_CPU_MASK_SHIFT;
	c->cpu |= E86_CPU_INT6;

	c->op[0x60] = &op_60;
	c->op[0x61] = &op_61;
	c->op[0x62] = &op_62;
	c->op[0x68] = &op_68;
	c->op[0x69] = &op_69;
	c->op[0x6a] = &op_6a;
	c->op[0x6b] = &op_6b;
	c->op[0x6c] = &op_6c;
	c->op[0x6d] = &op_6d;
	c->op[0x6e] = &op_6e;
	c->op[0x6f] = &op_6f;
	c->op[0xc0] = &op_c0;
	c->op[0xc1] = &op_c1;
	c->op[0xc8] = &op_c8;
	c->op[0xc9] = &op_c9;
}

void e86_set_80188 (e8086_t *c)
{
	e86_set_80186 (c);

	c->cpu |= E86_CPU_8BIT;

	e86_set_pq_size (c, 4);
}
