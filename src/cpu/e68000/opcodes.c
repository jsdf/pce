/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e68000/opcodes.c                                     *
 * Created:     2005-07-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#define e68_op_chk_addr(c, addr, wr) do { \
	if (((c)->flags & E68_FLAG_NOADDR) == 0) { \
		if ((addr) & 1) { \
			e68_exception_address (c, (addr), 1, (wr)); \
			return; \
		} \
	} \
	} while (0)

#define e68_op_supervisor(c) do { \
	if ((c)->supervisor == 0) { \
		e68_exception_privilege (c); \
		return; \
	} \
	} while (0)

#define e68_op_68010(c) \
	if (!((c)->flags & E68_FLAG_68010)) { \
		e68_op_undefined (c); \
		return; \
	}


static void e68_op_undefined (e68000_t *c)
{
	e68_exception_illegal (c);
	e68_set_clk (c, 2);
}

/* 0000: ORI.B #XX, <EA> */
static void op0000 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_op_prefetch8 (c, s1);

	if ((c->ir[0] & 0x3f) == 0x3c) {
		/* ORI.B #XX, CCR */
		e68_set_clk (c, 20);
		e68_op_prefetch (c);
		e68_set_ccr (c, (e68_get_ccr (c) | s1) & E68_SR_XNZVC);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 | s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0040_3C: ORI.W #XXXX, SR */
static void op0040_3c (e68000_t *c)
{
	uint16_t s1;

	e68_op_supervisor (c);
	e68_op_prefetch16 (c, s1);
	e68_set_clk (c, 20);
	e68_op_prefetch (c);
	e68_set_sr (c, (e68_get_sr (c) | s1) & E68_SR_MASK);
}

/* 0040: ORI.W #XXXX, <EA> */
static void op0040 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x3f) == 0x3c) {
		op0040_3c (c);
		return;
	}

	e68_op_prefetch16 (c, s1);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 | s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 0080: ORI.L #XXXXXXXX, <EA> */
static void op0080 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_op_prefetch32 (c, s1);
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 | s2;

	e68_set_clk (c, 16);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 0100_00: BTST Dn, Dn */
static void op0100_00 (e68000_t *c)
{
	unsigned i;
	uint32_t s, m;

	s = e68_get_dreg32 (c, e68_ir_reg0 (c));
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;

	e68_set_clk (c, 6);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
}

/* 0100_08: MOVEP.W d16(An), Dn */
static void op0100_08 (e68000_t *c)
{
	uint32_t addr;
	uint16_t v;

	e68_op_prefetch (c);
	addr = e68_get_areg32 (c, e68_ir_reg0 (c)) + e68_exts16 (c->ir[1]);

	v = e68_get_mem8 (c, addr);
	v = (v << 8) | e68_get_mem8 (c, addr + 2);

	e68_set_clk (c, 16);
	e68_set_dreg16 (c, e68_ir_reg9 (c), v);
	e68_op_prefetch (c);
}

/* 0100: BTST Dn, <EA> */
static void op0100 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, m;

	switch (c->ir[0] & 0x38) {
	case 0x00:
		op0100_00 (c);
		return;

	case 0x08:
		op0100_08 (c);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0xffc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;

	e68_set_clk (c, 4);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
}

/* 0140_00: BCHG Dn, Dn */
static void op0140_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;
	d = s ^ m;

	e68_set_clk (c, 8);
	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
}

/* 0140_01: MOVEP.L d16(An), Dn */
static void op0140_01 (e68000_t *c)
{
	uint32_t addr;
	uint32_t v;

	e68_op_prefetch (c);
	addr = e68_get_areg32 (c, e68_ir_reg0 (c)) + e68_exts16 (c->ir[1]);

	v = e68_get_mem8 (c, addr);
	v = (v << 8) | e68_get_mem8 (c, addr + 2);
	v = (v << 8) | e68_get_mem8 (c, addr + 4);
	v = (v << 8) | e68_get_mem8 (c, addr + 6);

	e68_set_clk (c, 24);
	e68_set_dreg32 (c, e68_ir_reg9 (c), v);
	e68_op_prefetch (c);
}

/* 0140: BCHG Dn, <EA> */
static void op0140 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		op0140_00 (c);
		return;

	case 0x01:
		op0140_01 (c);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;
	d = s ^ m;

	e68_set_clk (c, 8);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0180_00: BCLR Dn, Dn */
static void op0180_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;
	d = s & ~m;

	e68_set_clk (c, 10);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_dreg32 (c, r, d);
	e68_op_prefetch (c);
}

/* 0180_01: MOVEP.W Dn, d16(An) */
static void op0180_01 (e68000_t *c)
{
	uint32_t addr;
	uint16_t v;

	e68_op_prefetch (c);
	addr = e68_get_areg32 (c, e68_ir_reg0 (c)) + e68_exts16 (c->ir[1]);

	v = e68_get_dreg16 (c, e68_ir_reg9 (c));

	e68_set_mem8 (c, addr + 0, v >> 8);
	e68_set_mem8 (c, addr + 2, v & 0xff);

	e68_set_clk (c, 16);
	e68_op_prefetch (c);
}

/* 0180: BCLR Dn, <EA> */
static void op0180 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		op0180_00 (c);
		return;

	case 0x01:
		op0180_01 (c);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;
	d = s & ~m;

	e68_set_clk (c, 8);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 01C0_00: BSET Dn, Dn */
static void op01c0_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;
	d = s | m;

	e68_set_clk (c, 8);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_dreg32 (c, r, d);
	e68_op_prefetch (c);
}

/* 01C0_01: MOVEP.L Dn, d16(An) */
static void op01c0_01 (e68000_t *c)
{
	uint32_t addr;
	uint32_t v;

	e68_op_prefetch (c);
	addr = e68_get_areg32 (c, e68_ir_reg0 (c)) + e68_exts16 (c->ir[1]);

	v = e68_get_dreg32 (c, e68_ir_reg9 (c));

	e68_set_mem8 (c, addr + 0, (v >> 24) & 0xff);
	e68_set_mem8 (c, addr + 2, (v >> 16) & 0xff);
	e68_set_mem8 (c, addr + 4, (v >> 8) & 0xff);
	e68_set_mem8 (c, addr + 6, v & 0xff);

	e68_set_clk (c, 24);
	e68_op_prefetch (c);
}

/* 01C0: BSET Dn, <EA> */
static void op01c0 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		op01c0_00 (c);
		return;

	case 0x01:
		op01c0_01 (c);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;
	d = s | m;

	e68_set_clk (c, 8);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0200: ANDI.B #XX, <EA> */
static void op0200 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_op_prefetch8 (c, s1);

	if ((c->ir[0] & 0x3f) == 0x3c) {
		/* ANDI.B #XX, CCR */
		e68_set_clk (c, 20);
		e68_op_prefetch (c);
		e68_set_ccr (c, e68_get_ccr (c) & s1 & E68_SR_XNZVC);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 & s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0240_3C: ANDI.W #XXXX, SR */
static void op0240_4c (e68000_t *c)
{
	uint16_t s1;

	e68_op_supervisor (c);
	e68_op_prefetch16 (c, s1);
	e68_set_clk (c, 20);
	e68_op_prefetch (c);
	e68_set_sr (c, e68_get_sr (c) & s1);
}

/* 0240: ANDI.W #XXXX, <EA> */
static void op0240 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x3f) == 0x3c) {
		op0240_4c (c);
		return;
	}

	e68_op_prefetch16 (c, s1);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 & s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 0280: ANDI.L #XXXXXXXX, <EA> */
static void op0280 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_op_prefetch32 (c, s1);
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 & s2;

	e68_set_clk (c, 16);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 0400: SUBI.B #XX, <EA> */
static void op0400 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_op_prefetch8 (c, s1);
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0440: SUBI.W #XXXX, <EA> */
static void op0440 (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_op_prefetch16 (c, s1);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 0480: SUBI.L #XXXXXXXX, <EA> */
static void op0480 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_op_prefetch32 (c, s1);
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s2 - s1;

	e68_set_clk (c, 16);
	e68_cc_set_sub_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 0600: ADDI.B #XX, <EA> */
static void op0600 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_op_prefetch8 (c, s1);
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_cc_set_add_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0640: ADDI.W #XXXX, <EA> */
static void op0640 (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_op_prefetch16 (c, s1);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_cc_set_add_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 0680: ADDI.L #XXXXXXXX, <EA> */
static void op0680 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_op_prefetch32 (c, s1);
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 + s2;

	e68_set_clk (c, 16);
	e68_cc_set_add_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 0800_00: BTST #XX, Dn */
static void op0800_00 (e68000_t *c)
{
	uint16_t s1;
	uint32_t s2, d;

	e68_op_prefetch8 (c, s1);
	s2 = e68_get_dreg32 (c, e68_ir_reg0 (c));

	d = s2 & (1UL << (s1 & 0x1f));

	e68_set_clk (c, 10);
	e68_set_sr_z (c, d == 0);
	e68_op_prefetch (c);
}

/* 0800: BTST #XX, <EA> */
static void op0800 (e68000_t *c)
{
	uint8_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x00) {
		op0800_00 (c);
		return;
	}

	e68_op_prefetch8 (c, s1);
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x7fc, &s2);

	d = s2 & (1 << (s1 & 7));

	e68_set_clk (c, 8);
	e68_set_sr_z (c, d == 0);
	e68_op_prefetch (c);
}

/* 0840_00: BCHG #XX, Dn */
static void op0840_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	e68_op_prefetch (c);
	i = c->ir[1] & 0x1f;
	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	m = 1UL << i;
	d = s ^ m;

	e68_set_clk (c, 12);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* 0840: BCHG #XX, <EA> */
static void op0840 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	if ((c->ir[0] & 0x38) == 0x00) {
		op0840_00 (c);
		return;
	}

	e68_op_prefetch (c);
	i = c->ir[1] & 0x07;
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);

	m = 1 << i;
	d = s ^ m;

	e68_set_clk (c, 12);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0880_00: BCLR #XX, Dn */
static void op0880_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	e68_op_prefetch (c);
	i = c->ir[1] & 0x1f;
	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	m = 1UL << i;
	d = s & ~m;

	e68_set_clk (c, 14);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* 0880: BCLR #XX, <EA> */
static void op0880 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	if ((c->ir[0] & 0x38) == 0x00) {
		op0880_00 (c);
		return;
	}

	e68_op_prefetch (c);
	i = c->ir[1] & 0x07;
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);

	m = 1 << i;
	d = s & ~m;

	e68_set_clk (c, 12);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 08C0_00: BSET #XX, Dn */
static void op08c0_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	e68_op_prefetch (c);
	i = c->ir[1] & 0x1f;
	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	m = 1UL << i;
	d = s | m;

	e68_set_clk (c, 12);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* 08C0: BSET #XX, <EA> */
static void op08c0 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	if ((c->ir[0] & 0x38) == 0x00) {
		op08c0_00 (c);
		return;
	}

	e68_op_prefetch (c);
	i = c->ir[1] & 0x07;
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);

	m = 1 << i;
	d = s | m;

	e68_set_clk (c, 12);
	e68_set_sr_z (c, (s & m) == 0);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0A00: EORI.B #XX, <EA> */
