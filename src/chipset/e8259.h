/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/e8259/e8259.h                                          *
 * Created:       2003-04-21 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-21 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8259.h,v 1.1 2003/04/26 16:35:28 hampa Exp $ */


/* PIC 8259A */


#ifndef PCE_E8259_H
#define PCE_E8259_H 1


typedef struct {
  unsigned char icw[4];
  unsigned char ocw[3];

  unsigned char irr;
  unsigned char imr;
  unsigned char isr;

  unsigned      base;

  unsigned      next_icw;

  int           read_irr;

  unsigned long irq_cnt[8];

  void          *irq_ext;
  void          (*irq) (void *ext, unsigned char val);
} e8259_t;


e8259_t *e8259_new (void);
void e8259_del (e8259_t *pic);

void e8259_set_icw1 (e8259_t *pic, unsigned char val);
void e8259_set_icwn (e8259_t *pic, unsigned char val);
void e8259_set_ocw1 (e8259_t *pic, unsigned char val);
void e8259_set_ocw2 (e8259_t *pic, unsigned char val);
void e8259_set_ocw3 (e8259_t *pic, unsigned char val);

void e8259_set_irq (e8259_t *pic, unsigned char val);
void e8259_set_irq0 (e8259_t *pic, unsigned char val);
void e8259_set_irq1 (e8259_t *pic, unsigned char val);
void e8259_set_irq2 (e8259_t *pic, unsigned char val);
void e8259_set_irq3 (e8259_t *pic, unsigned char val);
void e8259_set_irq4 (e8259_t *pic, unsigned char val);
void e8259_set_irq5 (e8259_t *pic, unsigned char val);
void e8259_set_irq6 (e8259_t *pic, unsigned char val);
void e8259_set_irq7 (e8259_t *pic, unsigned char val);

unsigned char e8259_inta (e8259_t *pic);

void e8259_clock (e8259_t *pic);

void e8259_set_uint8 (e8259_t *pic, unsigned long addr, unsigned char val);
unsigned char e8259_get_uint8 (e8259_t *pic, unsigned long addr);


#endif
