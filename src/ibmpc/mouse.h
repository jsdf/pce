/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/mouse.h                                          *
 * Created:       2003-08-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-29 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: mouse.h,v 1.1 2003/08/29 19:18:14 hampa Exp $ */


#ifndef PCE_MOUSE_H
#define PCE_MOUSE_H 1


#define MSE_BUF 32


typedef void (*mse_intr_f) (void *ext, unsigned char val);


typedef struct {
  mem_blk_t *reg;

  void *intr_ext;
  void (*intr) (void *ext, unsigned char val);

  unsigned char rbuf[MSE_BUF];
  unsigned      rcnt;

  unsigned short divisor;
} mouse_t;


mouse_t *mse_new (unsigned short base);
void mse_del (mouse_t *mse);

mem_blk_t *mse_get_reg (mouse_t *mse);

void mse_set (mouse_t *mse, int dx, int dy, unsigned but);

void mse_reg_set_uint8 (mouse_t *mse, unsigned long addr, unsigned char val);
void mse_reg_set_uint16 (mouse_t *mse, unsigned long addr, unsigned short val);
unsigned char mse_reg_get_uint8 (mouse_t *mse, unsigned long addr);
unsigned short mse_reg_get_uint16 (mouse_t *mse, unsigned long addr);


#endif
