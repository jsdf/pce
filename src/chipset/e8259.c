/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/chipset/e8259.c                                        *
 * Created:       2003-04-21 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-02-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>
#include <stdio.h>

#include "e8259.h"


void e8259_init (e8259_t *pic)
{
  unsigned i;

  pic->icw[0] = 0;
  pic->icw[1] = 0;
  pic->icw[2] = 0;
  pic->icw[3] = 0;

  pic->ocw[0] = 0;
  pic->ocw[1] = 0;
  pic->ocw[2] = 0;

  pic->base = 0;

  pic->next_icw = 0;
  pic->read_irr = 1;

  pic->irr = 0x00;
  pic->imr = 0xff;
  pic->isr = 0x00;

  for (i = 0; i < 8; i++) {
    pic->irq_cnt[i] = 0;
  }

  pic->irq_ext = NULL;
  pic->irq = NULL;
}

e8259_t *e8259_new (void)
{
  e8259_t *pic;

  pic = malloc (sizeof (e8259_t));
  if (pic == NULL) {
    return (NULL);
  }

  e8259_init (pic);

  return (pic);
}

void e8259_free (e8259_t *pic)
{
}

void e8259_del (e8259_t *pic)
{
  if (pic != NULL) {
    e8259_free (pic);
    free (pic);
  }
}

e8259_irq_f e8259_get_irq (e8259_t *pic, unsigned irq)
{
  switch (irq & 7) {
    case 0:
      return ((e8259_irq_f) &e8259_set_irq0);

    case 1:
      return ((e8259_irq_f) &e8259_set_irq1);

    case 2:
      return ((e8259_irq_f) &e8259_set_irq2);

    case 3:
      return ((e8259_irq_f) &e8259_set_irq3);

    case 4:
      return ((e8259_irq_f) &e8259_set_irq4);

    case 5:
      return ((e8259_irq_f) &e8259_set_irq5);

    case 6:
      return ((e8259_irq_f) &e8259_set_irq6);

    case 7:
      return ((e8259_irq_f) &e8259_set_irq7);
  }

  return (NULL);
}

void e8259_set_irq (e8259_t *pic, void *ext, e8259_irq_f set)
{
  pic->irq_ext = ext;
  pic->irq = set;
}

void e8259_set_icw1 (e8259_t *pic, unsigned char val)
{
  pic->icw[0] = val;
  pic->icw[1] = 0;
  pic->icw[2] = 0;
  pic->icw[3] = 0;

  pic->ocw[0] = 0;
  pic->ocw[1] = 0;
  pic->ocw[2] = 0;

  pic->base = 0;

  pic->next_icw = 1;
  pic->read_irr = 1;

  pic->irr = 0x00;
  pic->imr = 0xff;
  pic->isr = 0x00;
}

void e8259_set_icwn (e8259_t *pic, unsigned char val)
{
  switch (pic->next_icw) {
    case 1:
      pic->icw[1] = val;
      pic->base = val & ~7;
      if (pic->icw[0] & 0x02) {
        pic->next_icw = 3;
      }
      else if (pic->icw[0] & 0x01) {
        pic->next_icw = 2;
      }
      else {
        pic->next_icw = 0;
      }
      break;

    case 2:
      pic->icw[2] = val;
      if (pic->icw[0] & 0x01) {
        pic->next_icw = 3;
      }
      else {
        pic->next_icw = 0;
      }
      break;

    case 3:
      pic->icw[3] = val;
      pic->next_icw = 0;
      break;
  }
}

void e8259_set_ocw1 (e8259_t *pic, unsigned char val)
{
  pic->ocw[0] = val;
  pic->imr = val;
}

void e8259_set_ocw2 (e8259_t *pic, unsigned char val)
{
  pic->ocw[1] = val;

  switch (val & 0xf9) {
    case 0x20: /* non-specific EOI */
      pic->isr &= (pic->isr - 1);
      break;

    case 0x60: /* specific EOI */
      pic->isr &= ~(1 << (val & 7));
      break;
  }
}

