/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/cga.h                                            *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-23 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: cga.h,v 1.1 2003/04/23 12:48:42 hampa Exp $ */


#ifndef PCE_CGA_H
#define PCE_CGA_H 1


typedef struct {
  mem_blk_t     *mem;
  mem_blk_t     *crtc;

  unsigned char crtc_reg[16];

  unsigned      crtc_mode;
  unsigned      crtc_pos;
  unsigned      crtc_ofs;

  term_t        trm;
} cga_t;


cga_t *cga_new (FILE *fp);
void cga_del (cga_t *cga);

void cga_clock (cga_t *cga);

void cga_prt_state (cga_t *cga, FILE *fp);

void cga_set_pos (cga_t *cga, unsigned pos);

void cga_mem_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val);
void cga_mem_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val);

void cga_crtc_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val);
void cga_crtc_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val);
unsigned char cga_crtc_get_uint8 (cga_t *cga, unsigned long addr);


#endif
