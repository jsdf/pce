/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/ega.c                                            *
 * Created:       2003-09-06 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-15 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ega.c,v 1.3 2003/09/15 07:53:49 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


static
unsigned char ega_rgbtab[4] = { 0x00, 0x80, 0xc0, 0xff };


video_t *ega_new (terminal_t *trm, ini_sct_t *sct)
{
  ega_t *ega;

  ega = (ega_t *) malloc (sizeof (ega_t));
  if (ega == NULL) {
    return (NULL);
  }

  ega->data = (unsigned char *) malloc (256UL * 1024);
  if (ega->data == NULL) {
    free (ega);
    return (NULL);
  }

  pce_video_init (&ega->vid);

  ega->vid.type = PCE_VIDEO_EGA;
  ega->vid.ext = ega;
  ega->vid.del = (pce_video_del_f) &ega_del;
  ega->vid.get_mem = (pce_video_get_mem_f) &ega_get_mem;
  ega->vid.get_reg = (pce_video_get_reg_f) &ega_get_reg;
  ega->vid.prt_state = (pce_video_prt_state_f) &ega_prt_state;
  ega->vid.screenshot = (pce_video_screenshot_f) &ega_screenshot;

  pce_smap_init (&ega->smap, 320, 200, 320, 200);

  memset (ega->crtc_reg, 0xff, 24 * sizeof (unsigned char));
  memset (ega->ts_reg, 0, 5 * sizeof (unsigned char));
  memset (ega->gdc_reg, 0, 9 * sizeof (unsigned char));
  memset (ega->atc_reg, 0, 21 * sizeof (unsigned char));

  ini_get_uint (sct, "mode_320x200_w", &ega->mode_320x200_w, 640);
  ini_get_uint (sct, "mode_320x200_h", &ega->mode_320x200_h, 400);
  ini_get_uint (sct, "mode_640x200_w", &ega->mode_640x200_w, 640);
  ini_get_uint (sct, "mode_640x200_h", &ega->mode_640x200_h, 400);
  ini_get_uint (sct, "mode_640x350_w", &ega->mode_640x350_w, 640);
  ini_get_uint (sct, "mode_640x350_h", &ega->mode_640x350_h, 480);

  pce_log (MSG_INF, "video:\tEGA io=0x03b0 membase=0xa000 memsize=262144\n");

  ega->mem = mem_blk_new (0xa0000, 128UL * 1024, 0);
  ega->mem->ext = ega;
  ega->mem->set_uint8 = (seta_uint8_f) &ega_mem_set_uint8;
  ega->mem->get_uint8 = (geta_uint8_f) &ega_mem_get_uint8;
  ega->mem->set_uint16 = (seta_uint16_f) &ega_mem_set_uint16;
  ega->mem->get_uint16 = (geta_uint16_f) &ega_mem_get_uint16;

  ega->reg = mem_blk_new (0x3b0, 64, 1);
  ega->reg->ext = ega;
  ega->reg->set_uint8 = (seta_uint8_f) &ega_reg_set_uint8;
  ega->reg->set_uint16 = (seta_uint16_f) &ega_reg_set_uint16;
  ega->reg->get_uint8 = (geta_uint8_f) &ega_reg_get_uint8;
  ega->reg->get_uint16 = (geta_uint16_f) &ega_reg_get_uint16;
  mem_blk_init (ega->reg, 0x00);

  ega->trm = trm;

  ega->crtc_pos = 0;
  ega->crtc_ofs = 0;

  ega->crs_on = 1;

  ega->mode = 0;

  trm_set_size (trm, TERM_MODE_TEXT, 80, 25);

  return (&ega->vid);
}

void ega_del (ega_t *ega)
{
  if (ega != NULL) {
    mem_blk_del (ega->mem);
    mem_blk_del (ega->reg);
    pce_smap_free (&ega->smap);
    free (ega->data);
    free (ega);
  }
}

void ega_clock (ega_t *ega)
{
}

