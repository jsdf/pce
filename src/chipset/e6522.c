/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6522.c                                          *
 * Created:     2007-11-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include "e6522.h"


#define DEBUG_VIA 0


#define E6522_IFR_CA2 0x01
#define E6522_IFR_CA1 0x02
#define E6522_IFR_SR  0x04
#define E6522_IFR_CB2 0x08
#define E6522_IFR_CB1 0x10
#define E6522_IFR_T2  0x20
#define E6522_IFR_T1  0x40
#define E6522_IFR_SET 0x80


void e6522_init (e6522_t *via, unsigned addr_shift)
{
	via->addr_shift = addr_shift;

	via->ora = 0x00;
	via->orb = 0x00;

	via->ira = 0x00;
	via->irb = 0x00;

	via->ddra = 0x00;
	via->ddrb = 0x00;

	via->shift_val = 0;
	via->shift_cnt = 0;

	via->acr = 0x00;
	via->pcr = 0x00;

	via->ifr = 0x00;
	via->ier = 0x00;

	via->t1_latch = 0;
	via->t1_val = 0;
	via->t1_hot = 0;

	via->t2_latch = 0;
	via->t2_val = 0;
	via->t2_hot = 0;

	via->ca1_inp = 0;
	via->ca2_inp = 0;

	via->cb1_inp = 0;
	via->cb2_inp = 0;

	via->set_ora_ext = NULL;
	via->set_ora = NULL;
	via->set_ora_val = 0;

	via->set_orb_ext = NULL;
	via->set_orb = NULL;
	via->set_orb_val = 0;

	via->set_cb2_ext = NULL;
	via->set_cb2 = NULL;
	via->set_cb2_val = 0;

	via->set_shift_out_ext = NULL;
	via->set_shift_out = NULL;

	via->irq_ext = NULL;
	via->irq = NULL;
	via->irq_val = 0;
}

void e6522_free (e6522_t *via)
{
}

void e6522_set_ora_fct (e6522_t *via, void *ext, void *fct)
{
	via->set_ora_ext = ext;
	via->set_ora = fct;
}

void e6522_set_orb_fct (e6522_t *via, void *ext, void *fct)
{
	via->set_orb_ext = ext;
	via->set_orb = fct;
}

void e6522_set_cb2_fct (e6522_t *via, void *ext, void *fct)
{
	via->set_cb2_ext = ext;
	via->set_cb2 = fct;
}

void e6522_set_shift_out_fct (e6522_t *via, void *ext, void *fct)
{
	via->set_shift_out_ext = ext;
	via->set_shift_out = fct;
}

void e6522_set_irq_fct (e6522_t *via, void *ext, void *fct)
{
	via->irq_ext = ext;
	via->irq = fct;
}


static
void e6522_set_ora_out (e6522_t *via)
{
	unsigned char val;

	val = via->ora & via->ddra;
	val |= via->ira & ~via->ddra;

	if (via->set_ora != NULL) {
		via->set_ora (via->set_ora_ext, val);
	}
}

static
void e6522_set_orb_out (e6522_t *via)
{
	unsigned char val;

	val = via->orb & via->ddrb;
	val |= via->irb & ~ via->ddrb;

	if (via->set_orb != NULL) {
		via->set_orb (via->set_orb_ext, val);
	}
}

/*
 * Set the CB2 output according to the VIA's state
 */
static
void e6522_set_cb2_out (e6522_t *via)
{
	unsigned char val;

	if ((via->acr & 0x1c) != 0) {
		/* shift register enabled */

		if (via->acr & 0x10) {
			val = ((via->shift_val & 1) != 0);
		}
		else {
			/* shift register input */
			val = 1;
		}
	}
	else if ((via->pcr & 0x80) == 0) {
		/* CB2 is input */
		val = 1;
	}
	else if ((via->pcr & 0x60) == 0x60) {
		/* manual output mode (1) */
		val = 1;
	}
	else if ((via->pcr & 0x60) == 0x40) {
		/* manual output mode (0) */
		val = 0;
	}
	else {
		/* not implemented */
		val = 1;
	}


	if (val == via->set_cb2_val) {
		return;
	}

	via->set_cb2_val = val;

	if (via->set_cb2 != NULL) {
		via->set_cb2 (via->set_cb2_ext, val);
	}
}

