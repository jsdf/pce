/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6845.c                                          *
 * Created:     2017-08-07 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017 Hampa Hug <hampa@hampa.ch>                          *
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


#include <stdlib.h>
#include <stdio.h>

#include "e6845.h"


#ifndef DEBUG_CRTC
#define DEBUG_CRTC 0
#endif


void e6845_init (e6845_t *crt)
{
	unsigned i;

	crt->ccol = 0;
	crt->crow = 0;
	crt->frame = 0;
	crt->ma = 0;
	crt->ra = 0;
	crt->vsync_cnt = 0;
	crt->index = 0;

	for (i = 0; i < E6845_REG_CNT; i++) {
		crt->reg[i] = 0;
	}

	crt->hsync_ext = NULL;
	crt->hsync_fct = NULL;

	crt->vsync_ext = NULL;
	crt->vsync_fct = NULL;
}

void e6845_free (e6845_t *crt)
{
}

void e6845_set_hsync_fct (e6845_t *crt, void *ext, void *fct)
{
	crt->hsync_ext = ext;
	crt->hsync_fct = fct;
}

void e6845_set_vsync_fct (e6845_t *crt, void *ext, void *fct)
{
	crt->vsync_ext = ext;
	crt->vsync_fct = fct;
}

unsigned e6845_get_start_address (const e6845_t *crt)
{
	unsigned val;

	val = crt->reg[E6845_REG_AH];
	val = (val << 8) | crt->reg[E6845_REG_AL];

	return (val & 0x3fff);
}

unsigned e6845_get_cursor_address (const e6845_t *crt)
{
	unsigned val;

	val = crt->reg[E6845_REG_CH];
	val = (val << 8) | crt->reg[E6845_REG_CL];

	return (val & 0x3fff);
}

unsigned e6845_get_cursor_mask (e6845_t *crt, int blink)
{
	unsigned mask;

	mask = 0;

	switch ((crt->reg[E6845_REG_CS] >> 5) & 3) {
	case 0:
		/* non-blink */
		blink = 0;
		break;

	case 1:
		/* non-display */
		return (0);

	case 2:
		/* blink at 1/16 field rate */
		mask = 16;
		break;

	case 3:
		/* blink 1/32 field rate */
		mask = 32;
		break;
	}

	if (blink) {
		if (crt->frame & mask) {
			return (0);
		}
	}

	if (crt->ra < (crt->reg[E6845_REG_CS] & 0x1f)) {
		return (0);
	}

	if (crt->ra > (crt->reg[E6845_REG_CE] & 0x1f)) {
		return (0);
	}

	return (~0U);
}

unsigned e6845_get_vdl (const e6845_t *crt)
{
	unsigned val;

	val = (crt->reg[E6845_REG_ML] & 0x1f) + 1;
	val *= crt->reg[E6845_REG_VD] & 0x7f;

	return (val);
}

unsigned e6845_get_vtl (const e6845_t *crt)
{
	unsigned val;

	val = (crt->reg[E6845_REG_ML] & 0x1f) + 1;
	val *= (crt->reg[E6845_REG_VT] & 0x7f) + 1;
	val += crt->reg[E6845_REG_VA] & 0x1f;

	return (val);
}

int e6845_get_hde (const e6845_t *crt)
{
	if (crt->ccol < crt->reg[E6845_REG_HD]) {
		return (1);
	}

	return (0);
}

int e6845_get_vde (const e6845_t *crt)
{
	if (crt->crow < (crt->reg[E6845_REG_VD] & 0x7f)) {
		return (1);
	}

	return (0);
}

int e6845_get_de (const e6845_t *crt)
{
	if (crt->crow >= (crt->reg[E6845_REG_VD] & 0x7f)) {
		return (0);
	}

	if (crt->ccol >= crt->reg[E6845_REG_HD]) {
		return (0);
	}

	return (1);
}

