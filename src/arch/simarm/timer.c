/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/timer.c                                      *
 * Created:     2004-11-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "timer.h"


#define IXP_TIMER_ACT (1UL << 7)


static unsigned char tmr_get_uint8 (ixp_timer_t *ic, unsigned long addr);
static unsigned short tmr_get_uint16 (ixp_timer_t *ic, unsigned long addr);
static unsigned long tmr_get_uint32 (ixp_timer_t *ic, unsigned long addr);
static void tmr_set_uint8 (ixp_timer_t *ic, unsigned long addr, unsigned char val);
static void tmr_set_uint16 (ixp_timer_t *ic, unsigned long addr, unsigned short val);
static void tmr_set_uint32 (ixp_timer_t *ic, unsigned long addr, unsigned long val);


static
void ctr_init (ixp_timer_counter_t *ctr)
{
	ctr->clock = NULL;

	ctr->irq = NULL;
	ctr->irq_ext = NULL;
	ctr->irq_val = 0;

	ctr->ctrl = 0;
	ctr->load = 0xffffffffUL;
	ctr->status = 0;
	ctr->clear = 0;

	ctr->clkdiv = 0;
}

static
void ctr_set_irq (ixp_timer_counter_t *ctr, unsigned char val)
{
	if (val) {
		if ((ctr->irq != NULL) && (ctr->irq_val == 0)) {
			ctr->irq (ctr->irq_ext, 1);
			ctr->irq_val = 1;
		}
	}
	else {
		if ((ctr->irq != NULL) && (ctr->irq_val != 0)) {
			ctr->irq (ctr->irq_ext, 0);
			ctr->irq_val = 0;
		}
	}
}

static
void ctr_clock_1 (ixp_timer_counter_t *ctr, unsigned long n)
{
	if (ctr->status == 0) {
		ctr->status = ctr->load;
	}

	if (n < ctr->status) {
		ctr->status -= n;
		return;
	}

	n -= ctr->status;

	while (n > ctr->load) {
		n -= ctr->load;
		n -= 1;
	}

	ctr->status = ctr->load - n;

	ctr_set_irq (ctr, 1);
}

static
void ctr_clock_16 (ixp_timer_counter_t *ctr, unsigned long n)
{
	n += ctr->clkdiv;

	ctr->clkdiv = n % 16;

	ctr_clock_1 (ctr, n / 16);
}

static
void ctr_clock_256 (ixp_timer_counter_t *ctr, unsigned long n)
{
	n += ctr->clkdiv;

	ctr->clkdiv = n % 256;

	ctr_clock_1 (ctr, n / 256);
}

static
void ctr_set_ctrl (ixp_timer_counter_t *ctr, unsigned long val)
{
	ctr->ctrl = val & 0x0000008cUL;

	switch ((val >> 2) & 0x03) {
	case 0x00:
		ctr->clock = ctr_clock_1;
		break;

	case 0x01:
		ctr->clock = ctr_clock_16;
		break;

	case 0x02:
		ctr->clock = ctr_clock_256;
		break;

	case 0x03:
		ctr->clock = ctr_clock_1;
		break;
	}

	if ((ctr->ctrl & IXP_TIMER_ACT) == 0) {
		ctr->clock = NULL;
	}
}

static
void ctr_set_load (ixp_timer_counter_t *ctr, unsigned long val)
{
	ctr->load = val;
	ctr->status = val;
}

static
void ctr_set_clear (ixp_timer_counter_t *ctr, unsigned long val)
{
	ctr->clear = val;
	ctr_set_irq (ctr, 0);
}

static
unsigned long ctr_get_ctrl (ixp_timer_counter_t *ctr)
{
	return (ctr->ctrl);
}

static
unsigned long ctr_get_load (ixp_timer_counter_t *ctr)
{
	return (ctr->load);
}

static
unsigned long ctr_get_status (ixp_timer_counter_t *ctr)
{
	return (ctr->status);
}