static void op0a00 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_op_prefetch8 (c, s1);

	if ((c->ir[0] & 0x3f) == 0x3c) {
		/* 0A00_3C: EORI.B #XX, CCR */
		e68_set_clk (c, 20);
		e68_op_prefetch (c);
		e68_set_ccr (c, (e68_get_ccr (c) ^ s1) & E68_SR_XNZVC);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 0A40_3C: EORI.W #XXXX, SR */
static void op0a40_3c (e68000_t *c)
{
	uint16_t s1;

	e68_op_supervisor (c);
	e68_op_prefetch16 (c, s1);
	e68_set_clk (c, 20);
	e68_op_prefetch (c);
	e68_set_sr (c, (e68_get_sr (c) ^ s1) & E68_SR_MASK);
}

/* 0A40: EORI.W #XXXX, <EA> */
static void op0a40 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x3f) == 0x3c) {
		op0a40_3c (c);
		return;
	}

	e68_op_prefetch16 (c, s1);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 0A80: EORI.L #XXXXXXXX, <EA> */
static void op0a80 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_op_prefetch32 (c, s1);
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_clk (c, 12);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 0C00: CMPI.B #XX, <EA> */
static void op0c00 (e68000_t *c)
{
	uint8_t s1, s2;

	e68_op_prefetch8 (c, s1);
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x07fd, &s2);

	e68_set_clk (c, 8);
	e68_cc_set_cmp_8 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* 0C40: CMPI.W #XXXX, <EA> */
static void op0c40 (e68000_t *c)
{
	uint16_t s1, s2;

	e68_op_prefetch16 (c, s1);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x07fd, &s2);

	e68_set_clk (c, 8);
	e68_cc_set_cmp_16 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* 0C80: CMPI.L #XXXXXXXX, <EA> */
static void op0c80 (e68000_t *c)
{
	uint32_t s1, s2;

	e68_op_prefetch32 (c, s1);
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x07fd, &s2);

	e68_set_clk (c, 12);
	e68_cc_set_cmp_32 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* 0E00: MOVES.B Rn, <EA> / MOVES.B <EA>, Rn*/
static void op0e00 (e68000_t *c)
{
	unsigned reg;
	uint8_t  v;

	e68_op_68010 (c);
	e68_op_prefetch (c);

	reg = (c->ir[1] >> 12) & 15;

	if (c->ir[1] & 0x0800) {
		if (reg & 8) {
			v = e68_get_areg32 (c, reg & 7) & 0xff;
		}
		else {
			v = e68_get_dreg8 (c, reg & 7);
		}

		e68_op_set_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, v);
	}
	else {
		e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &v);

		if (reg & 8) {
			e68_set_areg32 (c, reg & 7, e68_exts8 (v));
		}
		else {
			e68_set_dreg8 (c, reg & 7, v);
		}
	}

	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 0E40: MOVES.W Rn, <EA> / MOVES.W <EA>, Rn*/
static void op0e40 (e68000_t *c)
{
	unsigned reg;
	uint16_t v;

	e68_op_68010 (c);
	e68_op_prefetch (c);

	reg = (c->ir[1] >> 12) & 15;

	if (c->ir[1] & 0x0800) {
		if (reg & 8) {
			v = e68_get_areg16 (c, reg & 7);
		}
		else {
			v = e68_get_dreg16 (c, reg & 7);
		}

		e68_op_set_ea16 (c, 1, e68_ir_ea1 (c), 0x1fc, v);
	}
	else {
		e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x1fc, &v);

		if (reg & 8) {
			e68_set_areg16 (c, reg & 7, v);
		}
		else {
			e68_set_dreg16 (c, reg & 7, v);
		}
	}

	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 0E80: MOVES.L Rn, <EA> / MOVES.W <EA>, Rn*/
static void op0e80 (e68000_t *c)
{
	unsigned reg;
	uint32_t v;

	e68_op_68010 (c);
	e68_op_prefetch (c);

	reg = (c->ir[1] >> 12) & 15;

	if (c->ir[1] & 0x0800) {
		if (reg & 8) {
			v = e68_get_areg32 (c, reg & 7);
		}
		else {
			v = e68_get_dreg32 (c, reg & 7);
		}

		e68_op_set_ea32 (c, 1, e68_ir_ea1 (c), 0x1fc, v);
	}
	else {
		e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x1fc, &v);

		if (reg & 8) {
			e68_set_areg32 (c, reg & 7, v);
		}
		else {
			e68_set_dreg32 (c, reg & 7, v);
		}
	}

	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 1000: MOVE.B <EA>, <EA> */
static void op1000 (e68000_t *c)
{
	uint8_t val;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_op_set_ea8 (c, 1, e68_ir_ea2 (c), 0x01fd, val);
	e68_set_clk (c, 4);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, val);
	e68_op_prefetch (c);
}

/* 2000: MOVE.L <EA>, <EA> */
static void op2000 (e68000_t *c)
{
	uint32_t val;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_op_set_ea32 (c, 1, e68_ir_ea2 (c), 0x01fd, val);
	e68_set_clk (c, 4);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, val);
	e68_op_prefetch (c);
}

/* 2040: MOVEA.L <EA>, Ax */
static void op2040 (e68000_t *c)
{
	uint32_t val;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_areg32 (c, e68_ir_reg9 (c), val);
	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 3000: MOVE.W <EA>, <EA> */
static void op3000 (e68000_t *c)
{
	uint16_t val;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_op_set_ea16 (c, 1, e68_ir_ea2 (c), 0x01fd, val);
	e68_set_clk (c, 4);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, val);
	e68_op_prefetch (c);
}

/* 3040: MOVEA.W <EA>, Ax */
static void op3040 (e68000_t *c)
{
	uint16_t val;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_areg32 (c, e68_ir_reg9 (c), e68_exts16 (val));
	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 4000: NEGX.B <EA> */
static void op4000 (e68000_t *c)
{
	uint8_t s, d;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1 - e68_get_sr_x (c)) & 0xff;

	if (d != 0) {
		e68_set_sr_z (c, 0);
	}

	e68_set_clk (c, 8);
	e68_set_sr_n (c, d & 0x80);
	e68_set_sr_v (c, d & s & 0x80);
	e68_set_sr_xc (c, (d | s) & 0x80);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 4040: NEGX.W <EA> */
static void op4040 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1 - e68_get_sr_x (c)) & 0xffff;

	if (d != 0) {
		e68_set_sr_z (c, 0);
	}

	e68_set_clk (c, 8);
	e68_set_sr_n (c, d & 0x8000);
	e68_set_sr_v (c, d & s & 0x8000);
	e68_set_sr_xc (c, (d | s) & 0x8000);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 4080: NEGX.L <EA> */
static void op4080 (e68000_t *c)
{
	uint32_t s, d;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1 - e68_get_sr_x (c)) & 0xffffffff;

	if (d != 0) {
		e68_set_sr_z (c, 0);
	}

	e68_set_clk (c, 10);
	e68_set_sr_n (c, d & 0x80000000);
	e68_set_sr_v (c, d & s & 0x80000000);
	e68_set_sr_xc (c, (d | s) & 0x80000000);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 40C0: MOVE.W SR, <EA> */
static void op40c0 (e68000_t *c)
{
	uint16_t s;

	if (c->flags & E68_FLAG_68010) {
		e68_op_supervisor (c);
	}

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	s = e68_get_sr (c) & E68_SR_MASK;

	e68_set_clk (c, 4);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, s);
}

/* 4180: CHK <EA>, Dx */
static void op4180 (e68000_t *c)
{
	int      trap;
	uint16_t s1, s2;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg16 (c, e68_ir_reg9 (c));

	if (s2 & 0x8000) {
		trap = 1;
		e68_set_sr_n (c, 1);
	}
	else if ((s1 & 0x8000) || (s2 > s1)) {
		trap = 1;
		e68_set_sr_n (c, 0);
	}
	else {
		trap = 0;
	}

	e68_set_clk (c, 14);

	if (trap) {
		e68_exception_check (c);
	}
	else {
		e68_op_prefetch (c);
	}
}

/* 41C0: LEA <EA>, Ax */
static void op41c0 (e68000_t *c)
{
	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x7e4, 32)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	e68_set_clk (c, 4);
	e68_set_areg32 (c, e68_ir_reg9 (c), c->ea_val);
	e68_op_prefetch (c);
}

/* 4200: CLR.B <EA> */
static void op4200 (e68000_t *c)
{
	uint8_t s;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	e68_set_clk (c, 4);
	e68_set_cc (c, E68_SR_N | E68_SR_V | E68_SR_C, 0);
	e68_set_cc (c, E68_SR_Z, 1);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, 0);
}

/* 4240: CLR.W <EA> */
static void op4240 (e68000_t *c)
{
	uint16_t s;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	e68_set_clk (c, 4);
	e68_set_cc (c, E68_SR_N | E68_SR_V | E68_SR_C, 0);
	e68_set_cc (c, E68_SR_Z, 1);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, 0);
}

/* 4280: CLR.L <EA> */
static void op4280 (e68000_t *c)
{
	uint32_t s;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	e68_set_clk (c, 6);
	e68_set_cc (c, E68_SR_N | E68_SR_V | E68_SR_C, 0);
	e68_set_cc (c, E68_SR_Z, 1);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, 0);
}

/* 42C0: MOVE.W CCR, <EA> */
static void op42c0 (e68000_t *c)
{
	uint16_t s;

	e68_op_68010 (c);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	s = e68_get_sr (c) & E68_CCR_MASK;

	e68_set_clk (c, 4);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, s);
}

/* 4400: NEG.B <EA> */
static void op4400 (e68000_t *c)
{
	uint8_t s, d;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1) & 0xff;

	e68_set_clk (c, 4);
	e68_set_sr_n (c, d & 0x80);
	e68_set_sr_v (c, d & s & 0x80);
	e68_set_sr_z (c, d == 0);
	e68_set_sr_xc (c, (d | s) & 0x80);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 4440: NEG.W <EA> */
static void op4440 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1) & 0xffff;

	e68_set_clk (c, 4);
	e68_set_sr_n (c, d & 0x8000);
	e68_set_sr_v (c, d & s & 0x8000);
	e68_set_sr_z (c, d == 0);
	e68_set_sr_xc (c, (d | s) & 0x8000);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 4480: NEG.L <EA> */
static void op4480 (e68000_t *c)
{
	uint32_t s, d;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1) & 0xffffffff;

	e68_set_clk (c, 6);
	e68_set_sr_n (c, d & 0x80000000);
	e68_set_sr_v (c, d & s & 0x80000000);
	e68_set_sr_z (c, d == 0);
	e68_set_sr_xc (c, (d | s) & 0x80000000);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 44C0: MOVE.W <EA>, CCR */
static void op44c0 (e68000_t *c)
{
	uint16_t s;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s);
	e68_set_clk (c, 12);
	e68_set_ccr (c, s & E68_SR_XNZVC);
	e68_op_prefetch (c);
}

