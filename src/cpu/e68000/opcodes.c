/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e68000/opcodes.c                                   *
 * Created:       2005-07-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2005-2007 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


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



static unsigned e68_op_undefined (e68000_t *c)
{
	e68_undefined (c);
	e68_set_clk (c, 2);
	return (0);
}

/* 0000_3C: ORI.B #XX, CCR */
static unsigned op0000_3c (e68000_t *c)
{
	e68_ifetch_next (c);

	e68_set_ccr (c, (e68_get_ccr (c) | c->ir[1]) & E68_SR_XNZVC);
	e68_set_clk (c, 32);

	return (c->ircnt);
}

/* 0000_XX: ORI.B #XX, <EA> */
static unsigned op0000_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1] & 0xff;

	d = s1 | s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0000: misc */
static unsigned op0000 (e68000_t *c)
{
	switch (c->ir[0] & 0x3f) {
	case 0x3c:
		return (op0000_3c (c));

	default:
		return (op0000_xx (c));
	}
}

/* 0040_3C: ORI.W #XXXX, SR */
static unsigned op0040_3c (e68000_t *c)
{
	e68_ifetch_next (c);

	e68_set_sr (c, (e68_get_sr (c) | c->ir[1]) & E68_SR_MASK);
	e68_set_clk (c, 32);

	return (c->ircnt);
}

/* 0040_XX: ORI.W #XXXX, <EA> */
static unsigned op0040_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1];

	d = s1 | s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0040: misc */
static unsigned op0040 (e68000_t *c)
{
	switch (c->ir[0] & 0x3f) {
	case 0x3c:
		return (op0040_3c (c));

	default:
		return (op0040_xx (c));
	}
}

/* 0080: ORI.L #XXXXXXXX, <EA> */
static unsigned op0080 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_ifetch_next (c);
	e68_ifetch_next (c);

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = ((unsigned long) c->ir[1] << 16) | c->ir[2];

	d = s1 | s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0100_00: BTST Dn, Dn */
static unsigned op0100_00 (e68000_t *c)
{
	unsigned i;
	uint32_t s, m;

	s = e68_get_dreg32 (c, e68_ir_reg0 (c));
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;

	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0100_XX: BTST Dn, <EA> */
static unsigned op0100_xx (e68000_t *c)
{
	unsigned i;
	uint8_t  s, m;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;

	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0100: misc */
static unsigned op0100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op0100_00 (c));

	default:
		return (op0100_xx (c));
	}
}

/* 0140_00: BCHG Dn, Dn */
static unsigned op0140_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;
	d = s ^ m;

	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0140_XX: BCHG Dn, <EA> */
static unsigned op0140_xx (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;
	d = s ^ m;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0140: misc */
static unsigned op0140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op0140_00 (c));

	default:
		return (op0140_xx (c));
	}
}

/* 0180_00: BCLR Dn, Dn */
static unsigned op0180_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;
	d = s & ~m;

	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0180_XX: BCLR Dn, <EA> */
static unsigned op0180_xx (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;
	d = s & ~m;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0180: misc */
static unsigned op0180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op0180_00 (c));

	default:
		return (op0180_xx (c));
	}
}

/* 01C0_00: BSET Dn, Dn */
static unsigned op01c0_00 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x1f;

	m = 1UL << i;
	d = s | m;

	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 01C0_XX: BSET Dn, <EA> */
static unsigned op01c0_xx (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = e68_get_dreg32 (c, e68_ir_reg9 (c)) & 0x07;

	m = 1 << i;
	d = s | m;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 01C0: misc */
static unsigned op01c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op01c0_00 (c));

	default:
		return (op01c0_xx (c));
	}
}

/* 0200_3C: ANDI.B #XX, CCR */
static unsigned op0200_3c (e68000_t *c)
{
	e68_ifetch_next (c);

	e68_set_ccr (c, e68_get_ccr (c) & c->ir[1] & E68_SR_XNZVC);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* 0200_XX: ANDI.B #XX, <EA> */
static unsigned op0200_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1] & 0xff;

	d = s1 & s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0200: misc */
static unsigned op0200 (e68000_t *c)
{
	switch (c->ir[0] & 0x3f) {
	case 0x3c:
		return (op0200_3c (c));

	default:
		return (op0200_xx (c));
	}
}

/* 0240_3C: ANDI.W #XXXX, SR */
static unsigned op0240_3c (e68000_t *c)
{
	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	e68_ifetch_next (c);

	e68_set_sr (c, e68_get_sr (c) & c->ir[1]);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* 0240_XX: ANDI.W #XXXX, EA */
static unsigned op0240_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1];

	d = s1 & s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0240: misc */
static unsigned op0240 (e68000_t *c)
{
	switch (c->ir[0] & 0x3f) {
	case 0x3c:
		return (op0240_3c (c));

	default:
		return (op0240_xx (c));
	}
}

/* 0280: ANDI.L #XXXXXXXX, EA */
static unsigned op0280 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_ifetch_next (c);
	e68_ifetch_next (c);

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = ((unsigned long) c->ir[1] << 16) | c->ir[2];

	d = s1 & s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0400: SUBI.B #XX, <EA> */
static unsigned op0400 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1] & 0xff;

	d = s1 - s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_sub_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0440: SUBI.W #XXXX, <EA> */
static unsigned op0440 (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1];

	d = s1 - s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_sub_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0480: SUBI.L #XXXXXXXX, <EA> */
static unsigned op0480 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_ifetch_next (c);
	e68_ifetch_next (c);

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = ((unsigned long) c->ir[1] << 16) | c->ir[2];

	d = s1 - s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_sub_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0600: ADDI.B #XX, <EA> */
static unsigned op0600 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1] & 0xff;

	d = s1 + s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_add_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0640: ADDI.W #XXXX, <EA> */
static unsigned op0640 (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1];

	d = s1 + s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_add_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0680: ADDI.L #XXXXXXXX, <EA> */
static unsigned op0680 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_ifetch_next (c);
	e68_ifetch_next (c);

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = ((unsigned long) c->ir[1] << 16) | c->ir[2];

	d = s1 + s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_add_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0800_00: BTST #XX, Dn */
static unsigned op0800_00 (e68000_t *c)
{
	unsigned i;
	uint32_t s, m;

	e68_ifetch_next (c);
	s = e68_get_dreg32 (c, e68_ir_reg0 (c));
	i = c->ir[1] & 0x1f;

	m = 1UL << i;

	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 0800_XX: BTST #XX, <EA> */
static unsigned op0800_xx (e68000_t *c)
{
	unsigned i;
	uint8_t  s, m;

	e68_ifetch_next (c);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = c->ir[1] & 0x07;

	m = 1 << i;

	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 0800: misc */
static unsigned op0800 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op0800_00 (c));

	default:
		return (op0800_xx (c));
	}
}

/* 0840_0: BCHG #XX, Dn */
static unsigned op0840_0 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	e68_ifetch_next (c);
	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = c->ir[1] & 0x1f;

	m = 1UL << i;
	d = s ^ m;

	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 0840_1: BCHG #XX, <EA> */
static unsigned op0840_1 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	e68_ifetch_next (c);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = c->ir[1] & 0x07;

	m = 1 << i;
	d = s ^ m;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 0840: misc */
static unsigned op0840 (e68000_t *c)
{
	if ((c->ir[0] & 0x0038) == 0x0000) {
		return (op0840_0 (c));
	}

	return (op0840_1 (c));
}

/* 0880_0: BCLR #XX, Dn */
static unsigned op0880_0 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	e68_ifetch_next (c);
	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = c->ir[1] & 0x1f;

	m = 1UL << i;
	d = s & ~m;

	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 0880_1: BCLR #XX, <EA> */
static unsigned op0880_1 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	e68_ifetch_next (c);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = c->ir[1] & 0x07;

	m = 1 << i;
	d = s & ~m;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 0880: misc */
static unsigned op0880 (e68000_t *c)
{
	if ((c->ir[0] & 0x0038) == 0x0000) {
		return (op0880_0 (c));
	}

	return (op0880_1 (c));
}

/* 08C0_0: BSET #XX, Dn */
static unsigned op08c0_0 (e68000_t *c)
{
	unsigned i, r;
	uint32_t s, d, m;

	e68_ifetch_next (c);
	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);
	i = c->ir[1] & 0x1f;

	m = 1UL << i;
	d = s | m;

	e68_set_dreg32 (c, r, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 08C0_1: BSET #XX, <EA> */
static unsigned op08c0_1 (e68000_t *c)
{
	unsigned i;
	uint8_t  s, d, m;

	e68_ifetch_next (c);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x1fc, &s);
	i = c->ir[1] & 0x07;

	m = 1 << i;
	d = s | m;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_z (c, (s & m) == 0);
	e68_set_clk (c, 20);

	return (c->ircnt);
}

/* 08C0: misc */
static unsigned op08c0 (e68000_t *c)
{
	if ((c->ir[0] & 0x0038) == 0x0000) {
		return (op08c0_0 (c));
	}

	return (op08c0_1 (c));
}

/* 0A00_XX: EORI.B #XX, <EA> */
static unsigned op0a00_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1] & 0xff;

	d = s1 ^ s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0A00_3C: EORI.B #XX, CCR */
static unsigned op0a00_3c (e68000_t *c)
{
	e68_ifetch_next (c);

	e68_set_ccr (c, (e68_get_ccr (c) ^ c->ir[1]) & E68_SR_XNZVC);
	e68_set_clk (c, 32);

	return (c->ircnt);
}

/* 0A00: misc */
static unsigned op0a00 (e68000_t *c)
{
	if ((c->ir[0] & 0x3f) == 0x3c) {
		return (op0a00_3c (c));
	}

	return (op0a00_xx (c));
}

/* 0A40_3C: EORI.W #XXXX, SR */
static unsigned op0a40_3c (e68000_t *c)
{
	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	e68_ifetch_next (c);

	e68_set_sr (c, (e68_get_sr (c) ^ c->ir[1]) & E68_SR_MASK);
	e68_set_clk (c, 32);

	return (c->ircnt);
}

