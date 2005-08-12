/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/iniram.h                                           *
 * Created:       2005-07-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-08-12 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2005 Hampa Hug <hampa@hampa.ch>                        *
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


#ifndef PCE_LIB_INIRAM_H
#define PCE_LIB_INIRAM_H 1


#include <libini/libini.h>

#include <devices/memory.h>


void ini_get_ram (memory_t *mem, ini_sct_t *ini, mem_blk_t **addr0);
void ini_get_rom (memory_t *mem, ini_sct_t *ini);


#endif
