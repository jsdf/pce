/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e8530.c                                          *
 * Created:     2007-11-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2020 Hampa Hug <hampa@hampa.ch>                     *
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

#include "e8530.h"


#define DEBUG_SCC 0


#define scc_get_chn(chn) (((chn) == 0) ? 'A' : 'B')


static
void e8530_init_chn (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	c->txd_empty = 1;
	c->rxd_empty = 1;

	c->int_on_next_rx = 0;

	c->char_clk_cnt = 0;
	c->char_clk_div = 16384;

	c->read_char_cnt = 0;
	c->read_char_max = 1;

	c->write_char_cnt = 0;
	c->write_char_max = 1;

	c->rtxc = 0;

	c->tx_i = 0;
	c->tx_j = 0;

	c->rx_i = 0;
	c->rx_j = 0;

	c->set_inp_ext = NULL;
	c->set_inp = NULL;

	c->set_out_ext = NULL;
	c->set_out = NULL;

	c->set_rts_ext = NULL;
	c->set_rts = NULL;

	c->set_comm_ext = NULL;
	c->set_comm = NULL;
}

void e8530_init (e8530_t *scc)
{
	scc->index = 0;

	scc->pclk = 0;

	e8530_init_chn (scc, 0);
	e8530_init_chn (scc, 1);

	scc->irq_ext = NULL;
	scc->irq = NULL;
	scc->irq_val = 0;
}

void e8530_free (e8530_t *scc)
{
}

e8530_t *e8530_new (void)
{
	e8530_t *scc;

	if ((scc = malloc (sizeof (e8530_t))) == NULL) {
		return (NULL);
	}

	e8530_init (scc);

	return (scc);
}

void e8530_del (e8530_t *scc)
{
	if (scc != NULL) {
		e8530_free (scc);
		free (scc);
	}
}

void e8530_set_irq_fct (e8530_t *scc, void *ext, void *fct)
{
	scc->irq_ext = ext;
	scc->irq = fct;
}

void e8530_set_inp_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_inp_ext = ext;
		scc->chn[chn].set_inp = fct;
	}
}

void e8530_set_out_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_out_ext = ext;
		scc->chn[chn].set_out = fct;
	}
}

void e8530_set_rts_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_rts_ext = ext;
		scc->chn[chn].set_rts = fct;
	}
}

void e8530_set_comm_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_comm_ext = ext;
		scc->chn[chn].set_comm = fct;
	}
}

void e8530_set_multichar (e8530_t *scc, unsigned chn, unsigned read_max, unsigned write_max)
{
	if (chn > 1) {
		return;
	}

	scc->chn[chn].read_char_cnt = 0;
	scc->chn[chn].read_char_max = (read_max < 1) ? 1 : read_max;

	scc->chn[chn].write_char_cnt = 0;
	scc->chn[chn].write_char_max = (write_max < 1) ? 1 : write_max;
}

void e8530_set_clock (e8530_t *scc, unsigned long pclk, unsigned long rtxca, unsigned long rtxcb)
{
	scc->pclk = pclk;
	scc->chn[0].rtxc = rtxca;
	scc->chn[1].rtxc = rtxcb;
}

static
void e8530_set_irq (e8530_t *scc, unsigned char val)
{
	if (scc->irq_val == val) {
		return;
	}

	scc->irq_val = val;

	if (scc->irq != NULL) {
		scc->irq (scc->irq_ext, val);
	}
}

static
void e8530_set_rts (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (c->set_rts != NULL) {
		c->set_rts (c->set_rts_ext, val != 0);
	}
}