static
void e6522_set_shift_out (e6522_t *via, unsigned char val)
{
	if (via->set_shift_out != NULL) {
		via->set_shift_out (via->set_shift_out_ext, val);
	}
}

/*
 * Set the IRQ output
 */
static
void e6522_set_irq (e6522_t *via, unsigned char val)
{
	if (via->irq_val == val) {
		return;
	}

	via->irq_val = val;

	if (via->irq != NULL) {
		via->irq (via->irq_ext, val);
	}
}

/*
 * Set the IFR and trigger an interrupt if necessary
 */
static
void e6522_set_ifr (e6522_t *via, unsigned char val)
{
	val &= 0x7f;

	if (val & via->ier) {
		val |= 0x80;
	}

	via->ifr = val;

	e6522_set_irq (via, (via->ifr & 0x80) != 0);
}

/*
 * Set the IER and trigger an interrupt if necessary
 */
static
void e6522_set_ier (e6522_t *via, unsigned char val)
{
	if (val & 0x80) {
		via->ier |= (val & 0x7f);
	}
	else {
		via->ier &= (~val & 0x7f);
	}

	e6522_set_ifr (via, via->ifr);
}


unsigned char e6522_shift_out (e6522_t *via)
{
	unsigned char val;

	if ((via->acr & 0x1c) != 0x1c) {
		return (0);
	}

	val = (via->shift_val >> 7) & 1;

	via->shift_val = ((via->shift_val << 1) | (via->shift_val >> 7)) & 0xff;
	via->shift_cnt = (via->shift_cnt + 1) & 7;

	if (via->shift_cnt == 0) {
		e6522_set_ifr (via, via->ifr | E6522_IFR_SR);
	}

	return (val);
}

void e6522_shift_in (e6522_t *via, unsigned char val)
{
	if ((via->acr & 0x1c) != 0x0c) {
		return;
	}

	via->shift_val = ((via->shift_val << 1) | (val != 0)) & 0xff;
	via->shift_cnt = (via->shift_cnt + 1) & 7;

	if (via->shift_cnt == 0) {
		e6522_set_ifr (via, via->ifr | E6522_IFR_SR);
	}

	e6522_set_ifr (via, via->ifr | E6522_IFR_CB1);
}


static
unsigned char e6522_get_ora (e6522_t *via, int handshake)
{
	unsigned char val;

	if (handshake) {
		e6522_set_ifr (via, via->ifr & ~(E6522_IFR_CA1 | E6522_IFR_CA2));
	}

	val = via->ora & via->ddra;
	val |= via->ira & ~via->ddra;

	return (val);
}

static
unsigned char e6522_get_orb (e6522_t *via)
{
	unsigned char val;

	e6522_set_ifr (via, via->ifr & ~(E6522_IFR_CB1 | E6522_IFR_CB2));

	val = via->orb & via->ddrb;
	val |= via->irb & ~via->ddrb;

	return (val);
}

static
unsigned char e6522_get_t1_counter_low (e6522_t *via)
{
	e6522_set_ifr (via, via->ifr & ~E6522_IFR_T1);

	return (via->t1_val & 0xff);
}

static
unsigned char e6522_get_t1_counter_high (e6522_t *via)
{
	return ((via->t1_val >> 8) & 0xff);
}

static
unsigned char e6522_get_t1_latch_low (e6522_t *via)
{
	return (via->t1_latch & 0xff);
}

static
unsigned char e6522_get_t1_latch_high (e6522_t *via)
{
	return ((via->t1_latch >> 8) & 0xff);
}

static
unsigned char e6522_get_t2_counter_low (e6522_t *via)
{
	e6522_set_ifr (via, via->ifr & ~E6522_IFR_T2);

	return (via->t2_val & 0xff);
}

static
unsigned char e6522_get_t2_counter_high (e6522_t *via)
{
	return ((via->t2_val >> 8) & 0xff);
}

