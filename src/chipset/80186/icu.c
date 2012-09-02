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


#include "icu.h"

#include <stdio.h>
#include <stdlib.h>


#define ICU_CON_SFNM 0x0040
#define ICU_CON_CAS  0x0020
#define ICU_CON_LVL  0x0010
#define ICU_CON_MSK  0x0008


void e80186_icu_init (e80186_icu_t *icu)
{
	icu->valid_irqs = 0x00fd;
	icu->inp = 0;

	icu->icon_mask[0] = 0x000f;
	icu->icon_mask[1] = 0x0000;
	icu->icon_mask[2] = 0x000f;
	icu->icon_mask[3] = 0x000f;
	icu->icon_mask[4] = 0x007f;
	icu->icon_mask[5] = 0x007f;
	icu->icon_mask[6] = 0x001f;
	icu->icon_mask[7] = 0x001f;

	icu->intr_val = 0;
	icu->intr_ext = NULL;
	icu->intr = NULL;

	icu->inta0_ext = NULL;
	icu->inta0 = NULL;

	icu->inta1_ext = NULL;
	icu->inta1 = NULL;
}

void e80186_icu_free (e80186_icu_t *icu)
{
}

void e80186_icu_set_intr_fct (e80186_icu_t *icu, void *ext, void *fct)
{
	icu->intr_ext = ext;
	icu->intr = fct;
}

void e80186_icu_set_inta0_fct (e80186_icu_t *icu, void *ext, void *fct)
{
	icu->inta0_ext = ext;
	icu->inta0 = fct;
}

void e80186_icu_set_inta1_fct (e80186_icu_t *icu, void *ext, void *fct)
{
	icu->inta1_ext = ext;
	icu->inta1 = fct;
}

static
void e80186_icu_set_intr (e80186_icu_t *icu, int val)
{
	val = (val != 0);

	if (icu->intr_val == val) {
		return;
	}

	icu->intr_val = val;

	if (icu->intr != NULL) {
		icu->intr (icu->intr_ext, val);
	}
}

void e80186_icu_reset (e80186_icu_t *icu)
{
	unsigned i;

	icu->irr = 0;
	icu->imr = 0;
	icu->isr = 0;
	icu->pmr = 7;
	icu->intsts = 0;

	for (i = 0; i < 8; i++) {
		icu->icon[i] = 0;
		icu->int_cnt[i] = 0;
	}

	e80186_icu_set_intr (icu, 0);
}

static
int e80186_icu_get_max_req (const e80186_icu_t *icu, unsigned *index, unsigned *mask, unsigned *type)
{
	unsigned i;
	unsigned maxpri, maxmsk, maxidx;
	unsigned pri, msk;

	maxidx = -1;
	maxpri = 7;
	maxmsk = 0;

	for (i = 0; i < 8; i++) {
		msk = 0x80 >> i;

		if ((icu->valid_irqs & msk) == 0) {
			continue;
		}

		if (icu->imr & msk) {
			continue;
		}

		pri = icu->icon[7 - i] & 7;

		if (pri > maxpri) {
			continue;
		}

		if (pri > (icu->pmr & 7)) {
			continue;
		}

		if ((icu->isr & msk) && ((icu->icon[7 - 1] & ICU_CON_SFNM) == 0)) {
			maxidx = -1;
			maxpri = 7;
			maxmsk = 0;
		}
		else if (icu->irr & msk) {
			maxidx = 7 - i;
			maxpri = pri;
			maxmsk = msk;
		}
	}

	if (maxmsk == 0) {
		return (1);
	}

	if (index != NULL) {
		*index = maxidx;
	}

	if (mask != NULL) {
		*mask = maxmsk;
	}

	if (type != NULL) {
		*type = maxidx + 8;

		if (maxidx == 0) {
			if (icu->intsts & 1) {
				*type = 8;
			}
			else if (icu->intsts & 2) {
				*type = 18;
			}
			else if (icu->intsts & 4) {
				*type = 19;
			}
		}
	}

	return (0);
}

static
int e80186_icu_get_max_srv (const e80186_icu_t *icu, unsigned *index, unsigned *mask)
{
	unsigned i;
	unsigned maxidx, maxpri, maxmsk;
	unsigned pri, msk;

	maxidx = -1;
	maxpri = 7;
	maxmsk = 0;

	for (i = 0; i < 8; i++) {
		msk = 0x80 >> i;

		if ((icu->valid_irqs & msk) == 0) {
			continue;
		}

		if ((icu->isr & msk) == 0) {
			continue;
		}

		pri = icu->icon[7 - i] & 7;

		if (pri > maxpri) {
			continue;
		}

		maxidx = 7 - i;
		maxpri = pri;
		maxmsk = msk;
	}

	if (maxmsk == 0) {
		return (1);
	}

	if (index != NULL) {
		*index = maxidx;
	}

	if (mask != NULL) {
		*mask = maxmsk;
	}

	return (0);
}

static
void e80186_icu_check (e80186_icu_t *icu)
{
	if (e80186_icu_get_max_req (icu, NULL, NULL, NULL)) {
		e80186_icu_set_intr (icu, 0);
	}
	else {
		e80186_icu_set_intr (icu, 1);
	}
}

static inline
int e80186_icu_irq_valid (const e80186_icu_t *icu, unsigned idx)
{
	if (idx > 7) {
		return (0);
	}

	return ((icu->valid_irqs & (1U << idx)) != 0);
}

void e80186_icu_set_irq (e80186_icu_t *icu, unsigned idx, int val)
{
	unsigned short msk;

	msk = 1U << idx;

	if (~icu->valid_irqs & msk) {
		return;
	}

	if (val) {
		if (icu->inp & msk) {
			return;
		}

		icu->inp |= msk;
		icu->irr |= msk;
	}
	else {
		if (~icu->inp & msk) {
			return;
		}

		icu->inp &= ~msk;
		icu->irr &= ~msk;
	}

	e80186_icu_check (icu);
}