void ega_prt_state (ega_t *ega, FILE *fp)
{
  unsigned i;
  unsigned x, y;

  if (ega->crtc_pos < ega->crtc_ofs) {
    x = 0;
    y = 0;
  }
  else {
    x = (ega->crtc_pos - ega->crtc_ofs) % 80;
    y = (ega->crtc_pos - ega->crtc_ofs) / 80;
  }

  fprintf (fp, "EGA: MODE=%u  OFS=%04X  POS=%04X[%u/%u]  CRS=%s  LATCH=[%02x %02x %02x %02x]\n",
    ega->mode, ega->crtc_ofs, ega->crtc_pos, x, y,
    (ega->crtc_reg[0x0b] < ega->crtc_reg[0x0a]) ? "OFF" : "ON",
    ega->latch[0], ega->latch[1], ega->latch[2], ega->latch[3]
  );

  fprintf (fp, "REGS: 3CC=%02x  3DA=%02x\n",
    ega->reg->data[0x1c], ega->reg->data[0x2a]
  );

  fprintf (fp, "CRTC: [%02X", ega->crtc_reg[0]);
  for (i = 1; i < 24; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", ega->crtc_reg[i]);
  }
  fputs ("]\n", fp);

  fprintf (fp, "TS:   [%02X", ega->ts_reg[0]);
  for (i = 1; i < 5; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", ega->ts_reg[i]);
  }
  fputs ("]\n", fp);

  fprintf (fp, "GDC:  [%02X", ega->gdc_reg[0]);
  for (i = 1; i < 9; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", ega->gdc_reg[i]);
  }
  fputs ("]\n", fp);

  fprintf (fp, "ATC:  [%02X", ega->atc_reg[0]);
  for (i = 1; i < 21; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", ega->atc_reg[i]);
  }
  fputs ("]\n", fp);

  fflush (fp);
}

mem_blk_t *ega_get_mem (ega_t *ega)
{
  return (ega->mem);
}

mem_blk_t *ega_get_reg (ega_t *ega)
{
  return (ega->reg);
}


void ega_get_rgb (ega_t *ega, unsigned idx, unsigned char rgb[3])
{
  idx = ega->atc_reg[idx];

  rgb[0] = ((idx & 0x04) ? 0x02 : 0x00) | ((idx & 0x20) ? 0x01 : 0x00);
  rgb[0] = ega_rgbtab[rgb[0]];

  rgb[1] = ((idx & 0x02) ? 0x02 : 0x00) | ((idx & 0x10) ? 0x01 : 0x00);
  rgb[1] = ega_rgbtab[rgb[1]];

  rgb[2] = ((idx & 0x01) ? 0x02 : 0x00) | ((idx & 0x08) ? 0x01 : 0x00);
  rgb[2] = ega_rgbtab[rgb[2]];
}


/*****************************************************************************
 * mode 0 (text 80 * 25)
 *****************************************************************************/

static
int ega_mode0_screenshot (ega_t *ega, FILE *fp)
{
  unsigned i;
  unsigned x, y;

  i = (ega->crtc_ofs << 1) & 0xffff;

  for (y = 0; y < 25; y++) {
    for (x = 0; x < 80; x++) {
      fputc (ega->data[i], fp);
      i = (i + 2) & 0xffff;
    }

    fputs ("\n", fp);
  }

  return (0);
}

static
void ega_mode0_update (ega_t *ega)
{
  unsigned i;
  unsigned x, y;
  unsigned fg, bg;

  i = (ega->crtc_ofs << 1) & 0xffff;

  for (y = 0; y < 25; y++) {
    for (x = 0; x < 80; x++) {
      fg = ega->data[i + 65537] & 0x0f;
      bg = (ega->data[i + 65537] & 0xf0) >> 4;

      trm_set_col (ega->trm, fg, bg);
      trm_set_chr (ega->trm, x, y, ega->data[i]);

      i = (i + 2) & 0xffff;
    }
  }
}

void ega_mode0_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
  unsigned long i;
  unsigned      x, y;
  unsigned char c, a;

  if (addr < 0x18000) {
    return;
  }

  addr = (addr - 0x18000) & 0x7fff;

  i = (addr & 0xffff) + ((addr & 1) ? 65536 : 0);

  if (ega->data[i] == val) {
    return;
  }

  ega->data[i] = val;

  if (addr < (ega->crtc_ofs << 1)) {
    return;
  }

  if (addr & 1) {
    c = ega->data[addr - 1];
    a = val;
  }
  else {
    c = val;
    a = ega->data[addr + 65537];
  }

  addr -= (ega->crtc_ofs << 1);

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

