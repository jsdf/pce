/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/initerm.h                                            *
 * Created:     2008-10-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_INITERM_H
#define PCE_LIB_INITERM_H 1


#include <libini/libini.h>

#include <drivers/video/terminal.h>


terminal_t *ini_get_terminal (ini_sct_t *ini, const char *def);


#endif
