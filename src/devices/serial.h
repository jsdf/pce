/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/serial.h                                       *
 * Created:       2003-09-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-01-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 by Hampa Hug <hampa@hampa.ch>                *
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


/*
  UART 8250 based serial port. This is very limited. Data sent out over
  the wires by the UART are written to a file. No data is ever
  received.
*/

#ifndef PCE_IBMPC_SERIAL_H
#define PCE_IBMPC_SERIAL_H 1


#include <stdio.h>

#include <chipset/e8250.h>
#include "memory.h"


typedef struct serial_s {
  mem_blk_t     *prt;

  e8250_t       uart;

  unsigned long io;

  unsigned      bps;
  unsigned      databits;
  unsigned      stopbits;
  unsigned      parity;

  int           dtr;
  int           rts;

  FILE          *fp;
  int           fp_close;
} serial_t;


void ser_init (serial_t *ser, unsigned long base);
serial_t *ser_new (unsigned long base);
void ser_free (serial_t *ser);
void ser_del (serial_t *ser);

int ser_set_fp (serial_t *ser, FILE *fp, int close);
int ser_set_fname (serial_t *ser, const char *fname);

void ser_uart_setup (serial_t *ser, unsigned char val);
void ser_uart_out (serial_t *ser, unsigned char val);
void ser_uart_inp (serial_t *ser, unsigned char val);

void ser_clock (serial_t *ser, unsigned n);


#endif
