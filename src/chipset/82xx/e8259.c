/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8259.c                                     *
 * Created:     2003-04-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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


/*****************************************************************************
 * PIC 8259 eulator
 *  - no dynamic priority support (irq 0 always has highest priority)
 *  - no level sensitive inputs
 *  - no nesting support
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "e8259.h"


void e8259_init (e8259_t *pic)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		pic->icw[i] = 0;
	}

	for (i = 0; i < 3; i++) {
		pic->ocw[i] = 0;
	}

	pic->base = 0;

	pic->next_icw = 0;
	pic->read_irr = 1;

	pic->irr = 0x00;
	pic->imr = 0xff;
	pic->isr = 0x00;

	pic->irq_inp = 0x00;

	pic->priority = 0;

	pic->rot_on_aeoi = 0;

	for (i = 0; i < 8; i++) {
		pic->irq_cnt[i] = 0;
	}

	pic->intr_ext = NULL;
	pic->intr_val = 0;
	pic->intr = NULL;
}

e8259_t *e8259_new (void)
{
	e8259_t *pic;

	pic = malloc (sizeof (e8259_t));
	if (pic == NULL) {
		return (NULL);
	}

	e8259_init (pic);

	return (pic);
}

void e8259_free (e8259_t *pic)
{
}

void e8259_del (e8259_t *pic)
{
	if (pic != NULL) {
		e8259_free (pic);
		free (pic);
	}
}

e8259_irq_f e8259_get_irq_f (e8259_t *pic, unsigned irq)
{
	switch (irq & 0x07) {
		case 0:
			return (&e8259_set_irq0);

		case 1:
			return (&e8259_set_irq1);

		case 2:
			return (&e8259_set_irq2);

		case 3:
			return (&e8259_set_irq3);

		case 4:
			return (&e8259_set_irq4);

		case 5:
			return (&e8259_set_irq5);

		case 6:
			return (&e8259_set_irq6);

		case 7:
			return (&e8259_set_irq7);
	}

	return (NULL);
}

void e8259_set_int_fct (e8259_t *pic, void *ext, void *fct)
{
	pic->intr_ext = ext;
	pic->intr = fct;
}

static
void e8259_set_int (e8259_t *pic, unsigned char val)
{
	if (pic->intr_val != val) {
		pic->intr_val = val;
		if (pic->intr != NULL) {
			pic->intr (pic->intr_ext, val);
		}
	}
}

void e8259_set_irq (e8259_t *pic, unsigned irq, unsigned char val)
{
	unsigned char msk;

	msk = 0x01 << (irq & 0x07);

	if (val == 0) {
		pic->irq_inp &= ~msk;
		return;
	}

	if (pic->irq_inp & msk) {
		/* not an edge */
		return;
	}

	pic->irr |= msk;
}

void e8259_set_irq0 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 0, val != 0);
}

void e8259_set_irq1 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 1, val != 0);
}

void e8259_set_irq2 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 2, val != 0);
}

void e8259_set_irq3 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 3, val != 0);
}

void e8259_set_irq4 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 4, val != 0);
}

void e8259_set_irq5 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 5, val != 0);
}

void e8259_set_irq6 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 6, val != 0);
}

void e8259_set_irq7 (e8259_t *pic, unsigned char val)
{
	e8259_set_irq (pic, 7, val != 0);
}

static
unsigned e8259_get_priority (e8259_t *pic, unsigned char val)
{
	unsigned ret, msk;

	if (val == 0) {
		return (16);
	}

	msk = (val << 8) | val;
	ret = pic->priority & 7;

	while ((msk & (1 << ret)) == 0) {
		ret += 1;
	}

	return (ret);
}

unsigned char e8259_inta (e8259_t *pic)
{
	unsigned      irrp;
	unsigned char irrb;

	e8259_set_int (pic, 0);

	/* highest priority interrupt */
	irrp = e8259_get_priority (pic, pic->irr & ~pic->imr);

	if (irrp >= 16) {
		/* should not happen */
		fprintf (stderr, "e8259: INTA without IRQ\n");
		return (pic->base + 7);
	}

	irrp &= 7;
	irrb = 1 << irrp;

	pic->irr &= ~irrb;

	if (pic->icw[3] & E8259_ICW4_AEOI) {
		if (pic->rot_on_aeoi) {
			pic->priority = (irrp + 1) & 7;
		}
	}
	else {
		pic->isr |= irrb;
	}

	pic->irq_cnt[irrp] += 1;

	return (pic->base + irrp);
}

unsigned char e8259_get_irr (e8259_t *pic)
{
	return (pic->irr);
}

unsigned char e8259_get_imr (e8259_t *pic)
{
	return (pic->imr);
}

unsigned char e8259_get_isr (e8259_t *pic)
{
	return (pic->isr);
}

unsigned char e8259_get_icw (e8259_t *pic, unsigned i)
{
	if (i < 4) {
		return (pic->icw[i]);
	}

	return (0);
}

unsigned char e8259_get_ocw (e8259_t *pic, unsigned i)
{
	if (i < 3) {
		return (pic->ocw[i]);
	}

	return (0);
}

static
void e8259_set_icw1 (e8259_t *pic, unsigned char val)
{
	pic->icw[0] = val;
	pic->icw[1] = 0;
	pic->icw[2] = 0;
	pic->icw[3] = 0;

	pic->ocw[0] = 0;
	pic->ocw[1] = 0;
	pic->ocw[2] = 0;

	pic->base = 0;

	pic->irq_inp = 0;

	pic->next_icw = 1;
	pic->read_irr = 1;

	pic->priority = 0;
	pic->rot_on_aeoi = 0;

	pic->irr = 0x00;
	pic->imr = 0xff;
	pic->isr = 0x00;
}

