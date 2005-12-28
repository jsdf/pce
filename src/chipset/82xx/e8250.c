/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/chipset/82xx/e8250.c                                   *
 * Created:       2003-08-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2005 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "e8250.h"


void e8250_init (e8250_t *uart)
{
	uart->chip = E8250_CHIP_16450;

	uart->inp_i = 0;
	uart->inp_j = 0;
	uart->inp_n = 8;

	uart->out_i = 0;
	uart->out_j = 0;
	uart->out_n = 8;

	uart->txd[0] = 0;
	uart->txd[1] = 0;
	uart->rxd[0] = 0;
	uart->txd[1] = 0;

	uart->ier = 0;
	uart->iir = E8250_IIR_PND;

	uart->lcr = 0;
	uart->lsr = E8250_LSR_TXE | E8250_LSR_TBE;

	uart->mcr = 0;
	uart->msr = 0; /* E8250_MSR_DCD | E8250_MSR_DSR | E8250_MSR_CTS; */

	uart->rxd_read_cnt = 0;
	uart->txd_write_cnt = 0;

	uart->have_scratch = 1;

	uart->ipr = 0;

	uart->scratch = 0;

	uart->divisor = 0;

	uart->irq_val = 0;
	uart->irq_ext = NULL;
	uart->irq = NULL;

	uart->send_ext = NULL;
	uart->send = NULL;

	uart->recv_ext = NULL;
	uart->recv = NULL;

	uart->setup_ext = NULL;
	uart->setup = NULL;
}

e8250_t *e8250_new (void)
{
	e8250_t *uart;

	uart = (e8250_t *) malloc (sizeof (e8250_t));
	if (uart == NULL) {
		return (NULL);
	}

	e8250_init (uart);

	return (uart);
}

void e8250_free (e8250_t *uart)
{
}

void e8250_del (e8250_t *uart)
{
	if (uart != NULL) {
		e8250_free (uart);
		free (uart);
	}
}

void e8250_set_chip_8250 (e8250_t *uart)
{
	uart->chip = E8250_CHIP_8250;
	uart->have_scratch = 0;
}

void e8250_set_chip_16450 (e8250_t *uart)
{
	uart->chip = E8250_CHIP_16450;
	uart->have_scratch = 1;
}

int e8250_set_chip (e8250_t *uart, unsigned chip)
{
	switch (chip) {
		case E8250_CHIP_8250:
			e8250_set_chip_8250 (uart);
			return (0);

		case E8250_CHIP_16450:
			e8250_set_chip_16450 (uart);
			return (0);
	}

	return (1);
}

int e8250_set_chip_str (e8250_t *uart, const char *str)
{
	if (strcmp (str, "8250") == 0) {
		e8250_set_chip_8250 (uart);
		return (0);
	}

	if (strcmp (str, "16450") == 0) {
		e8250_set_chip_16450 (uart);
		return (0);
	}

	return (1);
}

void e8250_set_irq_f (e8250_t *uart, void *fct, void *ext)
{
	uart->irq = fct;
	uart->irq_ext = ext;
}

void e8250_set_send_f (e8250_t *uart, void *fct, void *ext)
{
	uart->send = fct;
	uart->send_ext = ext;
}

void e8250_set_recv_f (e8250_t *uart, void *fct, void *ext)
{
	uart->recv = fct;
	uart->recv_ext = ext;
}

void e8250_set_setup_f (e8250_t *uart, void *fct, void *ext)
{
	uart->setup = fct;
	uart->setup_ext = ext;
}

void e8250_set_buf_size (e8250_t *uart, unsigned inp, unsigned out)
{
	uart->inp_i = 0;
	uart->inp_j = 0;
	uart->inp_n = (inp <= E8250_BUF_MAX) ? inp : E8250_BUF_MAX;

	uart->out_i = 0;
	uart->out_j = 0;
	uart->out_n = (out <= E8250_BUF_MAX) ? out : E8250_BUF_MAX;
}

static
void e8250_set_irq (e8250_t *uart, unsigned char val)
{
	if (uart->irq_val != val) {
		uart->irq_val = val;
		if (uart->irq != NULL) {
			uart->irq (uart->irq_ext, val);
		}
	}
}

static
void e8250_set_int_cond (e8250_t *uart, unsigned char c)
{
	unsigned char val;

	uart->ipr |= c;

	val = uart->ipr & uart->ier;

	if ((val & E8250_IER_RRD) && (uart->iir < E8250_IIR_RRD)) {
		uart->iir = E8250_IIR_RRD;
	}
	else if ((val & E8250_IER_TBE) && (uart->iir < E8250_IIR_TBE)) {
		uart->iir = E8250_IIR_TBE;
	}

	if (uart->mcr & E8250_MCR_OUT2) {
		e8250_set_irq (uart, (uart->iir & E8250_IIR_PND) == 0);
	}
}

