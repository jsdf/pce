/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/intc.c                                       *
 * Created:     2004-11-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#include "main.h"


static unsigned char ict_get_uint8 (ixp_intc_t *ic, unsigned long addr);
static unsigned short ict_get_uint16 (ixp_intc_t *ic, unsigned long addr);
static unsigned long ict_get_uint32 (ixp_intc_t *ic, unsigned long addr);
static void ict_set_uint8 (ixp_intc_t *ic, unsigned long addr, unsigned char val);
static void ict_set_uint16 (ixp_intc_t *ic, unsigned long addr, unsigned short val);
static void ict_set_uint32 (ixp_intc_t *ic, unsigned long addr, unsigned long val);


static
void *ict_irqf[32] = {
	ict_set_irq0, ict_set_irq1, ict_set_irq2, ict_set_irq3,
	ict_set_irq4, ict_set_irq5, ict_set_irq6, ict_set_irq7,
	ict_set_irq8, ict_set_irq9, ict_set_irq10, ict_set_irq11,
	ict_set_irq12, ict_set_irq13, ict_set_irq14, ict_set_irq15,
	ict_set_irq16, ict_set_irq17, ict_set_irq18, ict_set_irq19,
	ict_set_irq20, ict_set_irq21, ict_set_irq22, ict_set_irq23,
	ict_set_irq24, ict_set_irq25, ict_set_irq26, ict_set_irq27,
	ict_set_irq28, ict_set_irq29, ict_set_irq30, ict_set_irq31
};


void ict_init (ixp_intc_t *ict, unsigned long base)
{
	ict->base = base;

	mem_blk_init (&ict->io, base, 0x01000000UL, 0);
	ict->io.ext = ict;
	ict->io.get_uint8 = (mem_get_uint8_f) ict_get_uint8;
	ict->io.set_uint8 = (mem_set_uint8_f) ict_set_uint8;
	ict->io.get_uint16 = (mem_get_uint16_f) ict_get_uint16;
	ict->io.set_uint16 = (mem_set_uint16_f) ict_set_uint16;
	ict->io.get_uint32 = (mem_get_uint32_f) ict_get_uint32;
	ict->io.set_uint32 = (mem_set_uint32_f) ict_set_uint32;

	ict->fiq = NULL;
	ict->fiq_ext = NULL;
	ict->fiq_val = 0;

	ict->irq = NULL;
	ict->irq_ext = NULL;
	ict->irq_val = 0;

	ict->status_raw = 0;
	ict->status_fiq = 0;
	ict->status_irq = 0;
	ict->enable_fiq = 0;
	ict->enable_irq = 0;
}

ixp_intc_t *ict_new (unsigned long base)
{
	ixp_intc_t *ict;

	ict = malloc (sizeof (ixp_intc_t));
	if (ict == NULL) {
		return (NULL);
	}

	ict_init (ict, base);

	return (ict);
}

void ict_free (ixp_intc_t *ict)
{
	mem_blk_free (&ict->io);
}

void ict_del (ixp_intc_t *ict)
{
	if (ict != NULL) {
		ict_free (ict);
		free (ict);
	}
}

mem_blk_t *ict_get_io (ixp_intc_t *ict, unsigned i)
{
	if (i == 0) {
		return (&ict->io);
	}

	return (NULL);
}

void ict_set_fiq_f (ixp_intc_t *ict, void *f, void *ext)
{
	ict->fiq = f;
	ict->fiq_ext = ext;
}

void ict_set_irq_f (ixp_intc_t *ict, void *f, void *ext)
{
	ict->irq = f;
	ict->irq_ext = ext;
}

void *ict_get_irq_f (ixp_intc_t *ict, unsigned irq)
{
	return (ict_irqf[irq & 0x1f]);
}

static
void ict_check_fiq (ixp_intc_t *ict)
{
	ict->status_fiq = ict->status_raw & ict->enable_fiq;

	if (ict->status_fiq) {
		if ((ict->fiq != NULL) && (ict->fiq_val == 0)) {
			ict->fiq (ict->fiq_ext, 1);
			ict->fiq_val = 1;
		}
	}
	else {
		if ((ict->fiq != NULL) && (ict->fiq_val != 0)) {
			ict->fiq (ict->fiq_ext, 0);
			ict->fiq_val = 0;
		}
	}
}

