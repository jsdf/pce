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


#define e68_ir_ea1(c) ((c)->ir[0] & 0x3f)
#define e68_ir_ea2(c) ((((c)->ir[0] >> 3) & 0x38) | (((c)->ir[0] >> 9) & 0x07))
#define e68_ir_reg0(c) ((c)->ir[0] & 7)
#define e68_ir_reg9(c) (((c)->ir[0] >> 9) & 7)

#define e68_get_ea8(c, ptr, ea, msk, val) do { \
	if (ptr) if (e68_ea_get_ptr (c, ea, msk, 8)) return (0); \
	if (e68_ea_get_val8 (c, val)) return (0); \
	} while (0)

#define e68_get_ea16(c, ptr, ea, msk, val) do { \
	if (ptr) if (e68_ea_get_ptr (c, ea, msk, 16)) return (0); \
	if (e68_ea_get_val16 (c, val)) return (0); \
	} while (0)

#define e68_get_ea32(c, ptr, ea, msk, val) do { \
	if (ptr) if (e68_ea_get_ptr (c, ea, msk, 32)) return (0); \
	if (e68_ea_get_val32 (c, val)) return (0); \
	} while (0)

#define e68_set_ea8(c, ptr, ea, msk, val) do { \
	if (ptr) if (e68_ea_get_ptr (c, ea, msk, 8)) return (0); \
	if (e68_ea_set_val8 (c, val)) return (0); \
	} while (0)

#define e68_set_ea16(c, ptr, ea, msk, val) do { \
	if (ptr) if (e68_ea_get_ptr (c, ea, msk, 16)) return (0); \
	if (e68_ea_set_val16 (c, val)) return (0); \
	} while (0)

#define e68_set_ea32(c, ptr, ea, msk, val) do { \
	if (ptr) if (e68_ea_get_ptr (c, ea, msk, 32)) return (0); \
	if (e68_ea_set_val32 (c, val)) return (0); \
	} while (0)

#define e68_chk_addr(c, addr, wr) do { \
	if (((c)->flags & E68_FLAG_NOADDR) == 0) { \
		if ((addr) & 1) { \
			e68_exception_address (c, (addr), 1, (wr)); \
			return (0); \
		} \
	} \
	} while (0)

#define e68_check_68010(c) \
	if (!((c)->flags & E68_FLAG_68010)) \
		return e68_op_undefined (c)

#define e68_check_68020(c) \
	if (!((c)->flags & E68_FLAG_68020)) \
		return e68_op_undefined (c)


static unsigned e68_op_undefined (e68000_t *c)
{
	e68_exception_illegal (c);
	e68_set_clk (c, 2);
	return (0);
}

/* 00C0: CHK2.X <EA>, Rx / CMP2.X <EA>, Rx */
static unsigned op00c0 (e68000_t *c)
{
	int      trap;
	unsigned size;
	uint32_t b1, b2, s2;

	size = (c->ir[0] >> 9) & 3;

	e68_ifetch (c, 1);

	if (c->ir[1] & 0x07ff) {
		return (e68_op_undefined (c));
	}

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x07e4, 8)) {
		return (0);
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
		return (e68_op_undefined (c));
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
			return (0);
		}
	}

	return (c->ircnt);
}

/* 4100: CHK.L <EA>, Dx */
static unsigned op4100 (e68000_t *c)
{
	int      trap;
	uint32_t s1, s2;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
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
		return (0);
	}

	e68_set_clk (c, 14);

	return (c->ircnt);
}

/* 49C0_00: EXTB.L Dx */
static unsigned op49c0_00 (e68000_t *c)
{
	unsigned r;
	uint32_t s, d;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, r);

	d = (s & 0x80) ? (s | 0xffffff00) : s;

	e68_set_dreg32 (c, r, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 4);

	return (1);
}

/* 4A00: TST.B <EA> */
static unsigned op4a00 (e68000_t *c)
{
	uint8_t s;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s);

	e68_cc_set_nz_8 (c, E68_SR_NZVC, s);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4A40: TST.W <EA> */