static
void e8530_set_int_cond (e8530_t *scc, unsigned chn, unsigned char cond)
{
	e8530_chn_t *c0, *c;

	c = &scc->chn[chn];
	c0 = &scc->chn[0];

	if ((cond & 0x01) && (c->wr[1] & 0x01)) {
		/* ext */

		/* should check if IP is already set */

		c0->rr[3] |= (chn == 0) ? 0x08 : 0x01;

		c->rr0_latch_msk = c->wr[15];
		c->rr0_latch_val = c->rr[0];
	}

	if ((cond & 0x02) && (c->wr[1] & 0x02)) {
		/* transmit interrupt */
		c0->rr[3] |= (chn == 0) ? 0x10 : 0x02;
	}

	if ((cond & 0x04) && (c->wr[1] & 0x18)) {
		/* receive interrupt */
		int data, spec, irq;

		data = (c->rxd_empty == 0);
		spec = (c->rr[0] & 0x80) != 0;
		irq = 0;

		switch ((c->wr[1] >> 3) & 3) {
		case 0: /* disabled */
			irq = 0;
			break;

		case 1: /* rx int on first char / special condition */
			irq = (data && c->int_on_next_rx) || spec;
			break;

		case 2: /* rx int on all chars / special condition */
			irq = data || spec;
			break;

		case 3: /* rx int on special condition */
			irq = spec;
			break;
		}

		if (data) {
			c->int_on_next_rx = 0;
		}

		if (irq) {
			c0->rr[3] |= (chn == 0) ? 0x20 : 0x04;
		}
	}

	if ((c0->wr[9] & 0x08) == 0) {
		/* MIE == 0 */
		e8530_set_irq (scc, 0);
		return;
	}

	e8530_set_irq (scc, c0->rr[3] != 0);
}

static
void e8530_clr_int_cond (e8530_t *scc, unsigned chn, unsigned char cond)
{
	e8530_chn_t *c, *c0;

	c = &scc->chn[chn];
	c0 = &scc->chn[0];

	if (cond & 0x01) {
		/* ext */

		c0->rr[3] &= (chn == 0) ? ~0x08 : ~0x01;

		c->rr0_latch_msk = 0;
	}

	if (cond & 0x02) {
		/* transmit interrupt */
		c0->rr[3] &= (chn == 0) ? ~0x10 : ~0x02;
	}

	if (cond & 0x04) {
		/* receive interrupt */
		c0->rr[3] &= (chn == 0) ? ~0x20 : ~0x04;
	}

	if ((c0->wr[9] & 0x08) == 0) {
		/* MIE == 0 */
		e8530_set_irq (scc, 0);
		return;
	}

	e8530_set_irq (scc, c0->rr[3] != 0);
}

/*
 * Move a character from the input queue to RxD and adjust interrupt
 * conditions.
 */
static
void e8530_check_rxd (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (c->rx_i == c->rx_j) {
		return;
	}

	if (c->read_char_cnt == 0) {
		return;
	}

	if ((c->wr[3] & 0x01) == 0) {
		/* receiver disabled */
		return;
	}

	if (c->rxd_empty == 0) {
		/* should overwrite old character */
		return;
	}

	c->read_char_cnt -= 1;

	c->rr[8] = c->rxbuf[c->rx_j];
	c->rx_j = (c->rx_j + 1) % E8530_BUF_MAX;

	if (c->set_inp != NULL) {
		c->set_inp (c->set_inp_ext, 1);
	}

	c->rr[0] |= 0x01;
	c->rxd_empty = 0;

	e8530_set_int_cond (scc, chn, 0x04);
}

/*
 * Move a character from TxD to the output queue and adjust interrupt
 * conditions.
 */
static
void e8530_check_txd (e8530_t *scc, unsigned chn)
{
	e8530_chn_t   *c;
	unsigned char val;

	c = &scc->chn[chn];

	if (c->write_char_cnt == 0) {
		return;
	}

	if (((c->tx_i + 1) % E8530_BUF_MAX) == c->tx_j) {
		return;
	}

	if (c->txd_empty) {
		/* tx underrun */
		c->rr[0] |= 0x40;
		return;
	}

	c->write_char_cnt -= 1;

	val = c->wr[8];

	c->txbuf[c->tx_i] = val;
	c->tx_i = (c->tx_i + 1) % E8530_BUF_MAX;

	if (c->set_out != NULL) {
		c->set_out (c->set_out_ext, val);
	}

	c->rr[0] |= 0x04;
	c->txd_empty = 1;

	e8530_set_int_cond (scc, chn, 0x02);

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: send %02X\n", scc_get_chn (chn), val);
#endif
}