//  pce_log (MSG_DEB, "ega: set char at (%u,%u) c=%02x a=%02x\n", x, y, c, a);

  trm_set_col (ega->trm, a & 0x0f, (a & 0xf0) >> 4);
  trm_set_chr (ega->trm, x, y, c);
}

void ega_mode0_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
  unsigned      x, y;
  unsigned char c, a;

  if (addr < 0x18000) {
    return;
  }

  if (addr & 1) {
    ega_mem_set_uint8 (ega, addr, val & 0xff);
    ega_mem_set_uint8 (ega, addr + 1, val >> 8);
    return;
  }

  addr = (addr - 0x18000) & 0x3fff;

  c = val & 0xff;
  a = (val >> 8) & 0xff;

  if ((ega->data[addr] == c) && (ega->data[addr + 65537] == a)) {
    return;
  }

  ega->data[addr] = c;
  ega->data[addr + 65537] = a;

  if (addr < (ega->crtc_ofs << 1)) {
    return;
  }

  addr -= (ega->crtc_ofs << 1);

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

  trm_set_col (ega->trm, a & 0x0f, (a & 0xf0) >> 4);
  trm_set_chr (ega->trm, x, y, c);
}

unsigned char ega_mode0_get_uint8 (ega_t *ega, unsigned long addr)
{
  if (addr < 0x18000) {
    return (0xff);
  }

  addr = (addr - 0x18000) & 0x7fff;

  if (addr & 1) {
    return (ega->data[addr + 65536]);
  }

  return (ega->data[addr]);
}

unsigned short ega_mode0_get_uint16 (ega_t *ega, unsigned long addr)
{
  unsigned short val;

  val = ega_mode0_get_uint8 (ega, addr);
  val |= ega_mode0_get_uint8 (ega, addr + 1) << 8;

  return (val);
}


/*****************************************************************************
 * mode 16 (320 * 200 * 16)
 *****************************************************************************/

static
int ega_mode16_screenshot (ega_t *ega, FILE *fp)
{
  unsigned      i, x, y, w;
  unsigned      idx;
  unsigned char rgb[3];
  unsigned long addr, rofs;
  unsigned char msk;

  fprintf (fp, "P6\n%u %u\n255 ", ega->mode_w, ega->mode_h);

  addr = ega->crtc_ofs;
  rofs = 2 * ega->crtc_reg[0x13];

  w = ega->mode_w / 8;

  for (y = 0; y < ega->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;

      for (i = 0; i < 8; i++) {
        idx = (ega->data[addr + x + 0 * 65536] & msk) ? 0x01 : 0x00;
        idx |= (ega->data[addr + x + 1 * 65536] & msk) ? 0x02 : 0x00;
        idx |= (ega->data[addr + x + 2 * 65536] & msk) ? 0x04 : 0x00;
        idx |= (ega->data[addr + x + 3 * 65536] & msk) ? 0x08 : 0x00;

        ega_get_rgb (ega, idx, rgb);
        fwrite (rgb, 1, 3, fp);

        msk = msk >> 1;
      }
    }

    addr += rofs;
  }

  return (0);
}

