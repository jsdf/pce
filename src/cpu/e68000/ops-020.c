/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e68000/ops-020.c                                     *
 * Created:     2011-07-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e68000.h"
#include "internal.h"


static void e68_op_undefined (e68000_t *c)
{
	e68_set_clk (c, 2);
	e68_exception_illegal (c);
}

/* 00C0: CHK2.X <EA>, Rx / CMP2.X <EA>, Rx */
static void op00c0 (e68000_t *c)
{
	int      trap;
	unsigned size;
	uint32_t b1, b2, s2;

	size = (c->ir[0] >> 9) & 3;

	e68_op_prefetch (c);

	if (c->ir[1] & 0x07ff) {
		e68_op_undefined (c);
		return;
	}

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x07e4, 8)) {
		return;
	}

	if (c->ea_typ == E68_EA_TYPE_MEM) {
		if (size == 0) {
			b1 = e68_exts8 (e68_get_mem8 (c, c->ea_val));
			b2 = e68_exts8 (e68_get_mem8 (c, c->ea_val + 1));
		}
		else if (size == 1) {
			b1 = e68_exts16 (e68_get_mem16 (c, c->ea_val));
			b2 = e68_exts16 (e68_get_mem16 (c, c->ea_val + 2));
		}
		else {
			b1 = e68_get_mem32 (c, c->ea_val);
			b2 = e68_get_mem32 (c, c->ea_val + 4);
		}
	}
	else {
		e68_op_undefined (c);
		return;
	}

	if (c->ir[1] & 0x8000) {
		s2 = e68_get_areg32 (c, (c->ir[1] >> 12) & 7);
	}
	else {
		s2 = e68_get_dreg32 (c, (c->ir[1] >> 12) & 7);

		if (size == 0) {
			s2 = e68_exts8 (s2);
		}
		else if (size == 1) {
			s2 = e68_exts16 (s2);
		}
	}

	b1 ^= 0x80000000;
	b2 ^= 0x80000000;
	s2 ^= 0x80000000;

	trap = (s2 < b1) || (s2 > b2);

	e68_set_sr_z (c, (s2 == b1) || (s2 == b2));
	e68_set_sr_c (c, trap);

	e68_set_clk (c, 14);

	if (c->ir[1] & 0x0800) {
		if (trap) {
			e68_exception_check (c);
			return;
		}
	}

	e68_op_prefetch (c);
}

/* 4100: CHK.L <EA>, Dx */
static void op4100 (e68000_t *c)
{
	int      trap;
	uint32_t s1, s2;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, e68_ir_reg9 (c));

	if (s2 & 0x80000000) {
		trap = 1;
		e68_set_sr_n (c, 1);
	}
	else if ((s1 & 0x80000000) || (s2 > s1)) {
		trap = 1;
		e68_set_sr_n (c, 0);
	}
	else {
		trap = 0;
	}

	if (trap) {
		e68_exception_check (c);
		return;
	}

	e68_set_clk (c, 14);
	e68_op_prefetch (c);
}

/* 49C0_00: EXTB.L Dx */
static void op49c0_00 (e68000_t *c)
{
	unsigned r;
	uint32_t s, d;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, r);

	d = (s & 0x80) ? (s | 0xffffff00) : s;

	e68_set_clk (c, 4);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* 4A00: TST.B <EA> */
static void op4a00 (e68000_t *c)
{
	uint8_t s;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s);

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
}

/* 4A40: TST.W <EA> */
static void op4a40 (e68000_t *c)
{
	uint16_t s;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s);

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
}

/* 4A80: TST.L <EA> */
static void op4a80 (e68000_t *c)
{
	uint32_t s;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s);

	e68_set_clk (c, 8);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
}

