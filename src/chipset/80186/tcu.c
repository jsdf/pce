/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/80186/tcu.c                                      *
 * Created:     2012-06-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#include "tcu.h"

#include <stdio.h>
#include <stdlib.h>


#define TCON_EN   0x8000
#define TCON_INH  0x4000
#define TCON_INT  0x2000
#define TCON_RIU  0x1000
#define TCON_MC   0x0020
#define TCON_RTG  0x0010
#define TCON_P    0x0008
#define TCON_EXT  0x0004
#define TCON_ALT  0x0002
#define TCON_CONT 0x0001


#ifndef DEBUG_TCU
#define DEBUG_TCU 0
#endif


static
void e80186_tcu_init_cntr (e80186_tcu_t *tcu, unsigned idx)
{
	e80186_tcu_cntr_t *cnt;

	cnt = &tcu->cntr[idx];

	cnt->control = 0;
	cnt->count = 0;
	cnt->max_count_a = 0;
	cnt->max_count_b = 0;
	cnt->last_input = 0;
	cnt->current_input = 0;
	cnt->mc = 0;

	cnt->intr_val = 0;
	cnt->intr_ext = NULL;
	cnt->intr = NULL;

	cnt->out_val = 0;
	cnt->out_ext = NULL;
	cnt->out = NULL;
}

void e80186_tcu_init (e80186_tcu_t *tcu)
{
	e80186_tcu_init_cntr (tcu, 0);
	e80186_tcu_init_cntr (tcu, 1);
	e80186_tcu_init_cntr (tcu, 2);

	tcu->clock = 0;
}

void e80186_tcu_free (e80186_tcu_t *tcu)
{
}

void e80186_tcu_set_int_fct (e80186_tcu_t *tcu, unsigned idx, void *ext, void *fct)
{
	tcu->cntr[idx].intr_ext = ext;
	tcu->cntr[idx].intr = fct;
}

void e80186_tcu_set_out_fct (e80186_tcu_t *tcu, unsigned idx, void *ext, void *fct)
{
	tcu->cntr[idx].out_ext = ext;
	tcu->cntr[idx].out = fct;
}

static
void e80186_tcu_set_int (e80186_tcu_t *tcu, unsigned idx, unsigned char val)
{
	e80186_tcu_cntr_t *cnt;

	cnt = &tcu->cntr[idx];

	val = (val != 0);

	if (cnt->intr_val == val) {
		return;
	}

	cnt->intr_val = val;

	if (cnt->intr != NULL) {
		cnt->intr (cnt->intr_ext, val);
	}
}

static
void e80186_tcu_set_out (e80186_tcu_t *tcu, unsigned idx, unsigned char val)
{
	e80186_tcu_cntr_t *cnt;

	cnt = &tcu->cntr[idx];

	val = (val != 0);

	if (cnt->out_val == val) {
		return;
	}

	cnt->out_val = val;

	if (cnt->out != NULL) {
		cnt->out (cnt->out_ext, val);
	}
}

unsigned short e80186_tcu_get_control (const e80186_tcu_t *tcu, unsigned idx)
{
	return (tcu->cntr[idx].control);
}

void e80186_tcu_set_control (e80186_tcu_t *tcu, unsigned idx, unsigned short val)
{
	e80186_tcu_cntr_t *cnt;

	cnt = &tcu->cntr[idx];

	if ((val & TCON_INH) == 0) {
		val = (val & ~TCON_EN) | (cnt->control & TCON_EN);
	}

	val = (val & ~TCON_RIU) | (cnt->control & TCON_RIU);

	cnt->control = val;

#if DEBUG_TCU >= 1
	if (cnt->control & TCON_EN) {
		fprintf (stderr, "TCU: TIMER %u ENABLE (%04X)\n", idx, cnt->control);
	}
#endif
}

unsigned short e80186_tcu_get_count (const e80186_tcu_t *tcu, unsigned idx)
{
	return (tcu->cntr[idx].count);
}

void e80186_tcu_set_count (e80186_tcu_t *tcu, unsigned idx, unsigned short val)
{
	tcu->cntr[idx].count = val;
}

