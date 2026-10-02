/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/thex.h                                               *
 * Created:     2020-11-27 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_THEX_H
#define PCE_LIB_THEX_H 1


typedef void (*thex_set_f) (void *ext, unsigned long addr, unsigned char val);
typedef unsigned char (*thex_get_f) (void *ext, unsigned long addr);


int thex_load_fp (FILE *fp, void *ext, thex_set_f set);
int thex_load (const char *fname, void *ext, thex_set_f set);

int thex_save_start (FILE *fp);
int thex_save_seg (FILE *fp, unsigned seg, unsigned ofs, unsigned long size, void *ext, thex_get_f get);
int thex_save (FILE *fp, unsigned long addr, unsigned long size, void *ext, thex_get_f get);
int thex_save_done (FILE *fp);

#endif
