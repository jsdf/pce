/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/ppc405/uic.c                                     *
 * Created:     2004-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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

#include "uic.h"


static void *p405uic_irqf[32] = {
	&p405uic_set_irq0,  &p405uic_set_irq1,  &p405uic_set_irq2,
	&p405uic_set_irq3,  &p405uic_set_irq4,  &p405uic_set_irq5,
	&p405uic_set_irq6,  &p405uic_set_irq7,  &p405uic_set_irq8,
	&p405uic_set_irq9,  &p405uic_set_irq10, &p405uic_set_irq11,
	&p405uic_set_irq12, &p405uic_set_irq13, &p405uic_set_irq14,
	&p405uic_set_irq15, &p405uic_set_irq16, &p405uic_set_irq17,
	&p405uic_set_irq18, &p405uic_set_irq19, &p405uic_set_irq20,
	&p405uic_set_irq21, &p405uic_set_irq22, &p405uic_set_irq23,
	&p405uic_set_irq24, &p405uic_set_irq25, &p405uic_set_irq26,
	&p405uic_set_irq27, &p405uic_set_irq28, &p405uic_set_irq29,
	&p405uic_set_irq30, &p405uic_set_irq31,
};


void p405uic_init (p405_uic_t *uic)
{
	unsigned i;

	uic->sr = 0;
	uic->er = 0;
	uic->msr = 0;
	uic->cr = 0;
	uic->pr = 0xffffffffUL;
	uic->tr = (0x80000000UL >> P405UIC_INT_EM);
	uic->vcr = 0;
	uic->vr = 0;

	uic->levels = 0x00000000;
	uic->invert = 0x00000000;
	uic->vr_msk = 0;

	uic->nint = NULL;
	uic->nint_ext = NULL;
	uic->nint_val = 0;

	uic->cint = NULL;
	uic->cint_ext = NULL;
	uic->cint_val = 0;

	for (i = 0; i < 32; i++) {
		uic->intcnt[i] = 0;
	}
}

p405_uic_t *p405uic_new (void)
{
	p405_uic_t *uic;

	uic = malloc (sizeof (p405_uic_t));
	if (uic == NULL) {
		return (NULL);
	}

	p405uic_init (uic);

	return (uic);
}

void p405uic_free (p405_uic_t *uic)
{
}

void p405uic_del (p405_uic_t *uic)
{
	if (uic != NULL) {
		p405uic_free (uic);
		free (uic);
	}
}

void p405uic_set_invert (p405_uic_t *uic, unsigned long inv)
{
	uic->invert = inv & 0xffffffff;
	uic->levels = inv & 0xffffffff;
}

void p405uic_set_cint_fct (p405_uic_t *uic, void *ext, void *fct)
{
	uic->cint_ext = ext;
	uic->cint = fct;
}

void p405uic_set_nint_fct (p405_uic_t *uic, void *ext, void *fct)
{
	uic->nint_ext = ext;
	uic->nint = fct;
}

void *p405uic_get_irq_fct (p405_uic_t *uic, unsigned irq)
{
	return (p405uic_irqf[irq & 0x1f]);
}

static
void p405uic_set_cint (p405_uic_t *uic, unsigned char val)
{
	val = (val != 0);

	if (val != uic->cint_val) {
		uic->cint_val = val;
		if (uic->cint != NULL) {
			uic->cint (uic->cint_ext, val);
		}
	}
}

static
void p405uic_set_nint (p405_uic_t *uic, unsigned char val)
{
	val = (val != 0);

	if (val != uic->nint_val) {
		uic->nint_val = val;
		if (uic->nint != NULL) {
			uic->nint (uic->nint_ext, val);
		}
	}
}

static
void p405uic_update (p405_uic_t *uic)
{
	unsigned i;
	uint32_t msk;

	uic->msr = uic->sr & uic->er;

	msk = uic->cr & uic->msr;

	if (msk == 0) {
		/* no critical interrupts active */
		uic->vr = 0;
	}
	else {
		i = 0;

		if (uic->vcr & P405UIC_VCR_PRO) {
			while ((msk & 0x80000000UL) == 0) {
				msk <<= 1;
				i += 1;
			}
		}
		else {
			while ((msk & 0x00000001UL) == 0) {
				msk >>= 1;
				i += 1;
			}
		}

		uic->vr = (uic->vcr & 0xfffffffcUL) + 512UL * i;
	}

	p405uic_set_cint (uic, (uic->msr & uic->cr) != 0);
	p405uic_set_nint (uic, (uic->msr & ~uic->cr) != 0);
}