/* 4C00: MULU.L <EA>, [Dh:]Dl */
static void op4c00 (e68000_t *c)
{
	int      sign;
	unsigned rl, rh;
	uint16_t ext;
	uint32_t dh, dl, st;
	uint64_t s1, s2;
	uint64_t d;

	e68_op_prefetch16 (c, ext);

	if (ext & 0x8308) {
		e68_op_undefined (c);
		return;
	}

	sign = (ext & 0x0800) != 0;

	rl = (ext >> 12) & 7;
	rh = ext & 7;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &st);
	s1 = st;
	s2 = e68_get_dreg32 (c, rl);

	if (sign) {
		if (s1 & 0x80000000) {
			s1 |= 0xffffffff00000000;
		}
		if (s2 & 0x80000000) {
			s2 |= 0xffffffff00000000;
		}
	}

	d = (uint64_t) s1 * (uint64_t) s2;

	dl = d & 0xffffffff;
	dh = (d >> 32) & 0xffffffff;

	e68_set_dreg32 (c, rl, dl);

	if (ext & 0x0400) {
		e68_set_dreg32 (c, rh, dh);

		e68_set_sr_n (c, dh & 0x80000000);
		e68_set_sr_z (c, (dh | dl) == 0);
		e68_set_sr_v (c, 0);
	}
	else {
		e68_set_sr_n (c, dh & 0x80000000);
		e68_set_sr_z (c, (dh | dl) == 0);

		if (sign && (dl & 0x80000000)) {
			dh = ~dh;
		}

		e68_set_sr_v (c, dh != 0);
	}

	/* fixme */
	e68_set_clk (c, 74);
	e68_set_sr_c (c, 0);
	e68_op_prefetch (c);
}

/* 4C40: DIVU[L].L <EA>, Dr:Dq */
static void op4c40 (e68000_t *c)
{
	int      sign;
	int      neg1, neg2;
	int      v;
	uint16_t ext;
	unsigned rq, rr;
	uint32_t s1, s2, s3, d1, d2;

	e68_op_prefetch16 (c, ext);

	if (ext & 0x8308) {
		e68_op_undefined (c);
		return;
	}

	sign = (ext & 0x0800) != 0;

	rq = (ext >> 12) & 7;
	rr = ext & 7;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, rq);

	if (s1 == 0) {
		e68_exception_divzero (c);
		return;
	}

	if (sign) {
		neg1 = (s1 & 0x80000000) != 0;
		neg2 = (s2 & 0x80000000) != 0;

		if (neg1) {
			s1 = (~s1 + 1) & 0xffffffff;
		}

		if (neg2) {
			s2 = (~s2 + 1) & 0xffffffff;
		}
	}
	else {
		neg1 = 0;
		neg2 = 0;
	}

	v = 0;

	if (ext & 0x0400) {
		uint64_t st, dt;

		s2 = e68_get_dreg32 (c, rq);
		s3 = e68_get_dreg32 (c, rr);

		if (sign) {
			neg2 = (s3 & 0x80000000) != 0;

			if (neg2) {
				s2 = (~s2 + 1) & 0xffffffff;
				s3 = (~s3 + (s2 == 0)) & 0xffffffff;
			}
		}

		st = ((uint64_t) s3 << 32) | s2;

		dt = st / s1;
		d1 = dt & 0xffffffff;
		d2 = st % s1;

		if (sign) {
			if ((dt > 0x80000000) || ((dt == 0x80000000) && (neg1 == neg2))) {
				v = 1;
			}
		}
		else {
			if (dt > 0xffffffff) {
				v = 1;
			}
		}
	}
	else {
		d1 = s2 / s1;
		d2 = s2 % s1;
	}

	if (v == 0) {
		if (neg1 != neg2) {
			d1 = (~d1 + 1) & 0xffffffff;
		}

		e68_set_dreg32 (c, rq, d1);

		if (rr != rq) {
			if (neg2) {
				d2 = (~d2 + 1) & 0xffffffff;
			}

			e68_set_dreg32 (c, rr, d2);
		}

		e68_cc_set_nz_32 (c, E68_SR_NZVC, d1);
	}
	else {
		e68_set_sr_v (c, 1);
	}

	/* fixme */
	e68_set_clk (c, 144);
	e68_op_prefetch (c);
}

/* TRAPcc.X */
void e68_op_trapcc (e68000_t *c, int cond)
{
	switch (c->ir[0] & 7) {
	case 2:
		e68_op_prefetch (c);
		break;

	case 3:
		e68_op_prefetch (c);
		e68_op_prefetch (c);
		break;

	case 4:
		break;

	default:
		e68_op_undefined (c);
		return;
	}

	e68_op_prefetch (c);

	if (cond) {
		e68_exception_overflow (c);
	}
	else {
		e68_set_clk (c, 4);
	}
}

