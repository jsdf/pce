/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/flags.c                                        *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


static
char parity[256] = {
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


/*************************************************************************
 * Flags functions
 *************************************************************************/

void e86_set_flg_szp_8 (e8086_t *c, unsigned char val)
{
	unsigned short set;

	set = 0;

	val &= 0xff;

	if (val == 0) {
		set |= E86_FLG_Z;
	}
	else if (val & 0x80) {
		set |= E86_FLG_S;
	}

	if (parity[val] == 0) {
		set |= E86_FLG_P;
	}

	c->flg &= ~(E86_FLG_S | E86_FLG_Z | E86_FLG_P);
	c->flg |= set;
}

void e86_set_flg_szp_16 (e8086_t *c, unsigned short val)
{
	unsigned short set;

	set = 0;

	if ((val & 0xffff) == 0) {
		set |= E86_FLG_Z;
	}
	else if (val & 0x8000) {
		set |= E86_FLG_S;
	}

	if (parity[val & 0xff] == 0) {
		set |= E86_FLG_P;
	}

	c->flg &= ~(E86_FLG_S | E86_FLG_Z | E86_FLG_P);
	c->flg |= set;
}

void e86_set_flg_log_8 (e8086_t *c, unsigned char val)
{
	e86_set_flg_szp_8 (c, val);

	c->flg &= ~(E86_FLG_C | E86_FLG_O);
}

void e86_set_flg_log_16 (e8086_t *c, unsigned short val)
{
	e86_set_flg_szp_16 (c, val);

	c->flg &= ~(E86_FLG_C | E86_FLG_O);
}

void e86_set_flg_add_8 (e8086_t *c, unsigned char s1, unsigned char s2)
{
	unsigned short set;
	unsigned short dst;

	e86_set_flg_szp_8 (c, s1 + s2);

	set = 0;

	dst = (unsigned short) s1 + (unsigned short) s2;

	if (dst & 0xff00) {
		set |= E86_FLG_C;
	}

	if ((dst ^ s1) & (dst ^ s2) & 0x80) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_add_16 (e8086_t *c, unsigned short s1, unsigned short s2)
{
	unsigned short set;
	unsigned long  dst;

	e86_set_flg_szp_16 (c, s1 + s2);

	set = 0;

	dst = (unsigned long) s1 + (unsigned long) s2;

	if (dst & 0xffff0000) {
		set |= E86_FLG_C;
	}

	if ((dst ^ s1) & (dst ^ s2) & 0x8000) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_adc_8 (e8086_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
	unsigned short set;
	unsigned short dst;

	e86_set_flg_szp_8 (c, s1 + s2 + s3);

	set = 0;

	dst = (unsigned short) s1 + (unsigned short) s2 + (unsigned short) s3;

	if (dst & 0xff00) {
		set |= E86_FLG_C;
	}

	if ((dst ^ s1) & (dst ^ s2) & 0x80) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_adc_16 (e8086_t *c, unsigned short s1, unsigned short s2, unsigned short s3)
{
	unsigned short set;
	unsigned long  dst;

	e86_set_flg_szp_16 (c, s1 + s2 + s3);

	set = 0;

	dst = (unsigned long) s1 + (unsigned long) s2 + (unsigned long) s3;

	if (dst & 0xffff0000) {
		set |= E86_FLG_C;
	}

	if ((dst ^ s1) & (dst ^ s2) & 0x8000) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_sbb_8 (e8086_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
	unsigned short set;
	unsigned short dst;

	e86_set_flg_szp_8 (c, s1 - s2 - s3);

	set = 0;

	dst = s1 - s2 - s3;

	if (dst & 0xff00) {
		set |= E86_FLG_C;
	}

	if ((s1 ^ dst) & (s1 ^ s2) & 0x80) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_sbb_16 (e8086_t *c, unsigned short s1, unsigned short s2, unsigned short s3)
{
	unsigned short set;
	unsigned long  dst;

	e86_set_flg_szp_16 (c, s1 - s2 - s3);

	set = 0;

	dst = (unsigned long) s1 - (unsigned long) s2 - (unsigned long) s3;

	if (dst & 0xffff0000) {
		set |= E86_FLG_C;
	}

	if ((s1 ^ dst) & (s1 ^ s2) & 0x8000) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_sub_8 (e8086_t *c, unsigned char s1, unsigned char s2)
{
	unsigned short set;
	unsigned short dst;

	e86_set_flg_szp_8 (c, s1 - s2);

	set = 0;

	dst = s1 - s2;

	if (dst & 0xff00) {
		set |= E86_FLG_C;
	}

	if ((s1 ^ dst) & (s1 ^ s2) & 0x80) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}

void e86_set_flg_sub_16 (e8086_t *c, unsigned short s1, unsigned short s2)
{
	unsigned short set;
	unsigned long  dst;

	e86_set_flg_szp_16 (c, s1 - s2);

	set = 0;

	dst = (unsigned long) s1 - (unsigned long) s2;

	if (dst & 0xffff0000) {
		set |= E86_FLG_C;
	}

	if ((s1 ^ dst) & (s1 ^ s2) & 0x8000) {
		set |= E86_FLG_O;
	}

	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_A;
	}

	c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
	c->flg |= set;
}