/* 4600: NOT.B <EA> */
static void op4600 (e68000_t *c)
{
	uint8_t s, d;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = ~s & 0xff;

	e68_set_clk (c, 4);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 4640: NOT.W <EA> */
static void op4640 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = ~s & 0xffff;

	e68_set_clk (c, 4);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 4680: NOT.L <EA> */
static void op4680 (e68000_t *c)
{
	uint32_t s, d;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = ~s & 0xffffffff;

	e68_set_clk (c, 6);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 46C0: MOVE.W <EA>, SR */
static void op46c0 (e68000_t *c)
{
	uint16_t s;

	e68_op_supervisor (c);
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s);
	e68_set_clk (c, 12);
	e68_set_sr (c, s & E68_SR_MASK);
	e68_op_prefetch (c);
}

/* 4800: NBCD.B <EA> */
static void op4800 (e68000_t *c)
{
	uint8_t  s;
	uint16_t d;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = 0U - s - e68_get_sr_x (c);

	if (d & 0x0f) {
		d -= 0x06;
	}

	if (d & 0xf0) {
		d -= 0x60;
	}

	e68_set_cc (c, E68_SR_XC, d & 0xff00);

	if (d & 0xff) {
		e68_set_sr_z (c, 0);
	}

	e68_set_clk (c, 6);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d & 0xff);
}

/* 4840_00: SWAP Dx */
static void op4840_00 (e68000_t *c)
{
	unsigned reg;
	uint32_t val;

	reg = e68_ir_reg0 (c);
	val = e68_get_dreg32 (c, reg);

	val = ((val << 16) | (val >> 16)) & 0xffffffff;

	e68_set_clk (c, 4);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, val);
	e68_set_dreg32 (c, reg, val);
	e68_op_prefetch (c);
}

/* 4840: PEA <EA> */
static void op4840 (e68000_t *c)
{
	if ((c->ir[0] & 0x38) == 0x00) {
		op4840_00 (c);
		return;
	}

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x7e4, 32)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	e68_set_clk (c, 12);
	e68_push32 (c, c->ea_val);
	e68_op_prefetch (c);
}

/* 4880_00: EXT.W Dn */
static void op4880_00 (e68000_t *c)
{
	unsigned r;
	uint16_t s, d;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, r);

	d = (s & 0x80) ? (s | 0xff00) : s;

	e68_set_clk (c, 4);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_dreg16 (c, r, d);
	e68_op_prefetch (c);
}

/* 4880_04: MOVEM.W list, -(Ax) */
static void op4880_04 (e68000_t *c)
{
	unsigned i;
	uint16_t r, v;
	uint32_t a;

	e68_op_prefetch16 (c, r);
	a = e68_get_areg32 (c, e68_ir_reg0 (c));

	if (r != 0) {
		e68_op_chk_addr (c, a, 1);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			a = (a - 2) & 0xffffffff;
			if (i < 8) {
				v = e68_get_areg16 (c, 7 - i);
			}
			else {
				v = e68_get_dreg16 (c, 15 - i);
			}

			e68_set_mem16 (c, a, v);

			e68_set_clk (c, 4);
		}
		r >>= 1;
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
}

/* 4880_XX: MOVEM.W list, <EA> */
static void op4880_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r, v;
	uint32_t a;

	e68_op_prefetch16 (c, r);

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x1e4, 16)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	a = c->ea_val;

	if (r != 0) {
		e68_op_chk_addr (c, a, 1);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			if (i < 8) {
				v = e68_get_dreg16 (c, i);
			}
			else {
				v = e68_get_areg16 (c, i - 8);
			}
			e68_set_mem16 (c, a, v);
			a = (a + 2) & 0xffffffff;

			e68_set_clk (c, 4);
		}
		r >>= 1;
	}

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
}

/* 4880: misc */
static void op4880 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		op4880_00 (c);
		break;

	case 0x04:
		op4880_04 (c);
		break;

	default:
		op4880_xx (c);
		break;
	}
}

/* 48C0_00: EXT.L Dn */
static void op48c0_00 (e68000_t *c)
{
	unsigned r;
	uint32_t s, d;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, r);

	d = (s & 0x8000) ? (s | 0xffff0000) : s;

	e68_set_clk (c, 4);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_dreg32 (c, r, d);
	e68_op_prefetch (c);
}

/* 48C0_04: MOVEM.L list, -(Ax) */
static void op48c0_04 (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_op_prefetch16 (c, r);
	a = e68_get_areg32 (c, e68_ir_reg0 (c));

	if (r != 0) {
		e68_op_chk_addr (c, a, 1);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			a = (a - 4) & 0xffffffff;
			if (i < 8) {
				v = e68_get_areg32 (c, 7 - i);
			}
			else {
				v = e68_get_dreg32 (c, 15 - i);
			}
			e68_set_mem32 (c, a, v);

			e68_set_clk (c, 8);
		}
		r >>= 1;
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
}

/* 48C0_XX: MOVEM.L list, <EA> */
static void op48c0_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_op_prefetch16 (c, r);

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x1e4, 32)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	a = c->ea_val;

	if (r != 0) {
		e68_op_chk_addr (c, a, 1);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			if (i < 8) {
				v = e68_get_dreg32 (c, i);
			}
			else {
				v = e68_get_areg32 (c, i - 8);
			}
			e68_set_mem32 (c, a, v);
			a = (a + 4) & 0xffffffff;

			e68_set_clk (c, 8);
		}
		r >>= 1;
	}

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
}

/* 48C0: misc */
static void op48c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		op48c0_00 (c);
		break;

	case 0x04:
		op48c0_04 (c);
		break;

	default:
		op48c0_xx (c);
		break;
	}
}

/* 49C0: misc */
static void op49c0 (e68000_t *c)
{
	c->op49c0[(c->ir[0] >> 3) & 7] (c);
}

/* 4A00: TST.B <EA> */
static void op4a00 (e68000_t *c)
{
	uint8_t s;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
}

/* 4A40: TST.W <EA> */
static void op4a40 (e68000_t *c)
{
	uint16_t s;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
}

/* 4A80: TST.L <EA> */
static void op4a80 (e68000_t *c)
{
	uint32_t s;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	e68_set_clk (c, 8);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
}

/* 4AFC: ILLEGAL */
static void op4afc (e68000_t *c)
{
	unsigned long pc;

	if (c->hook != NULL) {
		pc = e68_get_pc (c);

		if (c->hook (c->hook_ext, c->ir[2]) == 0) {
			if (e68_get_pc (c) == pc) {
				e68_op_prefetch (c);
				e68_op_prefetch (c);
			}

			e68_set_clk (c, 8);

			return;
		}
	}

	e68_exception_illegal (c);
}

/* 4AC0: TAS <EA> */
static void op4ac0 (e68000_t *c)
{
	uint8_t s, d;

	if (c->ir[0] == 0x4afc) {
		op4afc (c);
		return;
	}

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = s | 0x80;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, s);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 4C80_03: MOVEM.W (Ax)+, list */
static void op4c80_03 (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_op_prefetch16 (c, r);

	a = e68_get_areg32 (c, e68_ir_reg0 (c));

	if (r != 0) {
		e68_op_chk_addr (c, a, 0);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			v = e68_get_mem16 (c, a);
			v = e68_exts16 (v);
			if (i < 8) {
				e68_set_dreg32 (c, i, v);
			}
			else {
				e68_set_areg32 (c, i - 8, v);
			}
			a = (a + 2) & 0xffffffff;

			e68_set_clk (c, 4);
		}
		r >>= 1;
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
}

/* 4C80_XX: MOVEM.W <EA>, list */
static void op4c80_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_op_prefetch16 (c, r);

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x7ec, 16)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	a = c->ea_val;

	if (r != 0) {
		e68_op_chk_addr (c, a, 0);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			v = e68_get_mem16 (c, a);
			v = e68_exts16 (v);
			if (i < 8) {
				e68_set_dreg32 (c, i, v);
			}
			else {
				e68_set_areg32 (c, i - 8, v);
			}
			a = (a + 2) & 0xffffffff;

			e68_set_clk (c, 4);
		}
		r >>= 1;
	}

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
}

/* 4C80: misc */
static void op4c80 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		e68_op_undefined (c);
		break;

	case 0x03:
		op4c80_03 (c);
		break;

	default:
		op4c80_xx (c);
		break;
	}
}

/* 4CC0_03: MOVEM.L (Ax)+, list */
static void op4cc0_03 (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_op_prefetch16 (c, r);

	a = e68_get_areg32 (c, e68_ir_reg0 (c));

	if (r != 0) {
		e68_op_chk_addr (c, a, 0);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			v = e68_get_mem32 (c, a);
			if (i < 8) {
				e68_set_dreg32 (c, i, v);
			}
			else {
				e68_set_areg32 (c, i - 8, v);
			}
			a = (a + 4) & 0xffffffff;

			e68_set_clk (c, 8);
		}
		r >>= 1;
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
}

/* 4CC0_XX: MOVEM.L <EA>, list */
static void op4cc0_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_op_prefetch16 (c, r);

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x7ec, 32)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	a = c->ea_val;

	if (r != 0) {
		e68_op_chk_addr (c, a, 0);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			v = e68_get_mem32 (c, a);
			if (i < 8) {
				e68_set_dreg32 (c, i, v);
			}
			else {
				e68_set_areg32 (c, i - 8, v);
			}
			a = (a + 4) & 0xffffffff;

			e68_set_clk (c, 8);
		}
		r >>= 1;
	}

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
}

/* 4CC0: misc */
static void op4cc0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		e68_op_undefined (c);
		break;

	case 0x03:
		op4cc0_03 (c);
		break;

	default:
		op4cc0_xx (c);
		break;
	}
}

/* 4E40_00: TRAP #XX */
static void op4e40_00 (e68000_t *c)
{
	e68_op_prefetch (c);
	e68_exception_trap (c, c->ir[0] & 15);
}

/* 4E40_02: LINK An, #XXXX */
static void op4e40_02 (e68000_t *c)
{
	unsigned r;
	uint32_t s;

	e68_op_prefetch (c);

	r = e68_ir_reg0 (c);
	s = e68_exts16 (c->ir[1]);

	e68_set_clk (c, 16);
	e68_set_areg32 (c, 7, e68_get_areg32 (c, 7) - 4);
	e68_set_mem32 (c, e68_get_areg32 (c, 7), e68_get_areg32 (c, r));
	e68_set_areg32 (c, r, e68_get_areg32 (c, 7));
	e68_set_areg32 (c, 7, e68_get_areg32 (c, 7) + s);
	e68_op_prefetch (c);
}

/* 4E40_03: UNLK An */
static void op4e40_03 (e68000_t *c)
{
	unsigned r;
	uint32_t a;

	r = e68_ir_reg0 (c);
	a = e68_get_areg32 (c, r);

	e68_op_chk_addr (c, a, 0);

	e68_set_clk (c, 12);
	e68_set_areg32 (c, 7, a);
	e68_set_areg32 (c, r, e68_get_mem32 (c, a));
	e68_set_areg32 (c, 7, e68_get_areg32 (c, 7) + 4);
	e68_op_prefetch (c);
}