/* 0A40_XX: EORI.W #XXXX, <EA> */
static unsigned op0a40_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_ifetch_next (c);

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = c->ir[1] & 0xffff;

	d = s1 ^ s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0A40: misc */
static unsigned op0a40 (e68000_t *c)
{
	if ((c->ir[0] & 0x3f) == 0x3c) {
		return (op0a40_3c (c));
	}

	return (op0a40_xx (c));
}

/* 0A80: EORI.L #XXXXXXXX, <EA> */
static unsigned op0a80 (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_ifetch_next (c);
	e68_ifetch_next (c);

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = ((unsigned long) c->ir[1] << 16) | c->ir[2];

	d = s1 ^ s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 0C00: CMPI.B #XX, <EA> */
static unsigned op0c00 (e68000_t *c)
{
	uint8_t s1, s2;

	e68_ifetch_next (c);

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x07fd, &s1);
	s2 = c->ir[1] & 0xff;

	e68_cc_set_cmp_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0C40: CMPI.W #XXXX, <EA> */
static unsigned op0c40 (e68000_t *c)
{
	uint16_t s1, s2;

	e68_ifetch_next (c);

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x07fd, &s1);
	s2 = c->ir[1];

	e68_cc_set_cmp_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 0C80: CMPI.L #XXXXXXXX, <EA> */
static unsigned op0c80 (e68000_t *c)
{
	uint32_t s1, s2;

	e68_ifetch_next (c);
	e68_ifetch_next (c);

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x07fd, &s1);
	s2 = ((unsigned long) c->ir[1] << 16) | c->ir[2];

	e68_cc_set_cmp_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 1000: MOVE.B <EA>, <EA> */
static unsigned op1000 (e68000_t *c)
{
	uint8_t val;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_ea8 (c, 1, e68_ir_ea2 (c), 0x01fd, val);

	e68_cc_set_nz_8 (c, E68_SR_NZVC, val);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* 2000: MOVE.L <EA>, <EA> */
static unsigned op2000 (e68000_t *c)
{
	uint32_t val;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_ea32 (c, 1, e68_ir_ea2 (c), 0x01fd, val);

	e68_cc_set_nz_32 (c, E68_SR_NZVC, val);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* 2040: MOVEA.L <EA>, Ax */
static unsigned op2040 (e68000_t *c)
{
	uint32_t val;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_areg32 (c, (c->ir[0] >> 9) & 7, val);

	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* 3000: MOVE.W <EA>, <EA> */
static unsigned op3000 (e68000_t *c)
{
	uint16_t val;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_ea16 (c, 1, e68_ir_ea2 (c), 0x01fd, val);

	e68_cc_set_nz_16 (c, E68_SR_NZVC, val);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* 3040: MOVEA.W <EA>, Ax */
static unsigned op3040 (e68000_t *c)
{
	uint16_t val;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &val);
	e68_set_areg32 (c, e68_ir_reg9 (c), e68_exts (val, 16));

	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* 4000: NEGX.B <EA> */
static unsigned op4000 (e68000_t *c)
{
	uint8_t s, d, x;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	x = e68_get_sr_x (c);

	d = (0U - s - x) & 0xff;

	if (d != 0) {
		e68_set_sr_z (c, 0);
	}

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_v (c, d & s & 0x80);
	e68_set_cc (c, E68_SR_XC, s || x);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4040: NEGX.W <EA> */
static unsigned op4040 (e68000_t *c)
{
	uint16_t s, d, x;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	x = e68_get_sr_x (c);

	d = (0U - s - x) & 0xffff;

	if (d != 0) {
		e68_set_sr_z (c, 0);
	}

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_set_sr_v (c, d & s & 0x8000);
	e68_set_cc (c, E68_SR_XC, s || x);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4080: NEGX.L <EA> */
static unsigned op4080 (e68000_t *c)
{
	uint32_t s, d, x;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);
	x = e68_get_sr_x (c);

	d = (0U - s - x) & 0xffffffff;

	if (d != 0) {
		e68_set_sr_z (c, 0);
	}

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_set_sr_v (c, d & s & 0x80000000);
	e68_set_cc (c, E68_SR_XC, s || x);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 40C0: MOVE.W SR, <EA> */
static unsigned op40c0 (e68000_t *c)
{
	uint16_t s;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	s = e68_get_sr (c) & E68_SR_MASK;

	e68_set_ea16 (c, 0, 0, 0, s);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 4180: CHK <EA>, Dx */
static unsigned op4180 (e68000_t *c)
{
	int      trap;
	uint16_t s1, s2;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s1);
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

	if (trap) {
		e68_exception_check (c);
		return (0);
	}

	e68_set_clk (c, 14);

	return (c->ircnt);
}

/* 41C0: LEA <EA>, Ax */
static unsigned op41c0 (e68000_t *c)
{
	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x7e4, 32)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	e68_set_areg32 (c, e68_ir_reg9 (c), c->ea_val);

	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* 4200: CLR.B <EA> */
static unsigned op4200 (e68000_t *c)
{
	e68_set_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, 0);

	e68_set_cc (c, E68_SR_N | E68_SR_V | E68_SR_C, 0);
	e68_set_cc (c, E68_SR_Z, 1);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4240: CLR.W <EA> */
static unsigned op4240 (e68000_t *c)
{
	e68_set_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, 0);

	e68_set_cc (c, E68_SR_N | E68_SR_V | E68_SR_C, 0);
	e68_set_cc (c, E68_SR_Z, 1);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4280: CLR.L <EA> */
static unsigned op4280 (e68000_t *c)
{
	e68_set_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, 0);

	e68_set_cc (c, E68_SR_N | E68_SR_V | E68_SR_C, 0);
	e68_set_cc (c, E68_SR_Z, 1);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 4400: NEG.B <EA> */
static unsigned op4400 (e68000_t *c)
{
	uint8_t s, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1) & 0xff;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_sr_v (c, d & s & 0x80);
	e68_set_sr_z (c, d == 0);
	e68_set_cc (c, E68_SR_XC, d != 0);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4440: NEG.W <EA> */
static unsigned op4440 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1) & 0xffff;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_set_sr_v (c, s == 0x8000);
	e68_set_sr_z (c, d == 0);
	e68_set_cc (c, E68_SR_XC, d != 0);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4480: NEG.L <EA> */
static unsigned op4480 (e68000_t *c)
{
	uint32_t s, d;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = (~s + 1) & 0xffffffff;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_set_sr_v (c, s == 0x80000000);
	e68_set_sr_z (c, d == 0);
	e68_set_cc (c, E68_SR_XC, d != 0);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 44C0: MOVE.W <EA>, CCR */
static unsigned op44c0 (e68000_t *c)
{
	uint16_t s;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s);

	e68_set_ccr (c, s & E68_SR_XNZVC);
	e68_set_clk (c, 18);

	return (c->ircnt);
}

/* 4600: NOT.B <EA> */
static unsigned op4600 (e68000_t *c)
{
	uint8_t s, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = ~s & 0xff;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4640: NOT.W <EA> */
static unsigned op4640 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = ~s & 0xffff;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4680: NOT.L <EA> */
static unsigned op4680 (e68000_t *c)
{
	uint32_t s, d;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = ~s & 0xffffffff;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 46C0: MOVE.W <EA>, SR */
static unsigned op46c0 (e68000_t *c)
{
	uint16_t s;

	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s);

	e68_set_sr (c, s & E68_SR_MASK);
	e68_set_clk (c, 18);

	return (c->ircnt);
}

/* 4800: NBCD.B <EA> */
static unsigned op4800 (e68000_t *c)
{
	uint8_t  s;
	uint16_t d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = 0 - s - e68_get_sr_x (c);

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

	e68_set_ea8 (c, 0, 0, 0, d & 0xff);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 4840_00: SWAP Dx */
static unsigned op4840_00 (e68000_t *c)
{
	unsigned reg;
	uint32_t val;

	reg = e68_ir_reg0 (c);
	val = e68_get_dreg32 (c, reg);

	val = ((val << 16) | (val >> 16)) & 0xffffffff;

	e68_set_dreg32 (c, reg, val);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, val);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4840_XX: PEA <EA> */
static unsigned op4840_xx (e68000_t *c)
{
	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x7e4, 32)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	e68_push32 (c, c->ea_val);

	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* 4840: misc */
static unsigned op4840 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op4840_00 (c));

	default:
		return (op4840_xx (c));
	}
}

/* 4880_00: EXT.W Dn */
static unsigned op4880_00 (e68000_t *c)
{
	unsigned reg;
	uint16_t s, d;

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, reg);

	d = (s & 0x80) ? (s | 0xff00) : s;

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4880_04: MOVEM.W list, -(Ax) */
static unsigned op4880_04 (e68000_t *c)
{
	unsigned i;
	uint16_t r, v;
	uint32_t a;

	e68_ifetch_next (c);

	r = c->ir[1];

	a = e68_get_areg32 (c, e68_ir_reg0 (c));

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
		}
		r >>= 1;
		e68_set_clk (c, 8);
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 16);

	return (c->ircnt);
}

/* 4880_XX: MOVEM.W list, <EA> */
static unsigned op4880_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r, v;
	uint32_t a;

	e68_ifetch_next (c);

	r = c->ir[1];

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x1f4, 16)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	a = c->ea_val;

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
		}
		r >>= 1;
		e68_set_clk (c, 8);
	}

	e68_set_clk (c, 16);

	return (c->ircnt);
}

/* 4880: misc */
static unsigned op4880 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op4880_00 (c));

	case 0x04:
		return (op4880_04 (c));

	default:
		return (op4880_xx (c));
	}
}

/* 48C0_00: EXT.L Dn */
static unsigned op48c0_00 (e68000_t *c)
{
	unsigned reg;
	uint32_t s, d;

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, reg);

	d = (s & 0x8000) ? (s | 0xffff0000) : s;

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 48C0_04: MOVEM.L list, -(Ax) */
static unsigned op48c0_04 (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_ifetch_next (c);

	r = c->ir[1];
	a = e68_get_areg32 (c, e68_ir_reg0 (c));

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
		}
		r >>= 1;
		e68_set_clk (c, 16);
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 16);

	return (c->ircnt);
}