static
unsigned e8530_get_clock_mode (e8530_t *scc, unsigned chn)
{
	switch ((scc->chn[chn].wr[4] >> 6) & 3) {
	case 0:
		return (1);

	case 1:
		return (16);

	case 2:
		return (32);

	case 3:
		return (64);
	}

	return (0);
}

static
unsigned e8530_get_bits_per_char (e8530_t *scc, unsigned chn)
{
	switch ((scc->chn[chn].wr[5] >> 5) & 3) {
	case 0:
		return (5);

	case 1:
		return (7);

	case 2:
		return (6);

	case 3:
		return (8);
	}

	return (0);
}

static
unsigned e8530_get_parity (e8530_t *scc, unsigned chn)
{
	switch (scc->chn[chn].wr[4] & 3) {
	case 0:
		return (0);

	case 1: /* odd */
		return (1);

	case 2:
		return (0);

	case 3: /* even */
		return (2);
	}

	return (0);
}

/*
 * Get 2 * the number of stop bits
 */
static
unsigned e8530_get_stop_bits (e8530_t *scc, unsigned chn)
{
	switch ((scc->chn[chn].wr[4] >> 2) & 3) {
	case 0:
		return (0);

	case 1:
		return (2);

	case 2:
		return (3);

	case 3:
		return (4);

	}

	return (0);
}

/*
 * Update the communication parameters and call the callback function
 */
static
void e8530_set_params (e8530_t *scc, unsigned chn)
{
	unsigned long bps, clk, mul, div;
	e8530_chn_t   *c;

	c = &scc->chn[chn];

	c->bpc = e8530_get_bits_per_char (scc, chn);
	c->parity = e8530_get_parity (scc, chn);
	c->stop = e8530_get_stop_bits (scc, chn);

	mul = e8530_get_clock_mode (scc, chn);

	if (c->stop == 0) {
		/* sync mode */
		mul = 1;
	}

	if (c->wr[14] & 0x01) {
		/* baud rate generator enabled */

		if (c->wr[14] & 0x02) {
			clk = scc->pclk;
		}
		else {
			clk = c->rtxc;
		}

		div = (c->wr[13] << 8) | c->wr[12];
		div = 2 * mul * (div + 2);

		c->char_clk_div = ((2 * c->bpc + c->stop + 2) * div) / 2;

		if (c->parity != 0) {
			c->char_clk_div += div;
		}

		bps = clk / div;
	}
	else {
		c->char_clk_div = 16384;
		bps = 0;
	}

	c->bps = bps;

	if (c->set_comm != NULL) {
		c->set_comm (c->set_comm_ext, c->bps, c->parity, c->bpc, c->stop);
	}

#if DEBUG_SCC
	{
		static const char p[3] = { 'N', 'O', 'E' };
		static const char *s[5] = { "0", "", "1", "1.5", "2" };

		fprintf (stderr, "SCC %c: %lu%c%u%s\n",
			scc_get_chn (chn), c->bps, p[c->parity], c->bpc, s[c->stop]
		);
	}
#endif
}


/*
 * Set new RR0 value reflecting the external status and set an
 * interrupt condition if an enabled status changed.
 */
static
void e8530_set_rr0 (e8530_t *scc, unsigned chn, unsigned char val)
{
	unsigned char old;

	chn &= 1;

	old = scc->chn[chn].rr[0];
	scc->chn[chn].rr[0] = val;

	if ((old ^ val) & scc->chn[chn].wr[15] & 0xfa) {
		e8530_set_int_cond (scc, chn, 0x01);
	}
}


