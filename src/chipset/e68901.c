/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e68901.c                                         *
 * Created:     2011-06-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include "e68901.h"


#ifndef DEBUG_MFP
#define DEBUG_MFP 0
#endif


#define MFP_REG_GPIP  0
#define MFP_REG_AER   1
#define MFP_REG_DDR   2
#define MFP_REG_IERA  3
#define MFP_REG_IERB  4
#define MFP_REG_IPRA  5
#define MFP_REG_IPRB  6
#define MFP_REG_ISRA  7
#define MFP_REG_ISRB  8
#define MFP_REG_IMRA  9
#define MFP_REG_IMRB  10
#define MFP_REG_IVR   11
#define MFP_REG_TACR  12
#define MFP_REG_TBCR  13
#define MFP_REG_TCDCR 14
#define MFP_REG_TADR  15
#define MFP_REG_TBDR  16
#define MFP_REG_TCDR  17
#define MFP_REG_TDDR  18
#define MFP_REG_SYNC  19
#define MFP_REG_UCR   20
#define MFP_REG_RSR   21
#define MFP_REG_TSR   22
#define MFP_REG_DATA  23

#define MFP_RSR_BF   0x80
#define MFP_RSR_OE   0x40
#define MFP_RSR_PE   0x20
#define MFP_RSR_FE   0x10
#define MFP_RSR_FS   0x08
#define MFP_RSR_B    0x08
#define MFP_RSR_MCIP 0x04
#define MFP_RSR_SS   0x02
#define MFP_RSR_RE   0x01

#define MFP_TSR_BE  0x80
#define MFP_TSR_UE  0x40
#define MFP_TSR_AT  0x20
#define MFP_TSR_END 0x10
#define MFP_TSR_B   0x08
#define MFP_TSR_HI  0x04
#define MFP_TSR_LO  0x02
#define MFP_TSR_XE  0x01


static void timer_set_inp (e68901_t *mfp, unsigned idx, char val);


void e68901_init (e68901_t *mfp, unsigned addr_shift)
{
	unsigned i;

	mfp->addr_shift = addr_shift;

	mfp->gpip_xor = 0xff;
	mfp->gpip_inp = 0;
	mfp->irr1 = 0;
	mfp->irr2 = 0;

	mfp->usart_timer = 0xff;

	for (i = 0; i < 4; i++) {
		mfp->timer[i].inp = 0;
		mfp->timer[i].clk_div_inp = 1;
	}

	mfp->timer[0].int_mask = 1U << 13;
	mfp->timer[1].int_mask = 1U << 8;
	mfp->timer[2].int_mask = 1U << 5;
	mfp->timer[3].int_mask = 1U << 4;

	mfp->recv_ext = NULL;
	mfp->recv_fct = NULL;

	mfp->send_ext = NULL;
	mfp->send_fct = NULL;

	mfp->irq_val = 0;
	mfp->irq_ext = NULL;
	mfp->irq_fct = NULL;
}

void e68901_free (e68901_t *mfp)
{
}

void e68901_set_irq_fct (e68901_t *mfp, void *ext, void *fct)
{
	mfp->irq_ext = ext;
	mfp->irq_fct = fct;
}

void e68901_set_recv_fct (e68901_t *mfp, void *ext, void *fct)
{
	mfp->recv_ext = ext;
	mfp->recv_fct = fct;
}

void e68901_set_send_fct (e68901_t *mfp, void *ext, void *fct)
{
	mfp->send_ext = ext;
	mfp->send_fct = fct;
}

void e68901_set_usart_timer (e68901_t *mfp, unsigned idx)
{
	mfp->usart_timer = idx;
}

void e68901_set_clk_div (e68901_t *mfp, unsigned div)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		mfp->timer[i].clk_div_inp = div;
	}
}

static
void e68901_set_irq (e68901_t *mfp, int val)
{
	val = (val != 0);

	if (mfp->irq_val != val) {
		mfp->irq_val = val;

		if (mfp->irq_fct != NULL) {
			mfp->irq_fct (mfp->irq_ext, mfp->irq_val);
		}
	}
}

