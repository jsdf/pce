/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/mda.c                                            *
 * Created:       2003-04-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: mda.c,v 1.7 2003/08/30 03:08:53 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


static
unsigned char coltab[16] = {
 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};


video_t *mda_new (terminal_t *trm, ini_sct_t *sct)
{
  unsigned      i;
  unsigned      iobase, membase, memsize;
  unsigned char r, g, b;
  mda_t         *mda;

  mda = (mda_t *) malloc (sizeof (mda_t));
  if (mda == NULL) {
    return (NULL);
  }

  pce_video_init (&mda->vid);

  mda->vid.ext = mda;
  mda->vid.del = (pce_video_del_f) &mda_del;
  mda->vid.get_mem = (pce_video_get_mem_f) &mda_get_mem;
  mda->vid.get_reg = (pce_video_get_reg_f) &mda_get_reg;
  mda->vid.prt_state = (pce_video_prt_state_f) &mda_prt_state;

  for (i = 0; i < 16; i++) {
    mda->crtc_reg[i] = 0;
  }

  ini_get_ulng (sct, "color7", &mda->rgb_fg, 0xe89050);
  ini_get_ulng (sct, "color15", &mda->rgb_hi, 0xfff0c8);

  ini_get_uint (sct, "io", &iobase, 0x3b4);
  ini_get_uint (sct, "membase", &membase, 0xb0000);
  ini_get_uint (sct, "memsize", &memsize, 4096);

  memsize = (memsize < 4096) ? 4096 : memsize;

  pce_log (MSG_INF, "video:\tMDA io=0x%04x membase=0x%05x memsize=0x%05x\n",
    iobase, membase, memsize
  );

  mda->crtc_pos = 0;

  mda->mem = mem_blk_new (membase, memsize, 1);
  mda->mem->ext = mda;
  mda->mem->set_uint8 = (seta_uint8_f) &mda_mem_set_uint8;
  mda->mem->set_uint16 = (seta_uint16_f) &mda_mem_set_uint16;
  mem_blk_init (mda->mem, 0x00);

  mda->reg = mem_blk_new (iobase, 16, 1);
  mda->reg->ext = mda;
  mda->reg->set_uint8 = (seta_uint8_f) &mda_reg_set_uint8;
  mda->reg->set_uint16 = (seta_uint16_f) &mda_reg_set_uint16;
  mda->reg->get_uint8 = (geta_uint8_f) &mda_reg_get_uint8;
  mda->reg->get_uint16 = (geta_uint16_f) &mda_reg_get_uint16;
  mem_blk_init (mda->reg, 0x00);

  mda->trm = trm;

  r = (mda->rgb_fg >> 16) & 0xff;
  g = (mda->rgb_fg >> 8) & 0xff;
  b = mda->rgb_fg & 0xff;
  trm_set_map (trm, 7, r | (r << 8), g | (g << 8), b | (b << 8));

  r = (mda->rgb_hi >> 16) & 0xff;
  g = (mda->rgb_hi >> 8) & 0xff;
  b = mda->rgb_hi & 0xff;
  trm_set_map (trm, 15, r | (r << 8), g | (g << 8), b | (b << 8));

  trm_set_size (trm, TERM_MODE_TEXT, 80, 25);

  return (&mda->vid);
}

void mda_del (mda_t *mda)
{
  if (mda != NULL) {
    mem_blk_del (mda->mem);
    mem_blk_del (mda->reg);
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

  fprintf (fp, "MDA: 3B4=%02X  3B5=%02X  3B8=%02X  3BA=%02X  POS=%04X[%u/%u]\n",
    mda->reg->data[0], mda->reg->data[1], mda->reg->data[4], mda->reg->data[6],
    mda->crtc_pos, x, y
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

mem_blk_t *mda_get_mem (mda_t *mda)
{
  return (mda->mem);
}

mem_blk_t *mda_get_reg (mda_t *mda)
{
  return (mda->reg);
}

static
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

static
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

static
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

static
unsigned char mda_crtc_get_reg (mda_t *mda, unsigned reg)
{
  if (reg > 15) {
    return (0xff);
  }

  return (mda->crtc_reg[reg]);
}

void mda_reg_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
  mda->reg->data[addr] = val;

  switch (addr) {
    case 0x01:
      mda_crtc_set_reg (mda, mda->reg->data[0], val);
      break;
  }
}

void mda_reg_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
  mda_reg_set_uint8 (mda, addr, val & 0xff);

  if (addr < mda->reg->end) {
    mda_reg_set_uint8 (mda, addr + 1, val >> 8);
  }
}

unsigned char mda_reg_get_uint8 (mda_t *mda, unsigned long addr)
{
  static unsigned cnt = 0;

  switch (addr) {
    case 0x00:
      return (mda->reg->data[0]);

    case 0x01:
      return (mda_crtc_get_reg (mda, mda->reg->data[0]));

    case 0x06:
      cnt += 1;
      if ((cnt & 7) == 0) {
        cnt = 0;
        mda->reg->data[6] ^= 0x01;
      }
      mda->reg->data[6] ^= (cnt & 1) ? 0x08 : 0x00;
      return (mda->reg->data[6]);

    default:
      return (0xff);
  }
}

unsigned short mda_reg_get_uint16 (mda_t *mda, unsigned long addr)
{
  unsigned short ret;

  ret = mda_reg_get_uint8 (mda, addr);

  if (addr < mda->reg->end) {
    ret |= mda_reg_get_uint8 (mda, addr + 1) << 8;
  }

  return (ret);
}