static
void op_scc_dbcc_trapcc (e68000_t *c, int cond)
{
	unsigned op;

	op = c->ir[0] & 0x3f;

	if ((op == 0x3a) || (op == 0x3b) || (op == 0x3c)) {
		e68_op_trapcc (c, cond);
	}
	else if ((op & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 50C0: ST <EA> / DBT Dx, dist */
static void op50c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, 1);
}

/* 51C0: SF <EA> / DBF Dx, dist */
static void op51c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, 0);
}

/* 52C0: SHI <EA> / DBHI Dx, dist */
static void op52c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, !e68_get_sr_c (c) && !e68_get_sr_z (c));
}

/* 53C0: SLS <EA> / DBLS Dx, dist */
static void op53c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_c (c) || e68_get_sr_z (c));
}

/* 54C0: SCC <EA> / DBCC Dx, dist */
static void op54c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, !e68_get_sr_c (c));
}

/* 55C0: SCS <EA> / DBCS Dx, dist */
static void op55c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_c (c));
}

/* 56C0: SNE <EA> / DBNE Dx, dist */
static void op56c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, !e68_get_sr_z (c));
}

/* 57C0: SEQ <EA> / DBEQ Dx, dist */
static void op57c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_z (c));
}

/* 58C0: SVC <EA> / DBVC Dx, dist */
static void op58c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, !e68_get_sr_v (c));
}

/* 59C0: SVS <EA> / DBVS Dx, dist */
static void op59c0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_v (c));
}

/* 5AC0: SPL <EA> / DBPL Dx, dist */
static void op5ac0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, !e68_get_sr_n (c));
}

/* 5BC0: SMI <EA> / DBMI Dx, dist */
static void op5bc0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_n (c));
}

/* 5CC0: SGE <EA> / DBGE Dx, dist */
static void op5cc0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_n (c) == e68_get_sr_v (c));
}

/* 5DC0: SLT <EA> / DBLT Dx, dist */
static void op5dc0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, e68_get_sr_n (c) != e68_get_sr_v (c));
}

/* 5EC0: SGT <EA> / DBGT Dx, dist */
static void op5ec0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c));
}

/* 5FC0: SLE <EA> / DBLE Dx, dist */
static void op5fc0 (e68000_t *c)
{
	op_scc_dbcc_trapcc (c, (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c));
}

/* conditional jump */
static
void e68020_op_bcc (e68000_t *c, int cond)
{
	uint32_t addr, dist;

	addr = e68_get_pc (c) + 2;
	dist = e68_exts8 (c->ir[0]);

	if (dist == 0) {
		e68_op_prefetch (c);
		dist = e68_exts16 (c->ir[1]);
	}
	else if ((dist & 0xff) == 0xff) {
		e68_op_prefetch32 (c, dist);
	}

	if (cond) {
		e68_set_clk (c, 10);
		e68_set_ir_pc (c, addr + dist);
		e68_op_prefetch (c);
	}
	else {
		e68_set_clk (c, ((c->ir[0] & 0xff) == 0) ? 12 : 8);
	}

	e68_op_prefetch (c);
	e68_set_pc (c, e68_get_ir_pc (c) - 4);
}

/* 6000: BRA dist */
static void op6000 (e68000_t *c)
{
	e68020_op_bcc (c, 1);
}

/* 6100: BSR dist */
static void op6100 (e68000_t *c)
{
	uint32_t addr;

	switch (c->ir[0] & 0xff) {
	case 0x00:
		addr = e68_get_pc (c) + 4;
		break;

	case 0xff:
		addr = e68_get_pc (c) + 6;
		break;

	default:
		addr = e68_get_pc (c) + 2;
		break;
	}

	e68_push32 (c, addr);

	e68020_op_bcc (c, 1);
}

/* 6200: BHI dist */
static void op6200 (e68000_t *c)
{
	e68020_op_bcc (c, !e68_get_sr_c (c) && !e68_get_sr_z (c));
}

/* 6300: BLS dist */
static void op6300 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_c (c) || e68_get_sr_z (c));
}

/* 6400: BCC dist */
static void op6400 (e68000_t *c)
{
	e68020_op_bcc (c, !e68_get_sr_c (c));
}