static
void e68901_check_int (e68901_t *mfp)
{
	int            irq;
	unsigned short tmp, inp;

	inp = (mfp->gpip_inp ^ mfp->gpip_xor) ^ ~mfp->gpip_aer;
	mfp->irr1 &= 0x3f30;
	mfp->irr1 |= (inp & 0x0f) | ((inp << 2) & 0xc0) | ((inp << 8) & 0xc000);
	mfp->ipr |= (mfp->irr1 ^ mfp->irr2) & mfp->irr1;
	mfp->ipr &= mfp->ier;
	mfp->irr2 = mfp->irr1;

	if (mfp->ivr & 0x08) {
		tmp = mfp->ipr & mfp->imr;

		while (tmp & (tmp - 1)) {
			tmp &= tmp - 1;
		}

		irq = tmp > mfp->isr;
	}
	else {
		irq = (mfp->ipr & mfp->imr) != 0;
	}

	e68901_set_irq (mfp, irq);
}

static
void e68901_check_usart_int (e68901_t *mfp)
{
	unsigned short irr;

	irr = mfp->irr1 & 0xe1ff;

	if (mfp->rsr[0] & (MFP_RSR_OE | MFP_RSR_PE | MFP_RSR_FE)) {
		irr |= 1U << 11;
	}
	else if (mfp->rsr[0] & MFP_RSR_BF) {
		irr |= 1U << 12;
	}

	if (mfp->tsr[0] & (MFP_TSR_UE | MFP_TSR_END)) {
		irr |= 1U << 9;
	}
	else if (mfp->tsr[0] & MFP_TSR_BE) {
		irr |= 1U << 10;
	}

	if (mfp->irr1 != irr) {
		mfp->irr1 = irr;

		e68901_check_int (mfp);
	}
}

unsigned e68901_inta (e68901_t *mfp)
{
	unsigned short tmp, idx;

	tmp = mfp->ipr & mfp->imr;

	if (tmp == 0) {
		return (-1);
	}

	idx = 15;

	while ((tmp >> idx) == 0) {
		idx -= 1;
	}

	tmp = 1U << idx;

	mfp->ipr &= ~tmp;

	if (mfp->ivr & 0x08) {
		mfp->isr |= tmp;
	}

	mfp->vec = (mfp->ivr & 0xf0) | (idx & 0x0f);

	e68901_check_int (mfp);

	return (mfp->vec);
}

void e68901_set_inp (e68901_t *mfp, unsigned char val)
{
	if (mfp->gpip_inp == val) {
		return;
	}

	mfp->gpip_inp = val;

	e68901_check_int (mfp);
}

void e68901_set_inp_n (e68901_t *mfp, unsigned idx, unsigned char val)
{
	if (val) {
		val = mfp->gpip_inp | (1U << idx);
	}
	else {
		val = mfp->gpip_inp & ~(1U << idx);
	}

	e68901_set_inp (mfp, val);
}

void e68901_set_inp_4 (e68901_t *mfp, unsigned char val)
{
	e68901_set_inp_n (mfp, 4, val);
}

void e68901_set_inp_5 (e68901_t *mfp, unsigned char val)
{
	e68901_set_inp_n (mfp, 5, val);
}

void e68901_set_tbi (e68901_t *mfp, unsigned char val)
{
	timer_set_inp (mfp, 1, val != 0);
}


static
unsigned char e68901_get_gpip_val (const e68901_t *mfp)
{
	return ((mfp->gpip_val & mfp->gpip_ddr) | ((mfp->gpip_inp ^ mfp->gpip_xor) & ~mfp->gpip_ddr));
}


static
void timer_set_cr (e68901_t *mfp, unsigned idx, unsigned char val)
{
	e68901_timer_t *tmr;

	static unsigned char div[8] = {
		0, 4, 10, 16, 50, 64, 100, 200
	};

	tmr = &mfp->timer[idx & 3];

	tmr->cr = val & 0x0f;

	tmr->clk_div_set = div[val & 7];

	tmr->clk_div = tmr->clk_div_inp * tmr->clk_div_set;
	tmr->clk_val = 0;

	if (val & 0x10) {
		tmr->out = 0;
	}
}