static
void e8250_clr_int_cond (e8250_t *uart, unsigned char c)
{
	uart->ipr &= ~c;

	if ((c & E8250_IER_RRD) && (uart->iir == E8250_IIR_RRD)) {
		uart->iir = E8250_IIR_PND;
	}
	if ((c & E8250_IER_TBE) && (uart->iir == E8250_IIR_TBE)) {
		uart->iir = E8250_IIR_PND;
	}

	e8250_set_int_cond (uart, 0);
}

static
void e8250_check_rxd (e8250_t *uart)
{
	if (uart->rxd[1]) {
		return;
	}

	if (e8250_get_inp (uart, &uart->rxd[0])) {
		return;
	}

	uart->rxd[1] = 1;
	uart->lsr |= E8250_LSR_RRD;

	/* receive queue is not full */
	if (uart->recv != NULL) {
		uart->recv (uart->recv_ext, 1);
	}

	e8250_set_int_cond (uart, E8250_IER_RRD);
}

static
void e8250_check_txd (e8250_t *uart)
{
	if (uart->txd[1] == 0) {
		return;
	}

	if (uart->mcr & E8250_MCR_LOOP) {
		e8250_set_inp (uart, uart->txd[0]);
	}
	else {
		if (e8250_set_out (uart, uart->txd[0])) {
			return;
		}
	}

	uart->txd[1] = 0;
	uart->lsr |= E8250_LSR_TBE;

	if (uart->mcr & E8250_MCR_LOOP) {
		e8250_check_rxd (uart);
	}
	else {
		if (uart->send != NULL) {
			uart->send (uart->send_ext, 1);
		}
	}

	e8250_set_int_cond (uart, E8250_IER_TBE);
}

unsigned short e8250_get_divisor (e8250_t *uart)
{
	return (uart->divisor);
}

unsigned long e8250_get_bps (e8250_t *uart)
{
	return (115200UL / ((uart->divisor > 0) ? uart->divisor : 1));
}

unsigned e8250_get_databits (e8250_t *uart)
{
	return (5 + (uart->lcr & 0x03));
}

unsigned e8250_get_stopbits (e8250_t *uart)
{
	return ((uart->lcr & E8250_LCR_STOP) ? 2 : 1);
}

unsigned e8250_get_parity (e8250_t *uart)
{
	return ((uart->lcr & E8250_LCR_PARITY) >> 3);
}


int e8250_get_dtr (e8250_t *uart)
{
	return ((uart->mcr & E8250_MCR_DTR) != 0);
}

int e8250_get_rts (e8250_t *uart)
{
	return ((uart->mcr & E8250_MCR_RTS) != 0);
}


void e8250_set_dsr (e8250_t *uart, unsigned char val)
{
	unsigned char msr;

	msr = uart->msr;

	if (val) {
		uart->msr |= E8250_MSR_DSR;
	}
	else {
		uart->msr &= ~E8250_MSR_DSR;
	}

	if ((msr ^ uart->msr) & E8250_MSR_DSR) {
		uart->msr |= E8250_MSR_DDSR;
	}
	else {
		uart->msr &= ~E8250_MSR_DDSR;
	}
}

void e8250_set_cts (e8250_t *uart, unsigned char val)
{
	unsigned char msr;

	msr = uart->msr;

	if (val) {
		uart->msr |= E8250_MSR_CTS;
	}
	else {
		uart->msr &= ~E8250_MSR_CTS;
	}

	if ((msr ^ uart->msr) & E8250_MSR_CTS) {
		uart->msr |= E8250_MSR_DCTS;
	}
	else {
		uart->msr &= ~E8250_MSR_DCTS;
	}
}

void e8250_set_dcd (e8250_t *uart, unsigned char val)
{
	unsigned char msr;

	msr = uart->msr;

	if (val) {
		uart->msr |= E8250_MSR_DCD;
	}
	else {
		uart->msr &= ~E8250_MSR_DCD;
	}

	if ((msr ^ uart->msr) & E8250_MSR_DCD) {
		uart->msr |= E8250_MSR_DDCD;
	}
	else {
		uart->msr &= ~E8250_MSR_DDCD;
	}
}


int e8250_set_inp (e8250_t *uart, unsigned char val)
{
	unsigned t;

	t = uart->inp_j + 1;
	t = (t < uart->inp_n) ? t : 0;

	if (t == uart->inp_i) {
		return (1);
	}

	uart->inp[uart->inp_j] = val;
	uart->inp_j = t;

	return (0);
}