void e6845_set_pen (e6845_t *crt)
{
	unsigned addr;

	addr = (crt->ma + crt->ccol) & 0x3fff;

	crt->reg[E6845_REG_LH] = (addr >> 8) & 0xff;
	crt->reg[E6845_REG_LL] = addr & 0xff;
}

unsigned char e6845_get_index (const e6845_t *crt)
{
	return (crt->index);
}

unsigned char e6845_get_data (e6845_t *crt)
{
	if ((crt->index >= 12) && (crt->index <= 17)) {
		return (crt->reg[crt->index]);
	}

	return (0);
}

unsigned char e6845_get_uint8 (e6845_t *crt, unsigned long addr)
{
	switch (addr) {
	case 0:
		return (e6845_get_index (crt));

	case 1:
		return (e6845_get_data (crt));
	}

	return (0);
}

unsigned short e6845_get_uint16 (e6845_t *crt, unsigned long addr)
{
	return (0);
}

void e6845_set_index (e6845_t *crt, unsigned char val)
{
	crt->index = val & 0x1f;
}

void e6845_set_data (e6845_t *crt, unsigned char val)
{
	if (crt->index >= E6845_REG_CNT) {
		return;
	}

	crt->reg[crt->index] = val;
}

void e6845_set_uint8 (e6845_t *crt, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0:
		e6845_set_index (crt, val);
		break;

	case 1:
		e6845_set_data (crt, val);
		break;
	}
}

void e6850_set_uint16 (e6845_t *crt, unsigned long addr, unsigned short val)
{
}

void e6845_reset (e6845_t *crt)
{
	unsigned i;

	crt->ccol = 0;
	crt->crow = 0;
	crt->frame = 0;
	crt->ma = 0;
	crt->ra = 0;
	crt->vsync_cnt = 0;
	crt->index = 0;

	for (i = 0; i < E6845_REG_CNT; i++) {
		crt->reg[i] = 0;
	}
}

static
void e6845_hsync (e6845_t *crt)
{
	if (crt->hsync_fct == NULL) {
		return;
	}

	crt->hsync_fct (crt->hsync_ext);
}

static
void e6845_vsync (e6845_t *crt)
{
	if (crt->vsync_fct == NULL) {
		return;
	}

	crt->vsync_fct (crt->vsync_ext);
}

void e6845_clock (e6845_t *crt, unsigned cnt)
{
	unsigned char hs, ht, vs, vt, va;

	hs = crt->reg[E6845_REG_HS];
	ht = crt->reg[E6845_REG_HT] + 1;

	vs = crt->reg[E6845_REG_VS];
	vt = crt->reg[E6845_REG_VT] + 1;
	va = crt->reg[E6845_REG_VA];

	while (cnt > 0) {
		crt->ccol += 1;
		cnt -= 1;

		if (crt->ccol == hs) {
			crt->hsync_cnt = crt->reg[E6845_REG_SW] & 15;
			e6845_hsync (crt);
		}
		else if (crt->hsync_cnt > 0) {
			crt->hsync_cnt -= 1;
		}


		if (crt->ccol >= ht) {
			crt->ccol = 0;

			if (crt->vsync_cnt > 0) {
				crt->vsync_cnt -= 1;
			}

			crt->ra += 1;

			if (crt->ra > crt->reg[E6845_REG_ML]) {
				if (crt->crow < vt) {
					crt->ma += crt->reg[E6845_REG_HD];
					crt->ra = 0;
					crt->crow += 1;
				}
			}

			if ((crt->crow == vs) && (crt->ra == 0)) {
				crt->vsync_cnt = 16;

				e6845_vsync (crt);

				crt->frame += 1;
				crt->ma = e6845_get_start_address (crt);
			}

			if (((crt->crow == vt) && (crt->ra >= va)) || (crt->crow > vt)) {
				crt->crow = 0;
				crt->ma = e6845_get_start_address (crt);
				crt->ra = 0;
			}
		}
	}
}