/*
 * RR0: transmit/receive buffer status and external status
 */
static
unsigned char e8530_get_rr0 (e8530_t *scc, unsigned chn)
{
	unsigned char val;

	val = scc->chn[chn].rr[0] & ~scc->chn[chn].rr0_latch_msk;
	val |= scc->chn[chn].rr0_latch_val & scc->chn[chn].rr0_latch_msk;

	return (val);
}

/*
 * RR2: interrupt vector
 * If read from channel B, include status
 */
static
unsigned char e8530_get_rr2 (e8530_t *scc, unsigned chn)
{
	unsigned char val;
	e8530_chn_t   *c0;

	c0 = &scc->chn[0];

	val = c0->rr[2];

	if (chn == 1) {
		unsigned char st;

		/* include status in vector */

		if (c0->rr[3] & 0x20) {
			/* chn a rx */
			st = 0x06 | (0x03 << 3);
		}
		else if (c0->rr[3] & 0x10) {
			/* chn a tx */
			st = 0x04 | (0x01 << 3);
		}
		else if (c0->rr[3] & 0x08) {
			/* chn a ext */
			st = 0x05 | (0x05 << 3);
		}
		else if (c0->rr[3] & 0x04) {
			/* chn b rx */
			st = 0x02 | (0x02 << 3);
		}
		else if (c0->rr[3] & 0x02) {
			/* chn b tx */
			st = 0x00 | (0x00 << 3);
		}
		else if (c0->rr[3] & 0x01) {
			/* chn b ext */
			st = 0x01 | (0x04 << 3);
		}
		else {
			st = 0x03 | (0x06 << 3);
		}

		if (c0->wr[9] & 0x10) {
			/* status high */
			val = (val & 0x70) | ((st & 0x38) << 1);
		}
		else {
			val = (val & 0xf1) | ((st & 0x07) << 1);
		}
	}

	return (val);
}

/*
 * RR3: interrupt pending
 */
static
unsigned char e8530_get_rr3 (e8530_t *scc, unsigned chn)
{
	if (chn != 0) {
		return (0);
	}

	return (scc->chn[0].rr[3]);
}

/*
 * RR8: receive buffer
 */
static
unsigned char e8530_get_rr8 (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;
	unsigned char val;

	c = &scc->chn[chn];

	val = c->rr[8];

	c->rr[0] &= ~0x01;
	c->rxd_empty = 1;

	e8530_clr_int_cond (scc, chn, 0x04);

	e8530_check_rxd (scc, chn);

	return (val);
}

static
unsigned char e8530_get_reg (e8530_t *scc, unsigned chn, unsigned reg)
{
	unsigned char val;

	chn &= 1;

	switch (reg) {
	case 0x00:
		val = e8530_get_rr0 (scc, chn);
		break;

	case 0x02:
		val = e8530_get_rr2 (scc, chn);
		break;

	case 0x03:
		val = e8530_get_rr3 (scc, chn);
		break;

	case 0x08:
		val = e8530_get_rr8 (scc, chn);
		break;

	default:
		val = scc->chn[chn].rr[reg & 15];
		break;
	}


	scc->index = 0;

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: get RR%u -> %02x\n",
		scc_get_chn (chn), reg, val
	);
#endif

	return (val);
}

/*
 * WR0: command register
 */
static
void e8530_set_wr0 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[0] = val;

	scc->index = val & 7;

	switch ((val >> 3) & 7) {
	case 0x00: /* null command */
		break;

	case 0x01: /* point high */
		scc->index += 8;
		break;

	case 0x02: /* reset external/status interrupts */
		e8530_clr_int_cond (scc, chn, 0x01);
		break;

	case 0x03: /* send abort */
		break;

	case 0x04: /* enable interrupt on next rx character */
		scc->chn[chn].int_on_next_rx = 1;
		break;

	case 0x05: /* reset tx interrupt pending */
		e8530_clr_int_cond (scc, chn, 0x02);
		break;

	case 0x06: /* error reset */
		scc->chn[chn].rr[1] &= 0x7f;
		break;

	case 0x07: /* reset highest ius */
		break;
	}
}