static
unsigned char e6522_get_shift (e6522_t *via)
{
	e6522_set_ifr (via, via->ifr & ~E6522_IFR_SR);

	via->shift_cnt = 0;

	return (via->shift_val);
}

static
unsigned char e6522_get_acr (e6522_t *via)
{
	return (via->acr);
}

static
unsigned char e6522_get_pcr (e6522_t *via)
{
	return (via->pcr);
}


void e6522_set_ora (e6522_t *via, unsigned char val)
{
	via->ora = val;

	e6522_set_ora_out (via);
	e6522_set_ifr (via, via->ifr & ~(E6522_IFR_CA1 | E6522_IFR_CA2));
}

void e6522_set_ddra (e6522_t *via, unsigned char val)
{
	via->ddra = val;

	e6522_set_ora_out (via);
}

void e6522_set_orb (e6522_t *via, unsigned char val)
{
	via->orb = val;

	e6522_set_orb_out (via);
	e6522_set_ifr (via, via->ifr & ~(E6522_IFR_CB1 | E6522_IFR_CB2));
}

void e6522_set_ddrb (e6522_t *via, unsigned char val)
{
	via->ddrb = val;

	e6522_set_orb_out (via);
}

static
void e6522_set_t1_latch_low (e6522_t *via, unsigned char val)
{
	via->t1_latch &= 0xff00;
	via->t1_latch |= (val & 0xff);
}

static
void e6522_set_t1_counter_high (e6522_t *via, unsigned char val)
{
	via->t1_latch &= 0x00ff;
	via->t1_latch |= (val & 0xff) << 8;

	via->t1_val = via->t1_latch;

	if ((via->acr & 0x40) == 0) {
		via->t1_hot = 1;
	}

	e6522_set_ifr (via, via->ifr & ~E6522_IFR_T1);
}

static
void e6522_set_t1_latch_high (e6522_t *via, unsigned char val)
{
	via->t1_latch &= 0x00ff;
	via->t1_latch |= (val & 0xff) << 8;
}

static
void e6522_set_t2_latch_low (e6522_t *via, unsigned char val)
{
	via->t2_latch &= 0xff00;
	via->t2_latch |= (val & 0xff);
}

static
void e6522_set_t2_counter_high (e6522_t *via, unsigned char val)
{
	via->t2_latch &= 0x00ff;
	via->t2_latch |= (val & 0xff) << 8;

	via->t2_val = via->t2_latch;

	if ((via->acr & 0x20) == 0) {
		via->t2_hot = 1;
	}

	e6522_set_ifr (via, via->ifr & ~E6522_IFR_T2);
}

static
void e6522_set_shift (e6522_t *via, unsigned char val)
{
	e6522_set_ifr (via, via->ifr & ~E6522_IFR_SR);

	via->shift_val = val;
	via->shift_cnt = 0;

	if ((via->acr & 0x1c) == 0x1c) {
		/* shift out with external clock */
		if (via->set_shift_out != NULL) {
			e6522_set_shift_out (via, via->shift_val);
			e6522_set_ifr (via, via->ifr | E6522_IFR_SR);
		}
	}
}

static
void e6522_set_acr (e6522_t *via, unsigned char val)
{
	via->acr = val;

	if ((via->acr & 0x1c) == 0) {
		/* shift register disabled */
		via->shift_cnt = 0;
	}

	e6522_set_cb2_out (via);
}

static
void e6522_set_pcr (e6522_t *via, unsigned char val)
{
	via->pcr = val;

	e6522_set_cb2_out (via);
}


void e6522_set_ca1_inp (e6522_t *via, unsigned char val)
{
	unsigned char old;

	old = (via->ca1_inp != 0);
	val = (val != 0);

	via->ca1_inp = val;

	if (old != val) {
		if ((val != 0) == ((via->pcr & 0x01) != 0)) {
			e6522_set_ifr (via, via->ifr | E6522_IFR_CA1);
		}
	}
}

