/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     mda.h                                                      *
 * Created:       2003-04-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: mda.h,v 1.1 2003/04/15 04:03:56 hampa Exp $ */


#ifndef PCE_MDA_H
#define PCE_MDA_H 1


#include <pce.h>
#include <e8086/e8086.h>


typedef struct {
  mem_blk_t     *mem;
  mem_blk_t     *crtc;

  unsigned char crtc_reg[16];

  unsigned      cur_pos;

  FILE          *fp;
} mda_t;


mda_t *mda_new (void);
void mda_del (mda_t *mda);

void mda_clock (mda_t *mda);

void mda_set_pos (mda_t *mda, unsigned pos);

void mda_mem_set_uint8 (void *obj, unsigned long addr, unsigned char val);
void mda_mem_set_uint16 (void *obj, unsigned long addr, unsigned short val);

void mda_crtc_set_uint8 (void *obj, unsigned long addr, unsigned char val);
void mda_crtc_set_uint16 (void *obj, unsigned long addr, unsigned short val);
unsigned char mda_crtc_get_uint8 (void *obj, unsigned long addr);


#endif
