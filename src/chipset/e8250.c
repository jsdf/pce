/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/chipset/e8250.c                                        *
 * Created:       2003-08-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-04 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8250.c,v 1.1 2003/09/04 20:11:16 hampa Exp $ */


#include <stdlib.h>
#include <stdio.h>

#include "e8250.h"


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
#define E8250_MSR_DDCD 0x80
#define E8250_MSR_DRI  0x40
#define E8250_MSR_DDSR 0x20
#define E8250_MSR_DCTS 0x10


void e8250_init (e8250_t *uart)
{
  uart->inp_i = 0;
  uart->inp_j = 0;
  uart->inp_n = 16;

  uart->out_i = 0;
  uart->out_j = 0;
  uart->out_n = 16;

  uart->txd[0] = 0;
  uart->txd[1] = 0;
  uart->rxd[0] = 0;
  uart->txd[1] = 0;

  uart->ier = 0;
  uart->iir = E8250_IIR_PND;

  uart->lcr = 0;
  uart->lsr = E8250_LSR_TXE | E8250_LSR_TBE;

  uart->mcr = 0;
  uart->msr = E8250_MSR_DCD | E8250_MSR_DSR | E8250_MSR_CTS;

  uart->scratch = 0;

  uart->divisor = 0;

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

unsigned e8250_get_divisor (e8250_t *uart)
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

void e8250_check_txd (e8250_t *uart)
{
  if (uart->txd[1] == 0) {
    return;
  }

  if (e8250_set_out (uart, uart->txd[0])) {
    return;
  }

  uart->txd[1] = 0;
  uart->lsr |= E8250_LSR_TBE;

  if (uart->send != NULL) {
    uart->send (uart->send_ext, 1);
  }

  if ((uart->ier & E8250_IER_TBE) && (uart->iir < E8250_IIR_TBE)) {
    uart->iir = E8250_IIR_TBE;
    if (uart->irq != NULL) {
      uart->irq (uart->irq_ext, 1);
    }
  }
}

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

  if (uart->recv != NULL) {
    uart->recv (uart->recv_ext, 1);
  }

  if ((uart->ier & E8250_IER_RRD) && (uart->iir < E8250_IIR_RRD)) {
    uart->iir = E8250_IIR_RRD;
    if (uart->irq != NULL) {
      uart->irq (uart->irq_ext, 1);
    }
  }
}

void e8250_check (e8250_t *uart)
{
  e8250_check_txd (uart);
  e8250_check_rxd (uart);
}

void e8250_set_divisor (e8250_t *uart, unsigned short div)
{
  uart->divisor = div & 0xffff;

  if (uart->setup != NULL) {
    uart->setup (uart->setup_ext, 1);
  }
}

void e8250_set_ier (e8250_t *uart, unsigned char val)
{
  uart->ier = val & 0x0f;

  if ((uart->ier & E8250_IER_RRD) && (uart->iir < E8250_IIR_RRD)) {
    if (uart->rxd[1] != 0) {
      uart->iir = E8250_IIR_RRD;
      if (uart->irq != NULL) {
        uart->irq (uart->irq_ext, 1);
      }
    }
  }

  if ((uart->ier & E8250_IER_TBE) && (uart->iir < E8250_IIR_TBE)) {
    if (uart->txd[1] == 0) {
      uart->iir = E8250_IIR_TBE;
      if (uart->irq != NULL) {
        uart->irq (uart->irq_ext, 1);
      }
    }
  }
}

void e8250_set_uint8 (e8250_t *uart, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0x00:
      if (uart->lcr & E8250_LCR_DLAB) {
        e8250_set_divisor (uart, (uart->divisor & 0xff00) | (val & 0x00ff));
      }
      else {
        uart->txd[0] = val;
        uart->txd[1] = 1;
        uart->lsr &= ~E8250_LSR_TBE;
        if (uart->iir == E8250_IIR_TBE) {
          uart->iir = E8250_IIR_PND;
        }
        e8250_check_txd (uart);
      }
      break;

    case 0x01:
      if (uart->lcr & E8250_LCR_DLAB) {
        e8250_set_divisor (uart, (uart->divisor & 0xff) | ((val & 0xff) << 8));
      }
      else {
        e8250_set_ier (uart, val);
      }
      break;

    case 0x02:
      break;

    case 0x03:
      uart->lcr = val;
      if (uart->setup != NULL) {
        uart->setup (uart->setup_ext, 1);
      }
      break;

    case 0x04:
      uart->mcr = val;
      break;

    case 0x05:
      break;

    case 0x06:
      break;

    case 0x07:
      uart->scratch = val;
      break;
  }
}

void e8250_set_uint16 (e8250_t *uart, unsigned long addr, unsigned short val)
{
}

unsigned char e8250_get_uint8 (e8250_t *uart, unsigned long addr)
{
  switch (addr) {
    case 0x00:
      if (uart->lcr & E8250_LCR_DLAB) {
        return (uart->divisor & 0xff);
      }
      else {
        unsigned char val;
        val = uart->rxd[0];
        uart->rxd[1] = 0;
        uart->lsr &= ~E8250_LSR_RRD;
        if (uart->iir == E8250_IIR_RRD) {
          uart->iir = E8250_IIR_PND;
        }
        e8250_check_rxd (uart);
        return (val);
      }
      break;

    case 0x01:
      if (uart->lcr & 0x80) {
        return ((uart->divisor >> 8) & 0xff);
      }
      else {
        return (uart->ier);
      }
      break;

    case 0x02:
      {
        unsigned char val;

        val = uart->iir;

        if (val == E8250_IIR_TBE) {
          uart->iir = E8250_IIR_PND;
        }

        return (val);
      }

    case 0x03:
      return (uart->lcr);

    case 0x04:
      return (uart->mcr);

    case 0x05:
      return (uart->lsr);

    case 0x06:
      return (uart->msr);

    case 0x07:
      return (uart->scratch);
  }

  return (0xff);
}

unsigned short e8250_get_uint16 (e8250_t *uart, unsigned long addr)
{
  return (0xffff);
}