static unsigned op4a40 (e68000_t *c)
{
	uint16_t s;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s);

	e68_cc_set_nz_16 (c, E68_SR_NZVC, s);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4A80: TST.L <EA> */
static unsigned op4a80 (e68000_t *c)
{
	uint32_t s;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s);

	e68_cc_set_nz_32 (c, E68_SR_NZVC, s);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4C00: MULU.L <EA>, [Dh:]Dl */
static unsigned op4c00 (e68000_t *c)
{
	int      sign;
	unsigned rl, rh;
	uint32_t dh, dl, st;
	uint64_t s1, s2;
	uint64_t d;

	e68_ifetch (c, 1);

	if (c->ir[1] & 0x8308) {
		return (e68_op_undefined (c));
	}

	sign = (c->ir[1] & 0x0800) != 0;

	rl = (c->ir[1] >> 12) & 7;
	rh = c->ir[1] & 7;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &st);
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

	if (c->ir[1] & 0x0400) {
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

	e68_set_sr_c (c, 0);

	/* fixme */
	e68_set_clk (c, 74);

	return (c->ircnt);
}

/* 4C40: DIVU[L].L <EA>, Dr:Dq */
static unsigned op4c40 (e68000_t *c)
{
	int      sign;
	int      neg1, neg2;
	int      v;
	unsigned rq, rr;
	uint32_t s1, s2, s3, d1, d2;

	e68_ifetch (c, 1);

	if (c->ir[1] & 0x8308) {
		return (e68_op_undefined (c));
	}

	sign = (c->ir[1] & 0x0800) != 0;

	rq = (c->ir[1] >> 12) & 7;
	rr = c->ir[1] & 7;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, rq);

	if (s1 == 0) {
		e68_exception_divzero (c);
		return (0);
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

	if (c->ir[1] & 0x0400) {
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

	return (c->ircnt);
}

/* conditional jump */
static inline
unsigned op_bcc (e68000_t *c, int cond)
{
	uint32_t dist;

	dist = e68_exts8 (c->ir[0]);

	if (dist == 0) {
		e68_ifetch (c, 1);
		dist = e68_exts16 (c->ir[1]);
	}
	else if ((dist & 0xff) == 0xff) {
		e68_ifetch (c, 1);
		e68_ifetch (c, 2);
		dist = ((uint32_t) c->ir[1] << 16) | c->ir[2];
	}

	if (cond == 0) {
		e68_set_clk (c, ((c->ir[0] & 0xff) == 0) ? 12 : 8);
		return (c->ircnt);
	}

	e68_set_pc (c, e68_get_pc (c) + dist + 2);

	e68_set_clk (c, 10);

	return (0);
}

/* 6000: BRA dist */
static unsigned op6000 (e68000_t *c)
{
	uint32_t dist;

	dist = e68_exts8 (c->ir[0]);

	if (dist == 0) {
		e68_ifetch (c, 1);
		dist = e68_exts16 (c->ir[1]);
	}
	else if ((dist & 0xff) == 0xff) {
		e68_ifetch (c, 1);
		e68_ifetch (c, 2);
		dist = ((uint32_t) c->ir[1] << 16) | c->ir[2];
	}

	e68_set_pc (c, e68_get_pc (c) + dist + 2);

	e68_set_clk (c, 10);

	return (0);
}

/* 6100: BSR dist */
static unsigned op6100 (e68000_t *c)
{
	uint32_t dist;

	dist = e68_exts8 (c->ir[0]);

	if (dist == 0) {
		e68_ifetch (c, 1);
		dist = e68_exts16 (c->ir[1]);
	}
	else if ((dist & 0xff) == 0xff) {
		e68_ifetch (c, 1);
		e68_ifetch (c, 2);
		dist = ((uint32_t) c->ir[1] << 16) | c->ir[2];
	}

	e68_push32 (c, e68_get_pc (c) + 2 * c->ircnt);
	e68_set_pc (c, e68_get_pc (c) + dist + 2);

	e68_set_clk (c, 18);

	return (0);
}

/* 6200: BHI dist */
static unsigned op6200 (e68000_t *c)
{
	return (op_bcc (c, !e68_get_sr_c (c) && !e68_get_sr_z (c)));
}

/* 6300: BLS dist */
static unsigned op6300 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_c (c) || e68_get_sr_z (c)));
}