/*
 * WR1: transmit/receive interrupt and data transfer mode definition
 */
static
void e8530_set_wr1 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[1] = val;

	e8530_set_int_cond (scc, chn, 0x00);
}

/*
 * WR2: interrupt vector
 */
static
void e8530_set_wr2 (e8530_t *scc, unsigned char val)
{
	scc->chn[0].wr[2] = val;
	scc->chn[1].wr[2] = val;

	scc->chn[0].rr[2] = val;
	scc->chn[1].rr[2] = val;
}

/*
 * WR3: receive parameters and control
 */
static
void e8530_set_wr3 (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (c->wr[3] ^ val) {
		if (val & 0x01) {
#if DEBUG_SCC >= 1
			fprintf (stderr, "SCC %c: receiver enable\n", scc_get_chn (chn));
#endif
			c->int_on_next_rx = 1;
			c->rxd_empty = 1;
		}
		else {
			c->rr[0] &= ~0x01;
			e8530_clr_int_cond (scc, chn, 0x04);
#if DEBUG_SCC >= 1
			fprintf (stderr, "SCC %c: receiver disable\n", scc_get_chn (chn));
#endif
		}
	}

	if (val & 0x10) {
#if DEBUG_SCC >= 1
		if ((c->rr[0] & 0x10) == 0) {
			fprintf (stderr, "SCC %c: sync/hunt mode\n", scc_get_chn (chn));
		}
#endif
		c->rr[0] |= 0x10;
	}

	c->wr[3] = val;
}

/*
 * WR4: transmit/receive miscellaneous parameters and modes
 */
static
void e8530_set_wr4 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[4] = val;

	e8530_set_params (scc, chn);
}

/*
 * WR5: transmit parameters and controls
 */
static
void e8530_set_wr5 (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t   *c;
	unsigned char old;

	c = &scc->chn[chn];

	old = c->wr[5];
	c->wr[5] = val;

	if ((old ^ val) & 0x02) {
		e8530_set_rts (scc, chn, (val & 0x02) != 0);
	}

	e8530_set_params (scc, chn);
}

/*
 * WR8: transmit buffer
 */
static
void e8530_set_wr8 (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	c->wr[8] = val;

	c->rr[0] &= ~0x04;
	c->txd_empty = 0;

	e8530_clr_int_cond (scc, chn, 0x02);

	e8530_check_txd (scc, chn);

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: send %02X\n", scc_get_chn (chn), val);
#endif
}

/*
 * WR9: Master interrupt control
 */
static
void e8530_set_wr9 (e8530_t *scc, unsigned char val)
{
	unsigned char old;

	old = scc->chn[0].wr[9];

	scc->chn[0].wr[9] = val;
	scc->chn[1].wr[9] = val;

	if ((old ^ val) & val & 0x08) {
		/* MIE was enabled, re-check pending interrupts */
		e8530_set_int_cond (scc, 0, 0);
		e8530_set_int_cond (scc, 1, 0);
	}
}

/*
 * WR10: miscellaneous transmitter/receiver control bits
 */
static
void e8530_set_wr10 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[10] = val;
}

/*
 * WR11: clock mode control
 */
static
void e8530_set_wr11 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[11] = val;
}

/*
 * WR12: baud rate generator low byte
 */
static
void e8530_set_wr12 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[12] = val;

	e8530_set_params (scc, chn);
}

/*
 * WR13: baud rate generator high byte
 */
static
void e8530_set_wr13 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[13] = val;

	e8530_set_params (scc, chn);
}

/*
 * WR14: miscellaneous control bits
 */