void ega_mode16_set_latches (ega_t *ega, unsigned long addr, unsigned char latch[4])
{
  unsigned      i;
  unsigned      sx, sy, sw, sh;
  unsigned      x, y, c;
  unsigned      rofs;
  unsigned char msk;

//  fprintf (stderr, "ega: latches -> %lu\n", addr);

  addr &= 0xffff;

  msk = 0;

  if (ega->ts_reg[2] & 0x01) {
    msk |= ega->data[addr + 0 * 65536] ^ latch[0];
    ega->data[addr + 0 * 65536] = latch[0];
  }

  if (ega->ts_reg[2] & 0x02) {
    msk |= ega->data[addr + 1 * 65536] ^ latch[1];
    ega->data[addr + 1 * 65536] = latch[1];
  }

  if (ega->ts_reg[2] & 0x04) {
    msk |= ega->data[addr + 2 * 65536] ^ latch[2];
    ega->data[addr + 2 * 65536] = latch[2];
  }

  if (ega->ts_reg[2] & 0x08) {
    msk |= ega->data[addr + 3 * 65536] ^ latch[3];
    ega->data[addr + 3 * 65536] = latch[3];
  }

  /* mask does not work if palette changed */
  msk = 255;

  if (msk == 0) {
    return;
  }

  if (addr < ega->crtc_ofs) {
    return;
  }

  rofs = 2 * ega->crtc_reg[0x13];

  if ((8 * rofs) < ega->mode_w) {
    rofs = ega->mode_w / 8;
  }

  y = (addr - ega->crtc_ofs) / rofs;
  x = 8 * ((addr - ega->crtc_ofs) % rofs);

  if ((x >= ega->mode_w) || (y >= ega->mode_h)) {
    return;
  }

  trm_set_upd (ega->trm, 1);

  for (i = 0; i < 8; i++) {
    if (msk & (0x80 >> i)) {
      c = (ega->data[addr + 0 * 65536] & (0x80 >> i)) ? 0x01 : 0x00;
      c |= (ega->data[addr + 1 * 65536] & (0x80 >> i)) ? 0x02 : 0x00;
      c |= (ega->data[addr + 2 * 65536] & (0x80 >> i)) ? 0x04 : 0x00;
      c |= (ega->data[addr + 3 * 65536] & (0x80 >> i)) ? 0x08 : 0x00;

      trm_set_col (ega->trm, c, 0);
      pce_smap_get_pixel (&ega->smap, x + i, y, &sx, &sy, &sw, &sh);
      trm_set_pxl (ega->trm, sx, sy, sw, sh);
    }
  }

  trm_set_upd (ega->trm, 1);
}

void ega_mode16_update (ega_t *ega)
{
  unsigned      i, x, y, w;
  unsigned      sx, sy, sw, sh;
  unsigned      c;
  unsigned      rofs;
  unsigned long addr;
  unsigned char msk;

  addr = ega->crtc_ofs & 0xffff;
  rofs = 2 * ega->crtc_reg[0x13];

  w = ega->mode_w / 8;

  trm_set_upd (ega->trm, 1);

  for (y = 0; y < ega->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;
      for (i = 0; i < 8; i++) {
        c = (ega->data[addr + x + 0 * 65536] & msk) ? 0x01 : 0x00;
        c |= (ega->data[addr + x + 1 * 65536] & msk) ? 0x02 : 0x00;
        c |= (ega->data[addr + x + 2 * 65536] & msk) ? 0x04 : 0x00;
        c |= (ega->data[addr + x + 3 * 65536] & msk) ? 0x08 : 0x00;

        trm_set_col (ega->trm, c, 0);
        pce_smap_get_pixel (&ega->smap, 8 * x + i, y, &sx, &sy, &sw, &sh);
        trm_set_pxl (ega->trm, sx, sy, sw, sh);

        msk = msk >> 1;
      }
    }

    addr = (addr + rofs) & 0xffff;
  }

  trm_set_upd (ega->trm, 1);
}