void e80186_icu_set_irq_tmr0 (e80186_icu_t *icu, unsigned char val)
{
	if (val) {
		icu->intsts |= 1;
	}
	else {
		icu->intsts &= ~1U;
	}

	e80186_icu_set_irq (icu, 0, val);
}

void e80186_icu_set_irq_tmr1 (e80186_icu_t *icu, unsigned char val)
{
	if (val) {
		icu->intsts |= 2;
	}
	else {
		icu->intsts &= ~2U;
	}

	e80186_icu_set_irq (icu, 0, val);
}

void e80186_icu_set_irq_tmr2 (e80186_icu_t *icu, unsigned char val)
{
	if (val) {
		icu->intsts |= 4;
	}
	else {
		icu->intsts &= ~4U;
	}

	e80186_icu_set_irq (icu, 0, val);
}

void e80186_icu_set_irq_dma0 (e80186_icu_t *icu, unsigned char val)
{
	e80186_icu_set_irq (icu, 2, val);
}

void e80186_icu_set_irq_dma1 (e80186_icu_t *icu, unsigned char val)
{
	e80186_icu_set_irq (icu, 3, val);
}

void e80186_icu_set_irq_int0 (e80186_icu_t *icu, unsigned char val)
{
	e80186_icu_set_irq (icu, 4, val);
}

void e80186_icu_set_irq_int1 (e80186_icu_t *icu, unsigned char val)
{
	e80186_icu_set_irq (icu, 5, val);
}

unsigned short e80186_icu_get_icon (const e80186_icu_t *icu, unsigned idx)
{
	if (e80186_icu_irq_valid (icu, idx) == 0) {
		return (0);
	}

	return (icu->icon[idx]);
}

void e80186_icu_set_icon (e80186_icu_t *icu, unsigned idx, unsigned short val)
{
	unsigned short msk;

	if (e80186_icu_irq_valid (icu, idx) == 0) {
		return;
	}

	icu->icon[idx] = val & icu->icon_mask[idx];

	msk = 1U << idx;

	if (val & 0x08) {
		icu->imr |= msk;
	}
	else {
		icu->imr &= ~msk;
	}

	e80186_icu_check (icu);
}

unsigned short e80186_icu_get_imr (const e80186_icu_t *icu)
{
	return (icu->imr);
}

void e80186_icu_set_imr (e80186_icu_t *icu, unsigned short val)
{
	unsigned i;

	icu->imr = val;

	for (i = 0; i < 8; i++) {
		if (val & (1U << i)) {
			icu->icon[i] |= 0x08;
		}
		else {
			icu->icon[i] &= ~0x08;
		}
	}

	e80186_icu_check (icu);
}

unsigned short e80186_icu_get_pmr (const e80186_icu_t *icu)
{
	return (icu->pmr);
}

void e80186_icu_set_pmr (e80186_icu_t *icu, unsigned short val)
{
	icu->pmr = val;

	e80186_icu_check (icu);
}

unsigned short e80186_icu_get_isr (const e80186_icu_t *icu)
{
	return (icu->isr);
}

void e80186_icu_set_isr (e80186_icu_t *icu, unsigned short val)
{
	icu->isr = val;

	e80186_icu_check (icu);
}

unsigned short e80186_icu_get_irr (const e80186_icu_t *icu)
{
	return (icu->irr);
}

void e80186_icu_set_irr (e80186_icu_t *icu, unsigned short val)
{
	icu->irr = val;

	e80186_icu_check (icu);
}

void e80186_icu_set_eoi (e80186_icu_t *icu, unsigned short val)
{
	unsigned index, mask;

	if (val & 0x8000) {
		if (e80186_icu_get_max_srv (icu, &index, &mask)) {
			return;
		}

		icu->isr &= ~mask;
	}

	e80186_icu_check (icu);
}

unsigned short e80186_icu_get_intsts (const e80186_icu_t *icu)
{
	return (icu->intsts);
}

void e80186_icu_set_intsts (e80186_icu_t *icu, unsigned short val)
{
	icu->intsts = val;

	e80186_icu_check (icu);
}

unsigned short e80186_icu_get_pollst (e80186_icu_t *icu)
{
	unsigned type;

	if (e80186_icu_get_max_req (icu, NULL, NULL, &type)) {
		return (0);
	}

	return (type | 0x8000);
}

unsigned short e80186_icu_get_poll (e80186_icu_t *icu)
{
	unsigned idx, msk, type;

	if (e80186_icu_get_max_req (icu, &idx, &msk, &type)) {
		return (0);
	}

	if ((icu->icon[idx] & ICU_CON_LVL) == 0) {
		icu->irr &= ~msk;
	}

	icu->int_cnt[idx] += 1;

	icu->isr |= msk;

	if (idx == 0) {
		icu->intsts &= (icu->intsts - 1) | 0xfff8;
	}

	if (icu->icon[idx] & ICU_CON_CAS) {
		if (idx == 4) {
			if (icu->inta0 != NULL) {
				e80186_icu_check (icu);
				return (icu->inta0 (icu->inta0_ext));
			}
		}
		else if (idx == 5) {
			if (icu->inta1 != NULL) {
				e80186_icu_check (icu);
				return (icu->inta1 (icu->inta1_ext));
			}
		}
	}

	e80186_icu_check (icu);

	return (type | 0x8000);
}

unsigned char e80186_icu_inta (e80186_icu_t *icu)
{
	return (e80186_icu_get_poll (icu) & 0xff);
}
