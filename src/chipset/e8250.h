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

/* $Id: e8250.h,v 1.4 2003/09/06 13:52:51 hampa Exp $ */


/* UART 8250/16450/16550 */


#ifndef PCE_E8250_H
#define PCE_E8250_H 1


#define E8250_PARITY_N 0x00
#define E8250_PARITY_O 0x01
#define E8250_PARITY_E 0x03
#define E8250_PARITY_M 0x05
#define E8250_PARITY_S 0x07

#define E8250_IER_SINP 0x08
#define E8250_IER_ERBK 0x04
#define E8250_IER_TBE  0x02
#define E8250_IER_RRD  0x01

#define E8250_IIR_SINP (0x00 << 1)
#define E8250_IIR_TBE  (0x01 << 1)
#define E8250_IIR_RRD  (0x02 << 1)
#define E8250_IIR_ERBK (0x03 << 1)
#define E8250_IIR_PND  0x01

#define E8250_LCR_DLAB   0x80
#define E8250_LCR_BRK    0x40
#define E8250_LCR_PARITY 0x38
#define E8250_LCR_STOP   0x04
#define E8250_LCR_DATA   0x03

#define E8250_LSR_TXE 0x40
#define E8250_LSR_TBE 0x20
#define E8250_LSR_RRD 0x01

#define E8250_MCR_LOOP 0x10
#define E8250_MCR_OUT2 0x08
#define E8250_MCR_OUT1 0x04
#define E8250_MCR_RTS  0x02
#define E8250_MCR_DTR  0x01

#define E8250_MSR_DCD  0x80
#define E8250_MSR_RI   0x40
#define E8250_MSR_DSR  0x20
#define E8250_MSR_CTS  0x10
#define E8250_MSR_DDCD 0x08
#define E8250_MSR_DRI  0x04
#define E8250_MSR_DDSR 0x02
#define E8250_MSR_DCTS 0x01


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
int e8250_get_dtr (e8250_t *uart);
int e8250_get_rts (e8250_t *uart);

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
