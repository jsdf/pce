/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/mda.c                                            *
 * Created:       2003-04-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-19 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: mda.c,v 1.4 2003/08/19 17:07:15 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


static
unsigned char coltab[16] = {
 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};


mda_t *mda_new (terminal_t *trm)
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

  mda->crtc_mode = 0;
  mda->crtc_pos = 0;

  mda->mem = mem_blk_new (0xb0000, 4096, 1);
  mda->mem->ext = mda;
  mda->mem->set_uint8 = (seta_uint8_f) &mda_mem_set_uint8;
  mda->mem->set_uint16 = (seta_uint16_f) &mda_mem_set_uint16;

  mda->crtc = mem_blk_new (0x3b4, 16, 1);
  mda->crtc->ext = mda;
  mda->crtc->set_uint8 = (seta_uint8_f) &mda_crtc_set_uint8;
  mda->crtc->set_uint16 = (seta_uint16_f) &mda_crtc_set_uint16;
  mda->crtc->get_uint8 = (geta_uint8_f) &mda_crtc_get_uint8;
  mda->crtc->get_uint16 = (geta_uint16_f) &mda_crtc_get_uint16;

  mda->trm = trm;

  trm_set_map (trm, 7, 200 * 256, 104 * 256, 16 * 256);

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

void mda_prt_state (mda_t *mda, FILE *fp)
{
  unsigned i;
  unsigned x, y;

  x = mda->crtc_pos % 80;
  y = mda->crtc_pos / 80;

  fprintf (fp, "MDA: MODE=%02X  POS=%04X[%u/%u]\n",
    mda->crtc_mode, mda->crtc_pos, x, y
  );

  fprintf (fp, "CRTC=[%02X", mda->crtc_reg[0]);
  for (i = 1; i < 15; i++) {
    if (i == 8) {
      fputs ("-", fp);
    }
    else {
      fputs (" ", fp);
    }
    fprintf (fp, "%02X", mda->crtc_reg[i]);
  }
  fputs ("]\n", fp);

  fflush (fp);
}

void mda_set_pos (mda_t *mda, unsigned pos)
{
  unsigned x, y;

  if (mda->crtc_pos == pos) {
    return;
  }

  mda->crtc_pos = pos;

  x = pos % 80;
  y = pos / 80;

  trm_set_pos (mda->trm, pos % 80, pos / 80);
}

void mda_set_crs (mda_t *mda, unsigned y1, unsigned y2)
{
  if (y1 < y2) {
    y1 = 0;
    y2 = 13;
  }

  y1 = (y1 <= 13) ? (13 - y1) : 0;
  y2 = (y2 <= 13) ? (13 - y2) : 0;

  y1 = (255 * y1 + 6) / 13;
  y2 = (255 * y2 + 6) / 13;

  trm_set_crs (mda->trm, y1, y2);
}

void mda_mem_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
  unsigned      x, y;
  unsigned char c, a;
  unsigned      fg, bg;

  if (mda->mem->data[addr] == val) {
    return;
  }

  mda->mem->data[addr] = val;

  if (addr & 1) {
    c = mda->mem->data[addr - 1];
    a = val;
  }
  else {
    c = val;
    a = mda->mem->data[addr + 1];
  }

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

  fg = coltab[a & 0x0f];
  bg = coltab[(a & 0xf0) >> 4];

  trm_set_col (mda->trm, fg, bg);
  trm_set_chr (mda->trm, x, y, c);
}

void mda_mem_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
  unsigned      x, y;
  unsigned char c, a;
  unsigned      fg, bg;

  if (addr & 1) {
    mda_mem_set_uint8 (mda, addr, val & 0xff);

    if (addr < mda->mem->end) {
      mda_mem_set_uint8 (mda, addr + 1, val >> 8);
    }

    return;
  }

  c = val & 0xff;
  a = (val >> 8) & 0xff;

  if ((mda->mem->data[addr] == c) && (mda->mem->data[addr] == a)) {
    return;
  }

  mda->mem->data[addr] = c;
  mda->mem->data[addr + 1] = a;

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

  fg = coltab[a & 0x0f];
  bg = coltab[(a & 0xf0) >> 4];

  trm_set_col (mda->trm, fg, bg);
  trm_set_chr (mda->trm, x, y, c);
}

void mda_crtc_set_reg (mda_t *mda, unsigned reg, unsigned char val)
{
  if (reg > 15) {
    return;
  }

  mda->crtc_reg[reg] = val;

  switch (reg) {
    case 0x0a:
    case 0x0b:
      mda_set_crs (mda, mda->crtc_reg[0x0b], mda->crtc_reg[0x0a]);
      break;

    case 0x0e:
      mda_set_pos (mda, (val << 8) | (mda->crtc_reg[0x0f] & 0xff));
      break;

    case 0x0f:
      mda_set_pos (mda, (mda->crtc_reg[0x0e] << 8) | val);
      break;
  }
}

unsigned char mda_crtc_get_reg (mda_t *mda, unsigned reg)
{
  if (reg > 15) {
    return (0xff);
  }

  return (mda->crtc_reg[reg]);
}

void mda_crtc_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
  mda->crtc->data[addr] = val;

  switch (addr) {
    case 0x01:
      mda_crtc_set_reg (mda, mda->crtc->data[0], val);
      break;

    case 0x04:
      mda->crtc_mode = val;
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
    case 0x00:
      return (mda->crtc->data[0]);

    case 0x01:
      return (mda_crtc_get_reg (mda, mda->crtc->data[0]));

    case 0x04:
      return (mda->crtc->data[addr]);
      break;

    case 0x06:
      /* this is a quick hack for programs that wait for the horizontal sync */
      cnt += 1;
      if (cnt > 16) {
        cnt = 0;
        return 0x01;
      }
      return (0x00);

    default:
      return (0xff);
  }
}

unsigned short mda_crtc_get_uint16 (mda_t *mda, unsigned long addr)
{
  unsigned short ret;

  ret = mda_crtc_get_uint8 (mda, addr);

  if (addr < mda->crtc->end) {
    ret |= mda_crtc_get_uint8 (mda, addr + 1) << 8;
  }

  return (ret);
}