/* 4E40_04: MOVE.L Ax, USP */
static void op4e40_04 (e68000_t *c)
{
	uint32_t s;

	e68_op_supervisor (c);

	s = e68_get_areg32 (c, e68_ir_reg0 (c));

	e68_set_usp (c, s);
	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 4E40_05: MOVE.L USP, Ax */
static void op4e40_05 (e68000_t *c)
{
	uint32_t s;

	e68_op_supervisor (c);

	s = e68_get_usp (c);

	e68_set_areg32 (c, e68_ir_reg0 (c), s);
	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 4E70: RESET */
static void op4e70 (e68000_t *c)
{
	e68_op_supervisor (c);

	if (c->flags & E68_FLAG_NORESET) {
		e68_set_clk (c, 4);
		e68_op_prefetch (c);
		return;
	}

	e68_set_clk (c, 132);
	e68_reset (c);
}

/* 4E71: NOP */
static void op4e71 (e68000_t *c)
{
	e68_set_clk (c, 4);
	e68_op_prefetch (c);
}

/* 4E72: STOP #XXXX */
static void op4e72 (e68000_t *c)
{
	e68_op_supervisor (c);
	e68_op_prefetch (c);
	e68_set_sr (c, c->ir[1]);
	e68_set_clk (c, 4);
	c->halt |= 1;
	e68_op_prefetch (c);
}

/* 4E73: RTE */
static void op4e73 (e68000_t *c)
{
	uint32_t sp, pc;
	uint16_t sr, fmt;

	e68_op_supervisor (c);

	sp = e68_get_ssp (c);
	sr = e68_get_mem16 (c, sp);
	pc = e68_get_mem32 (c, sp + 2);

	if (c->flags & E68_FLAG_68010) {
		fmt = e68_get_mem16 (c, sp + 6);

		switch ((fmt >> 12) & 0x0f) {
		case 0:
		case 8:
			break;

		default:
			e68_exception_format (c);
			return;
		}
	}

	e68_set_sr (c, sr);
	e68_set_pc (c, pc);
	e68_set_ir_pc (c, pc);

	if (c->flags & E68_FLAG_68010) {
		e68_set_ssp (c, sp + 8);
	}
	else {
		e68_set_ssp (c, sp + 6);
	}

	e68_set_clk (c, 20);
	e68_op_prefetch (c);
	e68_op_prefetch (c);
	e68_set_pc (c, pc);
}

/* 4E74: RTD */
static void op4e74 (e68000_t *c)
{
	uint32_t sp, im;

	e68_op_68010 (c);
	e68_op_prefetch (c);

	im = e68_exts16 (c->ir[1]);
	sp = e68_get_areg32 (c, 7);

	e68_set_ir_pc (c, e68_get_mem32 (c, sp));
	e68_set_areg32 (c, 7, sp + 4 + im);

	e68_set_clk (c, 16);
	e68_op_prefetch (c);
	e68_op_prefetch (c);
	e68_set_pc (c, e68_get_ir_pc (c) - 4);
}

/* 4E75: RTS */
static void op4e75 (e68000_t *c)
{
	uint32_t sp;

	sp = e68_get_areg32 (c, 7);

	e68_set_ir_pc (c, e68_get_mem32 (c, sp));
	e68_set_areg32 (c, 7, sp + 4);

	e68_set_clk (c, 16);
	e68_op_prefetch (c);
	e68_op_prefetch (c);
	e68_set_pc (c, e68_get_ir_pc (c) - 4);
}

/* 4E76: TRAPV */
static void op4e76 (e68000_t *c)
{
	e68_op_prefetch (c);

	if (e68_get_sr_v (c)) {
		e68_exception_overflow (c);
	}
	else {
		e68_set_clk (c, 4);
	}
}

/* 4E77: RTR */
static void op4e77 (e68000_t *c)
{
	uint32_t sp;

	sp = e68_get_areg32 (c, 7);

	e68_set_ccr (c, e68_get_mem16 (c, sp) & E68_CCR_MASK);
	e68_set_ir_pc (c, e68_get_mem32 (c, sp + 2));
	e68_set_areg32 (c, 7, sp + 6);

	e68_set_clk (c, 20);
	e68_op_prefetch (c);
	e68_op_prefetch (c);
	e68_set_pc (c, e68_get_ir_pc (c) - 4);
}

/* 4E7A: MOVEC Rc, Rx */
static void op4e7a (e68000_t *c)
{
	unsigned cr, rx;
	uint32_t val;

	e68_op_68010 (c);
	e68_op_supervisor (c);
	e68_op_prefetch (c);

	cr = c->ir[1] & 0x0fff;
	rx = (c->ir[1] >> 12) & 0x0f;

	switch (cr) {
	case 0x000:
		val = e68_get_sfc (c);
		break;

	case 0x001:
		val = e68_get_dfc (c);
		break;

	case 0x002:
		if (((c)->flags & E68_FLAG_68020) == 0) {
			return (e68_op_undefined (c));
		}
		val = e68_get_cacr (c);
		break;

	case 0x800:
		val = e68_get_usp (c);
		break;

	case 0x801:
		val = e68_get_vbr (c);
		break;

	case 0x802:
		if (((c)->flags & E68_FLAG_68020) == 0) {
			return (e68_op_undefined (c));
		}
		val = e68_get_caar (c);
		break;

	default:
		e68_exception_illegal (c);
		return;
	}

	if (rx & 8) {
		e68_set_areg32 (c, rx & 7, val);
	}
	else {
		e68_set_dreg32 (c, rx & 7, val);
	}

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
}

/* 4E7B: MOVEC Rx, Rc */
static void op4e7b (e68000_t *c)
{
	unsigned cr, rx;
	uint32_t val;

	e68_op_68010 (c);
	e68_op_supervisor (c);
	e68_op_prefetch (c);

	cr = c->ir[1] & 0x0fff;
	rx = (c->ir[1] >> 12) & 0x0f;

	if (rx & 8) {
		val = e68_get_areg32 (c, rx & 7);
	}
	else {
		val = e68_get_dreg32 (c, rx & 7);
	}

	switch (cr) {
	case 0x000:
		e68_set_sfc (c, val);
		break;

	case 0x001:
		e68_set_dfc (c, val);
		break;

	case 0x002:
		if (((c)->flags & E68_FLAG_68020) == 0) {
			return (e68_op_undefined (c));
		}
		e68_set_cacr (c, val);
		break;

	case 0x800:
		e68_set_usp (c, val);
		break;

	case 0x801:
		e68_set_vbr (c, val);
		break;

	case 0x802:
		if (((c)->flags & E68_FLAG_68020) == 0) {
			return (e68_op_undefined (c));
		}
		e68_set_caar (c, val);
		break;

	default:
		e68_exception_illegal (c);
		return;
	}

	e68_set_clk (c, 10);
	e68_op_prefetch (c);
}

/* 4E40: misc */
static void op4e40 (e68000_t *c)
{
	switch (c->ir[0]) {
	case 0x4e70:
		op4e70 (c);
		return;

	case 0x4e71:
		op4e71 (c);
		return;

	case 0x4e72:
		op4e72 (c);
		return;

	case 0x4e73:
		op4e73 (c);
		return;

	case 0x4e74:
		op4e74 (c);
		return;

	case 0x4e75:
		op4e75 (c);
		return;

	case 0x4e76:
		op4e76 (c);
		return;

	case 0x4e77:
		op4e77 (c);
		return;

	case 0x4e7a:
		op4e7a (c);
		return;

	case 0x4e7b:
		op4e7b (c);
		return;
	}

	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		op4e40_00 (c);
		return;

	case 0x02:
		op4e40_02 (c);
		return;

	case 0x03:
		op4e40_03 (c);
		return;

	case 0x04:
		op4e40_04 (c);
		return;

	case 0x05:
		op4e40_05 (c);
		return;
	}

	e68_op_undefined (c);
}

/* 4E80: JSR <EA> */
static void op4e80 (e68000_t *c)
{
	uint32_t addr;

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x07e4, 32)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	e68_set_clk (c, 16);
	addr = e68_get_ir_pc (c) - 2;
	e68_set_ir_pc (c, c->ea_val);
	e68_op_prefetch (c);
	e68_push32 (c, addr);
	e68_op_prefetch (c);
	e68_set_pc (c, c->ea_val);
}

/* 4EC0: JMP <EA> */
static void op4ec0 (e68000_t *c)
{
	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x07e4, 32)) {
		return;
	}

	if (c->ea_typ != E68_EA_TYPE_MEM) {
		e68_exception_illegal (c);
		return;
	}

	e68_set_clk (c, 8);
	e68_set_ir_pc (c, c->ea_val);
	e68_op_prefetch (c);
	e68_op_prefetch (c);
	e68_set_pc (c, c->ea_val);
}

/* 5000: ADDQ.B #X, <EA> */
static void op5000 (e68000_t *c)
{
	uint8_t s1, s2, d;

	s1 = (c->ir[0] >> 9) & 7;
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_cc_set_add_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 5040_08: ADDQ.W #X, Ax */
static void op5040_08 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg0 (c);

	s1 = (c->ir[0] >> 9) & 7;
	s2 = e68_get_areg32 (c, r);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* 5040: ADDQ.W #X, <EA> */
static void op5040 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		op5040_08 (c);
		return;
	}

	s1 = (c->ir[0] >> 9) & 7;
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_cc_set_add_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 5080_08: ADDQ.L #X, Ax */
static void op5080_08 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg0 (c);

	s1 = (c->ir[0] >> 9) & 7;
	s2 = e68_get_areg32 (c, r);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s1 + s2;

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* 5080: ADDQ.L #X, <EA> */
static void op5080 (e68000_t *c)
{
	uint32_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		op5080_08 (c);
		return;
	}

	s1 = (c->ir[0] >> 9) & 7;
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s1 + s2;

	e68_set_clk (c, 12);
	e68_cc_set_add_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 5100: SUBQ.B #X, <EA> */
static void op5100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	s1 = (c->ir[0] >> 9) & 7;
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 5140_08: SUBQ.W #X, Ax */
static void op5140_08 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg0 (c);

	s1 = (c->ir[0] >> 9) & 7;
	s2 = e68_get_areg32 (c, r);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* 5140: SUBQ.W #X, <EA> */
static void op5140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		op5140_08 (c);
		return;
	}

	s1 = (c->ir[0] >> 9) & 7;
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 5180_08: SUBQ.L #X, Ax */
static void op5180_08 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg0 (c);

	s1 = (c->ir[0] >> 9) & 7;
	s2 = e68_get_areg32 (c, r);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s2 - s1;

	e68_set_clk (c, 12);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* 5180: SUBQ.L #X, <EA> */
static void op5180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		op5180_08 (c);
		return;
	}

	s1 = (c->ir[0] >> 9) & 7;
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	if (s1 == 0) {
		s1 = 8;
	}

	d = s2 - s1;

	e68_set_clk (c, 12);
	e68_cc_set_sub_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* DBcc Dx, dist */
void e68_op_dbcc (e68000_t *c, int cond)
{
	unsigned reg;
	uint16_t val;
	uint32_t addr, dist;

	e68_op_prefetch (c);

	addr = e68_get_pc (c);
	dist = e68_exts16 (c->ir[1]);

	if (cond) {
		e68_set_clk (c, 12);
		e68_op_prefetch (c);
		return;
	}

	reg = e68_ir_reg0 (c);
	val = e68_get_dreg16 (c, reg);
	val = (val - 1) & 0xffff;
	e68_set_dreg16 (c, reg, val);

	if (val == 0xffff) {
		e68_set_clk (c, 14);
		e68_op_prefetch (c);
		return;
	}

	e68_set_clk (c, 10);
	e68_set_ir_pc (c, addr + dist);
	e68_op_prefetch (c);
	e68_op_prefetch (c);
	e68_set_pc (c, e68_get_ir_pc (c) - 4);
}

