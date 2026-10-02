/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/serial.h                                    *
 * Created:     2007-12-19 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_SERIAL_H
#define PCE_MACPLUS_SERIAL_H 1


#include <chipset/e8530.h>
#include <drivers/char/char.h>


#define MAC_SER_BUF 256


typedef struct {
	e8530_t       *scc;

	unsigned      chn;

	unsigned long bps;
	unsigned      parity;
	unsigned      bpc;
	unsigned      stop;

	int           dtr;
	int           rts;

	unsigned      inp_idx;
	unsigned      inp_cnt;
	unsigned char inp_buf[MAC_SER_BUF];

	unsigned      out_idx;
	unsigned      out_cnt;
	unsigned char out_buf[MAC_SER_BUF];

	char_drv_t    *cdrv;
} mac_ser_t;


void mac_ser_init (mac_ser_t *ser);
void mac_ser_free (mac_ser_t *ser);

void mac_ser_set_scc (mac_ser_t *ser, e8530_t *scc, unsigned chn);

int mac_ser_set_driver (mac_ser_t *ser, const char *name);

int mac_ser_set_file (mac_ser_t *ser, const char *fname);

void mac_ser_process (mac_ser_t *ser);


#endif