/* 48C0_XX: MOVEM.L list, <EA> */
static unsigned op48c0_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_ifetch_next (c);

	r = c->ir[1];

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x1f4, 32)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	a = c->ea_val;

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
		}
		r >>= 1;
		e68_set_clk (c, 16);
	}

	e68_set_clk (c, 16);

	return (c->ircnt);
}

/* 48C0: misc */
static unsigned op48c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (op48c0_00 (c));

	case 0x04:
		return (op48c0_04 (c));

	default:
		return (op48c0_xx (c));
	}
}

/* 4A00: TST.B <EA> */
static unsigned op4a00 (e68000_t *c)
{
	uint8_t s;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0fff, &s);

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

/* 4AC0_XX: TAS <EA> */
static unsigned op4ac0_xx (e68000_t *c)
{
	uint8_t s, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s);

	d = s | 0x80;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, s);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4AC0: misc */
static unsigned op4ac0 (e68000_t *c)
{
	if (c->ir[0] == 0x4afc) {
		/* ILLEGAL */
		e68_exception_illegal (c);
		return (0);
	}

	return (op4ac0_xx (c));
}

/* 4C80_03: MOVEM.W (Ax)+, list */
static unsigned op4c80_03 (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_ifetch_next (c);

	r = c->ir[1];
	a = e68_get_areg32 (c, e68_ir_reg0 (c));

	if (a & 1) {
		e68_exception_address (c, a);
		return (0);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			v = e68_get_mem16 (c, a);
			v = e68_exts (v, 16);
			if (i < 8) {
				e68_set_dreg32 (c, i, v);
			}
			else {
				e68_set_areg32 (c, i - 8, v);
			}
			a = (a + 2) & 0xffffffff;
		}
		r >>= 1;
		e68_set_clk (c, 8);
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* 4C80_XX: MOVEM.W <EA>, list */
static unsigned op4c80_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_ifetch_next (c);

	r = c->ir[1];

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x1ec, 16)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	a = c->ea_val;

	if (a & 1) {
		e68_exception_address (c, a);
		return (0);
	}

	for (i = 0; i < 16; i++) {
		if (r & 1) {
			v = e68_get_mem16 (c, a);
			v = e68_exts (v, 16);
			if (i < 8) {
				e68_set_dreg32 (c, i, v);
			}
			else {
				e68_set_areg32 (c, i - 8, v);
			}
			a = (a + 2) & 0xffffffff;
		}
		r >>= 1;
		e68_set_clk (c, 8);
	}

	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* 4C80: misc */
static unsigned op4c80 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (e68_op_undefined (c));

	case 0x03:
		return (op4c80_03 (c));

	default:
		return (op4c80_xx (c));
	}
}

/* 4CC0_03: MOVEM.L (Ax)+, list */
static unsigned op4cc0_03 (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_ifetch_next (c);

	r = c->ir[1];
	a = e68_get_areg32 (c, e68_ir_reg0 (c));

	if (a & 1) {
		e68_exception_address (c, a);
		return (0);
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
		}
		r >>= 1;
		e68_set_clk (c, 16);
	}

	e68_set_areg32 (c, e68_ir_reg0 (c), a);

	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* 4CC0_XX: MOVEM.L <EA>, list */
static unsigned op4cc0_xx (e68000_t *c)
{
	unsigned i;
	uint16_t r;
	uint32_t a, v;

	e68_ifetch_next (c);

	r = c->ir[1];

	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x1ec, 32)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	a = c->ea_val;

	if (a & 1) {
		e68_exception_address (c, a);
		return (0);
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
		}
		r >>= 1;
		e68_set_clk (c, 16);
	}

	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* 4CC0: misc */
static unsigned op4cc0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (e68_op_undefined (c));

	case 0x03:
		return (op4cc0_03 (c));

	default:
		return (op4cc0_xx (c));
	}
}

/* 4E40_00: TRAP #XX */
static unsigned op4e40_00 (e68000_t *c)
{
	e68_set_pc (c, e68_get_pc (c) + 2);
	e68_exception_trap (c, c->ir[0] & 15);

	return (0);
}

/* 4E40_02: LINK An, #XXXX */
static unsigned op4e40_02 (e68000_t *c)
{
	unsigned reg;
	uint32_t s, d;

	e68_ifetch_next (c);

	reg = e68_ir_reg0 (c);
	d = e68_get_areg32 (c, reg);
	s = e68_exts (c->ir[1], 16);

	e68_push32 (c, d);
	e68_set_areg32 (c, reg, e68_get_areg32 (c, 7));
	e68_set_areg32 (c, 7, e68_get_areg32 (c, 7) + s);

	e68_set_clk (c, 32);

	return (c->ircnt);
}

/* 4E40_03: UNLK An */
static unsigned op4e40_03 (e68000_t *c)
{
	unsigned reg;
	uint32_t a;

	reg = e68_ir_reg0 (c);
	a = e68_get_areg32 (c, reg);

	if (a & 1) {
		e68_exception_address (c, a);
		return (0);
	}

	e68_set_areg32 (c, reg, e68_get_mem32 (c, a));
	e68_set_areg32 (c, 7, a + 4);

	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* 4E40_04: MOVE.L Ax, USP */
static unsigned op4e40_04 (e68000_t *c)
{
	uint32_t s;

	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	s = e68_get_areg32 (c, e68_ir_reg0 (c));

	e68_set_usp (c, s);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4E40_05: MOVE.L USP, Ax */
static unsigned op4e40_05 (e68000_t *c)
{
	uint32_t s;

	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	s = e68_get_usp (c);

	e68_set_areg32 (c, e68_ir_reg0 (c), s);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 4E70: RESET */
static unsigned op4e70 (e68000_t *c)
{
	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	e68_set_clk (c, 136);

	return (1);
}

/* 4E71: NOP */
static unsigned op4e71 (e68000_t *c)
{
	e68_set_clk (c, 8);

	return (1);
}

/* 4E72: STOP #XXXX */
static unsigned op4e72 (e68000_t *c)
{
	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	e68_ifetch_next (c);
	e68_set_sr (c, c->ir[1]);

	e68_set_clk (c, 4);

	c->halt |= 1;

	return (2);
}

/* 4E73: RTE */
static unsigned op4e73 (e68000_t *c)
{
	uint32_t sp, pc;
	uint16_t sr;

	if (c->supervisor == 0) {
		e68_exception_privilege (c);
		return (0);
	}

	sp = e68_get_ssp (c);
	sr = e68_get_mem16 (c, sp);
	pc = e68_get_mem32 (c, sp + 2);
	e68_set_sr (c, sr);
	e68_set_pc (c, pc);
	e68_set_ssp (c, sp + 6);

	e68_set_clk (c, 40);

	return (0);
}

/* 4E75: RTS */
static unsigned op4e75 (e68000_t *c)
{
	uint32_t sp;

	sp = e68_get_areg32 (c, 7);

	e68_set_pc (c, e68_get_mem32 (c, sp));
	e68_set_areg32 (c, 7, sp + 4);

	e68_set_clk (c, 32);

	return (0);
}

/* 4E76: TRAPV */
static unsigned op4e76 (e68000_t *c)
{
	e68_set_pc (c, e68_get_pc (c) + 2);
	e68_exception_overflow (c);

	return (0);
}

/* 4E77: RTR */
static unsigned op4e77 (e68000_t *c)
{
	uint32_t sp;

	sp = e68_get_areg32 (c, 7);

	e68_set_ccr (c, e68_get_mem16 (c, sp) & E68_CCR_MASK);
	e68_set_pc (c, e68_get_mem32 (c, sp + 2));
	e68_set_areg32 (c, 7, sp + 6);

	e68_set_clk (c, 40);

	return (0);
}

/* 4E40: misc */
static unsigned op4e40 (e68000_t *c)
{
	switch (c->ir[0]) {
	case 0x4e70:
		return (op4e70 (c));

	case 0x4e71:
		return (op4e71 (c));

	case 0x4e72:
		return (op4e72 (c));

	case 0x4e73:
		return (op4e73 (c));

	case 0x4e75:
		return (op4e75 (c));

	case 0x4e76:
		return (op4e76 (c));

	case 0x4e77:
		return (op4e77 (c));

	default:
		switch ((c->ir[0] >> 3) & 7) {
		case 0x00:
		case 0x01:
			return (op4e40_00 (c));

		case 0x02:
			return (op4e40_02 (c));

		case 0x03:
			return (op4e40_03 (c));

		case 0x04:
			return (op4e40_04 (c));

		case 0x05:
			return (op4e40_05 (c));
		}
	}

	return (e68_op_undefined (c));
}

/* 4E80: JSR <EA> */
static unsigned op4e80 (e68000_t *c)
{
	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x07e4, 32)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	e68_push32 (c, e68_get_pc (c) + 2 * c->ircnt);
	e68_set_pc (c, c->ea_val);

	e68_set_clk (c, 18);

	return (0);
}

/* 4EC0: JMP <EA> */
static unsigned op4ec0 (e68000_t *c)
{
	if (e68_ea_get_ptr (c, e68_ir_ea1 (c), 0x07e4, 32)) {
		return (0);
	}

	if (c->ea_typ != 2) {
		e68_exception_illegal (c);
		return (0);
	}

	e68_set_pc (c, c->ea_val);

	e68_set_clk (c, 2);

	return (0);
}

/* 5000: ADDQ.B #X, <EA> */
static unsigned op5000 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 + s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_add_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 5040_01: ADDQ.W #X, Ax */
static unsigned op5040_01 (e68000_t *c)
{
	unsigned reg;
	uint32_t  s1, s2, d;

	reg = c->ir[0] & 7;

	s1 = e68_get_areg32 (c, reg);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 + s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 5040_XX: ADDQ.W #X, <EA> */
static unsigned op5040_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 + s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_add_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 5040: misc */
static unsigned op5040 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5040_01 (c));

	default:
		return (op5040_xx (c));
	}
}