static
void e8259_set_icwn (e8259_t *pic, unsigned char val)
{
	switch (pic->next_icw) {
		case 1:
			pic->icw[1] = val;
			pic->base = val & ~7;
			if ((pic->icw[0] & E8259_ICW1_SNGL) == 0) {
				pic->next_icw = 2;
			}
			else if (pic->icw[0] & E8259_ICW1_IC4) {
				pic->next_icw = 3;
			}
			else {
				pic->next_icw = 0;
			}
			break;

		case 2:
			pic->icw[2] = val;
			if (pic->icw[0] & E8259_ICW1_IC4) {
				pic->next_icw = 3;
			}
			else {
				pic->next_icw = 0;
			}
			break;

		case 3:
			pic->icw[3] = val;
			pic->next_icw = 0;
			break;
	}
}

static
void e8259_set_ocw1 (e8259_t *pic, unsigned char val)
{
	pic->ocw[0] = val;
	pic->imr = val;
}

static
void e8259_set_ocw2 (e8259_t *pic, unsigned char val)
{
	unsigned      isrp;
	unsigned char isrb;

	pic->ocw[1] = val;

	isrp = e8259_get_priority (pic, pic->isr);
	isrb = 1 << (isrp & 7);

	switch ((val >> 5) & 7) {
		case 0x00: /* rotate in AEOI mode clear */
			pic->rot_on_aeoi = 0;
			break;

		case 0x01: /* non-specific EOI */
			if (isrp < 16) {
				pic->isr &= ~isrb;
			}
			break;

		case 0x02: /* no operation */
			break;

		case 0x03: /* specific EOI */
			pic->isr &= ~(1 << (val & 7));
			break;

		case 0x04: /* rotate in AEOI mode set */
			pic->rot_on_aeoi = 1;
			break;

		case 0x05: /* rotate on non-specific EOI */
			if (isrp < 16) {
				pic->isr &= ~isrb;
				pic->priority = (isrp + 1) & 7;
			}
			break;

		case 0x06: /* set priority */
			pic->priority = ((val + 1) & 7);
			break;

		case 0x07: /* rotate on specific EOI */
			pic->isr &= ~(1 << (val & 7));
			pic->priority = ((val + 1) & 7);
			break;
	}
}

static
void e8259_set_ocw3 (e8259_t *pic, unsigned char val)
{
	pic->ocw[2] = val;

	if (val & E8259_OCW3_RR) {
		if (val & E8259_OCW3_RIS) {
			pic->read_irr = 0;
		}
		else {
			pic->read_irr = 1;
		}
	}

	if (val & E8259_OCW3_P) {
		fprintf (stderr, "e8259: poll command\n");
	}

	if (val & E8259_OCW3_ESMM) {
		if (val & E8259_OCW3_SMM) {
			fprintf (stderr, "e8259: special mask mode enabled\n");
		}
	}
}

void e8259_set_uint8 (e8259_t *pic, unsigned long addr, unsigned char val)
{
	switch (addr) {
		case 0:
			if ((val & 0x10) == 0x10) {
				e8259_set_icw1 (pic, val);
			}
			else if ((val & 0x18) == 0) {
				e8259_set_ocw2 (pic, val);
			}
			else if ((val & 0x98) == 0x08) {
				e8259_set_ocw3 (pic, val);
			}
			break;

		case 1:
			if (pic->next_icw != 0) {
				e8259_set_icwn (pic, val);
			}
			else {
				e8259_set_ocw1 (pic, val);
			}
			break;
	}
}

void e8259_set_uint16 (e8259_t *pic, unsigned long addr, unsigned short val)
{
	e8259_set_uint8 (pic, addr, val & 0xff);
}

void e8259_set_uint32 (e8259_t *pic, unsigned long addr, unsigned long val)
{
	e8259_set_uint8 (pic, addr, val & 0xff);
}

unsigned char e8259_get_uint8 (e8259_t *pic, unsigned long addr)
{
	switch (addr) {
		case 0x00:
			return (pic->read_irr ? pic->irr : pic->isr);

		case 0x01:
			return (pic->imr);
	}

	return (0xff);
}

unsigned short e8259_get_uint16 (e8259_t *pic, unsigned long addr)
{
	return (e8259_get_uint8 (pic, addr));
}

unsigned long e8259_get_uint32 (e8259_t *pic, unsigned long addr)
{
	return (e8259_get_uint8 (pic, addr));
}

void e8259_reset (e8259_t *pic)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		pic->icw[i] = 0;
	}

	for (i = 0; i < 3; i++) {
		pic->ocw[i] = 0;
	}

	pic->next_icw = 0;
	pic->read_irr = 1;

	pic->irr = 0x00;
	pic->imr = 0xff;
	pic->isr = 0x00;

	pic->irq_inp = 0x00;

	pic->priority = 0;

	pic->rot_on_aeoi = 0;
}

void e8259_clock (e8259_t *pic)
{
	unsigned irrp, isrp;

	if (pic->irr == 0) {
		return;
	}

	irrp = e8259_get_priority (pic, pic->irr & ~pic->imr);
	isrp = e8259_get_priority (pic, pic->isr & ~pic->imr);

	if (irrp < isrp) {
		e8259_set_int (pic, 1);
	}
}