void ega_mode16_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
  unsigned char col[4];

  switch (ega->gdc_reg[5] & 0x03) {
    case 0x00: { /* write mode 0 */
      unsigned char ena, set, msk;

      ena = ega->gdc_reg[0x01];
      set = ega->gdc_reg[0x00];
      msk = ega->gdc_reg[0x08];

      col[0] = (ena & 0x01) ? ((set & 0x01) ? 0xff : 0x00) : val;
      col[1] = (ena & 0x02) ? ((set & 0x02) ? 0xff : 0x00) : val;
      col[2] = (ena & 0x04) ? ((set & 0x04) ? 0xff : 0x00) : val;
      col[3] = (ena & 0x08) ? ((set & 0x08) ? 0xff : 0x00) : val;

      switch (ega->gdc_reg[3] & 0x18) {
        case 0x00: /* copy */
          break;

        case 0x08: /* and */
          col[0] &= ega->latch[0];
          col[1] &= ega->latch[1];
          col[2] &= ega->latch[2];
          col[3] &= ega->latch[3];
          break;

        case 0x10: /* or */
          col[0] |= ega->latch[0];
          col[1] |= ega->latch[1];
          col[2] |= ega->latch[2];
          col[3] |= ega->latch[3];
          break;

        case 0x18: /* xor */
          col[0] ^= ega->latch[0];
          col[1] ^= ega->latch[1];
          col[2] ^= ega->latch[2];
          col[3] ^= ega->latch[3];
          break;
      }

      col[0] = (col[0] & msk) | (ega->latch[0] & ~msk);
      col[1] = (col[1] & msk) | (ega->latch[1] & ~msk);
      col[2] = (col[2] & msk) | (ega->latch[2] & ~msk);
      col[3] = (col[3] & msk) | (ega->latch[3] & ~msk);
    }
    break;

    case 0x01: /* write mode 1 */
      col[0] = ega->latch[0];
      col[1] = ega->latch[1];
      col[2] = ega->latch[2];
      col[3] = ega->latch[3];
      break;

    case 0x02: { /* write mode 2 */
      unsigned char msk;

      msk = ega->gdc_reg[0x08];

      col[0] = (val & 0x01) ? 0xff : 0x00;
      col[1] = (val & 0x02) ? 0xff : 0x00;
      col[2] = (val & 0x04) ? 0xff : 0x00;
      col[3] = (val & 0x08) ? 0xff : 0x00;

      switch (ega->gdc_reg[3] & 0x18) {
        case 0x00: /* copy */
          break;

        case 0x08: /* and */
          col[0] &= ega->latch[0];
          col[1] &= ega->latch[1];
          col[2] &= ega->latch[2];
          col[3] &= ega->latch[3];
          break;

        case 0x10: /* or */
          col[0] |= ega->latch[0];
          col[1] |= ega->latch[1];
          col[2] |= ega->latch[2];
          col[3] |= ega->latch[3];
          break;

        case 0x18: /* xor */
          col[0] ^= ega->latch[0];
          col[1] ^= ega->latch[1];
          col[2] ^= ega->latch[2];
          col[3] ^= ega->latch[3];
          break;
      }

      col[0] = (col[0] & msk) | (ega->latch[0] & ~msk);
      col[1] = (col[1] & msk) | (ega->latch[1] & ~msk);
      col[2] = (col[2] & msk) | (ega->latch[2] & ~msk);
      col[3] = (col[3] & msk) | (ega->latch[3] & ~msk);
    }
    break;

    default:
      col[0] = ega->latch[0];
      col[1] = ega->latch[1];
      col[2] = ega->latch[2];
      col[3] = ega->latch[3];
      break;
  }

  ega_mode16_set_latches (ega, addr, col);
}

unsigned char ega_mode16_get_uint8 (ega_t *ega, unsigned long addr)
{
  addr &= 0xffff;

  ega->latch[0] = ega->data[addr + 0 * 65536];
  ega->latch[1] = ega->data[addr + 1 * 65536];
  ega->latch[2] = ega->data[addr + 2 * 65536];
  ega->latch[3] = ega->data[addr + 3 * 65536];

  switch (ega->gdc_reg[5] & 0x08) {
    case 0x00: /* read mode 0 */
      return (ega->latch[ega->gdc_reg[4] & 0x03]);

    case 0x08: { /* read mode 1 */
      unsigned char ccare, ccmpr, cmp[4], ret;

      ccare = ega->gdc_reg[0x07];
      ccmpr = ega->gdc_reg[0x02];

      cmp[0] = (ccare & 0x01) ? ((ccmpr & 0x01) ? 0xff : 0x00) : ega->latch[0];
      cmp[1] = (ccare & 0x02) ? ((ccmpr & 0x02) ? 0xff : 0x00) : ega->latch[1];
      cmp[2] = (ccare & 0x04) ? ((ccmpr & 0x04) ? 0xff : 0x00) : ega->latch[2];
      cmp[3] = (ccare & 0x08) ? ((ccmpr & 0x08) ? 0xff : 0x00) : ega->latch[3];

      ret = cmp[0] ^ ega->latch[0];
      ret |= cmp[1] ^ ega->latch[1];
      ret |= cmp[2] ^ ega->latch[2];
      ret |= cmp[3] ^ ega->latch[3];

      return (~ret & 0xff);
    }
  }

  return (0xff);
}