unsigned short e80186_tcu_get_max_count_a (const e80186_tcu_t *tcu, unsigned idx)
{
	return (tcu->cntr[idx].max_count_a);
}

void e80186_tcu_set_max_count_a (e80186_tcu_t *tcu, unsigned idx, unsigned short val)
{
	tcu->cntr[idx].max_count_a = val;
}

unsigned short e80186_tcu_get_max_count_b (const e80186_tcu_t *tcu, unsigned idx)
{
	return (tcu->cntr[idx].max_count_b);
}

void e80186_tcu_set_max_count_b (e80186_tcu_t *tcu, unsigned idx, unsigned short val)
{
	tcu->cntr[idx].max_count_b = val;
}

void e80186_tcu_set_input (e80186_tcu_t *tcu, unsigned idx, unsigned char val)
{
	tcu->cntr[idx].current_input = (val != 0);
}

static
void e80186_tcu_reset_cntr (e80186_tcu_t *tcu, unsigned idx)
{
	e80186_tcu_cntr_t *cnt;

	cnt = &tcu->cntr[idx];

	cnt->control = 0;
	cnt->count = 0;
	cnt->max_count_a = 0;
	cnt->max_count_b = 0;
	cnt->mc = 0;
}

void e80186_tcu_reset (e80186_tcu_t *tcu)
{
	e80186_tcu_reset_cntr (tcu, 0);
	e80186_tcu_reset_cntr (tcu, 1);
	e80186_tcu_reset_cntr (tcu, 2);
}

static
void e80186_tcu_clock_cntr (e80186_tcu_t *tcu, unsigned idx)
{
	e80186_tcu_cntr_t *cnt;
	unsigned char     inp0, inp1;
	unsigned short    ctl;

	cnt = &tcu->cntr[idx];

	ctl = cnt->control;

	inp0 = cnt->last_input;
	inp1 = cnt->current_input;

	cnt->last_input = cnt->current_input;

	cnt->mc = 0;

	if ((ctl & TCON_EN) == 0) {
		return;
	}

	if (ctl & TCON_EXT) {
		if ((inp0 != 0) || (inp1 == 0)) {
			return;
		}
	}
	else {
		if (ctl & TCON_RTG) {
			if ((inp0 == 0) && (inp1 != 0)) {
				cnt->count = 0;
				return;
			}
		}
		else {
			if (inp1 == 0) {
				return;
			}
		}

		if (ctl & TCON_P) {
			if (tcu->cntr[2].mc == 0) {
				return;
			}
		}
	}

	cnt->count = (cnt->count + 1) & 0xffff;

	if (ctl & TCON_ALT) {
		if (ctl & TCON_RIU) {
			if (cnt->count != cnt->max_count_b) {
				return;
			}

			cnt->control &= ~TCON_RIU;

			e80186_tcu_set_out (tcu, idx, 1);

			if ((ctl & TCON_CONT) == 0) {
				cnt->control &= ~TCON_EN;
			}
		}
		else {
			if (cnt->count != cnt->max_count_a) {
				return;
			}

			cnt->control |= TCON_RIU;

			e80186_tcu_set_out (tcu, idx, 0);
		}
	}
	else {
		if (cnt->count != cnt->max_count_a) {
			return;
		}

		e80186_tcu_set_out (tcu, idx, 0);
		e80186_tcu_set_out (tcu, idx, 1);

		if ((ctl & TCON_CONT) == 0) {
			cnt->control &= ~TCON_EN;
		}
	}

	cnt->count = 0;
	cnt->control |= TCON_MC;
	cnt->mc = 1;

	if (ctl & TCON_INT) {
#if DEBUG_TCU >= 1
		fprintf (stderr, "TCU: TIMER %u INTERRUPT\n", idx);
#endif
		e80186_tcu_set_int (tcu, idx, 0);
		e80186_tcu_set_int (tcu, idx, 1);
	}
}

void e80186_tcu_clock (e80186_tcu_t *tcu, unsigned cnt)
{
	unsigned idx;

	while (cnt > 0) {
		idx = tcu->clock & 3;

		if (idx < 3) {
			e80186_tcu_clock_cntr (tcu, idx);
		}

		tcu->clock += 1;
		cnt -= 1;
	}
}