void tmr_init (ixp_timer_t *tmr, unsigned long base)
{
	unsigned i;

	tmr->base = base;

	mem_blk_init (&tmr->io, base, 0x00010000UL, 0);
	tmr->io.ext = tmr;
	tmr->io.get_uint8 = (mem_get_uint8_f) tmr_get_uint8;
	tmr->io.set_uint8 = (mem_set_uint8_f) tmr_set_uint8;
	tmr->io.get_uint16 = (mem_get_uint16_f) tmr_get_uint16;
	tmr->io.set_uint16 = (mem_set_uint16_f) tmr_set_uint16;
	tmr->io.get_uint32 = (mem_get_uint32_f) tmr_get_uint32;
	tmr->io.set_uint32 = (mem_set_uint32_f) tmr_set_uint32;

	for (i = 0; i < 4; i++) {
		ctr_init (&tmr->cntr[i]);
	}

	tmr->twde = 0;
}

ixp_timer_t *tmr_new (unsigned long base)
{
	ixp_timer_t *tmr;

	tmr = malloc (sizeof (ixp_timer_t));
	if (tmr == NULL) {
		return (NULL);
	}

	tmr_init (tmr, base);

	return (tmr);
}

void tmr_free (ixp_timer_t *tmr)
{
	mem_blk_free (&tmr->io);
}

void tmr_del (ixp_timer_t *tmr)
{
	if (tmr != NULL) {
		tmr_free (tmr);
		free (tmr);
	}
}

mem_blk_t *tmr_get_io (ixp_timer_t *tmr, unsigned i)
{
	if (i == 0) {
		return (&tmr->io);
	}

	return (NULL);
}

int tmr_get_active (ixp_timer_t *tmr, unsigned i)
{
	if (i <= 3) {
		if (tmr->cntr[i].ctrl & IXP_TIMER_ACT) {
			return (1);
		}
	}

	return (0);
}

void tmr_set_irq_f (ixp_timer_t *tmr, unsigned i, void *f, void *ext)
{
	if (i < 4) {
		tmr->cntr[i].irq = f;
		tmr->cntr[i].irq_ext = ext;
	}
}

static
unsigned char tmr_get_uint8 (ixp_timer_t *tmr, unsigned long addr)
{
	pce_log (MSG_DEB, "TMR: get_uint8 (%08lX)\n", addr);

	return (0);
}

static
unsigned short tmr_get_uint16 (ixp_timer_t *tmr, unsigned long addr)
{
	pce_log (MSG_DEB, "TMR: get_uint16 (%08lX)\n", addr);

	return (0);
}

static
unsigned long tmr_get_uint32 (ixp_timer_t *tmr, unsigned long addr)
{
	switch (addr) {
	case 0x00:
	case 0x04:
	case 0x08:
	case 0x0c:
		return (ctr_get_ctrl (&tmr->cntr[(addr >> 2) & 0x03]));

	case 0x10:
	case 0x14:
	case 0x18:
	case 0x1c:
		return (ctr_get_load (&tmr->cntr[(addr >> 2) & 0x03]));

	case 0x20:
	case 0x24:
	case 0x28:
	case 0x2c:
		return (ctr_get_status (&tmr->cntr[(addr >> 2) & 0x03]));
	}

	return (0);
}

static
void tmr_set_uint8 (ixp_timer_t *tmr, unsigned long addr, unsigned char val)
{
	pce_log (MSG_DEB, "TMR: set_uint8 (%08lX, %02X)\n", addr, (unsigned) val);
}

static
void tmr_set_uint16 (ixp_timer_t *tmr, unsigned long addr, unsigned short val)
{
	pce_log (MSG_DEB, "TMR: set_uint16 (%08lX, %04X)\n", addr, (unsigned) val);
}

static
void tmr_set_uint32 (ixp_timer_t *tmr, unsigned long addr, unsigned long val)
{
	switch (addr) {
	case 0x00:
	case 0x04:
	case 0x08:
	case 0x0c:
		ctr_set_ctrl (&tmr->cntr[(addr >> 2) & 0x03], val);
		break;

	case 0x10:
	case 0x14:
	case 0x18:
	case 0x1c:
		ctr_set_load (&tmr->cntr[(addr >> 2) & 0x03], val);
		break;

	case 0x30:
	case 0x34:
	case 0x38:
	case 0x3c:
		ctr_set_clear (&tmr->cntr[(addr >> 2) & 0x03], val);
		break;
	}
}

void tmr_clock (ixp_timer_t *tmr, unsigned long n)
{
	unsigned            i;
	ixp_timer_counter_t *c;

	for (i = 0; i < 4; i++) {
		c = &tmr->cntr[i];

		if (c->clock != NULL) {
			c->clock (c, n);
		}
	}
}