/* 6500: BCS dist */
static void op6500 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_c (c));
}

/* 6600: BNE dist */
static void op6600 (e68000_t *c)
{
	e68020_op_bcc (c, !e68_get_sr_z (c));
}

/* 6700: BEQ dist */
static void op6700 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_z (c));
}

/* 6800: BVC dist */
static void op6800 (e68000_t *c)
{
	e68020_op_bcc (c, !e68_get_sr_v (c));
}

/* 6900: BVS dist */
static void op6900 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_v (c));
}

/* 6A00: BPL dist */
static void op6a00 (e68000_t *c)
{
	e68020_op_bcc (c, !e68_get_sr_n (c));
}

/* 6B00: BMI dist */
static void op6b00 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_n (c));
}

/* 6C00: BGE dist */
static void op6c00 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_n (c) == e68_get_sr_v (c));
}

/* 6D00: BLT dist */
static void op6d00 (e68000_t *c)
{
	e68020_op_bcc (c, e68_get_sr_n (c) != e68_get_sr_v (c));
}

/* 6E00: BGT dist */
static void op6e00 (e68000_t *c)
{
	e68020_op_bcc (c, (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c));
}

/* 6F00: BLE dist */
static void op6f00 (e68000_t *c)
{
	e68020_op_bcc (c, (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c));
}

static
void e68_get_ea_bf_reg (e68000_t *c, uint32_t *bf)
{
	unsigned      rot;
	unsigned long val;

	val = e68_get_dreg32 (c, c->ea_val);

	rot = (c->ea_bf_ofs + c->ea_bf_width) & 31;

	if (rot > 0) {
		val = (val << rot) | (val >> (32 - rot));
	}

	*bf = val & 0xffffffff;
}

static
void e68_get_ea_bf_mem (e68000_t *c, uint32_t *bf)
{
	unsigned      cnt, n1, n2, n3;
	unsigned char m, v;
	unsigned char *buf;
	uint32_t      adr;

	if (c->ea_bf_ofs & 0x80000000) {
		adr = c->ea_val + ((c->ea_bf_ofs >> 3) | 0xe0000000);
	}
	else {
		adr = c->ea_val + (c->ea_bf_ofs >> 3);
	}

	*bf = 0;

	n1 = c->ea_bf_ofs & 7;
	cnt = c->ea_bf_width;

	buf = c->ea_bf_val;

	while (cnt > 0) {
		n2 = 8 - n1;

		if (n2 > cnt) {
			n2 = cnt;
		}

		n3 = 8 - n1 - n2;

		m = (0xff >> n1) & (0xff << n3);
		v = e68_get_mem8 (c, adr);

		*(buf++) = v;

		*bf = (*bf << n2) | ((v & m) >> n3);

		n1 = (n1 + n2) & 7;
		cnt = cnt - n2;
		adr += 1;
	}
}

static
int e68_get_ea_bf (e68000_t *c, uint32_t *bf, unsigned mask)
{
	uint32_t ofs, wth;

	if (e68_prefetch (c)) {
		return (1);
	}

	ofs = (c->ir[1] >> 6) & 31;
	wth = c->ir[1] & 31;

	if ((c->ir[1] >> 11) & 1) {
		ofs = e68_get_dreg32 (c, ofs & 7);
	}

	if ((c->ir[1] >> 5) & 1) {
		wth = e68_get_dreg32 (c, wth & 7) & 31;
	}

	if (wth == 0) {
		wth = 32;
	}

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), mask, 8)) {
		return (1);
	}

	c->ea_bf_ofs = ofs;
	c->ea_bf_width = wth;

	if (c->ea_typ == E68_EA_TYPE_REG) {
		e68_get_ea_bf_reg (c, bf);
	}
	else if (c->ea_typ == E68_EA_TYPE_MEM) {
		e68_get_ea_bf_mem (c, bf);
	}
	else {
		return (1);
	}

	/* fixme */
	e68_set_clk (c, 18);

	return (0);
}