int e8250_get_inp (e8250_t *uart, unsigned char *val)
{
	if (uart->inp_i == uart->inp_j) {
		return (1);
	}

	*val = uart->inp[uart->inp_i];

	uart->inp_i += 1;
	if (uart->inp_i >= uart->inp_n) {
		uart->inp_i = 0;
	}

	return (0);
}

void e8250_get_inp_all (e8250_t *uart)
{
	uart->inp_i = 0;
	uart->inp_j = 0;
}

int e8250_inp_full (e8250_t *uart)
{
	unsigned t;

	t = uart->inp_j + 1;
	t = (t < uart->inp_n) ? t : 0;

	if (t == uart->inp_i) {
		return (1);
	}

	return (0);
}

int e8250_inp_empty (e8250_t *uart)
{
	return (uart->inp_i == uart->inp_j);
}


int e8250_set_out (e8250_t *uart, unsigned char val)
{
	unsigned t;

	t = uart->out_j + 1;
	t = (t < uart->out_n) ? t : 0;

	if (t == uart->out_i) {
		return (1);
	}

	uart->out[uart->out_j] = val;
	uart->out_j = t;

	return (0);
}

int e8250_get_out (e8250_t *uart, unsigned char *val)
{
	if (uart->out_i == uart->out_j) {
		return (1);
	}

	*val = uart->out[uart->out_i];

	uart->out_i += 1;
	if (uart->out_i >= uart->out_n) {
		uart->out_i = 0;
	}

	return (0);
}

void e8250_get_out_all (e8250_t *uart)
{
	uart->out_i = 0;
	uart->out_j = 0;
}

int e8250_receive (e8250_t *uart, unsigned char val)
{
	if (e8250_set_inp (uart, val)) {
		return (1);
	}

	e8250_check_rxd (uart);

	return (0);
}


unsigned char e8250_get_div_lo (e8250_t *uart)
{
	return (uart->divisor & 0xff);
}

unsigned char e8250_get_div_hi (e8250_t *uart)
{
	return ((uart->divisor >> 8) & 0xff);
}

unsigned char e8250_get_rxd (e8250_t *uart)
{
	return (uart->rxd[0]);
}

unsigned char e8250_get_ier (e8250_t *uart)
{
	return (uart->ier);
}

unsigned char e8250_get_iir (e8250_t *uart)
{
	return (uart->iir);
}

unsigned char e8250_get_lcr (e8250_t *uart)
{
	return (uart->lcr);
}

unsigned char e8250_get_mcr (e8250_t *uart)
{
	return (uart->mcr);
}

unsigned char e8250_get_lsr (e8250_t *uart)
{
	return (uart->lsr);
}

unsigned char e8250_get_msr (e8250_t *uart)
{
	return (uart->msr);
}

unsigned char e8250_get_scratch (e8250_t *uart)
{
	return (uart->scratch);
}

static
unsigned char e8250_read_rxd (e8250_t *uart)
{
	unsigned char val;

	val = uart->rxd[0];

	uart->rxd[1] = 0;

	uart->lsr &= ~E8250_LSR_RRD;
	e8250_clr_int_cond (uart, E8250_IER_RRD);

	uart->rxd_read_cnt += 1;

	/* every uart->inp_n bytes, give the reader a break */
	if (uart->rxd_read_cnt < uart->inp_n) {
		e8250_check_rxd (uart);
	}

	/* rxd still empty -> reset read count */
	if (uart->rxd[1] == 0) {
		uart->rxd_read_cnt = 0;
	}

	return (val);
}

static
unsigned char e8250_read_iir (e8250_t *uart)
{
	unsigned char val;

	val = uart->iir;

	e8250_clr_int_cond (uart, E8250_IER_TBE);

	return (val);
}

static
unsigned char e8250_read_msr (e8250_t *uart)
{
	unsigned char val;

	val = uart->msr;

	uart->msr &= 0xf0;

	return (val);
}


void e8250_set_scratch (e8250_t *uart, unsigned char val)
{
	uart->scratch = val;
}