static
void timer_set_dr (e68901_timer_t *tmr, unsigned char val)
{
	tmr->dr[1] = val;

	if ((tmr->cr & 0x0f) == 0) {
		tmr->dr[0] = val;
		tmr->clk_val = 0;
	}
}

static
void timer_pulse (e68901_t *mfp, unsigned idx)
{
	e68901_timer_t *tmr;

	tmr = &mfp->timer[idx & 3];

	tmr->dr[0] = (tmr->dr[0] - 1) & 0xff;

	if (tmr->dr[0] == 0) {
		tmr->dr[0] = tmr->dr[1];
		tmr->out = !tmr->out;

		if (mfp->ier & tmr->int_mask) {
			mfp->ipr |= tmr->int_mask;
			e68901_check_int (mfp);
		}

		if (mfp->usart_timer == idx) {
			if (tmr->out) {
				e68901_clock_usart (mfp, 1);
			}
		}
	}
}

static
void timer_set_inp (e68901_t *mfp, unsigned idx, char val)
{
	e68901_timer_t *tmr;

	tmr = &mfp->timer[idx & 3];

	if (tmr->inp == val) {
		return;
	}

	tmr->inp = val;

	if ((tmr->cr & 8) == 0) {
		return;
	}

	if ((tmr->cr & 0x0f) == 8) {
		/* event counter */

		if (val) {
			timer_pulse (mfp, idx);
		}
	}
}

static
void timer_clock (e68901_t *mfp, unsigned idx, unsigned cnt)
{
	e68901_timer_t *tmr;

	tmr = &mfp->timer[idx & 3];

	if (tmr->clk_div == 0) {
		return;
	}

	if ((tmr->cr & 0x07) == 0) {
		return;
	}

	if ((tmr->cr & 8) && (tmr->inp == 0)) {
		return;
	}

	tmr->clk_val += cnt;

	while (tmr->clk_val >= tmr->clk_div) {
		tmr->clk_val -= tmr->clk_div;

		timer_pulse (mfp, idx);
	}
}

static
void usart_set_ucr (e68901_t *mfp, unsigned char val)
{
	unsigned bits;

	mfp->ucr = val & 0xfe;

	bits = 1;
	bits += 8 - ((val >> 5) & 3);

	switch ((val >> 3) & 3) {
	case 1:
		bits += 1;
		break;

	case 2:
		bits += 2;
		break;

	case 3:
		bits += 2;
		break;
	}

	if (val & 4) {
		/* parity enabled */
		bits += 1;
	}

	if (val & 0x80) {
		bits *= 16;
	}

	mfp->recv_clk_max = bits;
	mfp->send_clk_max = bits;
}

static
unsigned char usart_get_ucr (e68901_t *mfp)
{
	return (mfp->ucr);
}

static
void usart_set_rsr (e68901_t *mfp, unsigned char val)
{
	mfp->rsr[0] = (mfp->rsr[0] & 0xfe) | (val & ~0xfe);

	if ((val & MFP_RSR_RE) == 0) {
		mfp->rsr[0] = 0;
		mfp->recv_clk_cnt = 0;
	}

	e68901_check_usart_int (mfp);
}

static
unsigned char usart_get_rsr (e68901_t *mfp)
{
	unsigned char val;

	val = mfp->rsr[0];

	mfp->rsr[0] &= ~MFP_RSR_OE;

	e68901_check_usart_int (mfp);

	return (val);
}

static
void usart_set_tsr (e68901_t *mfp, unsigned char val)
{
	mfp->tsr[0] = (mfp->tsr[0] & 0xfe) | (val & ~0xfe);

	if (mfp->tsr[0] & MFP_TSR_XE) {
		mfp->tsr[0] &= ~MFP_TSR_END;
	}
	else {
		if (mfp->send_clk_cnt == 0) {
			mfp->tsr[0] |= MFP_TSR_END;
		}
	}

	e68901_check_usart_int (mfp);
}

