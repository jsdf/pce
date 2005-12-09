/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/serport.c                                      *
 * Created:       2003-09-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-09 by Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdio.h>
#include <stdlib.h>

#include "serport.h"


unsigned char ser_get_uint8 (serport_t *ser, unsigned long addr)
{
  return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift));
}

unsigned short ser_get_uint16 (serport_t *ser, unsigned long addr)
{
  return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift));
}

unsigned long ser_get_uint32 (serport_t *ser, unsigned long addr)
{
  return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift));
}

void ser_set_uint8 (serport_t *ser, unsigned long addr, unsigned char val)
{
  e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift, val);
}

void ser_set_uint16 (serport_t *ser, unsigned long addr, unsigned short val)
{
  e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift, val & 0xff);
}

void ser_set_uint32 (serport_t *ser, unsigned long addr, unsigned long val)
{
  e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift, val & 0xff);
}

void ser_init (serport_t *ser, unsigned long base, unsigned shift)
{
  dev_init (&ser->device, ser, "uart");

  ser->device.free = (void *) ser_free;
  ser->device.del = (void *) ser_del;
  ser->device.clock = (void *) ser_clock;

  ser->io = base;

  ser->addr_shift = shift;

  e8250_init (&ser->uart);
  e8250_set_setup_f (&ser->uart, ser_uart_check_setup, ser);
  e8250_set_send_f (&ser->uart, ser_uart_check_out, ser);
  e8250_set_recv_f (&ser->uart, ser_uart_check_inp, ser);

  mem_blk_init (&ser->port, base, 8 << shift, 0);
  ser->port.ext = ser;
  ser->port.get_uint8 = (mem_get_uint8_f) ser_get_uint8;
  ser->port.set_uint8 = (mem_set_uint8_f) ser_set_uint8;
  ser->port.get_uint16 = (mem_get_uint16_f) ser_get_uint16;
  ser->port.set_uint16 = (mem_set_uint16_f) ser_set_uint16;
  ser->port.get_uint32 = (mem_get_uint32_f) ser_get_uint32;
  ser->port.set_uint32 = (mem_set_uint32_f) ser_set_uint32;

  ser->bps = 2400;
  ser->databits = 8;
  ser->stopbits = 1;
  ser->parity = E8250_PARITY_N;

  ser->dtr = 0;
  ser->rts = 0;

  ser->check_out = 0;
  ser->check_inp = 0;

  ser->fp = NULL;
  ser->fp_close = 0;

  /* ready to receive data */
  e8250_set_dsr (&ser->uart, 1);
  e8250_set_cts (&ser->uart, 1);
}

serport_t *ser_new (unsigned long base, unsigned shift)
{
  serport_t *ser;

  ser = malloc (sizeof (serport_t));
  if (ser == NULL) {
    return (NULL);
  }

  ser_init (ser, base, shift);

  return (ser);
}

void ser_free (serport_t *ser)
{
  e8250_free (&ser->uart);
  mem_blk_free (&ser->port);

  if (ser->fp_close) {
    fclose (ser->fp);
  }
}

void ser_del (serport_t *ser)
{
  if (ser != NULL) {
    ser_free (ser);
    free (ser);
  }
}

device_t *ser_get_device (serport_t *ser)
{
  return (&ser->device);
}

mem_blk_t *ser_get_reg (serport_t *ser)
{
  return (&ser->port);
}

e8250_t *ser_get_uart (serport_t *ser)
{
  return (&ser->uart);
}

int ser_set_fp (serport_t *ser, FILE *fp, int close)
{
  if (ser->fp_close) {
    fclose (ser->fp);
  }

  ser->fp = fp;
  ser->fp_close = (fp != NULL) && close;

  return (0);
}

int ser_set_fname (serport_t *ser, const char *fname)
{
  FILE *fp;

  fp = fopen (fname, "wb");
  if (fp == NULL) {
    return (1);
  }

  if (ser_set_fp (ser, fp, 1)) {
    fclose (fp);
    return (1);
  }

  return (0);
}

void ser_uart_check_setup (serport_t *ser, unsigned char val)
{
  int           chg;
  unsigned long bps;
  unsigned      data, stop, parity;
  int           dtr, rts;

  chg = 0;

  bps = e8250_get_bps (&ser->uart);
  if (bps != ser->bps) {
    ser->bps = bps;
    chg = 1;
  }

  data = e8250_get_databits (&ser->uart);
  if (data != ser->databits) {
    ser->databits = data;
    chg = 1;
  }

  stop = e8250_get_stopbits (&ser->uart);
  if (stop != ser->stopbits) {
    ser->stopbits = stop;
    chg = 1;
  }

  parity = e8250_get_parity (&ser->uart);
  if (parity != ser->parity) {
    ser->parity = parity;
    chg = 1;
  }

  if (chg) {
/*    ser_line_setup (ser); */
  }

  chg = 0;

  dtr = e8250_get_dtr (&ser->uart);
  if (dtr != ser->dtr) {
    ser->dtr = dtr;
    chg = 1;
  }

  rts = e8250_get_rts (&ser->uart);
  if (rts != ser->rts) {
    ser->rts = rts;
    chg = 1;
  }

  if (chg) {
/*    ser_modem_setup (ser); */
  }
}

/* 8250 output buffer is not empty */
void ser_uart_check_out (serport_t *ser, unsigned char val)
{
  ser->check_out = 1;
}

/* 8250 input buffer is not full */
void ser_uart_check_inp (serport_t *ser, unsigned char val)
{
  ser->check_inp = 1;
}

void ser_receive (serport_t *ser, unsigned char val)
{
  e8250_receive (&ser->uart, val);
}

void ser_clock (serport_t *ser, unsigned n)
{
  unsigned char c;

  if (ser->check_out) {
    ser->check_out = 0;

    while (1) {
      if (e8250_get_out (&ser->uart, &c)) {
        break;
      }

      if (ser->fp != NULL) {
        fputc (c, ser->fp);
      }
    }

    if (ser->fp != NULL) {
      fflush (ser->fp);
    }
  }

  e8250_clock (&ser->uart, n);
}