static
void e68_set_ea_bf_reg (e68000_t *c, uint32_t bf)
{
	unsigned rot;
	uint32_t val, msk;

	val = e68_get_dreg32 (c, c->ea_val);
	msk = 0xffffffff >> (32 - c->ea_bf_width);
	rot = (32 - c->ea_bf_ofs - c->ea_bf_width) & 31;

	if (rot > 0) {
		bf = (bf << rot) | (bf >> (32 - rot));
		msk = (msk << rot) | (msk >> (32 - rot));
	}

	val = (val & ~msk) | (bf & msk);

	e68_set_dreg32 (c, c->ea_val, val);
}

static
void e68_set_ea_bf_mem (e68000_t *c, uint32_t bf)
{
	unsigned            cnt, n1, n2, n3;
	unsigned char       m, v;
	const unsigned char *buf;
	uint32_t            adr;

	if (c->ea_bf_ofs & 0x80000000) {
		adr = c->ea_val + ((c->ea_bf_ofs >> 3) | 0xe0000000);
	}
	else {
		adr = c->ea_val + (c->ea_bf_ofs >> 3);
	}

	n1 = c->ea_bf_ofs & 7;
	cnt = c->ea_bf_width;

	buf = c->ea_bf_val;

	while (cnt > 0) {
		n2 = 8 - n1;

		if (n2 > cnt) {
			n2 = cnt;
		}

		n3 = 8 - n1 - n2;

		m = (0xff >> n1) & (0xff << n3);

		v = *(buf++);
		v = (v & ~m) | ((bf >> (cnt - n2 - n3)) & m);

		e68_set_mem8 (c, adr, v);

		n1 = (n1 + n2) & 7;
		cnt = cnt - n2;
		adr += 1;
	}
}

static
int e68_set_ea_bf (e68000_t *c, uint32_t bf)
{
	if (c->ea_typ == E68_EA_TYPE_REG) {
		e68_set_ea_bf_reg (c, bf);
	}
	else if (c->ea_typ == E68_EA_TYPE_MEM) {
		e68_set_ea_bf_mem (c, bf);
	}
	else {
		return (1);
	}

	return (0);
}

/* E8C0: BFTST <EA> {offset:width} */
static void ope8c0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		e68_op_undefined (c);
		return;
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_op_prefetch (c);
}

/* E9C0: BFEXTU <EA> {offset:width}, Dn */
static void ope9c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t val, msk;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		e68_op_undefined (c);
		return;
	}

	reg = (c->ir[1] >> 12) & 7;
	msk = 0xffffffff >> (32 - c->ea_bf_width);

	val &= msk;

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_set_dreg32 (c, reg, val);
	e68_op_prefetch (c);
}

/* EAC0: BFCHG <EA> {offset:width} */
static void opeac0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x01e5)) {
		e68_op_undefined (c);
		return;
	}

	if (e68_set_ea_bf (c, ~val)) {
		e68_op_undefined (c);
		return;
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_op_prefetch (c);
}

/* EBC0: BFEXTS <EA> {offset:width}, Dn */
static void opebc0 (e68000_t *c)
{
	unsigned reg;
	uint32_t val, msk, sgn;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		e68_op_undefined (c);
		return;
	}

	reg = (c->ir[1] >> 12) & 7;
	msk = 0xffffffff >> (32 - c->ea_bf_width);
	sgn = 0x80000000 >> (32 - c->ea_bf_width);

	if (val & sgn) {
		val |= ~msk;
	}
	else {
		val &= msk;
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_set_dreg32 (c, reg, val);
	e68_op_prefetch (c);
}

/* ECC0: BFCLR <EA> {offset:width} */
static void opecc0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x01e5)) {
		e68_op_undefined (c);
		return;
	}

	if (e68_set_ea_bf (c, 0)) {
		e68_op_undefined (c);
		return;
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_op_prefetch (c);
}

/* EEC0: BFSET <EA> {offset:width} */
static void opeec0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x01e5)) {
		e68_op_undefined (c);
		return;
	}

	if (e68_set_ea_bf (c, 0xffffffff)) {
		e68_op_undefined (c);
		return;
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_op_prefetch (c);
}

/* EFC0: BFINS Dn, <EA> {offset:width} */
static void opefc0 (e68000_t *c)
{
	unsigned reg;
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		e68_op_undefined (c);
		return;
	}

	reg = (c->ir[1] >> 12) & 7;
	val = e68_get_dreg32 (c, reg);

	if (e68_set_ea_bf (c, val)) {
		e68_op_undefined (c);
		return;
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);
	e68_op_prefetch (c);
}

