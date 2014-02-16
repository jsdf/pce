/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/flags.c                                        *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2014 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e8080.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


static char parity[256] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};


static inline
void set_psw_sz (e8080_t *c, unsigned char val)
{
	val &= 0xff;

	if (val == 0) {
		c->psw |= E8080_FLG_Z;
	}
	else if (val & 0x80) {
		c->psw |= E8080_FLG_S;
	}
}

static inline
void set_psw_szp (e8080_t *c, unsigned char val)
{
	val &= 0xff;

	if (val == 0) {
		c->psw |= E8080_FLG_Z;
	}
	else if (val & 0x80) {
		c->psw |= E8080_FLG_S;
	}

	if (parity[val] == 0) {
		c->psw |= E8080_FLG_P;
	}
}

void e8080_set_psw_szp (e8080_t *c, unsigned char val, unsigned set, unsigned reset)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_P | reset);
	set_psw_szp (c, val);
	c->psw |= set;
}

void e8080_set_psw_log (e8080_t *c, unsigned char val)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_P | E8080_FLG_C);

	set_psw_szp (c, val);
}

void e8080_set_psw_inc (e8080_t *c, unsigned char val)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P);

	set_psw_szp (c, val + 1);

	if ((val ^ (val + 1)) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void e8080_set_psw_dec (e8080_t *c, unsigned char val)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P);

	set_psw_szp (c, val - 1);

	if ((val ^ (val - 1)) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void e8080_set_psw_add (e8080_t *c, unsigned char s1, unsigned char s2)
{
	unsigned d;

	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_C);

	d = (unsigned) s1 + (unsigned) s2;

	set_psw_szp (c, d);

	if (d > 255) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ s2 ^ d) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void e8080_set_psw_sub (e8080_t *c, unsigned char s1, unsigned char s2)
{
	unsigned d;

	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_C);

	d = (unsigned) s1 - (unsigned) s2;

	set_psw_szp (c, d);

	if (d > 255) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ s2 ^ d) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void e8080_set_psw_adc (e8080_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
	unsigned d;

	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_C);

	d = (unsigned) s1 + (unsigned) s2 + (unsigned) s3;

	set_psw_szp (c, d);

	if (d > 255) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ s2 ^ d) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void e8080_set_psw_sbb (e8080_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
	unsigned d;

	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_C);

	d = (unsigned) s1 - (unsigned) s2 - (unsigned) s3;

	set_psw_szp (c, d);

	if (d > 255) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ s2 ^ d) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void z80_set_psw_rot (e8080_t *c, unsigned char val, int cf)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_N | E8080_FLG_C);

	set_psw_szp (c, val);

	if (cf) {
		c->psw |= E8080_FLG_C;
	}
}

void z80_set_psw_inc (e8080_t *c, unsigned char val)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_N);

	set_psw_sz (c, val + 1);

	if ((val ^ (val + 1)) & 0x10) {
		c->psw |= E8080_FLG_A;
	}

	if (val == 0x7f) {
		c->psw |= E8080_FLG_P;
	}
}

void z80_set_psw_dec (e8080_t *c, unsigned char val)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P);

	set_psw_sz (c, val - 1);

	if ((val ^ (val - 1)) & 0x10) {
		c->psw |= E8080_FLG_A;
	}

	if (val == 0x80) {
		c->psw |= E8080_FLG_P;
	}

	c->psw |= E8080_FLG_N;
}

void z80_set_psw_add (e8080_t *c, unsigned d, unsigned char s1, unsigned char s2)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_N | E8080_FLG_C);

	set_psw_sz (c, d);

	if (d > 255) {
		c->psw |= E8080_FLG_C;
	}

	if ((d ^ s1) & (d ^ s2) & 0x80) {
		c->psw |= E8080_FLG_P;
	}

	if ((s1 ^ s2 ^ d) & 0x10) {
		c->psw |= E8080_FLG_A;
	}
}

void z80_set_psw_add16 (e8080_t *c, unsigned long d, unsigned s1, unsigned s2)
{
	c->psw &= ~(E8080_FLG_A | E8080_FLG_N | E8080_FLG_C);

	if (d > 65535) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ s2 ^ d) & 0x1000) {
		c->psw |= E8080_FLG_A;
	}
}

void z80_set_psw_add16_2 (e8080_t *c, unsigned long d, unsigned s1, unsigned s2)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_N | E8080_FLG_C);

	if (d & 0x8000) {
		c->psw |= E8080_FLG_S;
	}

	if ((d & 0xffff) == 0) {
		c->psw |= E8080_FLG_Z;
	}

	if (d > 65535) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x8000) {
		c->psw |= E8080_FLG_P;
	}

	if ((s1 ^ s2 ^ d) & 0x1000) {
		c->psw |= E8080_FLG_A;
	}
}

void z80_set_psw_sub (e8080_t *c, unsigned d, unsigned char s1, unsigned char s2)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_C);

	set_psw_sz (c, d);

	if (d > 255) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x80) {
		c->psw |= E8080_FLG_P;
	}

	if ((s1 ^ s2 ^ d) & 0x10) {
		c->psw |= E8080_FLG_A;
	}

	c->psw |= E8080_FLG_N;
}

void z80_set_psw_sub16_2 (e8080_t *c, unsigned long d, unsigned s1, unsigned s2)
{
	c->psw &= ~(E8080_FLG_S | E8080_FLG_Z | E8080_FLG_A | E8080_FLG_P | E8080_FLG_C);

	if (d & 0x8000) {
		c->psw |= E8080_FLG_S;
	}

	if ((d & 0xffff) == 0) {
		c->psw |= E8080_FLG_Z;
	}

	if (d > 65535) {
		c->psw |= E8080_FLG_C;
	}

	if ((s1 ^ d) & (s1 ^ s2) & 0x8000) {
		c->psw |= E8080_FLG_P;
	}

	if ((s1 ^ s2 ^ d) & 0x1000) {
		c->psw |= E8080_FLG_A;
	}

	c->psw |= E8080_FLG_N;
}
