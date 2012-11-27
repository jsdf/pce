/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-tcp.h                                  *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRIVERS_CHAR_TCP_H
#define PCE_DRIVERS_CHAR_TCP_H 1


#include <stdio.h>

#include <drivers/char/char.h>


typedef struct char_tcp_t {
	char_drv_t cdrv;

	int        connect;
	int        telnet;
	int        telnetinit;
	int        usectl;

	char       *host;
	unsigned   port;

	int        listen_fd;
	int        fd;

	unsigned   ctl;

	unsigned   telnet_state;
} char_tcp_t;


#endif
