/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     mda.c                                                      *
 * Created:       2003-04-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: mda.c,v 1.3 2003/04/18 20:05:50 hampa Exp $ */


#include <stdio.h>

#include <pce.h>


mda_t *mda_new (void)
{
  unsigned i;
  mda_t    *mda;

  mda = (mda_t *) malloc (sizeof (mda_t));
  if (mda == NULL) {
    return (NULL);
  }

  for (i = 0; i < 16; i++) {
    mda->crtc_reg[i] = 0;
  }

  mda->cur_pos = 0;

  mda->mem = mem_blk_new (0xb0000, 16384, 1);
  mda->mem->ext = mda;
  mda->mem->set_uint8 = (seta_uint8_f) &mda_mem_set_uint8;
  mda->mem->set_uint16 = (seta_uint16_f) &mda_mem_set_uint16;

  mda->crtc = mem_blk_new (0x3b4, 16, 1);
  mda->crtc->ext = mda;
  mda->crtc->set_uint8 = (seta_uint8_f) &mda_crtc_set_uint8;
  mda->crtc->set_uint16 = (seta_uint16_f) &mda_crtc_set_uint16;
  mda->crtc->get_uint8 = (geta_uint8_f) &mda_crtc_get_uint8;

  mda->scn_x = ~0;
  mda->scn_y = ~0;
  mda->scn_a = ~0;

  return (mda);
}

void mda_del (mda_t *mda)
{
  if (mda != NULL) {
    mem_blk_del (mda->mem);
    mem_blk_del (mda->crtc);
    free (mda);
  }
}

void mda_clock (mda_t *mda)
{
}

void mda_set_pos (mda_t *mda, unsigned pos)
{
  unsigned x, y;

  if (mda->cur_pos == pos) {
    return;
  }

  mda->cur_pos = pos;

  x = pos % 80;
  y = pos / 80;

  fprintf (mda->fp, "\x1b[%u;%uH", y + 1, x + 1);

  fflush (mda->fp);
}

void mda_print (mda_t *mda, unsigned x, unsigned y, unsigned char c, unsigned char a)
{
  unsigned it, fg, bg;

  if ((x != mda->scn_x) || (y != mda->scn_y)) {
    fprintf (mda->fp, "\x1b[%u;%uH", y + 1, x + 1);
  }

  if (a != mda->scn_a) {
    it = (a & 0x08) ? 1 : 0;
    fg = (a & 0x07) ? 7 : 0;
    bg = (a & 0x70) ? 7 : 0;

    fprintf (mda->fp, "\x1b[%u;%u;%um", it, 30 + fg, 40 + bg);
  }

  if ((c >= 32) && (c < 128)) {
    fputc (c, mda->fp);
  }
  else if (c == 0) {
    fputc (' ', mda->fp);
  }
  else {
    fputc ('.', mda->fp);
  }

  mda->scn_x = x + 1;
  mda->scn_y = y;
  mda->scn_a = a;

  fflush (mda->fp);
}

void mda_mem_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
  unsigned      x, y;
  unsigned char c, a;

  if (mda->mem->data[addr] == val) {
    return;
  }

  mda->mem->data[addr] = val;

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

  if (addr & 1) {
    c = mda->mem->data[addr - 1];
    a = val;
  }
  else {
    c = val;
    a = mda->mem->data[addr + 1];
  }

  mda_print (mda, x, y, c, a);
}

void mda_mem_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
  mda_mem_set_uint8 (mda, addr, val & 0xff);

  if (addr < mda->mem->end) {
    mda_mem_set_uint8 (mda, addr + 1, val >> 8);
  }
}

void mda_crtc_set_reg (mda_t *mda, unsigned reg, unsigned char val)
{
  if (reg > 15) {
    return;
  }

  mda->crtc_reg[reg] = val;

  switch (reg) {
    case 0x0e:
      mda_set_pos (mda, (val << 8) | (mda->crtc_reg[0x0f] & 0xff));
      break;

    case 0x0f:
      mda_set_pos (mda, (mda->crtc_reg[0x0e] << 8) | val);
      break;
  }
}

void mda_crtc_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0x00:
      mda->crtc->data[addr] = val;
      break;

    case 0x01:
      mda_crtc_set_reg (mda, mda->crtc->data[0], val);
      break;

    default:
      mda->crtc->data[addr] = val;
      break;
  }
}

void mda_crtc_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
  mda_mem_set_uint8 (mda, addr, val & 0xff);

  if (addr < mda->crtc->end) {
    mda_crtc_set_uint8 (mda, addr + 1, val >> 8);
  }
}

unsigned char mda_crtc_get_uint8 (mda_t *mda, unsigned long addr)
{
  static unsigned cnt = 0;

  switch (addr) {
    case 0x06:
      /* this is a quick hack for programs that wait for the horizontal sync */
      cnt += 1;
      if (cnt > 16) {
        cnt = 0;
        return 0x01;
      }
      return 0x00;

    default:
      return (mda->crtc->data[addr]);
  }
}
