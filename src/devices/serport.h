/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/serport.h                                      *
 * Created:       2003-09-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-01-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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


typedef struct serport_s {
  mem_blk_t     port;

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
} serport_t;


void ser_init (serport_t *ser, unsigned long base);
serport_t *ser_new (unsigned long base);
void ser_free (serport_t *ser);
void ser_del (serport_t *ser);

mem_blk_t *ser_get_reg (serport_t *ser);

int ser_set_fp (serport_t *ser, FILE *fp, int close);
int ser_set_fname (serport_t *ser, const char *fname);

void ser_uart_setup (serport_t *ser, unsigned char val);
void ser_uart_out (serport_t *ser, unsigned char val);
void ser_uart_inp (serport_t *ser, unsigned char val);

void ser_clock (serport_t *ser, unsigned n);


#endif