void e6522_set_ca2_inp (e6522_t *via, unsigned char val)
{
	unsigned char old;

	old = (via->ca2_inp != 0);
	val = (val != 0);

	via->ca2_inp = val;

	if (via->pcr & 0x08) {
		/* CA2 is output */
		return;
	}

	if (old != val) {
		if ((val != 0) == ((via->pcr & 0x04) != 0)) {
			e6522_set_ifr (via, via->ifr | E6522_IFR_CA2);
		}
	}
}

void e6522_set_cb1_inp (e6522_t *via, unsigned char val)
{
	unsigned char old;

	old = (via->cb1_inp != 0);
	val = (val != 0);

	via->cb1_inp = val;

	if (old != val) {
		if ((val != 0) == ((via->pcr & 0x10) != 0)) {
			e6522_set_ifr (via, via->ifr | E6522_IFR_CB1);
		}
	}
}

void e6522_set_cb2_inp (e6522_t *via, unsigned char val)
{
	unsigned char old;

	old = (via->cb2_inp != 0);
	val = (val != 0);

	via->cb2_inp = val;

	if ((via->acr & 0x1c) != 0) {
		/* shift register enabled */
		return;
	}

	if (via->pcr & 0x80) {
		/* CB2 is output */
		return;
	}

	if (old != val) {
		if ((val != 0) == ((via->pcr & 0x40) != 0)) {
			e6522_set_ifr (via, via->ifr | E6522_IFR_CB2);
		}
	}
}

void e6522_set_ira_inp (e6522_t *via, unsigned char val)
{
	via->ira = val;

	e6522_set_ora_out (via);
}

void e6522_set_irb_inp (e6522_t *via, unsigned char val)
{
	via->irb = val;

	e6522_set_orb_out (via);
}

void e6522_set_shift_inp (e6522_t *via, unsigned char val)
{
	if ((via->acr & 0x1c) == 0x0c) {
		/* shift in with external clock */
		via->shift_val = val;
		via->shift_cnt = 0;
		e6522_set_ifr (via, via->ifr | E6522_IFR_SR);
	}
}


static
unsigned long e6522_addr_decode (e6522_t *via, unsigned long addr)
{
	return (addr >> via->addr_shift);
}

unsigned char e6522_get_uint8 (e6522_t *via, unsigned long addr)
{
	unsigned char val;

	addr = e6522_addr_decode (via, addr);

	switch (addr) {
	case 0x00:
		val = e6522_get_orb (via);
		break;

	case 0x01:
		val = e6522_get_ora (via, 1);
		break;

	case 0x02:
		val = via->ddrb;
		break;

	case 0x03:
		val = via->ddra;
		break;

	case 0x04:
		val = e6522_get_t1_counter_low (via);
		break;

	case 0x05:
		val = e6522_get_t1_counter_high (via);
		break;

	case 0x06:
		val = e6522_get_t1_latch_low (via);
		break;

	case 0x07:
		val = e6522_get_t1_latch_high (via);
		break;

	case 0x08:
		val = e6522_get_t2_counter_low (via);
		break;

	case 0x09:
		val = e6522_get_t2_counter_high (via);
		break;

	case 0x0a:
		val = e6522_get_shift (via);
		break;

	case 0x0b:
		val = e6522_get_acr (via);
		break;

	case 0x0c:
		val = e6522_get_pcr (via);
		break;

	case 0x0d:
		val = via->ifr;
		break;

	case 0x0e:
		val = via->ier | 0x80;
		break;

	case 0x0f:
		val = e6522_get_ora (via, 0);
		break;

	default:
		val = 0xaa;
		break;
	}

#if DEBUG_VIA
	fprintf (stderr, "via: get %04lX -> %02X\n", addr, val);
#endif

	return (val);
}

unsigned short e6522_get_uint16 (e6522_t *via, unsigned long addr)
{
	return (e6522_get_uint8 (via, addr));
}

unsigned long e6522_get_uint32 (e6522_t *via, unsigned long addr)
{
	return (e6522_get_uint8 (via, addr));
}


