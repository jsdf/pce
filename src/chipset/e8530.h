/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e8530.h                                          *
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


/* Zilog Z8530 SCC */


#ifndef PCE_CHIPSET_E8530_H
#define PCE_CHIPSET_E8530_H 1


#define E8530_BUF_MAX 256


typedef struct {
	unsigned char wr[16];
	unsigned char rr[16];

	unsigned char rr0_latch_msk;
	unsigned char rr0_latch_val;

	unsigned char txd_empty;
	unsigned char rxd_empty;

	unsigned char int_on_next_rx;

	unsigned long bps;
	unsigned      parity;
	unsigned      bpc;
	unsigned      stop;

	unsigned      char_clk_cnt;
	unsigned      char_clk_div;

	unsigned      read_char_cnt;
	unsigned      read_char_max;

	unsigned      write_char_cnt;
	unsigned      write_char_max;

	unsigned long rtxc;

	unsigned      tx_i;
	unsigned      tx_j;
	unsigned char txbuf[E8530_BUF_MAX];

	unsigned      rx_i;
	unsigned      rx_j;
	unsigned char rxbuf[E8530_BUF_MAX];

	void          *set_inp_ext;
	void          (*set_inp) (void *ext, unsigned char val);

	void          *set_out_ext;
	void          (*set_out) (void *ext, unsigned char val);

	void          *set_rts_ext;
	void          (*set_rts) (void *ext, unsigned char val);

	void          *set_comm_ext;
	void          (*set_comm) (void *ext, unsigned long bps, unsigned parity, unsigned bpc, unsigned stop);
} e8530_chn_t;


typedef struct {
	unsigned      index;

	e8530_chn_t   chn[2];

	unsigned long pclk;

	void          *irq_ext;
	void          (*irq) (void *ext, unsigned char val);
	unsigned char irq_val;
} e8530_t;


void e8530_init (e8530_t *scc);
void e8530_free (e8530_t *scc);

e8530_t *e8530_new (void);
void e8530_del (e8530_t *scc);

void e8530_set_irq_fct (e8530_t *scc, void *ext, void *fct);

void e8530_set_inp_fct (e8530_t *scc, unsigned chn, void *ext, void *fct);
void e8530_set_out_fct (e8530_t *scc, unsigned chn, void *ext, void *fct);
void e8530_set_rts_fct (e8530_t *scc, unsigned chn, void *ext, void *fct);
void e8530_set_comm_fct (e8530_t *scc, unsigned chn, void *ext, void *fct);

void e8530_set_multichar (e8530_t *scc, unsigned chn, unsigned read_max, unsigned write_max);

void e8530_set_clock (e8530_t *scc, unsigned long pclk, unsigned long rtxca, unsigned long rtxcb);

unsigned char e8530_get_ctl_a (e8530_t *scc);
unsigned char e8530_get_ctl_b (e8530_t *scc);

void e8530_set_ctl (e8530_t *scc, unsigned chn, unsigned char val);
void e8530_set_ctl_a (e8530_t *scc, unsigned char val);
void e8530_set_ctl_b (e8530_t *scc, unsigned char val);

unsigned char e8530_get_data (e8530_t *scc, unsigned chn);
unsigned char e8530_get_data_a (e8530_t *scc);
unsigned char e8530_get_data_b (e8530_t *scc);

void e8530_set_data (e8530_t *scc, unsigned chn, unsigned char val);
void e8530_set_data_a (e8530_t *scc, unsigned char val);
void e8530_set_data_b (e8530_t *scc, unsigned char val);

void e8530_set_dcd (e8530_t *scc, unsigned chn, unsigned char val);
void e8530_set_dcd_a (e8530_t *scc, unsigned char val);
void e8530_set_dcd_b (e8530_t *scc, unsigned char val);

void e8530_set_cts (e8530_t *scc, unsigned chn, unsigned char val);
void e8530_set_cts_a (e8530_t *scc, unsigned char val);
void e8530_set_cts_b (e8530_t *scc, unsigned char val);

void e8530_receive (e8530_t *scc, unsigned chn, unsigned char val);
void e8530_receive_a (e8530_t *scc, unsigned char val);
void e8530_receive_b (e8530_t *scc, unsigned char val);

unsigned char e8530_send (e8530_t *scc, unsigned chn);
unsigned char e8530_send_a (e8530_t *scc);
unsigned char e8530_send_b (e8530_t *scc);

int e8530_inp_full (e8530_t *scc, unsigned chn);
int e8530_out_empty (e8530_t *scc, unsigned chn);

void e8530_reset (e8530_t *scc);
void e8530_clock (e8530_t *scc, unsigned n);


#endif