int ega_screenshot (ega_t *ega, FILE *fp, unsigned mode)
{
  if ((ega->mode == 0) && ((mode == 1) || (mode == 0))) {
    return (ega_mode0_screenshot (ega, fp));
  }
  else if ((ega->mode == 16) && ((mode == 2) || (mode == 0))) {
    return (ega_mode16_screenshot (ega, fp));
  }

  return (1);
}

void ega_update (ega_t *ega)
{
  switch (ega->mode) {
    case 0:
      ega_mode0_update (ega);
      break;

    case 16:
      ega_mode16_update (ega);
      break;
  }
}

void ega_set_mode (ega_t *ega, unsigned mode, unsigned w, unsigned h)
{
  unsigned sw, sh;

  fprintf (stderr, "ega: set mode %u (%u, %u)\n", mode, w, h);

  switch (mode) {
    case 0:
      trm_set_size (ega->trm, TERM_MODE_TEXT, 80, 25);
      break;

    case 16:
      if ((w == 320) && (h == 200)) {
        sw = ega->mode_320x200_w;
        sh = ega->mode_320x200_h;
      }
      else if ((w == 640) && (h == 200)) {
        sw = ega->mode_640x200_w;
        sh = ega->mode_640x200_h;
      }
      else if ((w == 640) && (h == 350)) {
        sw = ega->mode_640x350_w;
        sh = ega->mode_640x350_h;
      }
      else {
        sw = w;
        sh = h;
      }

      trm_set_size (ega->trm, TERM_MODE_GRAPH, sw, sh);
      pce_smap_free (&ega->smap);
      pce_smap_init (&ega->smap, w, h, sw, sh);
      break;
  }

  ega->mode = mode;
  ega->mode_w = w;
  ega->mode_h = h;
}

void ega_set_pos (ega_t *ega, unsigned pos)
{
  ega->crtc_pos = pos;

  if (ega->mode == 0) {
    if (pos < ega->crtc_ofs) {
      return;
    }

    pos -= ega->crtc_ofs;

    if (pos >= 2000) {
      return;
    }

    trm_set_pos (ega->trm, pos % 80, pos / 80);
  }
}

void ega_set_crs (ega_t *ega, unsigned y1, unsigned y2)
{
  if (ega->mode == 0) {
    if (y1 < y2) {
      y1 = 0;
      y2 = 13;
    }

    y1 = (y1 <= 13) ? (13 - y1) : 0;
    y2 = (y2 <= 13) ? (13 - y2) : 0;

    y1 = (255 * y1 + 6) / 13;
    y2 = (255 * y2 + 6) / 13;

    trm_set_crs (ega->trm, y1, y2);
  }
}

void ega_set_page_ofs (ega_t *ega, unsigned ofs)
{
  if (ega->crtc_ofs == ofs) {
    return;
  }

  ega->crtc_ofs = ofs;

  ega_update (ega);
}

void ega_mem_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
//  pce_log (MSG_DEB, "ega: set mem %04lx = %02x\n", addr, val);

  switch (ega->mode) {
    case 0:
      ega_mode0_set_uint8 (ega, addr, val);
      break;

    case 16:
      ega_mode16_set_uint8 (ega, addr, val);
      break;
  }
}

void ega_mem_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
  switch (ega->mode) {
    case 0:
      ega_mode0_set_uint16 (ega, addr, val);
      break;

    case 16:
      ega_mode16_set_uint8 (ega, addr, val & 0xff);
      ega_mode16_set_uint8 (ega, addr + 1, val >> 8);
      break;
  }
}

unsigned char ega_mem_get_uint8 (ega_t *ega, unsigned long addr)
{
//  pce_log (MSG_DEB, "ega: get mem %04lx\n", addr);

  switch (ega->mode) {
    case 0:
      return (ega_mode0_get_uint8 (ega, addr));

    case 16:
      return (ega_mode16_get_uint8 (ega, addr));
  }

  return (0x00);
}

unsigned short ega_mem_get_uint16 (ega_t *ega, unsigned long addr)
{
  switch (ega->mode) {
    case 0:
      return (ega_mode0_get_uint16 (ega, addr));
      break;

    case 16: {
        unsigned short ret;
        ret = ega_mode16_get_uint8 (ega, addr + 1) << 8;
        ret |= ega_mode16_get_uint8 (ega, addr);
        return (ret);
      }
      break;
  }

  return (0x0000);
}


