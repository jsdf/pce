/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-slip.h                                 *
 * Created:     2009-11-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_DRIVERS_CHAR_SLIP_H
#define PCE_DRIVERS_CHAR_SLIP_H 1


#include <drivers/char/char.h>


#define SLIP_BUF_MAX 4096


typedef struct char_slip_t {
	char_drv_t    cdrv;

	char          *tun_name;
	int           tun_fd;

	unsigned      out_cnt;
	char          out_esc;
	unsigned char out[SLIP_BUF_MAX];

	unsigned      inp_idx;
	unsigned      inp_cnt;
	unsigned char inp[SLIP_BUF_MAX];
} char_slip_t;


#endif