static
void e8530_set_wr14 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[14] = val;

#if DEBUG_SCC
	if ((val & 0xe0) != 0) {
		fprintf (stderr, "SCC %c: dpll cmd: %u\n",
			scc_get_chn (chn), (val >> 5) & 7
		);
	}
#endif

	e8530_set_params (scc, chn);
}

/*
 * WR15: external/status interrupt control
 */
static
void e8530_set_wr15 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[15] = val;
	scc->chn[chn].rr[15] = val;
}

void e8530_set_reg (e8530_t *scc, unsigned chn, unsigned reg, unsigned char val)
{
	chn &= 1;

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: set WR%u <- %02x\n",
		scc_get_chn (chn), reg, val
	);
#endif

	switch (reg) {
	case 0x00:
		e8530_set_wr0 (scc, chn, val);
		break;

	case 0x01:
		e8530_set_wr1 (scc, chn, val);
		break;

	case 0x02:
		e8530_set_wr2 (scc, val);
		break;

	case 0x03:
		e8530_set_wr3 (scc, chn, val);
		break;

	case 0x04:
		e8530_set_wr4 (scc, chn, val);
		break;

	case 0x05:
		e8530_set_wr5 (scc, chn, val);
		break;

	case 0x08:
		e8530_set_wr8 (scc, chn, val);
		break;

	case 0x09:
		e8530_set_wr9 (scc, val);
		break;

	case 0x0a:
		e8530_set_wr10 (scc, chn, val);
		break;

	case 0x0b:
		e8530_set_wr11 (scc, chn, val);
		break;

	case 0x0c:
		e8530_set_wr12 (scc, chn, val);
		break;

	case 0x0d:
		e8530_set_wr13 (scc, chn, val);
		break;

	case 0x0e:
		e8530_set_wr14 (scc, chn, val);
		break;

	case 0x0f:
		e8530_set_wr15 (scc, chn, val);
		break;

	default:
		scc->chn[chn].wr[reg & 15] = val;
		break;
	}

	if (reg != 0) {
		scc->index = 0;
	}
}


unsigned char e8530_get_ctl (e8530_t *scc, unsigned chn)
{
	return (e8530_get_reg (scc, chn & 1, scc->index));
}

unsigned char e8530_get_ctl_a (e8530_t *scc)
{
	return (e8530_get_reg (scc, 0, scc->index));
}

unsigned char e8530_get_ctl_b (e8530_t *scc)
{
	return (e8530_get_reg (scc, 1, scc->index));
}

void e8530_set_ctl (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_set_reg (scc, chn & 1, scc->index, val);
}

void e8530_set_ctl_a (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 0, scc->index, val);
}

void e8530_set_ctl_b (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 1, scc->index, val);
}

unsigned char e8530_get_data (e8530_t *scc, unsigned chn)
{
	return (e8530_get_reg (scc, chn & 1, 8));
}

unsigned char e8530_get_data_a (e8530_t *scc)
{
	return (e8530_get_reg (scc, 0, 8));
}

unsigned char e8530_get_data_b (e8530_t *scc)
{
	return (e8530_get_reg (scc, 1, 8));
}

void e8530_set_data (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_set_reg (scc, chn & 1, 8, val);
}

void e8530_set_data_a (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 0, 8, val);
}

void e8530_set_data_b (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 1, 8, val);
}


void e8530_set_dcd (e8530_t *scc, unsigned chn, unsigned char val)
{
	unsigned char old;

	chn &= 1;
	old = scc->chn[chn].rr[0];

	if (val) {
		old &= ~0x08;
	}
	else {
		old |= 0x08;
	}

	e8530_set_rr0 (scc, chn, old);
}

void e8530_set_dcd_a (e8530_t *scc, unsigned char val)
{
	e8530_set_dcd (scc, 0, val);
}

void e8530_set_dcd_b (e8530_t *scc, unsigned char val)
{
	e8530_set_dcd (scc, 1, val);
}