/* 5080_XX: ADDQ.L #X, <EA> */
static unsigned op5080_xx (e68000_t *c)
{
	uint32_t s1, s2, d;

	/* maybe 16 bit ? */
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 + s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_add_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 5080_01: ADDQ.L #X, Ax */
static unsigned op5080_01 (e68000_t *c)
{
	unsigned reg;
	uint32_t  s1, s2, d;

	reg = c->ir[0] & 7;

	s1 = e68_get_areg32 (c, reg);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 + s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 5080: misc */
static unsigned op5080 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5080_01 (c));

	default:
		return (op5080_xx (c));
	}
}

/* 5100: SUBQ.B #X, <EA> */
static unsigned op5100 (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 - s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_sub_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 5140_01: SUBQ.W #X, Ax */
static unsigned op5140_01 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = c->ir[0] & 7;

	s1 = e68_get_areg32 (c, reg);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 - s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 5140_XX: SUBQ.W #X, <EA> */
static unsigned op5140_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 - s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_sub_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 5140: misc */
static unsigned op5140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5140_01 (c));

	default:
		return (op5140_xx (c));
	}
}

/* 5180_01: SUBQ.L #X, Ax */
static unsigned op5180_01 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = c->ir[0] & 7;

	s1 = e68_get_areg32 (c, reg);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 - s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 5180_XX: SUBQ.L #X, <EA> */
static unsigned op5180_xx (e68000_t *c)
{
	uint32_t s1, s2, d;

	/* maybe 16 bit ? */
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s1);
	s2 = (c->ir[0] >> 9) & 7;

	if (s2 == 0) {
		s2 = 8;
	}

	d = s1 - s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_sub_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 5180: misc */
static unsigned op5180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5180_01 (c));

	default:
		return (op5180_xx (c));
	}
}

/* DBcc Dx, dist */
static unsigned op_dbcc (e68000_t *c, int cond)
{
	unsigned reg;
	uint16_t val;
	uint32_t dist;

	e68_ifetch_next (c);

	dist = e68_exts (c->ir[1], 16);

	if (cond) {
		e68_set_clk (c, 20);
		return (c->ircnt);
	}

	reg = e68_ir_reg0 (c);
	val = e68_get_dreg16 (c, reg);
	val = (val - 1) & 0xffff;
	e68_set_dreg16 (c, reg, val);

	if (val == 0xffff) {
		e68_set_clk (c, 20);
		return (c->ircnt);
	}

	e68_set_pc (c, e68_get_pc (c) + dist + 2);

	e68_set_clk (c, 18);

	return (0);
}

/* Scc <EA> */
static unsigned op_scc (e68000_t *c, int cond)
{
	uint8_t val;

	if (cond) {
		val = 0xff;
		e68_set_clk (c, 10);
	}
	else {
		val = 0x00;
		e68_set_clk (c, 8);
	}

	e68_set_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, val);

	return (c->ircnt);
}

/* 50C0_01: DBT Dx, dist */
static unsigned op50c0_01 (e68000_t *c)
{
	return (op_dbcc (c, 1));
}

/* 50C0_XX: ST <EA> */
static unsigned op50c0_xx (e68000_t *c)
{
	return (op_scc (c, 1));
}

/* 50C0: misc */
static unsigned op50c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op50c0_01 (c));

	default:
		return (op50c0_xx (c));
	}
}

/* 51C0_01: DBF Dx, dist */
static unsigned op51c0_01 (e68000_t *c)
{
	return (op_dbcc (c, 0));
}

/* 51C0_XX: SF <EA> */
static unsigned op51c0_xx (e68000_t *c)
{
	return (op_scc (c, 0));
}

/* 51C0: misc */
static unsigned op51c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op51c0_01 (c));

	default:
		return (op51c0_xx (c));
	}
}

/* 52C0_01: DBHI Dx, dist */
static unsigned op52c0_01 (e68000_t *c)
{
	return (op_dbcc (c, !e68_get_sr_c (c) && !e68_get_sr_z (c)));
}

/* 52C0_XX: SHI <EA> */
static unsigned op52c0_xx (e68000_t *c)
{
	return (op_scc (c, !e68_get_sr_c (c) && !e68_get_sr_z (c)));
}

/* 52C0: misc */
static unsigned op52c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op52c0_01 (c));

	default:
		return (op52c0_xx (c));
	}
}

/* 53C0_01: DBLS Dx, dist */
static unsigned op53c0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_c (c) || e68_get_sr_z (c)));
}

/* 53C0_XX: SLS <EA> */
static unsigned op53c0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_c (c) || e68_get_sr_z (c)));
}

/* 53C0: misc */
static unsigned op53c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op53c0_01 (c));

	default:
		return (op53c0_xx (c));
	}
}

/* 54C0_01: DBCC Dx, dist */
static unsigned op54c0_01 (e68000_t *c)
{
	return (op_dbcc (c, !e68_get_sr_c (c)));
}

/* 54C0_XX: SCC <EA> */
static unsigned op54c0_xx (e68000_t *c)
{
	return (op_scc (c, !e68_get_sr_c (c)));
}

/* 54C0: misc */
static unsigned op54c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op54c0_01 (c));

	default:
		return (op54c0_xx (c));
	}
}

/* 55C0_01: DBCS Dx, dist */
static unsigned op55c0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_c (c)));
}

/* 55C0_XX: SCS <EA> */
static unsigned op55c0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_c (c)));
}

/* 55C0: misc */
static unsigned op55c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op55c0_01 (c));

	default:
		return (op55c0_xx (c));
	}
}

/* 56C0_01: DBNE Dx, dist */
static unsigned op56c0_01 (e68000_t *c)
{
	return (op_dbcc (c, !e68_get_sr_z (c)));
}

/* 56C0_XX: SNE <EA> */
static unsigned op56c0_xx (e68000_t *c)
{
	return (op_scc (c, !e68_get_sr_z (c)));
}

/* 56C0: misc */
static unsigned op56c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op56c0_01 (c));

	default:
		return (op56c0_xx (c));
	}
}

/* 57C0_01: DBEQ Dx, dist */
static unsigned op57c0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_z (c)));
}

/* 57C0_XX: SEQ <EA> */
static unsigned op57c0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_z (c)));
}

/* 57C0: misc */
static unsigned op57c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op57c0_01 (c));

	default:
		return (op57c0_xx (c));
	}
}

/* 58C0_01: DBVC Dx, dist */
static unsigned op58c0_01 (e68000_t *c)
{
	return (op_dbcc (c, !e68_get_sr_v (c)));
}

/* 58C0_XX: SVC <EA> */
static unsigned op58c0_xx (e68000_t *c)
{
	return (op_scc (c, !e68_get_sr_v (c)));
}

/* 58C0: misc */
static unsigned op58c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op58c0_01 (c));

	default:
		return (op58c0_xx (c));
	}
}

/* 59C0_01: DBVS Dx, dist */
static unsigned op59c0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_v (c)));
}

/* 59C0_XX: SVS <EA> */
static unsigned op59c0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_v (c)));
}

/* 59C0: misc */
static unsigned op59c0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op59c0_01 (c));

	default:
		return (op59c0_xx (c));
	}
}

/* 5AC0_01: DBPL Dx, dist */
static unsigned op5ac0_01 (e68000_t *c)
{
	return (op_dbcc (c, !e68_get_sr_n (c)));
}

/* 5AC0_XX: SPL <EA> */
static unsigned op5ac0_xx (e68000_t *c)
{
	return (op_scc (c, !e68_get_sr_n (c)));
}

/* 5AC0: misc */
static unsigned op5ac0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5ac0_01 (c));

	default:
		return (op5ac0_xx (c));
	}
}

/* 5BC0_01: DBMI Dx, dist */
static unsigned op5bc0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_n (c)));
}

/* 5BC0_XX: SMI <EA> */
static unsigned op5bc0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_n (c)));
}

/* 5BC0: misc */
static unsigned op5bc0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5bc0_01 (c));

	default:
		return (op5bc0_xx (c));
	}
}

/* 5CC0_01: DBGE Dx, dist */
static unsigned op5cc0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_n (c) == e68_get_sr_v (c)));
}

/* 5CC0_XX: SGE <EA> */
static unsigned op5cc0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_n (c) == e68_get_sr_v (c)));
}

/* 5CC0: misc */
static unsigned op5cc0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5cc0_01 (c));

	default:
		return (op5cc0_xx (c));
	}
}

/* 5DC0_01: DBLT Dx, dist */
static unsigned op5dc0_01 (e68000_t *c)
{
	return (op_dbcc (c, e68_get_sr_n (c) != e68_get_sr_v (c)));
}

/* 5DC0_XX: SLT <EA> */
static unsigned op5dc0_xx (e68000_t *c)
{
	return (op_scc (c, e68_get_sr_n (c) != e68_get_sr_v (c)));
}

/* 5DC0: misc */
static unsigned op5dc0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5dc0_01 (c));

	default:
		return (op5dc0_xx (c));
	}
}

/* 5EC0_01: DBGT Dx, dist */
static unsigned op5ec0_01 (e68000_t *c)
{
	return (op_dbcc (c, (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c)));
}

/* 5EC0_XX: SGT <EA> */
static unsigned op5ec0_xx (e68000_t *c)
{
	return (op_scc (c, (e68_get_sr_n (c) == e68_get_sr_v (c)) && !e68_get_sr_z (c)));
}

/* 5EC0: misc */
static unsigned op5ec0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5ec0_01 (c));

	default:
		return (op5ec0_xx (c));
	}
}

/* 5FC0_01: DBLE Dx, dist */
static unsigned op5fc0_01 (e68000_t *c)
{
	return (op_dbcc (c, (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c)));
}

/* 5FC0_XX: SLE <EA> */
static unsigned op5fc0_xx (e68000_t *c)
{
	return (op_scc (c, (e68_get_sr_n (c) != e68_get_sr_v (c)) || e68_get_sr_z (c)));
}