/* Scc <EA> */
void e68_op_scc (e68000_t *c, int cond)
{
	uint8_t val;

	val = cond ? 0xff : 0x00;

	e68_set_clk (c, 4);
	e68_op_set_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, val);
	e68_op_prefetch (c);
}

/* 50C0: ST <EA> / DBT Dx, dist */
static void op50c0 (e68000_t *c)
{
	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, 1);
	}
	else {
		e68_op_scc (c, 1);
	}
}

/* 51C0: SF <EA> / DBF Dx, dist */
static void op51c0 (e68000_t *c)
{
	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, 0);
	}
	else {
		e68_op_scc (c, 0);
	}
}

/* 52C0: SHI <EA> / DBHI Dx, dist */
static void op52c0 (e68000_t *c)
{
	int cond;

	cond = (!e68_get_sr_c (c) && !e68_get_sr_z (c));

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 53C0: SLS <EA> / DBLS Dx, dist */
static void op53c0 (e68000_t *c)
{
	int cond;

	cond = (e68_get_sr_c (c) || e68_get_sr_z (c));

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 54C0: SCC <EA> / DBCC Dx, dist */
static void op54c0 (e68000_t *c)
{
	int cond;

	cond = !e68_get_sr_c (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 55C0: SCS <EA> / DBCS Dx, dist */
static void op55c0 (e68000_t *c)
{
	int cond;

	cond = e68_get_sr_c (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 56C0: SNE <EA> / DBNE Dx, dist */
static void op56c0 (e68000_t *c)
{
	int cond;

	cond = !e68_get_sr_z (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 57C0: SEQ <EA> / DBEQ Dx, dist */
static void op57c0 (e68000_t *c)
{
	int cond;

	cond = e68_get_sr_z (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 58C0: SVC <EA> / DBVC Dx, dist */
static void op58c0 (e68000_t *c)
{
	int cond;

	cond = !e68_get_sr_v (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 59C0: SVS <EA> / DBVS Dx, dist */
static void op59c0 (e68000_t *c)
{
	int cond;

	cond = e68_get_sr_v (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 5AC0: SPL <EA> / DBPL Dx, dist */
static void op5ac0 (e68000_t *c)
{
	int cond;

	cond = !e68_get_sr_n (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 5BC0: SMI <EA> / DBMI Dx, dist */
static void op5bc0 (e68000_t *c)
{
	int cond;

	cond = e68_get_sr_n (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 5CC0: SGE <EA> / DBGE Dx, dist */
static void op5cc0 (e68000_t *c)
{
	int cond;

	cond = (e68_get_sr_n (c) == e68_get_sr_v (c));

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 5DC0: SLT <EA> / DBLT Dx, dist */
static void op5dc0 (e68000_t *c)
{
	int cond;

	cond = (e68_get_sr_n (c) != e68_get_sr_v (c));

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 5EC0: SGT <EA> / DBGT Dx, dist */
static void op5ec0 (e68000_t *c)
{
	int cond;

	cond = (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* 5FC0: SLE <EA> / DBLE Dx, dist */
static void op5fc0 (e68000_t *c)
{
	int cond;

	cond = (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c);

	if ((c->ir[0] & 0x38) == 0x08) {
		e68_op_dbcc (c, cond);
	}
	else {
		e68_op_scc (c, cond);
	}
}

/* conditional jump */
static
void e68_op_bcc (e68000_t *c, int cond)
{
	uint32_t addr, dist;

	addr = e68_get_pc (c) + 2;
	dist = e68_exts8 (c->ir[0]);

	if (dist == 0) {
		e68_op_prefetch (c);
		dist = e68_exts16 (c->ir[1]);
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
	e68_op_bcc (c, 1);
}

/* 6100: BSR dist */
static void op6100 (e68000_t *c)
{
	uint32_t addr;

	addr = e68_get_pc (c) + ((c->ir[0] & 0xff) ? 2 : 4);

	e68_push32 (c, addr);

	e68_op_bcc (c, 1);
}

/* 6200: BHI dist */
static void op6200 (e68000_t *c)
{
	e68_op_bcc (c, !e68_get_sr_c (c) && !e68_get_sr_z (c));
}

/* 6300: BLS dist */
static void op6300 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_c (c) || e68_get_sr_z (c));
}

/* 6400: BCC dist */
static void op6400 (e68000_t *c)
{
	e68_op_bcc (c, !e68_get_sr_c (c));
}

/* 6500: BCS dist */
static void op6500 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_c (c));
}

/* 6600: BNE dist */
static void op6600 (e68000_t *c)
{
	e68_op_bcc (c, !e68_get_sr_z (c));
}

/* 6700: BEQ dist */
static void op6700 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_z (c));
}

/* 6800: BVC dist */
static void op6800 (e68000_t *c)
{
	e68_op_bcc (c, !e68_get_sr_v (c));
}

/* 6900: BVS dist */
static void op6900 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_v (c));
}

/* 6A00: BPL dist */
static void op6a00 (e68000_t *c)
{
	e68_op_bcc (c, !e68_get_sr_n (c));
}

/* 6B00: BMI dist */
static void op6b00 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_n (c));
}

/* 6C00: BGE dist */
static void op6c00 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_n (c) == e68_get_sr_v (c));
}

/* 6D00: BLT dist */
static void op6d00 (e68000_t *c)
{
	e68_op_bcc (c, e68_get_sr_n (c) != e68_get_sr_v (c));
}

/* 6E00: BGT dist */
static void op6e00 (e68000_t *c)
{
	e68_op_bcc (c, (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c));
}

/* 6F00: BLE dist */
static void op6f00 (e68000_t *c)
{
	e68_op_bcc (c, (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c));
}

/* 7000: MOVEQ #XX, Dx */
static void op7000 (e68000_t *c)
{
	uint32_t val;

	val = e68_exts8 (c->ir[0]);

	e68_set_clk (c, 4);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, val);
	e68_set_dreg32 (c, e68_ir_reg9 (c), val);
	e68_op_prefetch (c);
}

/* 8000: OR.B <EA>, Dx */
static void op8000 (e68000_t *c)
{
	unsigned r;
	uint8_t  s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg8 (c, r);

	d = s1 | s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* 8040: OR.W <EA>, Dx */
static void op8040 (e68000_t *c)
{
	unsigned r;
	uint16_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg16 (c, r);

	d = s1 | s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* 8080: OR.L <EA>, Dx */
static void op8080 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, r);

	d = s1 | s2;

	e68_set_clk (c, 10);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* 80C0: DIVU.W <EA>, Dx */
static void op80c0 (e68000_t *c)
{
	unsigned r;
	uint32_t s2, d1, d2;
	uint16_t s1;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, r);

	if (s1 == 0) {
		e68_op_prefetch (c);
		e68_exception_divzero (c);
		return;
	}

	d1 = s2 / s1;
	d2 = s2 % s1;

	if (d1 & 0xffff0000) {
		e68_set_sr_v (c, 1);
		e68_set_sr_c (c, 0);
	}
	else {
		e68_set_dreg32 (c, r, ((d2 & 0xffff) << 16) | (d1 & 0xffff));
		e68_cc_set_nz_16 (c, E68_SR_NZVC, d1 & 0xffff);
	}

	e68_set_clk (c, 144);
	e68_op_prefetch (c);
}

/* 8100_00: SBCD <EA>, <EA> */
static void op8100_00 (e68000_t *c)
{
	uint8_t  s1, s2;
	uint16_t d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x08) {
		/* -(Ax) */
		ea1 |= (4 << 3);
		ea2 |= (4 << 3);
	}

	e68_op_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea8 (c, 1, ea2, 0x0011, &s2);

	d = (uint16_t) s2 - (uint16_t) s1 - e68_get_sr_x (c);

	if ((s2 & 0x0f) < (s1 & 0x0f)) {
		d -= 0x06;
	}

	if (d >= 0xa0) {
		d -= 0x60;
	}

	e68_set_clk (c, 10);
	e68_set_cc (c, E68_SR_XC, d & 0xff00);

	if (d & 0xff) {
		c->sr &= ~E68_SR_Z;
	}

	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 8100: OR.B Dx, <EA> */
static void op8100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0) {
		op8100_00 (c);
		return;
	}

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 | s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 8140: OR.W Dx, <EA> */
static void op8140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 | s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 8180: OR.L Dx, <EA> */
static void op8180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 | s2;

	e68_set_clk (c, 10);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 81C0: DIVS.W <EA>, Dx */
static void op81c0 (e68000_t *c)
{
	unsigned r;
	int      n1, n2;
	uint32_t s2, d1, d2;
	uint16_t s1;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, r);

	if (s1 == 0) {
		e68_op_prefetch (c);
		e68_exception_divzero (c);
		return;
	}

	n1 = 0;
	n2 = 0;

	if (s2 & 0x80000000) {
		n2 = 1;
		s2 = (~s2 + 1) & 0xffffffff;
	}

	if (s1 & 0x8000) {
		n1 = 1;
		s1 = (~s1 + 1) & 0xffff;
	}

	d1 = s2 / s1;
	d2 = s2 % s1;

	if (n2) {
		d2 = (~d2 + 1) & 0xffff;
	}

	if (n1 != n2) {
		d1 = (~d1 + 1) & 0xffffffff;
	}

	e68_set_clk (c, 162);

	switch (d1 & 0xffff8000) {
	case 0x00000000:
	case 0xffff8000:
		e68_set_dreg32 (c, r, ((d2 & 0xffff) << 16) | (d1 & 0xffff));
		e68_cc_set_nz_16 (c, E68_SR_NZVC, d1 & 0xffff);
		break;

	default:
		e68_set_sr_v (c, 1);
		e68_set_sr_c (c, 0);
		break;
	}

	e68_op_prefetch (c);
}

/* 9000: SUB.B <EA>, Dx */
static void op9000 (e68000_t *c)
{
	unsigned r;
	uint8_t  s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg8 (c, r);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* 9040: SUB.W <EA>, Dx */
static void op9040 (e68000_t *c)
{
	unsigned r;
	uint16_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_dreg16 (c, r);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* 9080: SUB.L <EA>, Dx */
static void op9080 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_dreg32 (c, r);

	d = s2 - s1;

	e68_set_clk (c, 10);
	e68_cc_set_sub_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* 90C0: SUBA.W <EA>, Ax */
static void op90c0 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;
	uint16_t t;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &t);
	s1 = e68_exts16 (t);
	s2 = e68_get_areg32 (c, r);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* 9100_00: SUBX.B <EA>, <EA> */
static void op9100_00 (e68000_t *c)
{
	uint8_t  s1, s2, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		/* pre-decrement */
		ea1 |= (4 << 3);
		ea2 |= (4 << 3);
	}

	e68_op_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea8 (c, 1, ea2, 0x0011, &s2);

	d = s2 - s1 - e68_get_sr_x (c);

	e68_set_clk (c, 8);
	e68_cc_set_subx_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 9100: SUB.B Dx, <EA> */
static void op9100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0x00) {
		op9100_00 (c);
		return;
	}

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* 9140_00: SUBX.W <EA>, <EA> */
static void op9140_00 (e68000_t *c)
{
	uint16_t s1, s2, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		/* pre-decrement */
		ea1 |= (4 << 3);
		ea2 |= (4 << 3);
	}

	e68_op_get_ea16 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea16 (c, 1, ea2, 0x0011, &s2);

	d = s2 - s1 - e68_get_sr_x (c);

	e68_set_clk (c, 8);
	e68_cc_set_subx_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 9140: SUB.W Dx, <EA> */
static void op9140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0x00) {
		op9140_00 (c);
		return;
	}

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s2 - s1;

	e68_set_clk (c, 8);
	e68_cc_set_sub_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* 9180_00: SUBX.L <EA>, <EA> */
static void op9180_00 (e68000_t *c)
{
	uint32_t s1, s2, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		/* pre-decrement */
		ea1 |= (4 << 3);
		ea2 |= (4 << 3);
	}

	e68_op_get_ea32 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea32 (c, 1, ea2, 0x0011, &s2);

	d = s2 - s1 - e68_get_sr_x (c);

	e68_set_clk (c, 12);
	e68_cc_set_subx_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 9180: SUB.L Dx, <EA> */
static void op9180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0x00) {
		op9180_00 (c);
		return;
	}

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s2 - s1;

	e68_set_clk (c, 12);
	e68_cc_set_sub_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* 91C0: SUBA.L <EA>, Ax */
static void op91c0 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_areg32 (c, r);

	d = s2 - s1;

	e68_set_clk (c, 10);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* A000: AXXX */
static void opa000 (e68000_t *c)
{
	c->last_trap_a = c->ir[0];

	e68_exception_axxx (c);
}

/* B000: CMP.B <EA>, Dx */
static void opb000 (e68000_t *c)
{
	uint8_t s1, s2;

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg8 (c, e68_ir_reg9 (c));

	e68_set_clk (c, 4);
	e68_cc_set_cmp_8 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B040: CMP.W <EA>, Dx */
static void opb040 (e68000_t *c)
{
	uint16_t s1, s2;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_dreg16 (c, e68_ir_reg9 (c));

	e68_set_clk (c, 4);
	e68_cc_set_cmp_16 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B080: CMP.L <EA>, Dx */
static void opb080 (e68000_t *c)
{
	uint32_t s1, s2;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_dreg32 (c, e68_ir_reg9 (c));

	e68_set_clk (c, 6);
	e68_cc_set_cmp_32 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B0C0: CMPA.W <EA>, Ax */
static void opb0c0 (e68000_t *c)
{
	uint32_t s1, s2;
	uint16_t t;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &t);
	s1 = e68_exts16 (t);
	s2 = e68_get_areg32 (c, e68_ir_reg9 (c));

	e68_set_clk (c, 8);
	e68_cc_set_cmp_32 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B100_08: CMPM.B (Ay)+, (Ax)+ */
static void opb100_08 (e68000_t *c)
{
	uint8_t s1, s2;

	e68_op_get_ea8 (c, 1, e68_ir_reg0 (c) | (3 << 3), 0x0008, &s1);
	e68_op_get_ea8 (c, 1, e68_ir_reg9 (c) | (3 << 3), 0x0008, &s2);

	e68_set_clk (c, 16);
	e68_cc_set_cmp_8 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B100: EOR.B Dx, <EA> */
static void opb100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		opb100_08 (c);
		return;
	}

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* B140_08: CMPM.W (Ay)+, (Ax)+ */
static void opb140_08 (e68000_t *c)
{
	uint16_t s1, s2;

	e68_op_get_ea16 (c, 1, e68_ir_reg0 (c) | (3 << 3), 0x0008, &s1);
	e68_op_get_ea16 (c, 1, e68_ir_reg9 (c) | (3 << 3), 0x0008, &s2);

	e68_set_clk (c, 12);
	e68_cc_set_cmp_16 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B140: EOR.W Dx, <EA> */
static void opb140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		opb140_08 (c);
		return;
	}

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* B180_08: CMPM.L (Ay)+, (Ax)+ */
static void opb180_08 (e68000_t *c)
{
	uint32_t s1, s2;

	e68_op_get_ea32 (c, 1, e68_ir_reg0 (c) | (3 << 3), 0x0008, &s1);
	e68_op_get_ea32 (c, 1, e68_ir_reg9 (c) | (3 << 3), 0x0008, &s2);

	e68_set_clk (c, 20);
	e68_cc_set_cmp_32 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* B180: EOR.L Dx, <EA> */
static void opb180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		opb180_08 (c);
		return;
	}

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_clk (c, 12);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* B1C0: CMPA.L <EA>, Ax */
static void opb1c0 (e68000_t *c)
{
	uint32_t s1, s2;

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_areg32 (c, e68_ir_reg9 (c));

	e68_set_clk (c, 8);
	e68_cc_set_cmp_32 (c, s2 - s1, s1, s2);
	e68_op_prefetch (c);
}

/* C000: AND.B <EA>, Dx */
static void opc000 (e68000_t *c)
{
	unsigned r;
	uint8_t  s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg8 (c, r);

	d = s1 & s2;

	e68_set_clk (c, 4);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* C040: AND.W <EA>, Dx */
static void opc040 (e68000_t *c)
{
	unsigned r;
	uint16_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg16 (c, r);

	d = s1 & s2;

	e68_set_clk (c, 4);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* C080: AND.L <EA>, Dx */
static void opc080 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg32 (c, r);

	d = s1 & s2;

	e68_set_clk (c, 6);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* C0C0: MULU.W <EA>, Dx */
static void opc0c0 (e68000_t *c)
{
	unsigned r;
	uint32_t d;
	uint16_t s1, s2;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg16 (c, r);

	d = (uint32_t) s1 * (uint32_t) s2;

	e68_set_clk (c, 74);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* C100_00: ABCD <EA>, <EA> */
static void opc100_00 (e68000_t *c)
{
	uint8_t  s1, s2;
	uint16_t d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x08) {
		/* -(Ax) */
		ea1 |= (4 << 3);
		ea2 |= (4 << 3);
	}

	e68_op_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea8 (c, 1, ea2, 0x0011, &s2);

	d = (uint16_t) s1 + (uint16_t) s2 + e68_get_sr_x (c);

	/* incorrect */
	if (((s1 & 0x0f) + (s2 & 0x0f)) > 9) {
		d += 0x06;
	}

	if (d >= 0xa0) {
		d += 0x60;
		c->sr |= (E68_SR_X | E68_SR_C);
	}
	else {
		c->sr &= (~E68_SR_X & ~E68_SR_C);
	}


	if (d & 0xff) {
		c->sr &= ~E68_SR_Z;
	}

	e68_set_clk (c, 6);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* C100: AND.B Dx, <EA> */
static void opc100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0x00) {
		opc100_00 (c);
		return;
	}

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 & s2;

	e68_set_clk (c, 4);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* C140_00: EXG Dx, Dx */
static void opc140_00 (e68000_t *c)
{
	unsigned r1, r2;
	uint32_t s1, s2;

	r1 = e68_ir_reg9 (c);
	r2 = e68_ir_reg0 (c);

	s1 = e68_get_dreg32 (c, r1);
	s2 = e68_get_dreg32 (c, r2);

	e68_set_dreg32 (c, r1, s2);
	e68_set_dreg32 (c, r2, s1);

	e68_set_clk (c, 10);
	e68_op_prefetch (c);
}

/* C140_08: EXG Ax, Ax */
static void opc140_08 (e68000_t *c)
{
	unsigned r1, r2;
	uint32_t s1, s2;

	r1 = e68_ir_reg9 (c);
	r2 = e68_ir_reg0 (c);

	s1 = e68_get_areg32 (c, r1);
	s2 = e68_get_areg32 (c, r2);

	e68_set_areg32 (c, r1, s2);
	e68_set_areg32 (c, r2, s1);

	e68_set_clk (c, 10);
	e68_op_prefetch (c);
}

/* C140: AND.W Dx, <EA> */
static void opc140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		opc140_00 (c);
		return;

	case 0x01:
		opc140_08 (c);
		return;
	}

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 & s2;

	e68_set_clk (c, 4);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* C180_08: EXG Dx, Ay */
static void opc180_08 (e68000_t *c)
{
	unsigned r1, r2;
	uint32_t s1, s2;

	r1 = e68_ir_reg9 (c);
	r2 = e68_ir_reg0 (c);

	s1 = e68_get_dreg32 (c, r1);
	s2 = e68_get_areg32 (c, r2);

	e68_set_dreg32 (c, r1, s2);
	e68_set_areg32 (c, r2, s1);

	e68_set_clk (c, 10);
	e68_op_prefetch (c);
}

/* C180: AND.L Dx, <EA> */
static void opc180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	if ((c->ir[0] & 0x38) == 0x08) {
		opc180_08 (c);
		return;
	}

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 & s2;

	e68_set_clk (c, 6);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* C1C0: MULS.W <EA>, Dx */
static void opc1c0 (e68000_t *c)
{
	unsigned r;
	int      n1, n2;
	uint32_t d;
	uint16_t s1, s2;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg16 (c, r);

	n1 = 0;
	n2 = 0;

	if (s1 & 0x8000) {
		n1 = 1;
		s1 = (~s1 + 1) & 0xffff;
	}

	if (s2 & 0x8000) {
		n2 = 1;
		s2 = (~s2 + 1) & 0xffff;
	}

	d = (uint32_t) s1 * (uint32_t) s2;

	if (n1 != n2) {
		d = (~d + 1) & 0xffffffff;
	}

	e68_set_clk (c, 74);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* D000: ADD.B <EA>, Dx */
static void opd000 (e68000_t *c)
{
	unsigned r;
	uint8_t  s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
	s2 = e68_get_dreg8 (c, r);

	d = s1 + s2;

	e68_set_clk (c, 4);
	e68_cc_set_add_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* D040: ADD.W <EA>, Dx */
static void opd040 (e68000_t *c)
{
	unsigned r;
	uint16_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_dreg16 (c, r);

	d = s1 + s2;

	e68_set_clk (c, 4);
	e68_cc_set_add_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* D080: ADD.L <EA>, Dx */
static void opd080 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_dreg32 (c, r);

	d = s1 + s2;

	e68_set_clk (c, 6);
	e68_cc_set_add_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* D0C0: ADDA.W <EA>, Ax */
static void opd0c0 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;
	uint16_t t;

	r = e68_ir_reg9 (c);

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &t);
	s1 = e68_exts16 (t);
	s2 = e68_get_areg32 (c, r);

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

/* D100_00: ADDX.B <EA>, <EA> */
static void opd100_00 (e68000_t *c)
{
	uint8_t  s1, s2, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		/* pre-decrement */
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_op_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea8 (c, 1, ea2, 0x0011, &s2);

	d = s1 + s2 + e68_get_sr_x (c);

	e68_set_clk (c, 8);
	e68_cc_set_addx_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* D100: ADD.B Dx, <EA> */
static void opd100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0) {
		opd100_00 (c);
		return;
	}

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_op_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_cc_set_add_8 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea8 (c, 0, 0, 0, d);
}

/* D140_00: ADDX.W <EA>, <EA> */
static void opd140_00 (e68000_t *c)
{
	uint16_t s1, s2, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		/* pre-decrement */
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_op_get_ea16 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea16 (c, 1, ea2, 0x0011, &s2);

	d = s1 + s2 + e68_get_sr_x (c);

	e68_set_clk (c, 8);
	e68_cc_set_addx_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* D140: ADD.W Dx, <EA> */
static void opd140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0) {
		opd140_00 (c);
		return;
	}

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 + s2;

	e68_set_clk (c, 8);
	e68_cc_set_add_16 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* D180_00: ADDX.L <EA>, <EA> */
static void opd180_00 (e68000_t *c)
{
	uint32_t s1, s2, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		/* pre-decrement */
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_op_get_ea32 (c, 1, ea1, 0x0011, &s1);
	e68_op_get_ea32 (c, 1, ea2, 0x0011, &s2);

	d = s1 + s2 + e68_get_sr_x (c);

	e68_set_clk (c, 12);
	e68_cc_set_addx_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* D180: ADD.L Dx, <EA> */
static void opd180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	if ((c->ir[0] & 0x30) == 0) {
		opd180_00 (c);
		return;
	}

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 + s2;

	e68_set_clk (c, 12);
	e68_cc_set_add_32 (c, d, s1, s2);
	e68_op_prefetch (c);
	e68_op_set_ea32 (c, 0, 0, 0, d);
}

/* D1C0: ADDA.L <EA>, Ax */
static void opd1c0 (e68000_t *c)
{
	unsigned r;
	uint32_t s1, s2, d;

	r = e68_ir_reg9 (c);

	e68_op_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s1);
	s2 = e68_get_areg32 (c, r);

	d = s1 + s2;

	e68_set_clk (c, 6);
	e68_op_prefetch (c);
	e68_set_areg32 (c, r, d);
}

static inline
unsigned e68_get_shift_count (e68000_t *c)
{
	unsigned n;

	n = e68_ir_reg9 (c);

	if (c->ir[0] & 0x20) {
		return (e68_get_dreg8 (c, n) & 0x3f);
	}

	if (n == 0) {
		n = 8;
	}

	return (n);
}

/* E000_00: ASR.B <cnt>, Dx */
static void ope000_00 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	n = e68_get_shift_count (c);
	s = e68_get_dreg8 (c, r);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 8) {
		if (s & 0x80) {
			d = (s | 0xff00) >> n;
		}
		else {
			d = s >> n;
		}
		e68_set_sr_xc (c, s & (1 << (n - 1)));
	}
	else {
		if (s & 0x80) {
			d = 0xff;
			e68_set_sr_xc (c, 1);
		}
		else {
			d = 0x00;
			e68_set_sr_xc (c, 0);
		}
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E000_01: LSR.B <cnt>, Dx */
static void ope000_01 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg8 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 8) {
		d = s >> n;
		e68_set_sr_xc (c, s & (1 << (n - 1)));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 8) && (s & 0x80));
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E000_02: ROXR.B <cnt>, Dx */
static void ope000_02 (e68000_t *c)
{
	uint16_t v, x, t;
	unsigned r, i, n;

	r = e68_ir_reg0 (c);

	v = e68_get_dreg8 (c, r);
	x = e68_get_sr_x (c);
	n = e68_get_shift_count (c);

	for (i = 0; i < n; i++) {
		t = v & 1;
		v = (v >> 1) | (x << 7);
		x = t;
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, v & 0xff);
	e68_set_sr_xc (c, x);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, v & 0xff);
}

/* E000_03: ROR.B <cnt>, Dx */
static void ope000_03 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n, k;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg8 (c, r);
	n = e68_get_shift_count (c);

	k = n & 7;

	if (k > 0) {
		d = ((s >> k) | (s << (8 - k))) & 0xff;
		e68_set_sr_c (c, d & 0x80);
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (d & 0x80));
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E000: misc */
static void ope000 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		ope000_00 (c);
		return;

	case 0x01:
		ope000_01 (c);
		return;

	case 0x02:
		ope000_02 (c);
		return;

	case 0x03:
		ope000_03 (c);
		return;
	}

	e68_op_undefined (c);
}

/* E040_00: ASR.W <cnt>, Dx */
static void ope040_00 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 16) {
		if (s & 0x8000) {
			d = (s | 0xffff0000) >> n;
		}
		else {
			d = s >> n;
		}
		e68_set_sr_xc (c, s & (1U << (n - 1)));
	}
	else {
		if (s & 0x8000) {
			d = 0xffff;
			e68_set_sr_xc (c, 1);
		}
		else {
			d = 0x0000;
			e68_set_sr_xc (c, 0);
		}
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E040_01: LSR.W <cnt>, Dx */
static void ope040_01 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 16) {
		d = s >> n;
		e68_set_sr_xc (c, s & (1U << (n - 1)));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 16) && (s & 0x8000));
	}


	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E040_02: ROXR.W <cnt>, Dx */
static void ope040_02 (e68000_t *c)
{
	uint16_t d, x, t;
	unsigned r, i, n;

	r = e68_ir_reg0 (c);

	d = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);
	x = e68_get_sr_x (c);

	for (i = 0; i < n; i++) {
		t = d & 1;
		d = (d >> 1) | (x << 15);
		x = t;
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, x);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E040_03: ROR.W <cnt>, Dx */
static void ope040_03 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n, k;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);

	k = n & 15;

	if (k > 0) {
		d = ((s >> k) | (s << (16 - k))) & 0xffff;
		e68_set_sr_c (c, d & 0x8000);
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (d & 0x8000));
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E040: misc */
static void ope040 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		ope040_00 (c);
		return;

	case 0x01:
		ope040_01 (c);
		return;

	case 0x02:
		ope040_02 (c);
		return;

	case 0x03:
		ope040_03 (c);
		return;
	}

	e68_op_undefined (c);
}

