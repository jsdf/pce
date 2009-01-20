/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e68000/cc.c                                          *
 * Created:     2006-05-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2009 Hampa Hug <hampa@hampa.ch>                     *
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


void e68_cc_set_nz_8 (e68000_t *c, uint8_t msk, uint8_t val)
{
	uint16_t set = 0;

	if ((val & 0xff) == 0) {
		set |= E68_SR_Z;
	}
	else if (val & 0x80) {
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
	else if (val & 0x8000) {
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
	else if (val & 0x80000000) {
		set |= E68_SR_N;
	}

	c->sr &= (0xff00 | ~msk);
	c->sr |= (set & msk);
}

/*
 * Set XNVC after addition
 *
 * c = (s1 & s2) | (~d & s1) | (~d & s2)
 * v = (~d & s1 & s2) | (d & ~s1 & ~s2)
 */
static inline
void e68_cc_set_add (e68000_t *c, unsigned d, unsigned s1, unsigned s2)
{
	uint16_t set = 0;

	d &= 1;
	s1 &= 1;
	s2 &= 1;

	if (d) {
		set |= E68_SR_N;

		if (s1 && s2) {
			set |= E68_SR_C | E68_SR_X;
		}

		if (!(s1 || s2)) {
			set |= E68_SR_V;
		}
	}
	else {
		if (s1 || s2) {
			set |= E68_SR_C | E68_SR_X;
		}

		if (s1 && s2) {
			set |= E68_SR_V;
		}
	}

	c->sr &= ~(E68_SR_X | E68_SR_N | E68_SR_V | E68_SR_C);
	c->sr |= set;
}

void e68_cc_set_add_8 (e68000_t *c, uint8_t d, uint8_t s1, uint8_t s2)
{
	e68_set_sr_z (c, (d & 0xff) == 0);
	e68_cc_set_add (c, d >> 7, s1 >> 7, s2 >> 7);
}

void e68_cc_set_add_16 (e68000_t *c, uint16_t d, uint16_t s1, uint16_t s2)
{
	e68_set_sr_z (c, (d & 0xffff) == 0);
	e68_cc_set_add (c, d >> 15, s1 >> 15, s2 >> 15);
}

void e68_cc_set_add_32 (e68000_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	e68_set_sr_z (c, (d & 0xffffffff) == 0);
	e68_cc_set_add (c, d >> 31, s1 >> 31, s2 >> 31);
}

void e68_cc_set_addx_8 (e68000_t *c, uint8_t d, uint8_t s1, uint8_t s2)
{
	e68_cc_set_add (c, d >> 7, s1 >> 7, s2 >> 7);

	if (d & 0xff) {
		e68_set_sr_z (c, 0);
	}
}

void e68_cc_set_addx_16 (e68000_t *c, uint16_t d, uint16_t s1, uint16_t s2)
{
	e68_cc_set_add (c, d >> 15, s1 >> 15, s2 >> 15);

	if (d & 0xffff) {
		e68_set_sr_z (c, 0);
	}
}

void e68_cc_set_addx_32 (e68000_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	e68_cc_set_add (c, d >> 31, s1 >> 31, s2 >> 31);

	if (d & 0xffffffff) {
		e68_set_sr_z (c, 0);
	}
}


/*
 * Set NVC after subtraction (s2 - s1)
 *
 * c = (s1 & ~s2) | (d & ~s2) | (d & s1)
 * v = (~d & ~s1 & s2) | (d & s1 & ~s2)
 */
static inline
void e68_cc_set_sub (e68000_t *c, uint16_t msk, unsigned d, unsigned s1, unsigned s2)
{
	uint16_t set = 0;

	d &= 1;
	s1 &= 1;
	s2 &= 1;

	if (d) {
		set |= E68_SR_N;

		if (s1 || !s2) {
			set |= E68_SR_C | E68_SR_X;
		}

		if (s1 && !s2) {
			set |= E68_SR_V;
		}
	}
	else {
		if (s1 && !s2) {
			set |= E68_SR_C | E68_SR_X;
		}

		if (!s1 && s2) {
			set |= E68_SR_V;
		}
	}

	c->sr &= ~msk;
	c->sr |= (set & msk);
}

void e68_cc_set_cmp_8 (e68000_t *c, uint8_t d, uint8_t s1, uint8_t s2)
{
	e68_cc_set_sub (c, E68_SR_NZVC, d >> 7, s1 >> 7, s2 >> 7);

	if ((d & 0xff) == 0) {
		c->sr |= E68_SR_Z;
	}
}

void e68_cc_set_cmp_16 (e68000_t *c, uint16_t d, uint16_t s1, uint16_t s2)
{
	e68_cc_set_sub (c, E68_SR_NZVC, d >> 15, s1 >> 15, s2 >> 15);

	if ((d & 0xffff) == 0) {
		c->sr |= E68_SR_Z;
	}
}

void e68_cc_set_cmp_32 (e68000_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	e68_cc_set_sub (c, E68_SR_NZVC, d >> 31, s1 >> 31, s2 >> 31);

	if ((d & 0xffffffff) == 0) {
		c->sr |= E68_SR_Z;
	}
}

void e68_cc_set_sub_8 (e68000_t *c, uint8_t d, uint8_t s1, uint8_t s2)
{
	e68_cc_set_sub (c, E68_SR_XNZVC, d >> 7, s1 >> 7, s2 >> 7);

	if ((d & 0xff) == 0) {
		c->sr |= E68_SR_Z;
	}
}

void e68_cc_set_sub_16 (e68000_t *c, uint16_t d, uint16_t s1, uint16_t s2)
{
	e68_cc_set_sub (c, E68_SR_XNZVC, d >> 15, s1 >> 15, s2 >> 15);

	if ((d & 0xffff) == 0) {
		c->sr |= E68_SR_Z;
	}
}

void e68_cc_set_sub_32 (e68000_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	e68_cc_set_sub (c, E68_SR_XNZVC, d >> 31, s1 >> 31, s2 >> 31);

	if ((d & 0xffffffff) == 0) {
		c->sr |= E68_SR_Z;
	}
}

void e68_cc_set_subx_8 (e68000_t *c, uint8_t d, uint8_t s1, uint8_t s2)
{
	e68_cc_set_sub (c, E68_SR_XNVC, d >> 7, s1 >> 7, s2 >> 7);

	if (d & 0xff) {
		c->sr &= ~E68_SR_Z;
	}
}

void e68_cc_set_subx_16 (e68000_t *c, uint16_t d, uint16_t s1, uint16_t s2)
{
	e68_cc_set_sub (c, E68_SR_XNVC, d >> 15, s1 >> 15, s2 >> 15);

	if (d & 0xffff) {
		c->sr &= ~E68_SR_Z;
	}
}

void e68_cc_set_subx_32 (e68000_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
	e68_cc_set_sub (c, E68_SR_XNVC, d >> 31, s1 >> 31, s2 >> 31);

	if (d & 0xffffffff) {
		c->sr &= ~E68_SR_Z;
	}
}