/* 5FC0: misc */
static unsigned op5fc0 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (op5fc0_01 (c));

	default:
		return (op5fc0_xx (c));
	}
}

/* conditional jump */
static unsigned op_bcc (e68000_t *c, int cond)
{
	uint32_t dist;

	dist = e68_exts (c->ir[0], 8);

	if (dist == 0) {
		e68_ifetch_next (c);
		dist = e68_exts (c->ir[1], 16);
	}

	if (cond == 0) {
		e68_set_clk (c, 12);
		return (c->ircnt);
	}

	e68_set_pc (c, e68_get_pc (c) + dist + 2);

	e68_set_clk (c, 18);

	return (0);
}

/* 6000: BRA dist */
static unsigned op6000 (e68000_t *c)
{
	return (op_bcc (c, 1));
}

/* 6100: BSR dist */
static unsigned op6100 (e68000_t *c)
{
	uint32_t pc;

	pc = e68_get_pc (c);

	op_bcc (c, 1);

	e68_push32 (c, pc + 2 * c->ircnt);

	e68_set_clk (c, 16);

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

/* 7000: MOVEQ #XX, Dx */
static unsigned op7000 (e68000_t *c)
{
	uint32_t val;

	val = e68_exts (c->ir[0], 8);

	e68_set_dreg32 (c, e68_ir_reg9 (c), val);

	e68_cc_set_nz_32 (c, E68_SR_NZVC, val);
	e68_set_clk (c, 4);

	return (1);
}

/* 8000: OR.B <EA>, Dx */
static unsigned op8000 (e68000_t *c)
{
	unsigned reg;
	uint8_t  s1, s2, d;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg8 (c, reg);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 | s2;

	e68_set_dreg8 (c, reg, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 8040: OR.W <EA>, Dx */
static unsigned op8040 (e68000_t *c)
{
	unsigned reg;
	uint16_t s1, s2, d;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg16 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 | s2;

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 8080: OR.L <EA>, Dx */
static unsigned op8080 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg32 (c, reg);
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 | s2;

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 80C0: DIVU.W <EA>, Dx */
static unsigned op80c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, d, r;
	uint16_t s2;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg32 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	if (s2 == 0) {
		e68_exception_divzero (c);
		return (0);
	}

	d = s1 / s2;
	r = s1 % s2;

	if (d & 0xffff0000) {
		e68_set_sr_v (c, 1);
		e68_set_sr_c (c, 0);
	}
	else {
		e68_set_dreg32 (c, reg, ((r & 0xffff) << 16) | (d & 0xffff));
		e68_cc_set_nz_16 (c, E68_SR_NZVC, d & 0xffff);
	}

	e68_set_clk (c, 144);

	return (c->ircnt);
}

/* 8100_00: SBCD <EA>, <EA> */
static unsigned op8100_00 (e68000_t *c)
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

	e68_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_get_ea8 (c, 1, ea2, 0x0011, &s2);

	s1 += e68_get_sr_x (c);

	d = (uint16_t) s2 - (uint16_t) s1 - e68_get_sr_x (c);

	if ((s2 & 0x0f) < (s1 & 0x0f)) {
		d -= 0x06;
	}

	if (d >= 0xa0) {
		d -= 0x60;
	}

	e68_set_cc (c, E68_SR_XC, d & 0xff00);

	if (d & 0xff) {
		c->sr &= ~E68_SR_Z;
	}

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 8100_XX: AND.B Dx, <EA> */
static unsigned op8100_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 | s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 8100: misc */
static unsigned op8100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (op8100_00 (c));

	default:
		return (op8100_xx (c));
	}
}

/* 8140: AND.W Dx, <EA> */
static unsigned op8140 (e68000_t *c)
{
	uint16_t s1, s2, d;

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 | s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 8180: AND.L Dx, <EA> */
static unsigned op8180 (e68000_t *c)
{
	uint32_t s1, s2, d;

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 | s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 81C0: DIVS.W <EA>, Dx */
static unsigned op81c0 (e68000_t *c)
{
	unsigned reg;
	int      n1, n2;
	uint32_t s1, d, r;
	uint16_t s2;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg32 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	if (s2 == 0) {
		e68_exception_divzero (c);
		return (0);
	}

	n1 = 0;
	n2 = 0;

	if (s1 & 0x80000000) {
		n1 = 1;
		s1 = (~s1 + 1) & 0xffffffff;
	}

	if (s2 & 0x8000) {
		n2 = 1;
		s2 = (~s2 + 1) & 0xffff;
	}

	d = s1 / s2;
	r = s1 % s2;

	if (n1) {
		r = (~r + 1) & 0xffff;
	}

	if (n1 != n2) {
		d = (~d + 1) & 0xffffffff;
	}

	e68_set_clk (c, 162);

	switch (d & 0xffff8000) {
	case 0x00000000:
	case 0xffff8000:
		e68_set_dreg32 (c, reg, ((r & 0xffff) << 16) | (d & 0xffff));
		e68_cc_set_nz_16 (c, E68_SR_NZVC, d & 0xffff);
		break;

	default:
		e68_set_sr_v (c, 1);
		e68_set_sr_c (c, 0);
		break;
	}


	return (c->ircnt);
}

/* 9000: SUB.B <EA>, Dx */
static unsigned op9000 (e68000_t *c)
{
	unsigned reg;
	uint8_t  s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_dreg8 (c, reg);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 - s2;

	e68_set_dreg8 (c, reg, d);
	e68_cc_set_sub_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9040: SUB.W <EA>, Dx */
static unsigned op9040 (e68000_t *c)
{
	unsigned reg;
	uint16_t s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_dreg16 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	d = s1 - s2;

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_sub_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9080: SUB.L <EA>, Dx */
static unsigned op9080 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_dreg32 (c, reg);
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	d = s1 - s2;

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_sub_32 (c, s1, s2);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* 90C0: SUBA.W <EA>, Ax */
static unsigned op90c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;
	uint16_t t;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_areg32 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &t);
	s2 = e68_exts (t, 16);

	d = s1 - s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9100_00: SUBX.B <EA>, <EA> */
static unsigned op9100_00 (e68000_t *c)
{
	uint8_t  s1, s2, s3, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg9 (c);
	ea2 = e68_ir_reg0 (c);

	if (c->ir[0] & 0x0008) {
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_get_ea8 (c, 1, ea2, 0x0011, &s2);
	e68_get_ea8 (c, 1, ea1, 0x0011, &s1);
	s3 = e68_get_sr_x (c);

	d = s1 - s2 - s3;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_subx_8 (c, s1, s2, s3);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9100_XX: SUB.B Dx, <EA> */
static unsigned op9100_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s1);
	s2 = e68_get_dreg8 (c, (c->ir[0] >> 9) & 7);

	d = s1 - s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_sub_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9100: misc */
static unsigned op9100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (op9100_00 (c));

	default:
		return (op9100_xx (c));
	}
}

/* 9140_00: SUBX.W <EA>, <EA> */
static unsigned op9140_00 (e68000_t *c)
{
	uint16_t s1, s2, s3, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg9 (c);
	ea2 = e68_ir_reg0 (c);

	if (c->ir[0] & 0x0008) {
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_get_ea16 (c, 1, ea2, 0x0011, &s2);
	e68_get_ea16 (c, 1, ea1, 0x0011, &s1);
	s3 = e68_get_sr_x (c);

	d = s1 - s2 - s3;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_subx_16 (c, s1, s2, s3);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9140_XX: SUB.W Dx, <EA> */
static unsigned op9140_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s1);
	s2 = e68_get_dreg16 (c, (c->ir[0] >> 9) & 7);

	d = s1 - s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_sub_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* 9140: misc */
static unsigned op9140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (op9140_00 (c));

	default:
		return (op9140_xx (c));
	}
}

/* 9180_00: SUBX.L <EA>, <EA> */
static unsigned op9180_00 (e68000_t *c)
{
	uint32_t s1, s2, s3, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg9 (c);
	ea2 = e68_ir_reg0 (c);

	if (c->ir[0] & 0x0008) {
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_get_ea32 (c, 1, ea2, 0x0011, &s2);
	e68_get_ea32 (c, 1, ea1, 0x0011, &s1);
	s3 = e68_get_sr_x (c);

	d = s1 - s2 - s3;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_subx_32 (c, s1, s2, s3);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 9180_XX: SUB.L Dx, <EA> */
static unsigned op9180_xx (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s1);
	s2 = e68_get_dreg32 (c, (c->ir[0] >> 9) & 7);

	d = s1 - s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_sub_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* 9180: misc */
static unsigned op9180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (op9180_00 (c));

	default:
		return (op9180_xx (c));
	}
}

/* 91C0: SUBA.L <EA>, Ax */
static unsigned op91c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_areg32 (c, reg);
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	d = s1 - s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* A000: AXXX */
static unsigned opa000 (e68000_t *c)
{
	e68_exception_axxx (c);
	return (0);
}

/* B000: CMP.B <EA>, Dx */
static unsigned opb000 (e68000_t *c)
{
	uint8_t  s1, s2;

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	e68_cc_set_cmp_8 (c, s1, s2);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* B040: CMP.W <EA>, Dx */
static unsigned opb040 (e68000_t *c)
{
	uint16_t s1, s2;

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	e68_cc_set_cmp_16 (c, s1, s2);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* B080: CMP.L <EA>, Dx */
static unsigned opb080 (e68000_t *c)
{
	uint32_t s1, s2;

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	e68_cc_set_cmp_32 (c, s1, s2);
	e68_set_clk (c, 6);

	return (c->ircnt);
}

/* B0C0: CMPA.W <EA>, Ax */
static unsigned opb0c0 (e68000_t *c)
{
	uint32_t s1, s2;
	uint16_t t;

	s1 = e68_get_areg32 (c, e68_ir_reg9 (c));
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &t);
	s2 = e68_exts (t, 16);

	e68_cc_set_cmp_32 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* B100_01: CMPM.B (Ay)+, (Ax)+ */
static unsigned opb100_01 (e68000_t *c)
{
	uint8_t s1, s2;

	e68_get_ea8 (c, 1, e68_ir_reg9 (c) | (3 << 3), 0x0008, &s1);
	e68_get_ea8 (c, 1, e68_ir_reg0 (c) | (3 << 3), 0x0008, &s2);

	e68_cc_set_cmp_8 (c, s1, s2);
	e68_set_clk (c, 16);

	return (c->ircnt);
}

/* B100_XX: EOR.B Dx, <EA> */
static unsigned opb100_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* B100: misc */
static unsigned opb100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (opb100_01 (c));

	default:
		return (opb100_xx (c));
	}
}

/* B140_01: CMPM.W (Ay)+, (Ax)+ */
static unsigned opb140_01 (e68000_t *c)
{
	uint16_t s1, s2;

	e68_get_ea16 (c, 1, e68_ir_reg9 (c) | (3 << 3), 0x0008, &s1);
	e68_get_ea16 (c, 1, e68_ir_reg0 (c) | (3 << 3), 0x0008, &s2);

	e68_cc_set_cmp_16 (c, s1, s2);
	e68_set_clk (c, 24);

	return (c->ircnt);
}

/* B140_XX: EOR.W Dx, <EA> */
static unsigned opb140_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* B140: misc */
static unsigned opb140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (opb140_01 (c));

	default:
		return (opb140_xx (c));
	}
}

