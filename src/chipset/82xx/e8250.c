/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8250.c                                     *
 * Created:     2003-08-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include "e8250.h"


static int e8250_get_inp (e8250_t *uart, unsigned char *val);
static int e8250_set_out (e8250_t *uart, unsigned char val);


void e8250_init (e8250_t *uart)
{
	uart->chip = E8250_CHIP_16450;

	uart->inp_i = 0;
	uart->inp_j = 0;
	uart->inp_n = 9;

	uart->out_i = 0;
	uart->out_j = 0;
	uart->out_n = 9;

	uart->txd = 0;
	uart->rxd = 0;

	uart->ier = 0;
	uart->iir = E8250_IIR_PND;

	uart->lcr = 0;
	uart->lsr = E8250_LSR_TXE | E8250_LSR_TBE;

	uart->mcr = 0;
	uart->msr = 0; /* E8250_MSR_DCD | E8250_MSR_DSR | E8250_MSR_CTS; */

	uart->tbe_ack = 1;

	uart->bit_clk_div = 10;

	uart->clocking = 0;

	uart->read_clk_cnt = 0;
	uart->read_clk_div = 128;
	uart->read_char_cnt = 1;
	uart->read_char_max = 1;

	uart->write_clk_cnt = 0;
	uart->write_clk_div = 128;
	uart->write_char_cnt = 1;
	uart->write_char_max = 1;

	uart->have_scratch = 1;

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

	uart->check_ext = NULL;
	uart->check = NULL;
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

void e8250_set_irq_fct (e8250_t *uart, void *ext, void *fct)
{
	uart->irq = fct;
	uart->irq_ext = ext;
}

void e8250_set_send_fct (e8250_t *uart, void *ext, void *fct)
{
	uart->send = fct;
	uart->send_ext = ext;
}

void e8250_set_recv_fct (e8250_t *uart, void *ext, void *fct)
{
	uart->recv = fct;
	uart->recv_ext = ext;
}

void e8250_set_setup_fct (e8250_t *uart, void *ext, void *fct)
{
	uart->setup = fct;
	uart->setup_ext = ext;
}

void e8250_set_check_fct (e8250_t *uart, void *ext, void *fct)
{
	uart->check = fct;
	uart->check_ext = ext;
}

void e8250_set_buf_size (e8250_t *uart, unsigned inp, unsigned out)
{
	inp += 1;
	out += 1;

	uart->inp_i = 0;
	uart->inp_j = 0;
	uart->inp_n = (inp <= E8250_BUF_MAX) ? inp : E8250_BUF_MAX;

	uart->out_i = 0;
	uart->out_j = 0;
	uart->out_n = (out <= E8250_BUF_MAX) ? out : E8250_BUF_MAX;
}

void e8250_set_multichar (e8250_t *uart, unsigned read_max, unsigned write_max)
{
	uart->read_char_max = read_max;
	uart->write_char_max = write_max;
}

void e8250_set_bit_clk_div (e8250_t *uart, unsigned div)
{
	uart->bit_clk_div = div;
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
void e8250_set_int_cond (e8250_t *uart)
{
	uart->iir = E8250_IIR_PND;

	if (uart->iir < E8250_IIR_RRD) {
		if (uart->ier & E8250_IER_RRD) {
			if (uart->lsr & E8250_LSR_RRD) {
				uart->iir = E8250_IIR_RRD;
			}
		}
	}

	if (uart->iir < E8250_IIR_TBE) {
		if (uart->ier & E8250_IER_TBE) {
			if ((uart->lsr & E8250_LSR_TBE) && (uart->tbe_ack == 0)) {
				uart->iir = E8250_IIR_TBE;
			}
		}
	}

	e8250_set_irq (uart, (uart->iir & E8250_IIR_PND) == 0);
}

/*
 * Move a character from the input queue to RxD and adjust interrupt
 * conditions.
 */
static
void e8250_check_rxd (e8250_t *uart)
{
	if (uart->read_char_cnt == 0) {
		return;
	}

	if (uart->lsr & E8250_LSR_RRD) {
		return;
	}

	if (e8250_get_inp (uart, &uart->rxd)) {
		return;
	}

	uart->read_char_cnt -= 1;

	if (uart->read_clk_cnt == 0) {
		uart->clocking = 1;
		uart->read_clk_cnt = uart->read_clk_div;
	}

	uart->lsr |= E8250_LSR_RRD;

	/* receive queue is not full */
	if (uart->recv != NULL) {
		uart->recv (uart->recv_ext, 1);
	}

	e8250_set_int_cond (uart);
}

/*
 * Move a character from TxD to the output queue and adjust interrupt
 * conditions.
 */
static
void e8250_check_txd (e8250_t *uart)
{
	if (uart->write_char_cnt == 0) {
		return;
	}

	if (uart->lsr & E8250_LSR_TBE) {
		uart->lsr |= E8250_LSR_TXE;
		return;
	}

	if (uart->mcr & E8250_MCR_LOOP) {
		e8250_set_inp (uart, uart->txd);
	}
	else {
		if (e8250_set_out (uart, uart->txd)) {
			return;
		}
	}

	uart->write_char_cnt -= 1;

	if (uart->write_clk_cnt == 0) {
		uart->clocking = 1;
		uart->write_clk_cnt = uart->write_clk_div;
	}

	uart->lsr &= ~E8250_LSR_TXE;
	uart->lsr |= E8250_LSR_TBE;

	uart->tbe_ack = 0;

	if (uart->mcr & E8250_MCR_LOOP) {
		e8250_check_rxd (uart);
	}
	else {
		if (uart->send != NULL) {
			uart->send (uart->send_ext, 1);
		}
	}

	e8250_set_int_cond (uart);
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


/*
 * Set the DSR input signal
 */
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

/*
 * Set the CTS input signal
 */
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

/*
 * Set the DCD input signal
 */
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

/*
 * Set the RI input signal
 */
void e8250_set_ri (e8250_t *uart, unsigned char val)
{
	unsigned char msr;

	msr = uart->msr;

	if (val) {
		uart->msr |= E8250_MSR_RI;
	}
	else {
		uart->msr &= ~E8250_MSR_RI;
	}

	if ((msr ^ uart->msr) & E8250_MSR_RI) {
		uart->msr |= E8250_MSR_DRI;
	}
	else {
		uart->msr &= ~E8250_MSR_DRI;
	}
}


/*
 * Add a byte to the input queue
 */
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

/*
 * Remove a byte from the input queue
 */
static
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

/*
 * Clear the input queue
 */
void e8250_get_inp_all (e8250_t *uart)
{
	uart->inp_i = 0;
	uart->inp_j = 0;
}

/*
 * Check if the input queue is full
 */
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

/*
 * Check if the input queue is empty
 */
int e8250_inp_empty (e8250_t *uart)
{
	return (uart->inp_i == uart->inp_j);
}


/*
 * Add a byte to the output queue
 */
static
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

/*
 * Get a byte from the output queue and optionally remove it
 */
int e8250_get_out (e8250_t *uart, unsigned char *val, int remove)
{
	if (uart->out_i == uart->out_j) {
		return (1);
	}

	*val = uart->out[uart->out_i];

	if (remove) {
		uart->out_i += 1;
		if (uart->out_i >= uart->out_n) {
			uart->out_i = 0;
		}
	}

	return (0);
}

/*
 * Clear the output queue
 */
void e8250_get_out_all (e8250_t *uart)
{
	uart->out_i = 0;
	uart->out_j = 0;
}

/*
 * Check if the output queue is empty
 */
int e8250_out_empty (e8250_t *uart)
{
	return (uart->out_i == uart->out_j);
}


int e8250_send (e8250_t *uart, unsigned char *val)
{
	return (e8250_get_out (uart, val, 1));
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
	return (uart->rxd);
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

	val = uart->rxd;

	uart->lsr &= ~E8250_LSR_RRD;

	e8250_set_int_cond (uart);

	e8250_check_rxd (uart);

	return (val);
}

static
unsigned char e8250_read_iir (e8250_t *uart)
{
	unsigned char val;

	val = uart->iir;

	if (val == E8250_IIR_TBE) {
		uart->tbe_ack = 1;
		e8250_set_int_cond (uart);
	}

	return (val);
}

static
unsigned char e8250_read_msr (e8250_t *uart)
{
	unsigned char val;

	if (uart->check != NULL) {
		uart->check (uart->check_ext, 1);
	}

	val = uart->msr;

	uart->msr &= 0xf0;

	return (val);
}


void e8250_set_scratch (e8250_t *uart, unsigned char val)
{
	uart->scratch = val;
}

static
void e8250_set_char_clk (e8250_t *uart)
{
	unsigned long val;

	val = 1;
	val += e8250_get_databits (uart);
	val += e8250_get_stopbits (uart);

	val *= uart->bit_clk_div;

	if (uart->divisor > 1) {
		val *= uart->divisor;
	}

	uart->read_clk_div = val;
	uart->write_clk_div = val;
}

static
void e8250_write_div_lo (e8250_t *uart, unsigned char val)
{
	uart->divisor = (uart->divisor & 0xff00) | (val & 0x00ff);

	e8250_set_char_clk (uart);

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

static
void e8250_write_div_hi (e8250_t *uart, unsigned char val)
{
	uart->divisor = (uart->divisor & 0x00ff) | ((val & 0xff) << 8);

	e8250_set_char_clk (uart);

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

static
void e8250_write_txd (e8250_t *uart, unsigned char val)
{
	if ((uart->lsr & E8250_LSR_TBE) == 0) {
		fprintf (stderr,
			"e8250: overwrite txd  txd=%02X lsr=%02X iir=%02X val=%02X\n",
			uart->txd, uart->lsr, uart->iir, val
		);
	}

	uart->txd = val;

	uart->lsr &= ~E8250_LSR_TBE;

	e8250_set_int_cond (uart);

	e8250_check_txd (uart);
}

static
void e8250_write_ier (e8250_t *uart, unsigned char val)
{
	uart->ier = val & 0x0f;

	uart->tbe_ack = 0;

	e8250_set_int_cond (uart);
}

static
void e8250_write_lcr (e8250_t *uart, unsigned char val)
{
	if (uart->lcr == val) {
		return;
	}

	uart->lcr = val;

	e8250_set_char_clk (uart);

	if (uart->setup != NULL) {
		uart->setup (uart->setup_ext, 1);
	}
}

static
void e8250_write_mcr (e8250_t *uart, unsigned char val)
{
	unsigned char msr;

	if (uart->mcr == (val & 0x1f)) {
		return;
	}

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
	if (uart->clocking == 0) {
		return;
	}

	if (uart->read_clk_cnt > 0) {
		if (clk < uart->read_clk_cnt) {
			uart->read_clk_cnt -= clk;
		}
		else {
			uart->read_clk_cnt = 0;
			uart->read_char_cnt = uart->read_char_max;
			e8250_check_rxd (uart);
		}
	}

	if (uart->write_clk_cnt > 0) {
		if (clk < uart->write_clk_cnt) {
			uart->write_clk_cnt -= clk;
		}
		else {
			uart->write_clk_cnt = 0;
			uart->write_char_cnt = uart->write_char_max;
			e8250_check_txd (uart);
		}
	}

	uart->clocking = ((uart->read_clk_cnt > 0) || (uart->write_clk_cnt > 0));
}