void e6522_set_uint8 (e6522_t *via, unsigned long addr, unsigned char val)
{
	addr = e6522_addr_decode (via, addr);

#if DEBUG_VIA
	fprintf (stderr, "via: set %04lX <- %02X\n", addr, val);
#endif

	switch (addr) {
	case 0x00:
		e6522_set_orb (via, val);
		break;

	case 0x01:
		e6522_set_ora (via, val);
		break;

	case 0x02:
		e6522_set_ddrb (via, val);
		break;

	case 0x03:
		e6522_set_ddra (via, val);
		break;

	case 0x04:
		e6522_set_t1_latch_low (via, val);
		break;

	case 0x05:
		e6522_set_t1_counter_high (via, val);
		break;

	case 0x06:
		e6522_set_t1_latch_low (via, val);
		break;

	case 0x07:
		e6522_set_t1_latch_high (via, val);
		break;

	case 0x08:
		e6522_set_t2_latch_low (via, val);
		break;

	case 0x09:
		e6522_set_t2_counter_high (via, val);
		break;

	case 0x0a:
		e6522_set_shift (via, val);
		break;

	case 0x0b:
		e6522_set_acr (via, val);
		break;

	case 0x0c:
		e6522_set_pcr (via, val);
		break;

	case 0x0d:
		e6522_set_ifr (via, via->ifr & ~val & 0x7f);
		break;

	case 0x0e:
		e6522_set_ier (via, val);
		break;

	case 0x0f:
		e6522_set_ora (via, val);
		break;

	}
}

void e6522_set_uint16 (e6522_t *via, unsigned long addr, unsigned short val)
{
	e6522_set_uint8 (via, addr, val & 0xff);
}

void e6522_set_uint32 (e6522_t *via, unsigned long addr, unsigned long val)
{
	e6522_set_uint8 (via, addr, val & 0xff);
}

void e6522_reset (e6522_t *via)
{
	via->ora = 0x00;
	via->orb = 0x00;

	via->ddra = 0x00;
	via->ddrb = 0x00;

	via->shift_val = 0;
	via->shift_cnt = 0;

	via->pcr = 0x00;
	via->acr = 0x00;

	via->ifr = 0x00;
	via->ier = 0x00;

	via->t1_latch = 0;
	via->t1_val = 0;
	via->t1_hot = 0;

	via->t2_latch = 0;
	via->t2_val = 0;
	via->t2_hot = 0;

	e6522_set_ora_out (via);
	e6522_set_orb_out (via);

	e6522_set_ifr (via, via->ifr);

#if DEBUG_VIA
	fprintf (stderr, "via: reset\n");
#endif
}

static
void e6522_clock_t1 (e6522_t *via, unsigned long n)
{
	if ((n < via->t1_val) || (via->t1_val == 0)) {
		via->t1_val = (via->t1_val - n) & 0xffff;
		return;
	}

	if (via->acr & 0x40) {
		/* free running */

		n -= via->t1_val;

		e6522_set_ifr (via, via->ifr | E6522_IFR_T1);

		if (via->t1_latch > 0) {
			n = n % via->t1_latch;
		}
		else {
			n = n & 0xffff;
		}

		via->t1_val = (via->t1_latch - n) & 0xffff;
	}
	else {
		/* one shot */

		via->t1_val = (via->t1_val - n) & 0xffff;

		if (via->t1_hot) {
			via->t1_hot = 0;
			e6522_set_ifr (via, via->ifr | E6522_IFR_T1);
		}
	}
}

static
void e6522_clock_t2 (e6522_t *via, unsigned long n)
{
	if ((n < via->t2_val) || (via->t2_val == 0)) {
		via->t2_val = (via->t2_val - n) & 0xffff;
		return;
	}

	if (via->acr & 0x20) {
		/* free running */
		;
	}
	else {
		/* one shot */

		via->t2_val = (via->t2_val - n) & 0xffff;

		if (via->t2_hot) {
			via->t2_hot = 0;
			e6522_set_ifr (via, via->ifr | E6522_IFR_T2);
		}
	}
}

void e6522_clock (e6522_t *via, unsigned long n)
{
	e6522_clock_t1 (via, n);
	e6522_clock_t2 (via, n);
}