/* B180_01: CMPM.L (Ay)+, (Ax)+ */
static unsigned opb180_01 (e68000_t *c)
{
	uint32_t s1, s2;

	e68_get_ea32 (c, 1, e68_ir_reg9 (c) | (3 << 3), 0x0008, &s1);
	e68_get_ea32 (c, 1, e68_ir_reg0 (c) | (3 << 3), 0x0008, &s2);

	e68_cc_set_cmp_32 (c, s1, s2);
	e68_set_clk (c, 40);

	return (c->ircnt);
}

/* B180_XX: EOR.L Dx, <EA> */
static unsigned opb180_xx (e68000_t *c)
{
	uint32_t s1, s2, d;

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fd, &s2);

	d = s1 ^ s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* B180: misc */
static unsigned opb180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x01:
		return (opb180_01 (c));

	default:
		return (opb180_xx (c));
	}
}

/* B1C0: CMPA.L <EA>, Ax */
static unsigned opb1c0 (e68000_t *c)
{
	uint32_t s1, s2;

	s1 = e68_get_areg32 (c, e68_ir_reg9 (c));
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	e68_cc_set_cmp_32 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* C000: AND.B EA, Dx */
static unsigned opc000 (e68000_t *c)
{
	unsigned reg;
	uint8_t  s1, s2, d;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg8 (c, reg);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 & s2;

	e68_set_dreg8 (c, reg, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* C040: AND.W EA, Dx */
static unsigned opc040 (e68000_t *c)
{
	unsigned reg;
	uint16_t s1, s2, d;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg16 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 & s2;

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* C080: AND.L EA, Dx */
static unsigned opc080 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg32 (c, reg);
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 & s2;

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 6);

	return (c->ircnt);
}

/* C0C0: MULU.W <EA>, Dx */
static unsigned opc0c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t d;
	uint16_t s1, s2;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg16 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = (uint32_t) s1 * (uint32_t) s2;

	e68_set_clk (c, 74);

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);

	return (c->ircnt);
}

/* C100_00: ABCD EA, EA */
static unsigned opc100_00 (e68000_t *c)
{
	uint8_t  s1, s2;
	uint16_t d;
	unsigned ea1, ea2;

	ea1 = c->ir[0] & 0x07;
	ea2 = (c->ir[0] >> 9) & 0x07;

	if (c->ir[0] & 0x08) {
		/* -(Ax) */
		ea1 |= (4 << 3);
		ea2 |= (4 << 3);
	}

	e68_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_get_ea8 (c, 1, ea2, 0x0011, &s2);

	d = (uint16_t) s1 + (uint16_t) s2 + e68_get_sr_x (c);

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

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_set_clk (c, 6);

	return (c->ircnt);
}

/* C100_XX: AND.B Dx, <EA> */
static unsigned opc100_xx (e68000_t *c)
{
	uint8_t  s1, s2, d;

	s1 = e68_get_dreg8 (c, e68_ir_reg9 (c));
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 & s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* C100: misc */
static unsigned opc100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (opc100_00 (c));

	default:
		return (opc100_xx (c));
	}
}

/* C140_0: EXG Dx, Dx */
static unsigned opc140_0 (e68000_t *c)
{
	unsigned r1, r2;
	uint32_t s1, s2;

	r1 = e68_ir_reg0 (c);
	r2 = e68_ir_reg9 (c);

	s1 = e68_get_dreg32 (c, r1);
	s2 = e68_get_dreg32 (c, r2);

	e68_set_dreg32 (c, r1, s2);
	e68_set_dreg32 (c, r2, s1);

	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* C140_1: EXG Ax, Ax */
static unsigned opc140_1 (e68000_t *c)
{
	unsigned r1, r2;
	uint32_t s1, s2;

	r1 = e68_ir_reg0 (c);
	r2 = e68_ir_reg9 (c);

	s1 = e68_get_areg32 (c, r1);
	s2 = e68_get_areg32 (c, r2);

	e68_set_areg32 (c, r1, s2);
	e68_set_areg32 (c, r2, s1);

	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* C140_2: AND.W Dx, EA */
static unsigned opc140_2 (e68000_t *c)
{
	uint16_t s1, s2, d;

	s1 = e68_get_dreg16 (c, e68_ir_reg9 (c));
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 & s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* C140: misc */
static unsigned opc140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (opc140_0 (c));

	case 0x01:
		return (opc140_1 (c));

	default:
		return (opc140_2 (c));
	}
}

/* C180_1: EXG Dx, Ay */
static unsigned opc180_1 (e68000_t *c)
{
	unsigned r1, r2;
	uint32_t s1, s2;

	r1 = e68_ir_reg0 (c);
	r2 = e68_ir_reg9 (c);

	s1 = e68_get_areg32 (c, r1);
	s2 = e68_get_dreg32 (c, r2);

	e68_set_areg32 (c, r1, s2);
	e68_set_dreg32 (c, r2, s1);

	e68_set_clk (c, 10);

	return (c->ircnt);
}

/* C180_2: AND.L Dx, <EA> */
static unsigned opc180_2 (e68000_t *c)
{
	uint32_t s1, s2, d;

	s1 = e68_get_dreg32 (c, e68_ir_reg9 (c));
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s2);

	d = s1 & s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);
	e68_set_clk (c, 6);

	return (c->ircnt);
}

/* C180: misc */
static unsigned opc180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
		return (e68_op_undefined (c));

	case 0x01:
		return (opc180_1 (c));

	default:
		return (opc180_2 (c));
	}
}

/* C1C0: MULS.W <EA>, Dx */
static unsigned opc1c0 (e68000_t *c)
{
	unsigned reg;
	int      n1, n2;
	uint32_t d;
	uint16_t s1, s2;

	reg = e68_ir_reg9 (c);

	s1 = e68_get_dreg16 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

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

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);

	return (c->ircnt);
}

/* D000: ADD.B EA, Dx */
static unsigned opd000 (e68000_t *c)
{
	unsigned reg;
	uint8_t  s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_dreg8 (c, reg);
	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x0ffd, &s2);

	d = s1 + s2;

	e68_set_dreg8 (c, reg, d);
	e68_cc_set_add_8 (c, s1, s2);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* D040: ADD.W EA, Dx */
static unsigned opd040 (e68000_t *c)
{
	unsigned reg;
	uint16_t s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_dreg16 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	d = s1 + s2;

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_add_16 (c, s1, s2);
	e68_set_clk (c, 4);

	return (c->ircnt);
}

/* D080: ADD.L EA, Dx */
static unsigned opd080 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_dreg32 (c, reg);
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	d = s1 + s2;

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_add_32 (c, s1, s2);
	e68_set_clk (c, 6);

	return (c->ircnt);
}

/* D0C0: ADDA.W EA, Ax */
static unsigned opd0c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;
	uint16_t t;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_areg32 (c, reg);
	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x0fff, &t);
	s2 = e68_exts (t, 16);

	d = s1 + s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* D100_00: ADDX.B <EA>, <EA> */
static unsigned opd100_00 (e68000_t *c)
{
	uint8_t  s1, s2, s3, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_get_ea8 (c, 1, ea1, 0x0011, &s1);
	e68_get_ea8 (c, 1, ea2, 0x0011, &s2);
	s3 = e68_get_sr_x (c);

	d = s1 + s2 + s3;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_addx_8 (c, s1, s2, s3);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* D100_XX: ADD.B Dx, <EA> */
static unsigned opd100_xx (e68000_t *c)
{
	uint8_t s1, s2, d;

	e68_get_ea8 (c, 1, e68_ir_ea1 (c), 0x01fc, &s1);
	s2 = e68_get_dreg8 (c, (c->ir[0] >> 9) & 7);

	d = s1 + s2;

	e68_set_ea8 (c, 0, 0, 0, d);
	e68_cc_set_add_8 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* D100: misc */
static unsigned opd100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (opd100_00 (c));

	default:
		return (opd100_xx (c));
	}
}

/* D140_00: ADDX.W <EA>, <EA> */
static unsigned opd140_00 (e68000_t *c)
{
	uint16_t s1, s2, s3, d;
	unsigned ea1, ea2;

	ea1 = e68_ir_reg0 (c);
	ea2 = e68_ir_reg9 (c);

	if (c->ir[0] & 0x0008) {
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_get_ea16 (c, 1, ea1, 0x0011, &s1);
	e68_get_ea16 (c, 1, ea2, 0x0011, &s2);
	s3 = e68_get_sr_x (c);

	d = s1 + s2 + s3;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_addx_16 (c, s1, s2, s3);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* D140_XX: ADD.W Dx, <EA> */
static unsigned opd140_xx (e68000_t *c)
{
	uint16_t s1, s2, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s1);
	s2 = e68_get_dreg16 (c, (c->ir[0] >> 9) & 7);

	d = s1 + s2;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_add_16 (c, s1, s2);
	e68_set_clk (c, 8);

	return (c->ircnt);
}

/* D140: misc */
static unsigned opd140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (opd140_00 (c));

	default:
		return (opd140_xx (c));
	}
}

