/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/chipset/e8259.h                                        *
 * Created:       2003-04-21 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-24 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8259.h,v 1.4 2003/09/24 08:08:16 hampa Exp $ */


/* PIC 8259A */


#ifndef PCE_E8259_H
#define PCE_E8259_H 1


typedef void (*e8259_irq_f) (void *ext, unsigned char val);


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
  e8259_irq_f   irq;
} e8259_t;


e8259_t *e8259_new (void);
void e8259_del (e8259_t *pic);

/*!***************************************************************************
 * @short  Get an IRQ function
 * @param  pic The PIC
 * @param  irq The IRQ number
 * @return The function that is to be called to trigger IRQ irq.
 *
 * The return value is a pointer to one of the e8259_set_irq?() functions.
 *****************************************************************************/
e8259_irq_f e8259_get_irq (e8259_t *pic, unsigned irq);

/*!***************************************************************************
 * @short Set the IRQ function
 * @param pic The PIC
 * @param ext Parameter for set
 * @param set The function that is called when an IRQ occurs
 *****************************************************************************/
void e8259_set_irq (e8259_t *pic, void *ext, e8259_irq_f set);

void e8259_set_icw1 (e8259_t *pic, unsigned char val);
void e8259_set_icwn (e8259_t *pic, unsigned char val);
void e8259_set_ocw1 (e8259_t *pic, unsigned char val);
void e8259_set_ocw2 (e8259_t *pic, unsigned char val);
void e8259_set_ocw3 (e8259_t *pic, unsigned char val);

void e8259_set_irq0 (e8259_t *pic, unsigned char val);
void e8259_set_irq1 (e8259_t *pic, unsigned char val);
void e8259_set_irq2 (e8259_t *pic, unsigned char val);
void e8259_set_irq3 (e8259_t *pic, unsigned char val);
void e8259_set_irq4 (e8259_t *pic, unsigned char val);
void e8259_set_irq5 (e8259_t *pic, unsigned char val);
void e8259_set_irq6 (e8259_t *pic, unsigned char val);
void e8259_set_irq7 (e8259_t *pic, unsigned char val);

/*!***************************************************************************
 * @short  Acknowledge an IRQ
 * @param  pic The PIC
 * @return The IRQ that occurred
 *****************************************************************************/
unsigned char e8259_inta (e8259_t *pic);

void e8259_clock (e8259_t *pic);

void e8259_set_uint8 (e8259_t *pic, unsigned long addr, unsigned char val);
void e8259_set_uint16 (e8259_t *pic, unsigned long addr, unsigned short val);

unsigned char e8259_get_uint8 (e8259_t *pic, unsigned long addr);
unsigned short e8259_get_uint16 (e8259_t *pic, unsigned long addr);


#endif
