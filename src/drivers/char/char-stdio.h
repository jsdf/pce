/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-stdio.h                                *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRIVERS_CHAR_STDIO_H
#define PCE_DRIVERS_CHAR_STDIO_H 1


#include <stdio.h>

#include <drivers/char/char.h>


typedef struct char_stdio_t {
	char_drv_t cdrv;

	char       flush;
	char       reopen;
	char       append;

	char       *read_name;
	FILE       *read_fp;

	char       *write_name;
	FILE       *write_fp;
} char_stdio_t;


#endif