static
unsigned char usart_get_tsr (e68901_t *mfp)
{
	unsigned char val;

	val = mfp->tsr[0];

	mfp->tsr[0] &= ~MFP_TSR_UE;

	e68901_check_usart_int (mfp);

	return (val);
}

static
void usart_set_tdr (e68901_t *mfp, unsigned char val)
{
	mfp->tdr[0] = val;
	mfp->tsr[0] &= ~MFP_TSR_BE;

	e68901_check_usart_int (mfp);

	if (mfp->send_clk_cnt > 0) {
		return;
	}

	if ((mfp->tsr[0] & MFP_TSR_XE) == 0) {
		return;
	}

	mfp->tdr[1] = mfp->tdr[0];
	mfp->tsr[1] = mfp->tsr[0];
	mfp->tsr[0] |= MFP_TSR_BE;
	mfp->send_clk_cnt = mfp->send_clk_max;

	e68901_check_usart_int (mfp);
}

static
unsigned char usart_get_rdr (e68901_t *mfp)
{
	unsigned char val;

	val = mfp->rdr[0];

	mfp->rsr[0] &= ~MFP_RSR_BF;

	e68901_check_usart_int (mfp);

	return (val);
}

unsigned char e68901_get_uint8 (e68901_t *mfp, unsigned long addr)
{
	unsigned      reg;
	unsigned char val;

	reg = addr >> mfp->addr_shift;

	switch (reg) {
	case MFP_REG_GPIP:
		val = e68901_get_gpip_val (mfp);
		break;

	case MFP_REG_AER:
		val = mfp->gpip_aer;
		break;

	case MFP_REG_DDR:
		val = mfp->gpip_ddr;
		break;

	case MFP_REG_IERA:
		val = (mfp->ier >> 8) & 0xff;
		break;

	case MFP_REG_IERB:
		val = mfp->ier & 0xff;
		break;

	case MFP_REG_IPRA:
		val = (mfp->ipr >> 8) & 0xff;
		break;

	case MFP_REG_IPRB:
		val = mfp->ipr & 0xff;
		break;

	case MFP_REG_ISRA:
		val = (mfp->isr >> 8) & 0xff;
		break;

	case MFP_REG_ISRB:
		val = mfp->isr & 0xff;
		break;

	case MFP_REG_IMRA:
		val = (mfp->imr >> 8) & 0xff;
		break;

	case MFP_REG_IMRB:
		val = mfp->imr & 0xff;
		break;

	case MFP_REG_IVR:
		val = mfp->vec;
		break;

	case MFP_REG_TACR:
		return (mfp->timer[0].cr & 0x0f);

	case MFP_REG_TBCR:
		return (mfp->timer[1].cr & 0x0f);

	case MFP_REG_TCDCR:
		return (((mfp->timer[2].cr & 7) << 4) | (mfp->timer[3].cr & 7));

	case MFP_REG_TADR:
		return (mfp->timer[0].dr[0]);

	case MFP_REG_TBDR:
		return (mfp->timer[1].dr[0]);

	case MFP_REG_TCDR:
		return (mfp->timer[2].dr[0]);

	case MFP_REG_TDDR:
		return (mfp->timer[3].dr[0]);

	case MFP_REG_UCR:
		return (usart_get_ucr (mfp));

	case MFP_REG_RSR:
		return (usart_get_rsr (mfp));

	case MFP_REG_TSR:
		return (usart_get_tsr (mfp));

	case MFP_REG_DATA:
		return (usart_get_rdr (mfp));

	default:
		val = 0xaa;
#if (DEBUG_MFP >= 1) && (DEBUG_MFP < 2)
		fprintf (stderr, "mfp: get %04X -> %02X\n", reg, val);
#endif
		break;
	}

#if DEBUG_MFP >= 2
	fprintf (stderr, "mfp: get %04X -> %02X\n", reg, val);
#endif

	return (val);
}

