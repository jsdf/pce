/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/hexdump.h                                          *
 * Created:       2003-11-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-11-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: hexdump.h,v 1.1 2003/11/16 03:44:26 hampa Exp $ */


#ifndef PCE_LIB_HEXDUMP_H
#define PCE_LIB_HEXDUMP_H 1


void pce_dump_hex (FILE *fp, void *buf, unsigned long n,
  unsigned long addr, unsigned cols, char *prefix, int ascii);


#endif