/* E080_00: ASR.L <cnt>, Dx */
static void ope080_00 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 32) {
		if (s & 0x80000000) {
			d = (s >> n) | (0xffffffff << (32 - n));
		}
		else {
			d = s >> n;
		}
		e68_set_sr_xc (c, s & (1UL << (n - 1)));
	}
	else {
		if (s & 0x80000000) {
			d = 0xffffffff;
			e68_set_sr_xc (c, 1);
		}
		else {
			d = 0x00000000;
			e68_set_sr_xc (c, 0);
		}
	}


	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E080_01: LSR.L <cnt>, Dx */
static void ope080_01 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 32) {
		d = s >> n;
		e68_set_sr_xc (c, s & (1UL << (n - 1)));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 32) && (s & 0x80000000));
	}


	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E080_02: ROXR.L <cnt>, Dx */
static void ope080_02 (e68000_t *c)
{
	uint32_t d, x, t;
	unsigned r, i, n;

	r = e68_ir_reg0 (c);

	d = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);
	x = e68_get_sr_x (c);

	for (i = 0; i < n; i++) {
		t = d & 1;
		d = (d >> 1) | (x << 31);
		x = t;
	}

	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, x);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E080_03: ROR.L <cnt>, Dx */
static void ope080_03 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n, k;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);

	k = n & 31;

	if (k > 0) {
		d = ((s >> k) | (s << (32 - k))) & 0xffffffff;
		e68_set_sr_c (c, d & 0x80000000);
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (d & 0x80000000));
	}

	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E080: misc */