static
void ict_check_irq (ixp_intc_t *ict)
{
	ict->status_irq = ict->status_raw & ict->enable_irq;

	if (ict->status_irq) {
		if ((ict->irq != NULL) && (ict->irq_val == 0)) {
			ict->irq (ict->irq_ext, 1);
			ict->irq_val = 1;
		}
	}
	else {
		if ((ict->irq != NULL) && (ict->irq_val != 0)) {
			ict->irq (ict->irq_ext, 0);
			ict->irq_val = 0;
		}
	}
}

void ict_set_irq (ixp_intc_t *ict, unsigned i, unsigned char val)
{
	if (val) {
		ict->status_raw |= (1UL << i);
	}
	else {
		ict->status_raw &= ~(1UL << i);
	}

	ict_check_fiq (ict);
	ict_check_irq (ict);
}

void ict_set_irq0 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 0, val);
}

void ict_set_irq1 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 1, val);
}

void ict_set_irq2 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 2, val);
}

void ict_set_irq3 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 3, val);
}

void ict_set_irq4 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 4, val);
}

void ict_set_irq5 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 5, val);
}

void ict_set_irq6 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 6, val);
}

void ict_set_irq7 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 7, val);
}

void ict_set_irq8 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 8, val);
}

void ict_set_irq9 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 9, val);
}

void ict_set_irq10 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 10, val);
}

void ict_set_irq11 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 11, val);
}

void ict_set_irq12 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 12, val);
}

void ict_set_irq13 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 13, val);
}

void ict_set_irq14 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 14, val);
}

void ict_set_irq15 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 15, val);
}

void ict_set_irq16 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 16, val);
}

void ict_set_irq17 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 17, val);
}

void ict_set_irq18 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 18, val);
}

void ict_set_irq19 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 19, val);
}

void ict_set_irq20 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 20, val);
}

void ict_set_irq21 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 21, val);
}

void ict_set_irq22 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 22, val);
}

void ict_set_irq23 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 23, val);
}

void ict_set_irq24 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 24, val);
}

void ict_set_irq25 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 25, val);
}

void ict_set_irq26 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 26, val);
}

void ict_set_irq27 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 27, val);
}

void ict_set_irq28 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 28, val);
}

void ict_set_irq29 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 29, val);
}

void ict_set_irq30 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 30, val);
}

void ict_set_irq31 (ixp_intc_t *ict, unsigned char val)
{
	ict_set_irq (ict, 31, val);
}

static
unsigned char ict_get_uint8 (ixp_intc_t *ict, unsigned long addr)
{
	pce_log (MSG_DEB, "ICT: get_uint8 (%08lX)\n", addr);

	return (0);
}

static
unsigned short ict_get_uint16 (ixp_intc_t *ict, unsigned long addr)
{
	pce_log (MSG_DEB, "ICT: get_uint16 (%08lX)\n", addr);

	return (0);
}

static
unsigned long ict_get_uint32 (ixp_intc_t *ict, unsigned long addr)
{
	switch (addr) {
	case 0x00:
		return (ict->status_raw);

	case 0x04:
		return (ict->status_fiq);

	case 0x08:
		return (ict->status_irq);

	case 0x0c:
		return (ict->enable_fiq);

	case 0x10:
		return (ict->enable_irq);

	case 0x14: /* fiq enable clr */
		return (0);

	case 0x18: /* irq enable clr */
		return (0);
	}

	return (0);
}

static
void ict_set_uint8 (ixp_intc_t *ict, unsigned long addr, unsigned char val)
{
	pce_log (MSG_DEB, "ICT: set_uint8 (%08lX, %02X)\n", addr, (unsigned) val);
}

static
void ict_set_uint16 (ixp_intc_t *ict, unsigned long addr, unsigned short val)
{
	pce_log (MSG_DEB, "ICT: set_uint16 (%08lX, %04X)\n", addr, (unsigned) val);
}

static
void ict_set_uint32 (ixp_intc_t *ict, unsigned long addr, unsigned long val)
{
	switch (addr) {
	case 0x0c: /* fiq enable set */
		ict->enable_fiq |= val;
		ict_check_fiq (ict);
		break;

	case 0x10: /* irq enable set */
		ict->enable_irq |= val;
		ict_check_irq (ict);
		break;

	case 0x14: /* fiq enable clr */
		ict->enable_fiq &= ~val;
		ict_check_fiq (ict);
		break;

	case 0x18: /* irq enable clr */
		ict->enable_irq &= ~val;
		ict_check_irq (ict);
		break;
	}
}