void ega_gdc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
  switch (reg) {
    case 0x00: /* set / reset */
      ega->gdc_reg[0x00] = val & 0x0f;
      break;

    case 0x01: /* enable set / reset */
      ega->gdc_reg[0x01] = val & 0x0f;
      break;

    case 0x02: /* color compare */
      ega->gdc_reg[0x02] = val & 0x0f;
      break;

    case 0x03: /* function select */
      ega->gdc_reg[0x03] = val & 0x1f;
      break;

    case 0x04: /* read plane select */
      ega->gdc_reg[0x04] = val & 0x0f;
      break;

    case 0x05: /* gdc mode */
      ega->gdc_reg[0x05] = val & 0x3f;
      break;

    case 0x06: /* misc */
      ega->gdc_reg[0x06] = val & 0x0f;
      break;

    case 0x07: /* color don't care */
      ega->gdc_reg[0x07] = val & 0x0f;
      break;

    case 0x08: /* bit mask */
      ega->gdc_reg[0x08] = val & 0xff;
      break;
  }
}

unsigned char ega_gdc_get_reg (ega_t *ega, unsigned reg)
{
  return (0xff);
}

void ega_atc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
  if (reg >= 0x15) {
    return;
  }

  if (reg < 16) {
    if ((ega->reg->data[0x10] & 0x20) == 0) {
      unsigned      r, g, b;
      unsigned char rgb[3];

      if (ega->atc_reg[reg] == val) {
        return;
      }

      ega->atc_reg[reg] = val;

      ega_get_rgb (ega, reg, rgb);

      r = (rgb[0] << 8) | rgb[0];
      g = (rgb[1] << 8) | rgb[1];
      b = (rgb[2] << 8) | rgb[2];

      trm_set_map (ega->trm, reg, r, g, b);

//      ega_update (ega);
    }
    return;
  }

  switch (reg) {
    case 0x10: /* mode control */
      ega->atc_reg[reg] = val;
      break;

    case 0x11: /* overscan color */
      ega->atc_reg[reg] = val;
      break;

    case 0x12: /* color plane enable */
      ega->atc_reg[reg] = val & 0x0f;
      break;

    case 0x13: /* horizontal pixel panning */
      ega->atc_reg[reg] = val & 0x0f;
      break;

    case 0x14: /* color select */
      ega->atc_reg[reg] = val & 0x0f;
      break;

  }
}

unsigned char ega_atc_get_reg (ega_t *ega, unsigned reg)
{
  return (0xff);
}

void ega_ts_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
  if (reg >= 5) {
    return;
  }

  ega->ts_reg[reg] = val;

  switch (reg) {
    case 0x00: /* reset */
      ega->ts_reg[reg] = val & 0x03;

      if (val & 0x02) {
        ; /* sync reset */
      }
      else if (val & 0x01) {
        ; /* async reset */
      }
      break;

    case 0x01: /* TS mode */
      ega->ts_reg[reg] = val;
      break;

    case 0x02: /* write plane mask */
      ega->ts_reg[reg] = val & 0x0f;
      break;

    case 0x03: /* font select */
      ega->ts_reg[reg] = val & 0x0f;
      break;

    case 0x04: /* memory mode */
      ega->ts_reg[reg] = val & 0x0f;
      break;
  }
}

unsigned char ega_ts_get_reg (ega_t *ega, unsigned reg)
{
  if (reg < 5) {
    return (ega->ts_reg[reg]);
  }

  return (0xff);
}

void ega_crtc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
  if (reg > 24) {
    return;
  }

  ega->crtc_reg[reg] = val;

  switch (reg) {
    case 0x0a:
    case 0x0b:
      ega_set_crs (ega, ega->crtc_reg[0x0b], ega->crtc_reg[0x0a]);
      break;

    case 0x0c:
//      ega_set_page_ofs (ega, (ega->crtc_reg[0x0c] << 8) | val);
      break;

    case 0x0d:
      ega_set_page_ofs (ega, (ega->crtc_reg[0x0c] << 8) | val);
      break;

    case 0x0e:
//      ega_set_pos (ega, (val << 8) | (ega->crtc_reg[0x0f] & 0xff));
      break;

    case 0x0f:
      ega_set_pos (ega, (ega->crtc_reg[0x0e] << 8) | val);
      break;
  }
}

