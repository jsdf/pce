/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/serial.h                                    *
 * Created:     2007-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


typedef struct {
	e8530_t       *scc;

	unsigned      chn;

	unsigned long bps;
	unsigned      parity;
	unsigned      bpc;
	unsigned      stop;

	int           dtr;
	int           rts;

	FILE          *fp;
	int           fp_close;

	int           fd;
	int           fd_close;
} mac_ser_t;


void mac_ser_init (mac_ser_t *ser);
void mac_ser_free (mac_ser_t *ser);

void mac_ser_set_scc (mac_ser_t *ser, e8530_t *scc, unsigned chn);

int mac_ser_set_fp (mac_ser_t *ser, FILE *fp, int close);
int mac_ser_set_fname (mac_ser_t *ser, const char *fname);

int mac_ser_set_fd (mac_ser_t *ser, int fd, int close);
int mac_ser_set_dname (mac_ser_t *ser, const char *dname);

void mac_ser_clock (mac_ser_t *ser, unsigned n);


#endif