unsigned short e68901_get_uint16 (e68901_t *mfp, unsigned long addr)
{
	return (e68901_get_uint8 (mfp, addr));
}

unsigned long e68901_get_uint32 (e68901_t *mfp, unsigned long addr)
{
	unsigned long val;

	val = e68901_get_uint16 (mfp, addr);
	val <<= 16;
	val |= e68901_get_uint16 (mfp, addr + 2);

	return (val);
}


void e68901_set_uint8 (e68901_t *mfp, unsigned long addr, unsigned char val)
{
	unsigned reg;

	reg = addr >> mfp->addr_shift;

	val &= 0xff;

#if DEBUG_MFP >= 2
	fprintf (stderr, "mfp: set %04X <- %02X\n", reg, val);
#endif

	switch (reg) {
	case MFP_REG_GPIP:
		mfp->gpip_val = val;
		break;

	case MFP_REG_AER:
		mfp->gpip_aer = val;
		e68901_check_int (mfp);
		break;

	case MFP_REG_DDR:
		mfp->gpip_ddr = val;
		break;

	case MFP_REG_IERA:
		mfp->ier = (mfp->ier & 0x00ff) | (val << 8);
		e68901_check_int (mfp);
		break;

	case MFP_REG_IERB:
		mfp->ier = (mfp->ier & 0xff00) | val;
		e68901_check_int (mfp);
		break;

	case MFP_REG_IPRA:
		mfp->ipr &= (val << 8) | 0x00ff;
		e68901_check_int (mfp);
		break;

	case MFP_REG_IPRB:
		mfp->ipr &= val | 0xff00;
		e68901_check_int (mfp);
		break;

	case MFP_REG_ISRA:
		mfp->isr &= (val << 8) | 0x00ff;
		e68901_check_int (mfp);
		break;

	case MFP_REG_ISRB:
		mfp->isr &= val | 0xff00;
		e68901_check_int (mfp);
		break;

	case MFP_REG_IMRA:
		mfp->imr = (mfp->imr & 0x00ff) | (val << 8);
		e68901_check_int (mfp);
		break;

	case MFP_REG_IMRB:
		mfp->imr = (mfp->imr & 0xff00) | val;
		e68901_check_int (mfp);
		break;

	case MFP_REG_IVR:
		mfp->ivr = val & 0xf8;
		break;

	case MFP_REG_TACR:
		timer_set_cr (mfp, 0, val);
		break;

	case MFP_REG_TBCR:
		timer_set_cr (mfp, 1, val);
		break;

	case MFP_REG_TCDCR:
		timer_set_cr (mfp, 2, (val >> 4) & 7);
		timer_set_cr (mfp, 3, val & 7);
		break;

	case MFP_REG_TADR:
		timer_set_dr (mfp->timer + 0, val);
		break;

	case MFP_REG_TBDR:
		timer_set_dr (mfp->timer + 1, val);
		break;

	case MFP_REG_TCDR:
		timer_set_dr (mfp->timer + 2, val);
		break;

	case MFP_REG_TDDR:
		timer_set_dr (mfp->timer + 3, val);
		break;

	case MFP_REG_UCR:
		usart_set_ucr (mfp, val);
		break;

	case MFP_REG_RSR:
		usart_set_rsr (mfp, val);
		break;

	case MFP_REG_TSR:
		usart_set_tsr (mfp, val);
		break;

	case MFP_REG_DATA:
		usart_set_tdr (mfp, val);
		break;

	default:
#if (DEBUG_MFP >= 1) && (DEBUG_MFP < 2)
		fprintf (stderr, "mfp: set %04X <- %02X\n", reg, val);
#endif
		break;
	}
}

void e68901_set_uint16 (e68901_t *mfp, unsigned long addr, unsigned short val)
{
	e68901_set_uint8 (mfp, addr, val & 0xff);
}

void e68901_set_uint32 (e68901_t *mfp, unsigned long addr, unsigned long val)
{
	e68901_set_uint16 (mfp, addr, val >> 16);
	e68901_set_uint16 (mfp, addr + 2, val);
}