void e8530_set_cts (e8530_t *scc, unsigned chn, unsigned char val)
{
	unsigned char rr0;

	chn &= 1;
	rr0 = scc->chn[chn].rr[0];

	if (val) {
		rr0 &= ~0x20;
	}
	else {
		rr0 |= 0x20;
	}

	e8530_set_rr0 (scc, chn, rr0);
}

void e8530_set_cts_a (e8530_t *scc, unsigned char val)
{
	e8530_set_cts (scc, 0, val);
}

void e8530_set_cts_b (e8530_t *scc, unsigned char val)
{
	e8530_set_cts (scc, 1, val);
}

void e8530_receive (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

	if (((c->rx_i + 1) % E8530_BUF_MAX) != c->rx_j) {
		c->rxbuf[c->rx_i] = val;
		c->rx_i = (c->rx_i + 1) % E8530_BUF_MAX;
	}
}

void e8530_receive_a (e8530_t *scc, unsigned char val)
{
	e8530_receive (scc, 0, val);
}

void e8530_receive_b (e8530_t *scc, unsigned char val)
{
	e8530_receive (scc, 1, val);
}

unsigned char e8530_send (e8530_t *scc, unsigned chn)
{
	unsigned char val;
	e8530_chn_t   *c;

	chn &= 1;
	c = &scc->chn[chn];

	if (c->tx_i == c->tx_j) {
		return (0);
	}

	val = c->txbuf[c->tx_j];
	c->tx_j = (c->tx_j + 1) % E8530_BUF_MAX;

	return (val);
}

unsigned char e8530_send_a (e8530_t *scc)
{
	return (e8530_send (scc, 0));
}

unsigned char e8530_send_b (e8530_t *scc)
{
	return (e8530_send (scc, 1));
}

int e8530_inp_full (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

	if (((c->rx_i + 1) % E8530_BUF_MAX) == c->rx_j) {
		return (1);
	}

	return (0);
}

int e8530_out_empty (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

	if (c->tx_i == c->tx_j) {
		return (1);
	}

	return (0);
}

static
void e8530_reset_channel (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

	c->rr[0] |= 0x04; /* tx empty */
//	c->rr[0] |= 0x20; /* cts */
	c->rr[1] |= 0x01; /* all sent */

	c->rr0_latch_msk = 0;

	c->bps = 0;
	c->parity = 0;
	c->bpc = 0;
	c->stop = 0;

	c->tx_i = 0;
	c->tx_j = 0;

	c->rx_i = 0;
	c->rx_j = 0;
}

void e8530_reset (e8530_t *scc)
{
	unsigned i;

	scc->index = 0;

	for (i = 0; i < 16; i++) {
		scc->chn[0].rr[i] = 0;
		scc->chn[0].wr[i] = 0;
		scc->chn[1].rr[i] = 0;
		scc->chn[1].wr[i] = 0;
	}

	e8530_reset_channel (scc, 0);
	e8530_reset_channel (scc, 1);

	e8530_set_rts (scc, 0, 0);
	e8530_set_rts (scc, 1, 0);

	e8530_set_irq (scc, 0);
}

static inline
void e8530_chn_clock (e8530_t *scc, unsigned chn, unsigned n)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (n < c->char_clk_cnt) {
		c->char_clk_cnt -= n;
		return;
	}
	else {
		n -= c->char_clk_cnt;
	}

	if (n > c->char_clk_div) {
		n = n % c->char_clk_div;
	}

	c->char_clk_cnt = c->char_clk_div - n;

	c->read_char_cnt = c->read_char_max;
	c->write_char_cnt = c->write_char_max;

	e8530_check_rxd (scc, chn);
	e8530_check_txd (scc, chn);
}

void e8530_clock (e8530_t *scc, unsigned n)
{
	e8530_chn_clock (scc, 0, n);
	e8530_chn_clock (scc, 1, n);
}