static
e68_opcode_f e68020_opcodes[1024] = {
	  NULL,   NULL,   NULL, op00c0,   NULL,   NULL,   NULL,   NULL, /* 0000 */
	  NULL,   NULL,   NULL, op00c0,   NULL,   NULL,   NULL,   NULL, /* 0200 */
	  NULL,   NULL,   NULL, op00c0,   NULL,   NULL,   NULL,   NULL, /* 0400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 0600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 0800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 0A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 0C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 0E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 1E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 2E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 3E00 */
	  NULL,   NULL,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4000 */
	  NULL,   NULL,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4200 */
	  NULL,   NULL,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4400 */
	  NULL,   NULL,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4600 */
	  NULL,   NULL,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4800 */
	op4a00, op4a40, op4a80,   NULL, op4100,   NULL,   NULL,   NULL, /* 4A00 */
	op4c00, op4c40,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4C00 */
	  NULL,   NULL,   NULL,   NULL, op4100,   NULL,   NULL,   NULL, /* 4E00 */
	  NULL,   NULL,   NULL, op50c0,   NULL,   NULL,   NULL, op51c0, /* 5000 */
	  NULL,   NULL,   NULL, op52c0,   NULL,   NULL,   NULL, op53c0, /* 5200 */
	  NULL,   NULL,   NULL, op54c0,   NULL,   NULL,   NULL, op55c0, /* 5400 */
	  NULL,   NULL,   NULL, op56c0,   NULL,   NULL,   NULL, op57c0, /* 5600 */
	  NULL,   NULL,   NULL, op58c0,   NULL,   NULL,   NULL, op59c0, /* 5800 */
	  NULL,   NULL,   NULL, op5ac0,   NULL,   NULL,   NULL, op5bc0, /* 5A00 */
	  NULL,   NULL,   NULL, op5cc0,   NULL,   NULL,   NULL, op5dc0, /* 5C00 */
	  NULL,   NULL,   NULL, op5ec0,   NULL,   NULL,   NULL, op5fc0, /* 5E00 */
	op6000, op6000, op6000, op6000, op6100, op6100, op6100, op6100, /* 6000 */
	op6200, op6200, op6200, op6200, op6300, op6300, op6300, op6300, /* 6200 */
	op6400, op6400, op6400, op6400, op6500, op6500, op6500, op6500, /* 6400 */
	op6600, op6600, op6600, op6600, op6700, op6700, op6700, op6700, /* 6600 */
	op6800, op6800, op6800, op6800, op6900, op6900, op6900, op6900, /* 6800 */
	op6a00, op6a00, op6a00, op6a00, op6b00, op6b00, op6b00, op6b00, /* 6A00 */
	op6c00, op6c00, op6c00, op6c00, op6d00, op6d00, op6d00, op6d00, /* 6C00 */
	op6e00, op6e00, op6e00, op6e00, op6f00, op6f00, op6f00, op6f00, /* 6E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 7E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 8E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 9E00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* AA00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* AC00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* AE00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* BA00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* BC00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* BE00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* CA00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* CC00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* CE00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* DA00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* DC00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* DE00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E600 */
	  NULL,   NULL,   NULL, ope8c0,   NULL,   NULL,   NULL, ope9c0, /* E800 */
	  NULL,   NULL,   NULL, opeac0,   NULL,   NULL,   NULL, opebc0, /* EA00 */
	  NULL,   NULL,   NULL, opecc0,   NULL,   NULL,   NULL,   NULL, /* EC00 */
	  NULL,   NULL,   NULL, opeec0,   NULL,   NULL,   NULL, opefc0, /* EE00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* FA00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* FC00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL  /* FE00 */
};

static e68_opcode_f e68_op_49c0[8] = {
	op49c0_00, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


void e68_set_opcodes_020 (e68000_t *c)
{
	unsigned i;

	e68_set_opcodes (c);

	for (i = 0; i < 1024; i++) {
		if (e68020_opcodes[i] != NULL) {
			c->opcodes[i] = e68020_opcodes[i];
		}
	}

	for (i = 0; i < 8; i++) {
		if (e68_op_49c0[i] != NULL) {
			c->op49c0[i] = e68_op_49c0[i];
		}
	}
}
