/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/ihex.h                                             *
 * Created:       2004-06-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-06-23 by Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_LIB_IHEX_H
#define PCE_LIB_IHEX_H 1


int ihex_load_fp (FILE *fp, void *buf, unsigned long base, unsigned long size);
int ihex_load (const char *fname, void *buf, unsigned long base, unsigned long size);

int ihex_save_linear (FILE *fp, void *buf, unsigned long base, unsigned long size);
int ihex_save_done (FILE *fp);

#endif
