/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-mouse.h                                *
 * Created:     2011-10-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRIVERS_CHAR_MOUSE_H
#define PCE_DRIVERS_CHAR_MOUSE_H 1


#include <drivers/char/char.h>


#define CHAR_MOUSE_BUF 256


typedef struct char_mouse_t {
	char_drv_t    cdrv;

	unsigned      protocol;

	unsigned      buf_hd;
	unsigned      buf_tl;
	unsigned char buf[CHAR_MOUSE_BUF];

	char          dtr;
	char          rts;

	int           dx;
	int           dy;
	unsigned      button;

	int           scale_x[3];
	int           scale_y[3];
} char_mouse_t;


#endif
