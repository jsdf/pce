/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/srec.h                                               *
 * Created:     2005-03-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_SREC_H
#define PCE_LIB_SREC_H 1


#include <stdio.h>


typedef void (*srec_set_f) (void *ext, unsigned long addr, unsigned char val);
typedef unsigned char (*srec_get_f) (void *ext, unsigned long addr);


int srec_load_fp (FILE *fp, void *ext, srec_set_f set);
int srec_load (const char *fname, void *ext, srec_set_f set);

int srec_save_start (FILE *fp, const char *name);
int srec_save (FILE *fp, unsigned long base, unsigned long size, void *ext, srec_get_f get);
int srec_save_done (FILE *fp);


#endif