/* D180_00: ADDX.L <EA>, <EA> */
static unsigned opd180_00 (e68000_t *c)
{
	uint32_t s1, s2, s3, d;
	unsigned ea1, ea2;

	ea1 = c->ir[0] & 7;
	ea2 = (c->ir[0] >> 9) & 7;

	if (c->ir[0] & 0x0008) {
		ea1 |= 0x20;
		ea2 |= 0x20;
	}

	e68_get_ea32 (c, 1, ea1, 0x0011, &s1);
	e68_get_ea32 (c, 1, ea2, 0x0011, &s2);
	s3 = e68_get_sr_x (c);

	d = s1 + s2 + s3;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_addx_32 (c, s1, s2, s3);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* D180_XX: ADD.L Dx, <EA> */
static unsigned opd180_xx (e68000_t *c)
{
	uint32_t s1, s2, d;

	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x01fc, &s1);
	s2 = e68_get_dreg32 (c, (c->ir[0] >> 9) & 7);

	d = s1 + s2;

	e68_set_ea32 (c, 0, 0, 0, d);
	e68_cc_set_add_32 (c, s1, s2);
	e68_set_clk (c, 12);

	return (c->ircnt);
}

/* D180: misc */
static unsigned opd180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		return (opd180_00 (c));

	default:
		return (opd180_xx (c));
	}
}

/* D1C0: ADDA.L EA, Ax */
static unsigned opd1c0 (e68000_t *c)
{
	unsigned reg;
	uint32_t s1, s2, d;

	reg = (c->ir[0] >> 9) & 7;

	s1 = e68_get_areg32 (c, reg);
	e68_get_ea32 (c, 1, e68_ir_ea1 (c), 0x0fff, &s2);

	d = s1 + s2;

	e68_set_areg32 (c, reg, d);
	e68_set_clk (c, 6);

	return (c->ircnt);
}

/* E000_00: ASR.B <cnt>, Dx */
static unsigned ope000_00 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else if (cnt < 8) {
		if (s & 0x80) {
			d = (s | 0xff00) >> cnt;
		}
		else {
			d = s >> cnt;
		}
		e68_set_cc (c, E68_SR_XC, s & (1 << (cnt - 1)));
	}
	else {
		if (s & 0x80) {
			d = 0xff;
			e68_set_cc (c, E68_SR_XC, 1);
		}
		else {
			d = 0x00;
			e68_set_cc (c, E68_SR_XC, 0);
		}
	}


	e68_set_dreg8 (c, reg, d);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E000_01: LSR.B <cnt>, Dx */
static unsigned ope000_01 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else if (cnt < 8) {
		d = s >> cnt;
		e68_set_cc (c, E68_SR_XC, s & (1 << (cnt - 1)));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (cnt == 8) && (s & 0x80));
	}


	e68_set_dreg8 (c, reg, d);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E000_02: ROXR.B <cnt>, Dx */
static unsigned ope000_02 (e68000_t *c)
{
	uint16_t s, d;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, reg) | (e68_get_sr_x (c) << 8);

	if (n % 9) {
		n = n % 9;
		d = ((s >> n) | (s << (9 - n))) & 0x1ff;
		e68_set_cc (c, E68_SR_XC, d & 0x100);
	}
	else {
		d = s;
		e68_set_sr_c (c, e68_get_sr_x (c));
	}

	e68_set_dreg8 (c, reg, d & 0xff);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d & 0xff);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E000_03: ROR.B <cnt>, Dx */
static unsigned ope000_03 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, reg);

	if (n & 7) {
		n &= 7;
		d = ((s >> n) | (s << (8 - n))) & 0xff;
		e68_set_sr_c (c, s & (1 << (n - 1)));
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (s & 0x80));
	}

	e68_set_dreg8 (c, reg, d);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E000: misc */
static unsigned ope000 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		return (ope000_00 (c));

	case 0x01:
		return (ope000_01 (c));

	case 0x02:
		return (ope000_02 (c));

	case 0x03:
		return (ope000_03 (c));

	default:
		return (e68_op_undefined (c));
	}
}

/* E040_00: ASR.W <cnt>, Dx */
static unsigned ope040_00 (e68000_t *c)
{
	uint16_t s, d;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = c->ir[0] & 7;

	s = e68_get_dreg16 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else if (cnt < 16) {
		if (s & 0x8000) {
			d = (s | 0xffff0000) >> cnt;
		}
		else {
			d = s >> cnt;
		}
		e68_set_cc (c, E68_SR_XC, s & (1U << (cnt - 1)));
	}
	else {
		if (s & 0x8000) {
			d = 0xffff;
			e68_set_cc (c, E68_SR_XC, 1);
		}
		else {
			d = 0x0000;
			e68_set_cc (c, E68_SR_XC, 0);
		}
	}


	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E040_01: LSR.W <cnt>, Dx */
static unsigned ope040_01 (e68000_t *c)
{
	uint16_t s, d;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else if (cnt < 16) {
		d = s >> cnt;
		e68_set_cc (c, E68_SR_XC, s & (1U << (cnt - 1)));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (cnt == 16) && (s & 0x8000));
	}


	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E040_02: ROXR.W <cnt>, Dx */
static unsigned ope040_02 (e68000_t *c)
{
	uint16_t s, d, x;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, reg);
	x = e68_get_sr_x (c);

	if (n % 17) {
		n = n % 17;
		d = ((s >> n) | (x << (16 - n)));
		if (n > 1) {
			d |= s << (17 - n);
		}
		d &= 0xffff;
		e68_set_cc (c, E68_SR_XC, s & (1U << (n - 1)));
	}
	else {
		d = s;
		e68_set_sr_c (c, e68_get_sr_x (c));
	}

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E040_03: ROR.W <cnt>, Dx */
static unsigned ope040_03 (e68000_t *c)
{
	uint16_t s, d;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, reg);

	if (n == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else {
		n &= 15;
		if (n > 0) {
			d = ((s >> n) | (s << (16 - n))) & 0xffff;
			e68_set_cc (c, E68_SR_C, s & (1 << (n - 1)));
		}
		else {
			d = s;
			e68_set_cc (c, E68_SR_C, s & 0x8000);
		}
	}

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E040: misc */
static unsigned ope040 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		return (ope040_00 (c));

	case 0x01:
		return (ope040_01 (c));

	case 0x02:
		return (ope040_02 (c));

	case 0x03:
		return (ope040_03 (c));

	default:
		return (e68_op_undefined (c));
	}

}

/* E080_00: ASR.L <cnt>, Dx */
static unsigned ope080_00 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else if (cnt < 32) {
		if (s & 0x80000000) {
			d = (s >> cnt) | (0xffffffff << (32 - cnt));
		}
		else {
			d = s >> cnt;
		}
		e68_set_cc (c, E68_SR_XC, s & (1UL << (cnt - 1)));
	}
	else {
		if (s & 0x80000000) {
			d = 0xffffffff;
			e68_set_cc (c, E68_SR_XC, 1);
		}
		else {
			d = 0x00000000;
			e68_set_cc (c, E68_SR_XC, 0);
		}
	}


	e68_set_dreg32 (c, r, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E080_01: LSR.L <cnt>, Dx */
static unsigned ope080_01 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else if (cnt < 32) {
		d = s >> cnt;
		e68_set_cc (c, E68_SR_XC, s & (1UL << (cnt - 1)));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (cnt == 32) && (s & 0x80000000));
	}


	e68_set_dreg32 (c, r, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E080_02: ROXR.L <cnt>, Dx */
static unsigned ope080_02 (e68000_t *c)
{
	uint32_t s, d, x;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, reg);
	x = e68_get_sr_x (c);

	if (n % 33) {
		n = n % 33;
		d = ((s >> n) | (x << (32 - n)));
		if (n > 1) {
			d |= s << (33 - n);
		}
		d &= 0xffffffff;
		e68_set_cc (c, E68_SR_XC, s & (1UL << (n - 1)));
	}
	else {
		d = s;
		e68_set_sr_c (c, e68_get_sr_x (c));
	}

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E080_03: ROR.L <cnt>, Dx */
static unsigned ope080_03 (e68000_t *c)
{
	uint32_t s, d;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, reg);

	if (n == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C, 0);
	}
	else {
		n &= 31;
		if (n > 0) {
			d = ((s >> n) | (s << (32 - n))) & 0xffffffff;
			e68_set_cc (c, E68_SR_C, s & (1 << (n - 1)));
		}
		else {
			d = s;
			e68_set_cc (c, E68_SR_C, s & 0x80000000);
		}
	}

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E080: misc */
static unsigned ope080 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		return (ope080_00 (c));

	case 0x01:
		return (ope080_01 (c));

	case 0x02:
		return (ope080_02 (c));

	case 0x03:
		return (ope080_03 (c));

	default:
		return (e68_op_undefined (c));
	}
}

/* E0C0: ASR.W <EA> */
static unsigned ope0c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	if (s & 0x8000) {
		d = (s >> 1) | 0x8000;
	}
	else {
		d = s >> 1;
	}

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_XC, s & 1);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E100_00: ASL.B <cnt>, Dx */
static unsigned ope100_00 (e68000_t *c)
{
	uint8_t  s, d, b;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = c->ir[0] & 7;

	s = e68_get_dreg8 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C | E68_SR_V, 0);
	}
	else if (cnt < 8) {
		d = s << cnt;
		b = (0xff << (7 - cnt)) & 0xff;
		e68_set_cc (c, E68_SR_XC, s & (1 << (8 - cnt)));
		e68_set_sr_v (c, ((s & b) != 0) && ((s & b) != b));
	}
	else {
		d = 0x00;
		e68_set_cc (c, E68_SR_XC, (cnt == 8) && (s & 0x01));
		e68_set_sr_v (c, s != 0);
	}


	e68_set_dreg8 (c, reg, d);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E100_01: LSL.B <cnt>, Dx */
