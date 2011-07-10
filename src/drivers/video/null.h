/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/null.h                                     *
 * Created:     2003-10-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_NULL_H
#define PCE_VIDEO_NULL_H 1


#include <stdio.h>

#include <libini/libini.h>
#include <drivers/video/terminal.h>


typedef struct {
	terminal_t     trm;
} null_t;


/*!***************************************************************************
 * @short Create a new null terminal
 *****************************************************************************/
terminal_t *null_new (ini_sct_t *ini);


#endif
