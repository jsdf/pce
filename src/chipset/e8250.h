/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/chipset/e8250.h                                        *
 * Created:       2003-08-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-05 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: e8250.h,v 1.3 2003/09/05 14:43:36 hampa Exp $ */


/* UART 8250/16450/16550 */


#ifndef PCE_E8250_H
#define PCE_E8250_H 1


#define E8250_PARITY_N 0x00
#define E8250_PARITY_O 0x01
#define E8250_PARITY_E 0x03
#define E8250_PARITY_M 0x05
#define E8250_PARITY_S 0x07


typedef void (*e8250_irq_f) (void *ext, unsigned char val);
typedef void (*e8250_recv_f) (void *ext, unsigned char val);
typedef void (*e8250_send_f) (void *ext, unsigned char val);
typedef void (*e8250_setup_f) (void *ext, unsigned char val);


typedef struct {
  unsigned      inp_i;
  unsigned      inp_j;
  unsigned      inp_n;
  unsigned char inp[16];

  unsigned      out_i;
  unsigned      out_j;
  unsigned      out_n;
  unsigned char out[16];

  unsigned char txd[2];
  unsigned char rxd[2];
  unsigned char ier;
  unsigned char iir;
  unsigned char lcr;
  unsigned char lsr;
  unsigned char mcr;
  unsigned char msr;
  unsigned char scratch;

  unsigned char ipr;

  unsigned short divisor;

  void           *irq_ext;
  e8250_irq_f    irq;

  void           *send_ext;
  e8250_send_f   send;

  void           *recv_ext;
  e8250_recv_f   recv;

  void           *setup_ext;
  e8250_setup_f  setup;
} e8250_t;


void e8250_init (e8250_t *uart);
e8250_t *e8250_new (void);

void e8250_free (e8250_t *uart);
void e8250_del (e8250_t *uart);

void e8250_clock (e8250_t *uart, unsigned long clk);

unsigned e8250_get_divisor (e8250_t *uart);
unsigned long e8250_get_bps (e8250_t *uart);
unsigned e8250_get_databits (e8250_t *uart);
unsigned e8250_get_stopbits (e8250_t *uart);
unsigned e8250_get_parity (e8250_t *uart);

int e8250_set_inp (e8250_t *uart, unsigned char val);
int e8250_get_inp (e8250_t *uart, unsigned char *val);
int e8250_inp_full (e8250_t *uart);

int e8250_set_out (e8250_t *uart, unsigned char val);
int e8250_get_out (e8250_t *uart, unsigned char *val);

void e8250_set_uint8 (e8250_t *uart, unsigned long addr, unsigned char val);
void e8250_set_uint16 (e8250_t *uart, unsigned long addr, unsigned short val);
unsigned char e8250_get_uint8 (e8250_t *uart, unsigned long addr);
unsigned short e8250_get_uint16 (e8250_t *uart, unsigned long addr);


#endif