static void ope080 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		ope080_00 (c);
		return;

	case 0x01:
		ope080_01 (c);
		return;

	case 0x02:
		ope080_02 (c);
		return;

	case 0x03:
		ope080_03 (c);
		return;
	}

	e68_op_undefined (c);
}

/* E0C0: ASR.W <EA> */
static void ope0c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = (s >> 1) | (s & 0x8000);

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_XC, s & 1);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E100_00: ASL.B <cnt>, Dx */
static void ope100_00 (e68000_t *c)
{
	uint8_t  s, d, b;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg8 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C | E68_SR_V, 0);
	}
	else if (n < 8) {
		d = s << n;
		b = (0xff << (7 - n)) & 0xff;
		e68_set_sr_xc (c, s & (1 << (8 - n)));
		e68_set_sr_v (c, ((s & b) != 0) && ((s & b) != b));
	}
	else {
		d = 0x00;
		e68_set_sr_xc (c, (n == 8) && (s & 1));
		e68_set_sr_v (c, s != 0);
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E100_01: LSL.B <cnt>, Dx */
static void ope100_01 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg8 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 8) {
		d = s << n;
		e68_set_sr_xc (c, s & (1 << (8 - n)));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 8) && (s & 1));
	}


	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E100_02: ROXL.B <cnt>, Dx */
static void ope100_02 (e68000_t *c)
{
	uint16_t d, x, t;
	unsigned r, i, n;

	r = e68_ir_reg0 (c);

	d = e68_get_dreg8 (c, r);
	n = e68_get_shift_count (c);
	x = e68_get_sr_x (c);

	for (i = 0; i < n; i++) {
		t = (d >> 7) & 1;
		d = (d << 1) | x;
		x = t;
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, x);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E100_03: ROL.B <cnt>, Dx */
static void ope100_03 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n, k;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg8 (c, r);
	n = e68_get_shift_count (c);

	k = n & 7;

	if (k != 0) {
		d = ((s << k) | (s >> (8 - k))) & 0xff;
		e68_set_sr_c (c, d & 1);
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (d & 1));
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg8 (c, r, d);
}

/* E100: misc */
static void ope100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		ope100_00 (c);
		return;

	case 0x01:
		ope100_01 (c);
		return;

	case 0x02:
		ope100_02 (c);
		return;

	case 0x03:
		ope100_03 (c);
		return;
	}

	e68_op_undefined (c);
}

/* E140_00: ASL.W <cnt>, Dx */
static void ope140_00 (e68000_t *c)
{
	uint16_t s, d, b;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C | E68_SR_V, 0);
	}
	else if (n < 16) {
		d = s << n;
		b = (0xffff << (15 - n)) & 0xffff;
		e68_set_sr_xc (c, s & (1U << (16 - n)));
		e68_set_sr_v (c, ((s & b) != 0) && ((s & b) != b));
	}
	else {
		d = 0x0000;
		e68_set_sr_xc (c, (n == 16) && (s & 1));
		e68_set_sr_v (c, s != 0);
	}


	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E140_01: LSL.W <cnt>, Dx */
static void ope140_01 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 16) {
		d = s << n;
		e68_set_sr_xc (c, s & (1U << (16 - n)));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 16) && (s & 1));
	}


	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E140_02: ROXL.W <cnt>, Dx */
static void ope140_02 (e68000_t *c)
{
	uint16_t d, x, t;
	unsigned r, i, n;

	r = e68_ir_reg0 (c);

	d = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);
	x = e68_get_sr_x (c);

	for (i = 0; i < n; i++) {
		t = (d >> 15) & 1;
		d = (d << 1) | x;
		x = t;
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, x);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E140_03: ROL.W <cnt>, Dx */
static void ope140_03 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n, k;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg16 (c, r);
	n = e68_get_shift_count (c);

	k = n & 15;

	if (k != 0) {
		d = ((s << k) | (s >> (16 - k))) & 0xffff;
		e68_set_sr_c (c, d & 1);
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (d & 1));
	}

	e68_set_clk (c, 6 + 2 * n);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg16 (c, r, d);
}