int e68901_receive (e68901_t *mfp, unsigned char val)
{
	if ((mfp->rsr[0] & MFP_RSR_RE) == 0) {
		return (0);
	}

	if (mfp->rsr[1] & MFP_RSR_BF) {
		return (1);
	}

	mfp->rdr[1] = val;
	mfp->rsr[1] = MFP_RSR_BF;
	mfp->recv_clk_cnt = mfp->recv_clk_max;

	return (0);
}

void e68901_reset (e68901_t *mfp)
{
	unsigned i;

#if DEBUG_MFP >= 1
	fprintf (stderr, "mfp: reset\n");
#endif

	mfp->gpip_val = 0;
	mfp->gpip_aer = 0;
	mfp->gpip_ddr = 0;

	mfp->ier = 0;
	mfp->ipr = 0;
	mfp->isr = 0;
	mfp->imr = 0;
	mfp->ivr = 0;

	for (i = 0; i < 4; i++) {
		mfp->timer[i].cr = 0;
		mfp->timer[i].dr[0] = 0;
		mfp->timer[i].dr[1] = 0;
		mfp->timer[i].out = 0;
		mfp->timer[i].clk_div_set = 0;
		mfp->timer[i].clk_div = 0;
		mfp->timer[i].clk_val = 0;
	}

	mfp->ucr = 0;
	mfp->recv_clk_cnt = 0;
	mfp->recv_clk_max = 0;
	mfp->send_clk_cnt = 0;
	mfp->send_clk_max = 0;

	for (i = 0; i < 2; i++) {
		mfp->rsr[i] = 0;
		mfp->tsr[i] = MFP_TSR_BE;
		mfp->rdr[i] = 0;
		mfp->tdr[i] = 0;
	}

	e68901_check_int (mfp);
}

void e68901_clock_usart (e68901_t *mfp, unsigned n)
{
	if (mfp->recv_clk_cnt > 0) {
		if (n < mfp->recv_clk_cnt) {
			mfp->recv_clk_cnt -= n;
		}
		else {
			mfp->recv_clk_cnt = 0;

			if (mfp->rsr[0] & MFP_RSR_BF) {
				mfp->rsr[1] |= MFP_RSR_OE;
			}
			else {
				mfp->rsr[0] &= MFP_RSR_RE;
				mfp->rsr[0] |= (mfp->rsr[1] | MFP_RSR_BF) & ~MFP_RSR_RE;
				mfp->rdr[0] = mfp->rdr[1];
				mfp->rsr[1] = mfp->rsr[0] & MFP_RSR_RE;
			}

			if (mfp->recv_fct != NULL) {
				if (mfp->recv_fct (mfp->recv_ext, mfp->rdr + 1) == 0) {
					mfp->rsr[1] |= MFP_RSR_BF;
					mfp->recv_clk_cnt = mfp->recv_clk_max;
				}
			}

			e68901_check_usart_int (mfp);
		}
	}

	if (mfp->send_clk_cnt > 0) {
		if (n < mfp->send_clk_cnt) {
			mfp->send_clk_cnt -= n;
		}
		else {
			mfp->send_clk_cnt = 0;

			if (mfp->send_fct != NULL) {
				mfp->send_fct (mfp->send_ext, mfp->tdr[1]);
			}

			if ((mfp->tsr[0] & MFP_TSR_XE) == 0) {
				mfp->tsr[0] |= MFP_TSR_END;
			}
			else if (mfp->tsr[0] & MFP_TSR_BE) {
				mfp->tsr[0] |= MFP_TSR_UE;
			}
			else {
				mfp->tsr[1] = mfp->tsr[0];
				mfp->tdr[1] = mfp->tdr[0];

				mfp->tsr[0] |= MFP_TSR_BE;

				mfp->send_clk_cnt = mfp->send_clk_max;
			}

			e68901_check_usart_int (mfp);
		}
	}
}

void e68901_clock (e68901_t *mfp, unsigned n)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		timer_clock (mfp, i, n);
	}
}