void e8259_set_ocw3 (e8259_t *pic, unsigned char val)
{
  pic->ocw[2] = val;

  if ((val & 0x03) == 0x02) {
    pic->read_irr = 1;
  }
  else if ((val & 0x03) == 0x03) {
    pic->read_irr = 0;
  }
}

void e8259_set_irq0 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x01 & ~pic->imr;
  }
}

void e8259_set_irq1 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x02 & ~pic->imr;
  }
}

void e8259_set_irq2 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x04 & ~pic->imr;
  }
}

void e8259_set_irq3 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x08 & ~pic->imr;
  }
}

void e8259_set_irq4 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x10 & ~pic->imr;
  }
}

void e8259_set_irq5 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x20 & ~pic->imr;
  }
}

void e8259_set_irq6 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x40 & ~pic->imr;
  }
}

void e8259_set_irq7 (e8259_t *pic, unsigned char val)
{
  if (val) {
    pic->irr |= 0x80 & ~pic->imr;
  }
}

unsigned char e8259_inta (e8259_t *pic)
{
  unsigned char irr, irq;

  irr = pic->irr ^ (pic->irr & (pic->irr - 1));
  if (irr == 0) {
    return (0);
  }

  pic->irr &= ~irr;

  if ((pic->icw[3] & 0x02) == 0) {
    /* no automatic EOI */
    pic->isr |= irr;
  }

  irq = 0;
  while ((irr & 1) == 0) {
    irr = irr >> 1;
    irq += 1;
  }

  pic->irq_cnt[irq] += 1;

  return (pic->base + irq);
}

unsigned char e8259_get_isr (e8259_t *pic)
{
  return (pic->isr);
}

unsigned char e8259_get_irr (e8259_t *pic)
{
  return (pic->irr);
}

void e8259_set_uint8 (e8259_t *pic, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0:
      if ((val & 0xf4) == 0x10) {
        e8259_set_icw1 (pic, val);
      }
      else if ((val & 0x18) == 0) {
        e8259_set_ocw2 (pic, val);
      }
      else if ((val & 0x98) == 0x08) {
        e8259_set_ocw3 (pic, val);
      }
      break;

    case 1:
      if (pic->next_icw != 0) {
        e8259_set_icwn (pic, val);
      }
      else {
        e8259_set_ocw1 (pic, val);
      }
      break;
  }
}

void e8259_set_uint16 (e8259_t *pic, unsigned long addr, unsigned short val)
{
  e8259_set_uint8 (pic, addr, val & 0xff);
}

void e8259_set_uint32 (e8259_t *pic, unsigned long addr, unsigned long val)
{
  e8259_set_uint8 (pic, addr, val & 0xff);
}

unsigned char e8259_get_uint8 (e8259_t *pic, unsigned long addr)
{
  switch (addr) {
    case 0x00:
      return (pic->read_irr ? pic->irr : pic->isr);

    case 0x01:
      return (pic->imr);
  }

  return (0xff);
}

unsigned short e8259_get_uint16 (e8259_t *pic, unsigned long addr)
{
  return (e8259_get_uint8 (pic, addr));
}

unsigned long e8259_get_uint32 (e8259_t *pic, unsigned long addr)
{
  return (e8259_get_uint8 (pic, addr));
}

void e8259_clock (e8259_t *pic)
{
  unsigned char irr, isr;

  if (pic->irr == 0) {
    return;
  }

  irr = pic->irr ^ (pic->irr & (pic->irr - 1));
  isr = pic->isr ^ (pic->isr & (pic->isr - 1));

  if ((irr != 0) && ((isr == 0) || (irr < isr))) {
    if (pic->irq != NULL) {
      pic->irq (pic->irq_ext, 1);
    }
  }
}
