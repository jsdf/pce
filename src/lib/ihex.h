/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/ihex.h                                               *
 * Created:     2004-06-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_IHEX_H
#define PCE_LIB_IHEX_H 1


typedef void (*ihex_set_f) (void *ext, unsigned long addr, unsigned char val);
typedef unsigned char (*ihex_get_f) (void *ext, unsigned long addr);


int ihex_load_fp (FILE *fp, void *ext, ihex_set_f set);
int ihex_load (const char *fname, void *ext, ihex_set_f set);

int ihex_save (FILE *fp, unsigned seg, unsigned ofs, unsigned long size, void *ext, ihex_get_f get);
int ihex_save_linear (FILE *fp, unsigned long base, unsigned long size, void *ext, ihex_get_f get);
int ihex_save_done (FILE *fp);

#endif