static
void e8250_write_div_lo (e8250_t *uart, unsigned char val)
{
	uart->divisor &= 0xff00U;
	uart->divisor |= (val & 0xff);

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

static
void e8250_write_div_hi (e8250_t *uart, unsigned char val)
{
	uart->divisor &= 0x00ffU;
	uart->divisor |= (val & 0xff) << 8;

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

static
void e8250_write_txd (e8250_t *uart, unsigned char val)
{
	uart->txd[0] = val;
	uart->txd[1] = 1;

	uart->lsr &= ~E8250_LSR_TBE;

	e8250_clr_int_cond (uart, E8250_IER_TBE);

	uart->txd_write_cnt += 1;

	/* every uart->out_n bytes, give the reader a break */
	if (uart->txd_write_cnt < uart->out_n) {
		e8250_check_txd (uart);
	}

	/* txd still full -> reset write count */
	if (uart->txd[1]) {
		uart->txd_write_cnt = 0;
	}
}

static
void e8250_write_ier (e8250_t *uart, unsigned char val)
{
	unsigned char i;

	uart->ier = val & 0x0f;

	i = (uart->lsr & E8250_LSR_RRD) ? E8250_IER_RRD : 0;
	i |= (uart->lsr & E8250_LSR_TBE) ? E8250_IER_TBE : 0;

	e8250_set_int_cond (uart, i);
}

static
void e8250_write_lcr (e8250_t *uart, unsigned char val)
{
	uart->lcr = val;

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

static
void e8250_write_mcr (e8250_t *uart, unsigned char val)
{
	unsigned char msr;

	if ((uart->mcr & E8250_MCR_LOOP) && ((val & E8250_MCR_LOOP) == 0)) {
		/* leave loop mode */

		msr = uart->msr;

		uart->msr &= 0x0f;

		uart->msr |= ((msr ^ uart->msr) & ~(uart->msr & E8250_MSR_RI)) >> 4;
	}

	uart->mcr = val & 0x1f;

	if (val & E8250_MCR_LOOP) {
		msr = uart->msr;

		uart->msr &= ~(E8250_MSR_CTS | E8250_MSR_DSR | E8250_MSR_RI | E8250_MSR_DCD);

		if (val & E8250_MCR_RTS) {
			uart->msr |= E8250_MSR_CTS;
		}
		if (val & E8250_MCR_DTR) {
			uart->msr |= E8250_MSR_DSR;
		}
		if (val & E8250_MCR_OUT1) {
			uart->msr |= E8250_MSR_RI;
		}
		if (val & E8250_MCR_OUT2) {
			uart->msr |= E8250_MSR_DCD;
		}

		uart->msr |= ((msr ^ uart->msr) & ~(uart->msr & E8250_MSR_RI)) >> 4;
	}

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

unsigned char e8250_get_uint8 (e8250_t *uart, unsigned long addr)
{
	switch (addr) {
		case 0x00:
			if (uart->lcr & E8250_LCR_DLAB) {
				return (e8250_get_div_lo (uart));
			}
			else {
				return (e8250_read_rxd (uart));
			}
			break;

		case 0x01:
			if (uart->lcr & 0x80) {
				return (e8250_get_div_hi (uart));
			}
			else {
				return (e8250_get_ier (uart));
			}
			break;

		case 0x02:
			return (e8250_read_iir (uart));

		case 0x03:
			return (e8250_get_lcr (uart));

		case 0x04:
			return (e8250_get_mcr (uart));

		case 0x05:
			return (e8250_get_lsr (uart));

		case 0x06:
			return (e8250_read_msr (uart));

		case 0x07:
			if (uart->have_scratch) {
				return (e8250_get_scratch (uart));
			}
			return (0xff);
	}

	return (0xff);
}

unsigned short e8250_get_uint16 (e8250_t *uart, unsigned long addr)
{
	return (e8250_get_uint8 (uart, addr));
}

unsigned long e8250_get_uint32 (e8250_t *uart, unsigned long addr)
{
	return (e8250_get_uint8 (uart, addr));
}

void e8250_set_uint8 (e8250_t *uart, unsigned long addr, unsigned char val)
{
	switch (addr) {
		case 0x00:
			if (uart->lcr & E8250_LCR_DLAB) {
				e8250_write_div_lo (uart, val);
			}
			else {
				e8250_write_txd (uart, val);
			}
			break;

		case 0x01:
			if (uart->lcr & E8250_LCR_DLAB) {
				e8250_write_div_hi (uart, val);
			}
			else {
				e8250_write_ier (uart, val);
			}
			break;

		case 0x02:
			/* IIR */
			break;

		case 0x03:
			e8250_write_lcr (uart, val);
			break;

		case 0x04:
			e8250_write_mcr (uart, val);
			break;

		case 0x05:
			/* LSR */
			break;

		case 0x06:
			/* MSR */
			break;

		case 0x07:
			if (uart->have_scratch) {
				e8250_set_scratch (uart, val);
			}
			break;
	}
}

void e8250_set_uint16 (e8250_t *uart, unsigned long addr, unsigned short val)
{
	e8250_set_uint8 (uart, addr, val & 0xff);
}

void e8250_set_uint32 (e8250_t *uart, unsigned long addr, unsigned long val)
{
	e8250_set_uint8 (uart, addr, val & 0xff);
}

void e8250_clock (e8250_t *uart, unsigned clk)
{
	e8250_check_rxd (uart);
	e8250_check_txd (uart);
}
