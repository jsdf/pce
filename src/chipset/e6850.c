/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6850.c                                          *
 * Created:     2013-05-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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

#include "e6850.h"


#ifndef DEBUG_ACIA
#define DEBUG_ACIA 0
#endif


#define E6850_SR_RDRF 0x01
#define E6850_SR_TDRE 0x02
#define E6850_SR_DCD  0x04
#define E6850_SR_CTS  0x08
#define E6850_SR_FE   0x10
#define E6850_SR_OVRN 0x20
#define E6850_SR_PE   0x40
#define E6850_SR_IRQ  0x80


void e6850_init (e6850_t *acia)
{
	acia->send_ext = NULL;
	acia->send_fct = NULL;

	acia->recv_ext = NULL;
	acia->recv_fct = NULL;

	acia->irq_val = 0;
	acia->irq_ext = NULL;
	acia->irq_fct = NULL;
}

void e6850_free (e6850_t *acia)
{
}

void e6850_set_irq_fct (e6850_t *acia, void *ext, void *fct)
{
	acia->irq_ext = ext;
	acia->irq_fct = fct;
}

void e6850_set_send_fct (e6850_t *acia, void *ext, void *fct)
{
	acia->send_ext = ext;
	acia->send_fct = fct;
}

void e6850_set_recv_fct (e6850_t *acia, void *ext, void *fct)
{
	acia->recv_ext = ext;
	acia->recv_fct = fct;
}

static
void e6850_set_irq (e6850_t *acia, int val)
{
	val = (val != 0);

	if (acia->irq_val != val) {
		acia->irq_val = val;

#if DEBUG_ACIA >= 2
		fprintf (stderr, "ACIA: irq = %d\n", val);
#endif

		if (acia->irq_fct != NULL) {
			acia->irq_fct (acia->irq_ext, acia->irq_val);
		}
	}
}

static
void e6850_check_int (e6850_t *acia)
{
	acia->sr &= ~E6850_SR_IRQ;

	if ((acia->cr & 0x80) && (acia->sr & E6850_SR_RDRF)) {
		acia->sr |= E6850_SR_IRQ;
	}
	else if (((acia->cr & 0x60) == 0x20) && (acia->sr & E6850_SR_TDRE)) {
		acia->sr |= E6850_SR_IRQ;
	}

	if ((acia->cr & 3) == 3) {
		/* master reset */
		acia->sr &= ~E6850_SR_IRQ;
	}

	e6850_set_irq (acia, acia->sr & E6850_SR_IRQ);
}

static
void e6850_send_tdr (e6850_t *acia)
{
	acia->tsr = acia->tdr;
	acia->sr |= E6850_SR_TDRE;
	acia->send_timer = (unsigned long) acia->char_bits << acia->clock_div;

	e6850_check_int (acia);
}

static
void e6850_send_tsr (e6850_t *acia)
{
	if (acia->send_fct != NULL) {
		acia->send_fct (acia->send_ext, acia->tsr);
	}

	if ((acia->sr & E6850_SR_TDRE) == 0) {
		e6850_send_tdr (acia);
	}
}

static
void e6850_receive_rsr (e6850_t *acia)
{
	unsigned char val;

	if (acia->recv_fct == NULL) {
		return;
	}

	if (acia->recv_fct (acia->recv_ext, &val)) {
		return;
	}

	acia->rsr = val;
	acia->recv_timer = (unsigned long) acia->char_bits << acia->clock_div;
}

static
void e6850_receive_rdr (e6850_t *acia)
{
	acia->rdr = acia->rsr;
	acia->sr |= E6850_SR_RDRF;

	e6850_receive_rsr (acia);
	e6850_check_int (acia);
}

unsigned char e6850_get_status (e6850_t *acia)
{
	return (acia->sr);
}

unsigned char e6850_get_data (e6850_t *acia)
{
	e6850_set_irq (acia, 0);

	acia->sr &= ~E6850_SR_RDRF;

	e6850_check_int (acia);

#if DEBUG_ACIA >= 2
	fprintf (stderr, "ACIA: read data %02X\n", acia->rdr);
#endif

	return (acia->rdr);
}

void e6850_set_control (e6850_t *acia, unsigned char val)
{
	unsigned char ws;

	if (acia->cr == val) {
		return;
	}

#if DEBUG_ACIA >= 1
	fprintf (stderr, "ACIA: set cr %02X\n", val);
#endif

	acia->cr = val;

	switch (val & 3) {
	case 0:
		acia->clock_div = 0;
		break;

	case 1:
		acia->clock_div = 4;
		break;

	case 2:
		acia->clock_div = 6;
		break;

	default:
		acia->clock_div = 0;
		acia->recv_timer = 0;
#if DEBUG_ACIA >= 1
		fprintf (stderr, "ACIA: master reset\n");
#endif
		return;
	}

	ws = (val >> 2) & 7;

	acia->data_bits = (ws & 2) ? 8 : 7;
	acia->stop_bits = (ws & 1) ? 1 : 2;

	if (ws == 4) {
		acia->stop_bits = 2;
	}

	acia->char_bits = acia->data_bits + acia->stop_bits;

	if ((ws != 4) && (ws != 5)) {
		acia->char_bits += 1;
	}

	e6850_check_int (acia);
}

void e6850_set_data (e6850_t *acia, unsigned char val)
{
	e6850_set_irq (acia, 0);

	acia->tdr = val;
	acia->sr &= ~E6850_SR_TDRE;

	e6850_check_int (acia);

	if (acia->send_timer == 0) {
		e6850_send_tdr (acia);
	}

	e6850_check_int (acia);
}

unsigned char e6850_get_uint8 (e6850_t *acia, unsigned long addr)
{
	switch (addr) {
	case 0:
		return (e6850_get_status (acia));

	case 1:
		return (e6850_get_data (acia));
	}

	return (0);
}

unsigned short e6850_get_uint16 (e6850_t *acia, unsigned long addr)
{
	return (0);
}

unsigned long e6850_get_uint32 (e6850_t *acia, unsigned long addr)
{
	return (0);
}

void e6850_set_uint8 (e6850_t *acia, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0:
		e6850_set_control (acia, val);
		break;

	case 1:
		e6850_set_data (acia, val);
		break;
	}
}

void e6850_set_uint16 (e6850_t *acia, unsigned long addr, unsigned short val)
{
}

void e6850_set_uint32 (e6850_t *acia, unsigned long addr, unsigned long val)
{
}

void e6850_receive (e6850_t *acia, unsigned char val)
{
	acia->rsr = val;
	acia->recv_timer = (unsigned long) acia->char_bits << acia->clock_div;
}

void e6850_reset (e6850_t *acia)
{
	acia->cr = 0;
	acia->sr = E6850_SR_TDRE;
	acia->rdr = 0;
	acia->tdr = 0;
	acia->rsr = 0;
	acia->tsr = 0;

	acia->clock_div = 0;
	acia->data_bits = 0;
	acia->stop_bits = 0;
	acia->char_bits = 0;

	acia->recv_timer = 0;
	acia->send_timer = 0;

	e6850_check_int (acia);
}

void e6850_clock (e6850_t *acia, unsigned cnt)
{
	if (acia->recv_timer > 0) {
		if (cnt < acia->recv_timer) {
			acia->recv_timer -= cnt;
		}
		else {
			acia->recv_timer = 0;

			e6850_receive_rdr (acia);
		}
	}

	if (acia->send_timer > 0) {
		if (cnt < acia->send_timer) {
			acia->send_timer -= cnt;
		}
		else {
			acia->send_timer = 0;

			e6850_send_tsr (acia);
		}
	}
}