/* E140: misc */
static void ope140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		ope140_00 (c);
		return;

	case 0x01:
		ope140_01 (c);
		return;

	case 0x02:
		ope140_02 (c);
		return;

	case 0x03:
		ope140_03 (c);
		return;
	}

	e68_op_undefined (c);
}

/* E180_00: ASL.L <cnt>, Dx */
static void ope180_00 (e68000_t *c)
{
	uint32_t s, d, b;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C | E68_SR_V, 0);
	}
	else if (n < 32) {
		d = s << n;
		b = (0xffffffff << (31 - n)) & 0xffffffff;
		e68_set_sr_xc (c, s & (1UL << (32 - n)));
		e68_set_sr_v (c, ((s & b) != 0) && ((s & b) != b));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 32) && (s & 1));
		e68_set_sr_v (c, s != 0);
	}

	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E180_01: LSL.L <cnt>, Dx */
static void ope180_01 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 32) {
		d = s << n;
		e68_set_sr_xc (c, s & (1UL << (32 - n)));
	}
	else {
		d = 0;
		e68_set_sr_xc (c, (n == 32) && (s & 1));
	}


	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E180_02: ROXL.L <cnt>, Dx */
static void ope180_02 (e68000_t *c)
{
	uint32_t d, x, t;
	unsigned r, i, n;

	r = e68_ir_reg0 (c);

	n = e68_get_shift_count (c);
	d = e68_get_dreg32 (c, r);
	x = e68_get_sr_x (c);

	for (i = 0; i < n; i++) {
		t = (d >> 31) & 1;
		d = (d << 1) | x;
		x = t;
	}

	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, x);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E180_03: ROL.L <cnt>, Dx */
static void ope180_03 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n, k;

	r = e68_ir_reg0 (c);

	s = e68_get_dreg32 (c, r);
	n = e68_get_shift_count (c);

	k = n & 31;

	if (k > 0) {
		d = ((s << k) | (s >> (32 - k))) & 0xffffffff;
		e68_set_sr_c (c, d & 1);
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (d & 1));
	}

	e68_set_clk (c, 8 + 2 * n);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_op_prefetch (c);
	e68_set_dreg32 (c, r, d);
}

/* E180: misc */
static void ope180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		ope180_00 (c);
		return;

	case 0x01:
		ope180_01 (c);
		return;

	case 0x02:
		ope180_02 (c);
		return;

	case 0x03:
		ope180_03 (c);
		return;
	}

	e68_op_undefined (c);
}

/* E1C0: ASL.W <EA> */
static void ope1c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = (s << 1) & 0xffff;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z, d);
	e68_set_sr_xc (c, s & 0x8000);
	e68_set_sr_v (c, (s ^ d) & 0x8000);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E2C0: LSR.W <EA> */
static void ope2c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = s >> 1;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, s & 1);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E3C0: LSL.W <EA> */
static void ope3c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = (s << 1) & 0xffff;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, s & 0x8000);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E4C0: ROXR.W <EA> */
static void ope4c0 (e68000_t *c)
{
	uint16_t s, d, x;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);
	x = e68_get_sr_x (c);

	d = (s >> 1) | (x << 15);

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, s & 1);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E5C0: ROXL.W <EA> */
static void ope5c0 (e68000_t *c)
{
	uint16_t s, d, x;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);
	x = e68_get_sr_x (c);

	d = ((s << 1) | x) & 0xffff;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_xc (c, s & 0x8000);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E6C0: ROR.W <EA> */
static void ope6c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = ((s >> 1) | (s << 15)) & 0xffff;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_c (c, s & 1);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* E7C0: ROL.W <EA> */
static void ope7c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_op_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = ((s << 1) | (s >> 15)) & 0xffff;

	e68_set_clk (c, 8);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_sr_c (c, d & 1);
	e68_op_prefetch (c);
	e68_op_set_ea16 (c, 0, 0, 0, d);
}

/* F000: FXXX */
static void opf000 (e68000_t *c)
{
	c->last_trap_f = c->ir[0];

	e68_exception_fxxx (c);
}

static
e68_opcode_f e68_opcodes[1024] = {
	op0000, op0040, op0080,   NULL, op0100, op0140, op0180, op01c0, /* 0000 */
	op0200, op0240, op0280,   NULL, op0100, op0140, op0180, op01c0, /* 0200 */
	op0400, op0440, op0480,   NULL, op0100, op0140, op0180, op01c0, /* 0400 */
	op0600, op0640, op0680,   NULL, op0100, op0140, op0180, op01c0, /* 0600 */
	op0800, op0840, op0880, op08c0, op0100, op0140, op0180, op01c0, /* 0800 */
	op0a00, op0a40, op0a80,   NULL, op0100, op0140, op0180, op01c0, /* 0A00 */
	op0c00, op0c40, op0c80,   NULL, op0100, op0140, op0180, op01c0, /* 0C00 */
	op0e00, op0e40, op0e80,   NULL, op0100, op0140, op0180, op01c0, /* 0E00 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1000 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1200 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1400 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1600 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1800 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1A00 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1C00 */
	op1000,   NULL, op1000, op1000, op1000, op1000, op1000, op1000, /* 1E00 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2000 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2200 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2400 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2600 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2800 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2A00 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2C00 */
	op2000, op2040, op2000, op2000, op2000, op2000, op2000, op2000, /* 2E00 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3000 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3200 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3400 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3600 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3800 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3A00 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3C00 */
	op3000, op3040, op3000, op3000, op3000, op3000, op3000, op3000, /* 3E00 */
	op4000, op4040, op4080, op40c0,   NULL,   NULL, op4180, op41c0, /* 4000 */
	op4200, op4240, op4280, op42c0,   NULL,   NULL, op4180, op41c0, /* 4200 */
	op4400, op4440, op4480, op44c0,   NULL,   NULL, op4180, op41c0, /* 4400 */
	op4600, op4640, op4680, op46c0,   NULL,   NULL, op4180, op41c0, /* 4600 */
	op4800, op4840, op4880, op48c0,   NULL,   NULL, op4180, op49c0, /* 4800 */
	op4a00, op4a40, op4a80, op4ac0,   NULL,   NULL, op4180, op41c0, /* 4A00 */
	  NULL,   NULL, op4c80, op4cc0,   NULL,   NULL, op4180, op41c0, /* 4C00 */
	  NULL, op4e40, op4e80, op4ec0,   NULL,   NULL, op4180, op41c0, /* 4E00 */
	op5000, op5040, op5080, op50c0, op5100, op5140, op5180, op51c0, /* 5000 */
	op5000, op5040, op5080, op52c0, op5100, op5140, op5180, op53c0, /* 5200 */
	op5000, op5040, op5080, op54c0, op5100, op5140, op5180, op55c0, /* 5400 */
	op5000, op5040, op5080, op56c0, op5100, op5140, op5180, op57c0, /* 5600 */
	op5000, op5040, op5080, op58c0, op5100, op5140, op5180, op59c0, /* 5800 */
	op5000, op5040, op5080, op5ac0, op5100, op5140, op5180, op5bc0, /* 5A00 */
	op5000, op5040, op5080, op5cc0, op5100, op5140, op5180, op5dc0, /* 5C00 */
	op5000, op5040, op5080, op5ec0, op5100, op5140, op5180, op5fc0, /* 5E00 */
	op6000, op6000, op6000, op6000, op6100, op6100, op6100, op6100, /* 6000 */
	op6200, op6200, op6200, op6200, op6300, op6300, op6300, op6300, /* 6200 */
	op6400, op6400, op6400, op6400, op6500, op6500, op6500, op6500, /* 6400 */
	op6600, op6600, op6600, op6600, op6700, op6700, op6700, op6700, /* 6600 */
	op6800, op6800, op6800, op6800, op6900, op6900, op6900, op6900, /* 6800 */
	op6a00, op6a00, op6a00, op6a00, op6b00, op6b00, op6b00, op6b00, /* 6A00 */
	op6c00, op6c00, op6c00, op6c00, op6d00, op6d00, op6d00, op6d00, /* 6C00 */
	op6e00, op6e00, op6e00, op6e00, op6f00, op6f00, op6f00, op6f00, /* 6E00 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7000 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7200 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7400 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7600 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7800 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7A00 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7C00 */
	op7000, op7000, op7000, op7000,   NULL,   NULL,   NULL,   NULL, /* 7E00 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8000 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8200 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8400 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8600 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8800 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8A00 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8C00 */
	op8000, op8040, op8080, op80c0, op8100, op8140, op8180, op81c0, /* 8E00 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9000 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9200 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9400 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9600 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9800 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9A00 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9C00 */
	op9000, op9040, op9080, op90c0, op9100, op9140, op9180, op91c0, /* 9E00 */
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000, /* A000 */
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opa000, opa000, opa000, opa000, opa000, opa000, opa000, opa000,
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* B000 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* B200 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* B400 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* B600 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* B800 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* BA00 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* BC00 */
	opb000, opb040, opb080, opb0c0, opb100, opb140, opb180, opb1c0, /* BE00 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* C000 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* C200 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* C400 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* C600 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* C800 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* CA00 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* CC00 */
	opc000, opc040, opc080, opc0c0, opc100, opc140, opc180, opc1c0, /* CE00 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* D000 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* D200 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* D400 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* D600 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* D800 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* DA00 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* DC00 */
	opd000, opd040, opd080, opd0c0, opd100, opd140, opd180, opd1c0, /* DE00 */
	ope000, ope040, ope080, ope0c0, ope100, ope140, ope180, ope1c0, /* E000 */
	ope000, ope040, ope080, ope2c0, ope100, ope140, ope180, ope3c0, /* E200 */
	ope000, ope040, ope080, ope4c0, ope100, ope140, ope180, ope5c0, /* E400 */
	ope000, ope040, ope080, ope6c0, ope100, ope140, ope180, ope7c0, /* E600 */
	ope000, ope040, ope080,   NULL, ope100, ope140, ope180,   NULL, /* E800 */
	ope000, ope040, ope080,   NULL, ope100, ope140, ope180,   NULL, /* EA00 */
	ope000, ope040, ope080,   NULL, ope100, ope140, ope180,   NULL, /* EC00 */
	ope000, ope040, ope080,   NULL, ope100, ope140, ope180,   NULL, /* EE00 */
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000, /* F000 */
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000,
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000,
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000,
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000,
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000,
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000,
	opf000, opf000, opf000, opf000, opf000, opf000, opf000, opf000
};

static e68_opcode_f e68_op_49c0[8] = {
	  NULL, op41c0, op41c0, op41c0, op41c0, op41c0, op41c0, op41c0
};

void e68_set_opcodes (e68000_t *c)
{
	unsigned i;

	for (i = 0; i < 1024; i++) {
		if (e68_opcodes[i] != NULL) {
			c->opcodes[i] = e68_opcodes[i];
		}
		else {
			c->opcodes[i] = e68_op_undefined;
		}
	}

	for (i = 0; i < 8; i++) {
		c->op49c0[i] = (e68_op_49c0[i] == NULL) ? e68_op_undefined : e68_op_49c0[i];
	}
}
