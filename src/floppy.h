/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     floppy.h                                                   *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: floppy.h,v 1.1 2003/04/15 04:03:56 hampa Exp $ */


#ifndef PCE_FLOPPY_H
#define PCE_FLOPPY_H 1


#include <pce.h>
#include <e8086/e8086.h>


typedef struct {
  unsigned      c;
  unsigned      h;
  unsigned      s;
  unsigned long size;
  unsigned char *data;
} floppy_t;


floppy_t *flp_new (unsigned c, unsigned h, unsigned s);
void flp_del (floppy_t *flp);
int flp_load_img (floppy_t *flp, const char *fname);
void flp_int_13 (floppy_t *flp, e8086_t *cpu);


#endif
