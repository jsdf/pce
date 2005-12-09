/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/serport.h                                      *
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


/*
  UART 8250 based serial port. This is very limited. Data sent out over
  the wires by the UART are written to a file. No data is ever
  received unless ser_receive() is called.
*/

#ifndef PCE_DEVICES_SERPORT_H
#define PCE_DEVICES_SERPORT_H 1


#include <stdio.h>

#include <chipset/82xx/e8250.h>

#include <devices/device.h>
#include <devices/memory.h>


/*!***************************************************************************
 * @short The serial port context
 *****************************************************************************/
typedef struct serport_s {
  device_t      device;

  /* the 8250 I/O ports. size is (8 << addr_shift). */
  mem_blk_t     port;

  e8250_t       uart;

  /* the I/O base address */
  unsigned long io;

  /* the 8250 register address shift */
  unsigned      addr_shift;

  unsigned      bps;
  unsigned      databits;
  unsigned      stopbits;
  unsigned      parity;

  int           dtr;
  int           rts;

  int           check_out;
  int           check_inp;

  FILE          *fp;
  int           fp_close;
} serport_t;


/*!***************************************************************************
 * @short Initialize a serial port context
 * @param ser   The serial port context
 * @param base  The I/O base address
 * @param shift The 8250 register address shift
 *****************************************************************************/
void ser_init (serport_t *ser, unsigned long base, unsigned shift);

/*!***************************************************************************
 * @short Create a new serial port context
 * @param base  The I/O base address
 * @param shift The 8250 register address shift
 *****************************************************************************/
serport_t *ser_new (unsigned long base, unsigned shift);

/*!***************************************************************************
 * @short Free the resources used by a serial port context
 * @param ser The serial port context
 *****************************************************************************/
void ser_free (serport_t *ser);

/*!***************************************************************************
 * @short Delete a serial port context
 * @param ser The serial port context
 *****************************************************************************/
void ser_del (serport_t *ser);

device_t *ser_get_device (serport_t *ser);

mem_blk_t *ser_get_reg (serport_t *ser);
e8250_t *ser_get_uart (serport_t *ser);

int ser_set_fp (serport_t *ser, FILE *fp, int close);
int ser_set_fname (serport_t *ser, const char *fname);

void ser_uart_check_setup (serport_t *ser, unsigned char val);
void ser_uart_check_out (serport_t *ser, unsigned char val);
void ser_uart_check_inp (serport_t *ser, unsigned char val);

void ser_receive (serport_t *ser, unsigned char val);

void ser_clock (serport_t *ser, unsigned n);


#endif