static unsigned ope100_01 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, r);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 8) {
		d = s << n;
		e68_set_cc (c, E68_SR_XC, s & (1 << (8 - n)));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (n == 8) && (s & 1));
	}


	e68_set_dreg8 (c, r, d);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E100_02: ROXL.B <cnt>, Dx */
static unsigned ope100_02 (e68000_t *c)
{
	uint16_t s, d;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, reg) | (e68_get_sr_x (c) << 8);

	if (n % 9) {
		n = n % 9;
		d = ((s << n) | (s >> (9 - n))) & 0x1ff;
		e68_set_cc (c, E68_SR_XC, d & 0x100);
	}
	else {
		d = s;
		e68_set_sr_c (c, e68_get_sr_x (c));
	}

	e68_set_dreg8 (c, reg, d & 0xff);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d & 0xff);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E100_03: ROL.B <cnt>, Dx */
static unsigned ope100_03 (e68000_t *c)
{
	uint8_t  s, d;
	unsigned r, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg8 (c, r);

	if (n & 7) {
		n &= 7;
		d = ((s << n) | (s >> (8 - n))) & 0xff;
		e68_set_sr_c (c, s & (1 << (8 - n)));
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (s & 1));
	}

	e68_set_dreg8 (c, r, d);
	e68_cc_set_nz_8 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E100: misc */
static unsigned ope100 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		return (ope100_00 (c));

	case 0x01:
		return (ope100_01 (c));

	case 0x02:
		return (ope100_02 (c));

	case 0x03:
		return (ope100_03 (c));

	default:
		return (e68_op_undefined (c));
	}
}

/* E140_00: ASL.W <cnt>, Dx */
static unsigned ope140_00 (e68000_t *c)
{
	uint16_t s, d, b;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = c->ir[0] & 7;

	s = e68_get_dreg16 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C | E68_SR_V, 0);
	}
	else if (cnt < 16) {
		d = s << cnt;
		b = (0xffff << (15 - cnt)) & 0xffff;
		e68_set_cc (c, E68_SR_XC, s & (1U << (16 - cnt)));
		e68_set_sr_v (c, ((s & b) != 0) && ((s & b) != b));
	}
	else {
		d = 0x0000;
		e68_set_cc (c, E68_SR_XC, (cnt == 16) && (s & 0x0001));
		e68_set_sr_v (c, s != 0);
	}


	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E140_01: LSL.W <cnt>, Dx */
static unsigned ope140_01 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, r);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 16) {
		d = s << n;
		e68_set_cc (c, E68_SR_XC, s & (1U << (16 - n)));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (n == 16) && (s & 1));
	}


	e68_set_dreg16 (c, r, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E140_02: ROXL.W <cnt>, Dx */
static unsigned ope140_02 (e68000_t *c)
{
	uint16_t s, d, x;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, reg);
	x = e68_get_sr_x (c);

	if (n % 17) {
		n = n % 17;
		d = ((s << n) | (x << (n - 1)));
		if (n > 1) {
			d |= s >> (17 - n);
		}
		d &= 0xffff;
		e68_set_cc (c, E68_SR_XC, s & (1U << (16 - n)));
	}
	else {
		d = s;
		e68_set_sr_c (c, e68_get_sr_x (c));
	}

	e68_set_dreg16 (c, reg, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E140_03: ROL.W <cnt>, Dx */
static unsigned ope140_03 (e68000_t *c)
{
	uint16_t s, d;
	unsigned r, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg16 (c, r);

	if (n & 15) {
		n &= 15;
		d = ((s << n) | (s >> (16 - n))) & 0xffff;
		e68_set_sr_c (c, s & (1U << (16 - n)));
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (s & 1));
	}

	e68_set_dreg16 (c, r, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E140: misc */
static unsigned ope140 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		return (ope140_00 (c));

	case 0x01:
		return (ope140_01 (c));

	case 0x02:
		return (ope140_02 (c));

	case 0x03:
		return (ope140_03 (c));

	default:
		return (e68_op_undefined (c));
	}
}

/* E180_00: ASL.L <cnt>, Dx */
static unsigned ope180_00 (e68000_t *c)
{
	uint32_t s, d, b;
	unsigned reg, cnt;

	if (c->ir[0] & 0x20) {
		cnt = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		cnt = e68_ir_reg9 (c);
		cnt = (cnt == 0) ? 8 : cnt;
	}

	reg = c->ir[0] & 7;

	s = e68_get_dreg32 (c, reg);

	if (cnt == 0) {
		d = s;
		e68_set_cc (c, E68_SR_C | E68_SR_V, 0);
	}
	else if (cnt < 32) {
		d = s << cnt;
		b = (0xffffffff << (31 - cnt)) & 0xffffffff;
		e68_set_cc (c, E68_SR_XC, s & (1UL << (32 - cnt)));
		e68_set_sr_v (c, ((s & b) != 0) && ((s & b) != b));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (cnt == 32) && (s & 1));
		e68_set_sr_v (c, s != 0);
	}


	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E180_01: LSL.L <cnt>, Dx */
static unsigned ope180_01 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	if (n == 0) {
		d = s;
		e68_set_sr_c (c, 0);
	}
	else if (n < 32) {
		d = s << n;
		e68_set_cc (c, E68_SR_XC, s & (1UL << (32 - n)));
	}
	else {
		d = 0;
		e68_set_cc (c, E68_SR_XC, (n == 32) && (s & 1));
	}


	e68_set_dreg32 (c, r, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E180_02: ROXL.W <cnt>, Dx */
static unsigned ope180_02 (e68000_t *c)
{
	uint32_t s, d, x;
	unsigned reg, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	reg = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, reg);
	x = e68_get_sr_x (c);

	if (n % 33) {
		n = n % 33;
		d = ((s << n) | (x << (n - 1)));
		if (n > 1) {
			d |= s >> (33 - n);
		}
		d &= 0xffffffff;
		e68_set_cc (c, E68_SR_XC, s & (1UL << (32 - n)));
	}
	else {
		d = s;
		e68_set_sr_c (c, e68_get_sr_x (c));
	}

	e68_set_dreg32 (c, reg, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E180_03: ROL.L <cnt>, Dx */
static unsigned ope180_03 (e68000_t *c)
{
	uint32_t s, d;
	unsigned r, n;

	if (c->ir[0] & 0x20) {
		n = e68_get_dreg8 (c, e68_ir_reg9 (c)) & 0x3f;
	}
	else {
		n = e68_ir_reg9 (c);
		n = (n == 0) ? 8 : n;
	}

	r = e68_ir_reg0 (c);
	s = e68_get_dreg32 (c, r);

	if (n & 31) {
		n &= 31;
		d = ((s << n) | (s >> (32 - n))) & 0xffffffff;
		e68_set_sr_c (c, s & (1UL << (32 - n)));
	}
	else {
		d = s;
		e68_set_sr_c (c, (n != 0) && (s & 1));
	}

	e68_set_dreg32 (c, r, d);
	e68_cc_set_nz_32 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E180: misc */
static unsigned ope180 (e68000_t *c)
{
	switch ((c->ir[0] >> 3) & 3) {
	case 0x00:
		return (ope180_00 (c));

	case 0x01:
		return (ope180_01 (c));

	case 0x02:
		return (ope180_02 (c));

	case 0x03:
		return (ope180_03 (c));

	default:
		return (e68_op_undefined (c));
	}
}

/* E1C0: ASL.W <EA> */
static unsigned ope1c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = s << 1;

	e68_set_cc (c, E68_SR_XC, s & 0x8000);
	e68_set_sr_v (c, (s ^ d) & 0x8000);

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z, d);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E2C0: LSR.W <EA> */
static unsigned ope2c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = s >> 1;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_XC, s & 1);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E3C0: LSL.W <EA> */
static unsigned ope3c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = s << 1;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_XC, s & 0x8000);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E4C0: ROXR.W <EA> */
static unsigned ope4c0 (e68000_t *c)
{
	uint16_t s, d, x;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);
	x = e68_get_sr_x (c);

	d = ((s >> 1) | (x << 15)) & 0xffff;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_XC, s & 1);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E5C0: ROXL.W <EA> */
static unsigned ope5c0 (e68000_t *c)
{
	uint16_t s, d, x;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);
	x = e68_get_sr_x (c);

	d = ((s << 1) | x) & 0xffff;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_XC, s & 0x8000);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E6C0: ROR.W <EA> */
static unsigned ope6c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = ((s >> 1) | (s << 15)) & 0xffff;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_C, s & 1);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* E7C0: ROL.W <EA> */
static unsigned ope7c0 (e68000_t *c)
{
	uint16_t s, d;

	e68_get_ea16 (c, 1, e68_ir_ea1 (c), 0x01fc, &s);

	d = ((s << 1) || (s >> 15)) & 0xffff;

	e68_set_ea16 (c, 0, 0, 0, d);
	e68_cc_set_nz_16 (c, E68_SR_N | E68_SR_Z | E68_SR_V, d);
	e68_set_cc (c, E68_SR_C, s & 0x8000);
	e68_set_clk (c, 1);

	return (c->ircnt);
}

/* F000: FXXX */
static unsigned opf000 (e68000_t *c)
{
	e68_exception_fxxx (c);
	return (0);
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
	  NULL,   NULL,   NULL,   NULL, op0100, op0140, op0180, op01c0, /* 0E00 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1000 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1200 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1400 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1600 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1800 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1A00 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1C00 */
	op1000, op1000, op1000, op1000, op1000, op1000, op1000, op1000, /* 1E00 */
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
	op4200, op4240, op4280,   NULL,   NULL,   NULL, op4180, op41c0, /* 4200 */
	op4400, op4440, op4480, op44c0,   NULL,   NULL, op4180, op41c0, /* 4400 */
	op4600, op4640, op4680, op46c0,   NULL,   NULL, op4180, op41c0, /* 4600 */
	op4800, op4840, op4880, op48c0,   NULL,   NULL, op4180, op41c0, /* 4800 */
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
}