uint32_t p405uic_get_sr (p405_uic_t *uic)
{
	return (uic->sr);
}

void p405uic_set_sr (p405_uic_t *uic, uint32_t val)
{
	/* can't clear level sensitive interrupts that are still active */
	val &= (uic->tr | (uic->levels ^ uic->pr));

	uic->sr &= ~val;

	p405uic_update (uic);
}

uint32_t p405uic_get_levels (const p405_uic_t *uic)
{
	return (uic->levels);
}

uint32_t p405uic_get_er (p405_uic_t *uic)
{
	return (uic->er);
}

void p405uic_set_er (p405_uic_t *uic, uint32_t val)
{
	uic->er = val;
	p405uic_update (uic);
}

uint32_t p405uic_get_msr (p405_uic_t *uic)
{
	return (uic->msr);
}

uint32_t p405uic_get_cr (p405_uic_t *uic)
{
	return (uic->cr);
}

void p405uic_set_cr (p405_uic_t *uic, uint32_t val)
{
	uic->cr = val;
	p405uic_update (uic);
}

uint32_t p405uic_get_pr (p405_uic_t *uic)
{
	return (uic->pr);
}

void p405uic_set_pr (p405_uic_t *uic, uint32_t val)
{
	uic->pr = val;
	p405uic_update (uic);
}

uint32_t p405uic_get_tr (p405_uic_t *uic)
{
	return (uic->tr);
}

void p405uic_set_tr (p405_uic_t *uic, uint32_t val)
{
	uic->tr = val;
	p405uic_update (uic);
}

uint32_t p405uic_get_vcr (p405_uic_t *uic)
{
	return (uic->vcr);
}

void p405uic_set_vcr (p405_uic_t *uic, uint32_t val)
{
	uic->vcr = val;
	p405uic_update (uic);
}

uint32_t p405uic_get_vr (p405_uic_t *uic)
{
	return (uic->vr);
}

unsigned long p405uic_get_int_cnt (p405_uic_t *uic, unsigned i)
{
	if (i < 32) {
		return (uic->intcnt[i]);
	}

	return (0);
}

void p405uic_set_irq (p405_uic_t *uic, unsigned i, unsigned char val)
{
	uint32_t msk;

	msk = 0x80000000UL >> (i & 0x1f);

	if (uic->invert & msk) {
		val = !val;
	}

	if ((val != 0) == ((uic->levels & msk) != 0)) {
		/* level has not changed */
		return;
	}

	if (val) {
		uic->levels |= msk;

		if ((uic->pr & msk) == 0) {
			return;
		}
	}
	else {
		uic->levels &= ~msk;

		if (uic->pr & msk) {
			return;
		}
	}

	if (uic->sr & msk) {
		/* already active */
		return;
	}

	uic->intcnt[i] += 1;

	uic->sr |= msk;

	p405uic_update (uic);
}

void p405uic_set_irq0 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 0, val);
}

void p405uic_set_irq1 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 1, val);
}

void p405uic_set_irq2 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 2, val);
}

void p405uic_set_irq3 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 3, val);
}

void p405uic_set_irq4 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 4, val);
}

void p405uic_set_irq5 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 5, val);
}

void p405uic_set_irq6 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 6, val);
}

void p405uic_set_irq7 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 7, val);
}

void p405uic_set_irq8 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 8, val);
}

void p405uic_set_irq9 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 9, val);
}

void p405uic_set_irq10 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 10, val);
}

void p405uic_set_irq11 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 11, val);
}

void p405uic_set_irq12 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 12, val);
}

void p405uic_set_irq13 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 13, val);
}

void p405uic_set_irq14 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 14, val);
}

void p405uic_set_irq15 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 15, val);
}

void p405uic_set_irq16 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 16, val);
}

void p405uic_set_irq17 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 17, val);
}

void p405uic_set_irq18 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 18, val);
}

void p405uic_set_irq19 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 19, val);
}

void p405uic_set_irq20 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 20, val);
}

void p405uic_set_irq21 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 21, val);
}

void p405uic_set_irq22 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 22, val);
}

void p405uic_set_irq23 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 23, val);
}

void p405uic_set_irq24 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 24, val);
}

void p405uic_set_irq25 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 25, val);
}

void p405uic_set_irq26 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 26, val);
}

void p405uic_set_irq27 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 27, val);
}

void p405uic_set_irq28 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 28, val);
}

void p405uic_set_irq29 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 29, val);
}

void p405uic_set_irq30 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 30, val);
}

void p405uic_set_irq31 (p405_uic_t *uic, unsigned char val)
{
	p405uic_set_irq (uic, 31, val);
}
