/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/serial.h                                         *
 * Created:       2003-09-04 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: serial.h,v 1.1 2003/09/04 20:20:45 hampa Exp $ */


#ifndef PCE_IBMPC_SERIAL_H
#define PCE_IBMPC_SERIAL_H 1


#include <stdio.h>

#include "pce.h"


typedef struct {
  mem_blk_t     *prt;

  e8250_t       uart;

  unsigned long io;

  unsigned      bps;
  unsigned      databits;
  unsigned      stopbits;
  unsigned      parity;

  int           fd;
  int           close;
} serial_t;


serial_t *ser_new (unsigned base);
void ser_del (serial_t *ser);;

void ser_set_fd (serial_t *ser, int fd, int close);
int ser_set_fname (serial_t *ser, const char *fname);

void ser_setup (serial_t *ser, unsigned char val);
void ser_send (serial_t *ser, unsigned char val);
void ser_recv (serial_t *ser, unsigned char val);

void ser_check (serial_t *ser);


#endif