unsigned char ega_crtc_get_reg (ega_t *ega, unsigned reg)
{
  if (reg > 24) {
    return (0xff);
  }

  return (ega->crtc_reg[reg]);
}

void ega_reg_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
//  pce_log (MSG_DEB, "ega: set reg %04lx = %02x\n", addr, val);

  switch (addr) {
    case 0x10: /* 0x3c0: ATC index/data */
      if (ega->atc_index) {
        ega->atc_index = 0;
        ega->reg->data[addr] = val;
      }
      else {
        ega->atc_index = 1;
        ega_atc_set_reg (ega, ega->reg->data[0x10], val);
      }
      break;

    case 0x12: /* 0x3c2: misc output register */
      ega->reg->data[0x1c] = val;
      break;

    case 0x14: /* 0x3c4: TS index */
      ega->reg->data[addr] = val;
      break;

    case 0x15: /* 0x3c5: TS data */
      ega_ts_set_reg (ega, ega->reg->data[0x14], val);
      break;

    case 0x1e: /* 0x3ce: GDC index */
      ega->reg->data[addr] = val;
      break;

    case 0x1f: /* 0x3cf: GDC data */
      ega_gdc_set_reg (ega, ega->reg->data[0x1e], val);
      break;

    case 0x20: /* 0x3d0: pce extension */
      switch (val) {
        case 0x03:
          ega_set_mode (ega, 0, 80, 25);
          break;

        case 0x0d:
          ega_set_mode (ega, 16, 320, 200);
          break;

        case 0x0e:
          ega_set_mode (ega, 16, 640, 200);
          break;

        case 0x10:
          ega_set_mode (ega, 16, 640, 350);
          break;
      }
      break;

    case 0x24: /* 0x3d4: CRTC index */
      ega->reg->data[addr] = val;
      break;

    case 0x25: /* 0x3d5: CRTC data */
      ega_crtc_set_reg (ega, ega->reg->data[0x24], val);
      break;
  }
}

void ega_reg_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
  ega_reg_set_uint8 (ega, addr, val & 0xff);
  ega_reg_set_uint8 (ega, addr + 1, val >> 8);
}

unsigned char ega_reg_get_uint8 (ega_t *ega, unsigned long addr)
{
  static unsigned cnt = 0;

//  pce_log (MSG_DEB, "ega: get reg %04lx\n", addr);

  switch (addr) {
    case 0x12: /* 0x3c2: input status register 0 */
      return (0x00);

    case 0x14: /* 0x3c4 */
      return (ega->reg->data[addr]);

    case 0x15: /* 0x3c5 */
      return (ega_ts_get_reg (ega, ega->reg->data[0x14]));

    case 0x1c: /* 0x3cc: misc output register */
      return (ega->reg->data[0x1c]);

    case 0x1e: /* 0x3ce: GDC index */
      return (ega->reg->data[addr]);

    case 0x1f: /* 0x3cf: GDC data */
      return (ega_gdc_get_reg (ega, ega->reg->data[0x1e]));

    case 0x24: /* 0x3d4 */
      return (ega->reg->data[addr]);

    case 0x25:
      return (ega_crtc_get_reg (ega, ega->reg->data[0x24]));

    case 0x2a: /* 0x3da */
      ega->atc_index = 1;

      cnt += 1;
      if ((cnt & 7) == 0) {
        ega->reg->data[0x2a] ^= 1;
      }
      if (cnt >= 64) {
        cnt = 0;
        ega->reg->data[0x2a] ^= 8;
      }
      return (ega->reg->data[0x2a]);

    default:
      return (0xff);
  }
}

unsigned short ega_reg_get_uint16 (ega_t *ega, unsigned long addr)
{
  unsigned short ret;

  ret = ega_reg_get_uint8 (ega, addr);
  ret |= ega_reg_get_uint8 (ega, addr + 1) << 8;

  return (ret);
}
