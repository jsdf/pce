/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/serport.h                                        *
 * Created:     2003-09-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


/*
 * UART 8250 based serial port.
 */

#ifndef PCE_DEVICES_SERPORT_H
#define PCE_DEVICES_SERPORT_H 1


#include <stdio.h>

#include <chipset/82xx/e8250.h>

#include <devices/memory.h>

#include <drivers/char/char.h>


#define PCE_SERPORT_BUF 256


/*!***************************************************************************
 * @short The serial port context
 *****************************************************************************/
typedef struct {
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

	unsigned      out_idx;
	unsigned      out_cnt;
	unsigned char out_buf[PCE_SERPORT_BUF];

	unsigned      inp_idx;
	unsigned      inp_cnt;
	unsigned char inp_buf[PCE_SERPORT_BUF];

	int           check_out;
	int           check_inp;

	char_drv_t    *cdrv;
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

mem_blk_t *ser_get_reg (serport_t *ser);
e8250_t *ser_get_uart (serport_t *ser);

int ser_set_driver (serport_t *ser, const char *name);

int ser_set_log (serport_t *ser, const char *fname);

/*!***************************************************************************
 * @short Receive a byte from the outside
 * @param ser The serial port context
 * @param val The byte
 *
 * The value will be received by the UART as though it had arrived over the
 * wire.
 *****************************************************************************/
void ser_receive (serport_t *ser, unsigned char val);

void ser_clock (serport_t *ser, unsigned n);


#endif
