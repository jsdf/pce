/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/load.h                                             *
 * Created:       2004-08-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-08-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#ifndef PCE_LIB_LOAD_H
#define PCE_LIB_LOAD_H 1


#include <devices/memory.h>


int pce_load_blk_bin (mem_blk_t *blk, const char *fname);

int pce_load_mem_hex (memory_t *mem, const char *fname);

int pce_load_mem_bin (memory_t *mem, const char *fname, unsigned long base);


#endif