/* 6400: BCC dist */
static unsigned op6400 (e68000_t *c)
{
	return (op_bcc (c, !e68_get_sr_c (c)));
}

/* 6500: BCS dist */
static unsigned op6500 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_c (c)));
}

/* 6600: BNE dist */
static unsigned op6600 (e68000_t *c)
{
	return (op_bcc (c, !e68_get_sr_z (c)));
}

/* 6700: BEQ dist */
static unsigned op6700 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_z (c)));
}

/* 6800: BVC dist */
static unsigned op6800 (e68000_t *c)
{
	return (op_bcc (c, !e68_get_sr_v (c)));
}

/* 6900: BVS dist */
static unsigned op6900 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_v (c)));
}

/* 6A00: BPL dist */
static unsigned op6a00 (e68000_t *c)
{
	return (op_bcc (c, !e68_get_sr_n (c)));
}

/* 6B00: BMI dist */
static unsigned op6b00 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_n (c)));
}

/* 6C00: BGE dist */
static unsigned op6c00 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_n (c) == e68_get_sr_v (c)));
}

/* 6D00: BLT dist */
static unsigned op6d00 (e68000_t *c)
{
	return (op_bcc (c, e68_get_sr_n (c) != e68_get_sr_v (c)));
}

/* 6E00: BGT dist */
static unsigned op6e00 (e68000_t *c)
{
	return (op_bcc (c, (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c)));
}

/* 6F00: BLE dist */
static unsigned op6f00 (e68000_t *c)
{
	return (op_bcc (c, (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c)));
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

	e68_ifetch (c, 1);

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
static unsigned ope8c0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		return (e68_op_undefined (c));
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
}

/* E9C0: BFEXTU <EA> {offset:width}, Dn */
static unsigned ope9c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t val, msk;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		return (e68_op_undefined (c));
	}

	reg = (c->ir[1] >> 12) & 7;
	msk = 0xffffffff >> (32 - c->ea_bf_width);

	val &= msk;

	e68_set_dreg32 (c, reg, val);

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
}

/* EAC0: BFCHG <EA> {offset:width} */
static unsigned opeac0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x01e5)) {
		return (e68_op_undefined (c));
	}

	if (e68_set_ea_bf (c, ~val)) {
		return (e68_op_undefined (c));
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
}

/* EBC0: BFEXTS <EA> {offset:width}, Dn */
static unsigned opebc0 (e68000_t *c)
{
	unsigned reg;
	uint32_t val, msk, sgn;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		return (e68_op_undefined (c));
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

	e68_set_dreg32 (c, reg, val);

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
}

/* ECC0: BFCLR <EA> {offset:width} */
static unsigned opecc0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x01e5)) {
		return (e68_op_undefined (c));
	}

	if (e68_set_ea_bf (c, 0)) {
		return (e68_op_undefined (c));
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
}

/* EEC0: BFSET <EA> {offset:width} */
static unsigned opeec0 (e68000_t *c)
{
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x01e5)) {
		return (e68_op_undefined (c));
	}

	if (e68_set_ea_bf (c, 0xffffffff)) {
		return (e68_op_undefined (c));
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
}

/* EFC0: BFINS Dn, <EA> {offset:width} */
static unsigned opefc0 (e68000_t *c)
{
	unsigned reg;
	uint32_t val;

	if (e68_get_ea_bf (c, &val, 0x07e5)) {
		return (e68_op_undefined (c));
	}

	reg = (c->ir[1] >> 12) & 7;
	val = e68_get_dreg32 (c, reg);

	if (e68_set_ea_bf (c, val)) {
		return (e68_op_undefined (c));
	}

	e68_cc_set_nz_32 (c, E68_SR_NZVC, (val << (32 - c->ea_bf_width)) & 0xffffffff);

	return (c->ircnt);
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
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5000 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5200 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5400 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5600 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5800 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5A00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5C00 */
	  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 5E00 */
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
