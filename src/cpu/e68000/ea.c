/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e68000/ea.c                                        *
 * Created:       2006-05-17 by Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e68000.h"
#include "internal.h"


/*
 0 001: Dx
 1 002: Ax
 2 004: (Ax)
 3 008: (Ax)+
 4 010: -(Ax)
 5 020: (Ax + XXXX)
 6 040: (Ax + Rx.S + XX)
 7 080: XXXX
 8 100: XXXXXXXX
 9 200: (PC + XXXX)
10 400: (PC + Rx.S + XX)
11 800: #XXXX
*/


#define e68_set_ea_clk(c, size, w, l) do { \
	if ((size) == 32) (c)->delay += (l); else (c)->delay += (w); \
	} while (0)


int e68_ea_get_ptr (e68000_t *c, unsigned ea, unsigned mask, unsigned size)
{
	unsigned msk;
	unsigned mod, reg;
	uint32_t idx;
	uint16_t *ir0, *ir1;

	ir0 = &c->ir[0];
	ir1 = &c->ir[c->ircnt];

	mod = (ea >> 3) & 7;
	reg = ea & 7;

	switch (mod) {
	case 0x00: /* Dx */
		c->ea_typ = 1;
		c->ea_val = reg;
		msk = 0x0001;
		break;

	case 0x01: /* Ax */
		c->ea_typ = 1;
		c->ea_val = reg + 8;
		msk = 0x0002;
		break;

	case 0x02: /* (Ax) */
		c->ea_typ = 2;
		c->ea_val = e68_get_areg32 (c, reg);
		e68_set_ea_clk (c, size, 4, 8);
		msk = 0x0004;
		break;

	case 0x03: /* (Ax)+ */
		if ((reg == 7) && (size == 8)) {
			size = 16;
		}
		c->ea_typ = 2;
		c->ea_val = e68_get_areg32 (c, reg);
		e68_set_areg32 (c, reg, c->ea_val + size / 8);
		e68_set_ea_clk (c, size, 4, 8);
		msk = 0x0008;
		break;

	case 0x04: /* -(Ax) */
		if ((reg == 7) && (size == 8)) {
			size = 16;
		}
		c->ea_typ = 2;
		c->ea_val = e68_get_areg32 (c, reg) - size / 8;
		e68_set_areg32 (c, reg, c->ea_val);
		e68_set_ea_clk (c, size, 6, 10);
		msk = 0x0010;
		break;

	case 0x05: /* (Ax + XXXX) */
		e68_ifetch_next (c);
		c->ea_typ = 2;
		c->ea_val = e68_get_areg32 (c, reg) + e68_exts (ir1[0], 16);
		e68_set_ea_clk (c, size, 8, 12);
		msk = 0x0020;
		break;

	case 0x06: /* (Ax + Rx.S + XX) */
		e68_ifetch_next (c);
		c->ea_typ = 2;
		c->ea_val = e68_get_areg32 (c, reg) + e68_exts (ir1[0], 8);

		switch (ir1[0] & 0x8800) {
		case 0x0000:
			idx = e68_get_dreg32 (c, (ir1[0] >> 12) & 7);
			idx = e68_exts (idx, 16);
			break;

		case 0x0800:
			idx = e68_get_dreg32 (c, (ir1[0] >> 12) & 7);
			break;

		case 0x8000:
			idx = e68_get_areg32 (c, (ir1[0] >> 12) & 7);
			idx = e68_exts (idx, 16);
			break;

		case 0x8800:
			idx = e68_get_areg32 (c, (ir1[0] >> 12) & 7);
			break;

		default:
			idx = 0;
			break;
		}

		c->ea_val += idx;

		e68_set_ea_clk (c, size, 10, 14);
		msk = 0x0040;
		break;

	case 0x07:
		switch (reg) {
		case 0x00: /* XXXX */
			e68_ifetch_next (c);
			c->ea_typ = 2;
			c->ea_val = e68_exts (ir1[0], 16);
			e68_set_ea_clk (c, size, 8, 12);
			msk = 0x080;
			break;

		case 0x01: /* XXXXXXXX */
			e68_ifetch_next (c);
			e68_ifetch_next (c);
			c->ea_typ = 2;
			c->ea_val = (ir1[0] << 16) | ir1[1];
			e68_set_ea_clk (c, size, 12, 16);
			msk = 0x0100;
			break;

		case 0x02: /* (PC + XXXX) */
			c->ea_val = e68_get_pc (c) + 2 * c->ircnt;
			e68_ifetch_next (c);
			c->ea_val += e68_exts (ir1[0], 16);
			c->ea_typ = 2;
			e68_set_ea_clk (c, size, 8, 12);
			msk = 0x0200;
			break;

		case 0x03: /* (PC + Rx.S + XX) */
			c->ea_val = e68_get_pc (c) + 2 * c->ircnt;
			e68_ifetch_next (c);
			c->ea_val += e68_exts (ir1[0], 8);
			c->ea_typ = 2;

			switch (ir1[0] & 0x8800) {
			case 0x0000:
				idx = e68_get_dreg32 (c, (ir1[0] >> 12) & 7);
				idx = e68_exts (idx, 16);
				break;

			case 0x0800:
				idx = e68_get_dreg32 (c, (ir1[0] >> 12) & 7);
				break;

			case 0x8000:
				idx = e68_get_areg32 (c, (ir1[0] >> 12) & 7);
				idx = e68_exts (idx, 16);
				break;

			case 0x8800:
				idx = e68_get_areg32 (c, (ir1[0] >> 12) & 7);
				break;

			default:
				idx = 0;
				break;
			}

			c->ea_val += idx;

			e68_set_ea_clk (c, size, 10, 14);
			msk = 0x0400;
			break;

		case 0x04: /* #XXXX */
			if (size == 32) {
				e68_ifetch_next (c);
				e68_ifetch_next (c);
				c->ea_val = ((unsigned long) ir1[0] << 16) | ir1[1];
			}
			else {
				e68_ifetch_next (c);
				c->ea_val = ir1[0] & ((size == 8) ? 0xff : 0xffff);
			}
			c->ea_typ = 0;
			e68_set_ea_clk (c, size, 4, 8);
			msk = 0x0800;
			break;

		default:
			e68_exception_illegal (c);
			return (1);
		}
		break;

	default:
		e68_exception_illegal (c);
		return (1);
	}

	if ((msk & mask) == 0) {
		e68_exception_illegal (c);
		return (1);
	}

	return (0);
}

