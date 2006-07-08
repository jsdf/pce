/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e68000/cc.c                                        *
 * Created:       2006-05-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-05-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2005-2006 Hampa Hug <hampa@hampa.ch>                   *
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


void e68_cc_set_nz_8 (e68000_t *c, uint8_t msk, uint8_t val)
{
	uint16_t set = 0;

	if ((val & 0xff) == 0) {
		set |= E68_SR_Z;
	}

	if (val & 0x80) {
		set |= E68_SR_N;
	}

	c->sr &= (0xff00 | ~msk);
	c->sr |= (set & msk);
}

void e68_cc_set_nz_16 (e68000_t *c, uint8_t msk, uint16_t val)
{
	uint16_t set = 0;

	if ((val & 0xffff) == 0) {
		set |= E68_SR_Z;
	}

	if (val & 0x8000) {
		set |= E68_SR_N;
	}

	c->sr &= (0xff00 | ~msk);
	c->sr |= (set & msk);
}

void e68_cc_set_nz_32 (e68000_t *c, uint8_t msk, uint32_t val)
{
	uint16_t set = 0;

	if ((val & 0xffffffff) == 0) {
		set |= E68_SR_Z;
	}

	if (val & 0x80000000) {
		set |= E68_SR_N;
	}

	c->sr &= (0xff00 | ~msk);
	c->sr |= (set & msk);
}

void e68_cc_set_add_8 (e68000_t *c, uint8_t s1, uint8_t s2)
{
	uint8_t d;

	d = (s1 + s2) & 0xff;

	e68_cc_set_nz_8 (c, E68_SR_XNZVC, d);

	if (d < s1) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((d ^ s1) & (d ^ s2) & 0x80) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_add_16 (e68000_t *c, uint16_t s1, uint16_t s2)
{
	uint16_t d;

	d = (s1 + s2) & 0xffff;

	e68_cc_set_nz_16 (c, E68_SR_XNZVC, d);

	if (d < s1) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((d ^ s1) & (d ^ s2) & 0x8000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_add_32 (e68000_t *c, uint32_t s1, uint32_t s2)
{
	uint32_t d;

	d = (s1 + s2) & 0xffffffff;

	e68_cc_set_nz_32 (c, E68_SR_XNZVC, d);

	if (d < s1) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((d ^ s1) & (d ^ s2) & 0x80000000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_addx_8 (e68000_t *c, uint8_t s1, uint8_t s2, uint8_t s3)
{
	uint8_t d;

	d = (s1 + s2 + s3) & 0xff;

	e68_cc_set_nz_8 (c, E68_SR_XNZVC, d);

	if ((d < s1) || ((d == s1) && s3)) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((d ^ s1) & (d ^ s2) & 0x80) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_addx_16 (e68000_t *c, uint16_t s1, uint16_t s2, uint16_t s3)
{
	uint16_t d;

	d = (s1 + s2 + s3) & 0xffff;

	e68_cc_set_nz_16 (c, E68_SR_XNZVC, d);

	if ((d < s1) || ((d == s1) && s3)) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((d ^ s1) & (d ^ s2) & 0x8000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_addx_32 (e68000_t *c, uint32_t s1, uint32_t s2, uint32_t s3)
{
	uint32_t d;

	d = (s1 + s2 + s3) & 0xffffffff;

	e68_cc_set_nz_32 (c, E68_SR_XNZVC, d);

	if ((d < s1) || ((d == s1) && s3)) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((d ^ s1) & (d ^ s2) & 0x80000000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_cmp_8 (e68000_t *c, uint8_t s1, uint8_t s2)
{
	uint8_t d;

	d = (s1 - s2) & 0xff;

	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);

	if (s1 < s2) {
		c->sr |= E68_SR_C;
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_cmp_16 (e68000_t *c, uint16_t s1, uint16_t s2)
{
	uint16_t d;

	d = (s1 - s2) & 0xffff;

	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);

	if (s1 < s2) {
		c->sr |= E68_SR_C;
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x8000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_cmp_32 (e68000_t *c, uint32_t s1, uint32_t s2)
{
	uint32_t d;

	d = (s1 - s2) & 0xffffffff;

	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);

	if (s1 < s2) {
		c->sr |= E68_SR_C;
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80000000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_sub_8 (e68000_t *c, uint8_t s1, uint8_t s2)
{
	uint8_t d;

	d = (s1 - s2) & 0xff;

	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);

	if (s1 < s2) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_sub_16 (e68000_t *c, uint16_t s1, uint16_t s2)
{
	uint16_t d;

	d = (s1 - s2) & 0xffff;

	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);

	if (s1 < s2) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x8000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_sub_32 (e68000_t *c, uint32_t s1, uint32_t s2)
{
	uint32_t d;

	d = (s1 - s2) & 0xffffffff;

	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);

	if (s1 < s2) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80000000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_subx_8 (e68000_t *c, uint8_t s1, uint8_t s2, uint8_t s3)
{
	uint8_t d;

	d = (s1 - s2 - s3) & 0xff;

	e68_cc_set_nz_8 (c, E68_SR_NZVC, d);

	if ((s1 < s2) || ((s1 == s2) && s3)) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_subx_16 (e68000_t *c, uint16_t s1, uint16_t s2, uint16_t s3)
{
	uint16_t d;

	d = (s1 - s2 - s3) & 0xffff;

	e68_cc_set_nz_16 (c, E68_SR_NZVC, d);

	if ((s1 < s2) || ((s1 == s2) && s3)) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x8000) {
		c->sr |= E68_SR_V;
	}
}

void e68_cc_set_subx_32 (e68000_t *c, uint32_t s1, uint32_t s2, uint32_t s3)
{
	uint32_t d;

	d = (s1 - s2 - s3) & 0xffffffff;

	e68_cc_set_nz_32 (c, E68_SR_NZVC, d);

	if ((s1 < s2) || ((s1 == s2) && s3)) {
		c->sr |= (E68_SR_X | E68_SR_C);
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80000000) {
		c->sr |= E68_SR_V;
	}
}
