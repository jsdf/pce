/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/xms.h                                            *
 * Created:       2003-09-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-02 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: xms.h,v 1.1 2003/09/02 11:44:47 hampa Exp $ */


#ifndef PCE_XMS_H
#define PCE_XMS_H 1


typedef struct {
  unsigned long size;
  unsigned      lock;
  unsigned char *data;
} xms_block_t;


typedef struct {
  unsigned      cnt;
  xms_block_t   **blk;
  unsigned long used;
  unsigned long max;
} xms_t;


xms_t *xms_new (unsigned long max);
void xms_del (xms_t *xms);

void xms_handler (xms_t *xms, e8086_t *cpu);


#endif
