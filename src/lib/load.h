/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/load.h                                               *
 * Created:     2004-08-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_LOAD_H
#define PCE_LIB_LOAD_H 1


#include <devices/memory.h>

#include <libini/libini.h>


int pce_load_blk_bin (mem_blk_t *blk, const char *fname);

int pce_load_mem_ihex (memory_t *mem, const char *fname);
int pce_load_mem_mhex (memory_t *mem, const char *fname);
int pce_load_mem_srec (memory_t *mem, const char *fname);
int pce_load_mem_bin (memory_t *mem, const char *fname, unsigned long base);

int pce_load_mem (memory_t *mem, const char *fname, const char *fmt, unsigned long addr);

int pce_load_mem_ini (memory_t *mem, ini_sct_t *ini);


#endif