int e68_ea_get_val8 (e68000_t *c, uint8_t *val)
{
	switch (c->ea_typ) {
	case 0:
		*val = c->ea_val & 0xff;
		return (0);

	case 1:
		if (c->ea_val < 8) {
			*val = e68_get_dreg8 (c, c->ea_val);
		}
		else if (c->ea_val < 16) {
			e68_exception_illegal (c);
			return (1);
		}
		else {
			e68_exception_illegal (c);
			return (1);
		}
		return (0);

	case 2:
		*val = e68_get_mem8 (c, c->ea_val);
		return (0);
	}

	e68_exception_illegal (c);

	return (1);
}

int e68_ea_get_val16 (e68000_t *c, uint16_t *val)
{
	switch (c->ea_typ) {
	case 0:
		*val = c->ea_val & 0xffff;
		return (0);

	case 1:
		if (c->ea_val < 8) {
			*val = e68_get_dreg16 (c, c->ea_val);
		}
		else if (c->ea_val < 16) {
			*val = e68_get_areg16 (c, c->ea_val - 8);
		}
		else {
			e68_exception_illegal (c);
			return (1);
		}
		return (0);

	case 2:
		if (c->ea_val & 1) {
			e68_exception_address (c, c->ea_val);
			return (1);
		}
		*val = e68_get_mem16 (c, c->ea_val);
		return (0);
	}

	e68_exception_illegal (c);

	return (1);
}

int e68_ea_get_val32 (e68000_t *c, uint32_t *val)
{
	switch (c->ea_typ) {
	case 0:
		*val = c->ea_val & 0xffffffff;
		return (0);

	case 1:
		if (c->ea_val < 8) {
			*val = e68_get_dreg32 (c, c->ea_val);
		}
		else if (c->ea_val < 16) {
			*val = e68_get_areg32 (c, c->ea_val - 8);
		}
		else {
			e68_exception_illegal (c);
			return (1);
		}
		return (0);

	case 2:
		if (c->ea_val & 1) {
			e68_exception_address (c, c->ea_val);
			return (1);
		}
		*val = e68_get_mem32 (c, c->ea_val);
		return (0);
	}

	e68_exception_illegal (c);

	return (1);
}

int e68_ea_set_val8 (e68000_t *c, uint8_t val)
{
	switch (c->ea_typ) {
	case 0:
		e68_exception_illegal (c);
		return (1);

	case 1:
		if (c->ea_val < 8) {
			e68_set_dreg8 (c, c->ea_val, val);
		}
		else if (c->ea_val < 16) {
			e68_exception_illegal (c);
			return (1);
		}
		else {
			e68_exception_illegal (c);
			return (1);
		}
		return (0);

	case 2:
		e68_set_mem8 (c, c->ea_val, val);
		return (0);
	}

	e68_exception_illegal (c);

	return (1);
}

int e68_ea_set_val16 (e68000_t *c, uint16_t val)
{
	switch (c->ea_typ) {
	case 0:
		e68_exception_illegal (c);
		return (1);

	case 1:
		if (c->ea_val < 8) {
			e68_set_dreg16 (c, c->ea_val, val);
		}
		else if (c->ea_val < 16) {
			e68_set_areg16 (c, c->ea_val - 8, val);
		}
		else {
			e68_exception_illegal (c);
			return (1);
		}
		return (0);

	case 2:
		if (c->ea_val & 1) {
			e68_exception_address (c, c->ea_val);
			return (1);
		}
		e68_set_mem16 (c, c->ea_val, val);
		return (0);
	}

	e68_exception_illegal (c);

	return (1);
}

int e68_ea_set_val32 (e68000_t *c, uint32_t val)
{
	switch (c->ea_typ) {
	case 0:
		e68_exception_illegal (c);
		return (1);

	case 1:
		if (c->ea_val < 8) {
			e68_set_dreg32 (c, c->ea_val, val);
		}
		else if (c->ea_val < 16) {
			e68_set_areg32 (c, c->ea_val - 8, val);
		}
		else {
			e68_exception_illegal (c);
			return (1);
		}
		return (0);

	case 2:
		if (c->ea_val & 1) {
			e68_exception_address (c, c->ea_val);
			return (1);
		}
		e68_set_mem32 (c, c->ea_val, val);
		return (0);
	}

	e68_exception_illegal (c);

	return (1);
}
