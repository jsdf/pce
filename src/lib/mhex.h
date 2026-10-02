/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/mhex.h                                               *
 * Created:     2020-05-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_LIB_MHEX_H
#define PCE_LIB_MHEX_H 1


typedef void (*mhex_set_f) (void *ext, unsigned long addr, unsigned char val);
typedef unsigned char (*mhex_get_f) (void *ext, unsigned long addr);


int mhex_load_fp (FILE *fp, void *ext, mhex_set_f set);
int mhex_load (const char *fname, void *ext, mhex_set_f set);

int mhex_save_fp (FILE *fp, unsigned long base, unsigned addr, unsigned size, void *ext, mhex_get_f get);
int mhex_save (const char *fname, unsigned long base, unsigned addr, unsigned size, void *ext, mhex_get_f set);


#endif
