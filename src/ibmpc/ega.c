/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/ega.c                                            *
 * Created:       2003-09-06 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-10-04 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ega.c,v 1.12 2003/10/03 23:17:21 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


#define CRTC_INDEX   0x0024
#define CRTC_DATA    0x0025
#define CRTC_CSIZ_HI 0x0a
#define CRTC_CSIZ_LO 0x0b
#define CRTC_OFFS_HI 0x0c
#define CRTC_OFFS_LO 0x0d
#define CRTC_CPOS_HI 0x0e
#define CRTC_CPOS_LO 0x0f
#define CRTC_ROFS    0x13

#define TS_INDEX     0x0014
#define TS_DATA      0x0015
#define TS_WRPL      0x02

#define GDC_INDEX    0x002e
#define GDC_DATA     0x002f
#define GDC_SETR     0x00
#define GDC_ENAB     0x01
#define GDC_CCMP     0x02
#define GDC_FSEL     0x03
#define GDC_RDPL     0x04
#define GDC_MODE     0x05
#define GDC_MISC     0x06
#define GDC_CARE     0x07
#define GDC_BMSK     0x08

#define ATC_INDEX    0x0010
#define ATC_DATA     0x0011
#define ATC_OSCN     0x11


static void ega_update (ega_t *ega);

static void ega_update_cga_txt (ega_t *ega);
static int ega_screenshot_cga_txt (ega_t *ega, FILE *fp);
static void ega_set_latches_cga_txt (ega_t *cga, unsigned long addr, unsigned char val[4]);

static void ega_set_uint8_odd_even (ega_t *ega, unsigned long addr, unsigned char val);
static unsigned char ega_get_uint8_odd_even (ega_t *ega, unsigned long addr);


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

  ega->base = 0x18000;
  ega->size = 0x08000;

  ega->update = &ega_update_cga_txt;
  ega->screenshot = &ega_screenshot_cga_txt;
  ega->set_latches = &ega_set_latches_cga_txt;
  ega->set_uint8 = &ega_set_uint8_odd_even;
  ega->get_uint8 = &ega_get_uint8_odd_even;

  pce_video_init (&ega->vid);

  ega->vid.type = PCE_VIDEO_EGA;
  ega->vid.ext = ega;
  ega->vid.del = (pce_video_del_f) &ega_del;
  ega->vid.get_mem = (pce_video_get_mem_f) &ega_get_mem;
  ega->vid.get_reg = (pce_video_get_reg_f) &ega_get_reg;
  ega->vid.prt_state = (pce_video_prt_state_f) &ega_prt_state;
  ega->vid.update = (pce_video_update_f) &ega_update;
  ega->vid.dump = (pce_video_dump_f) &ega_dump;
  ega->vid.screenshot = (pce_video_screenshot_f) &ega_screenshot;
  ega->vid.clock = (pce_video_clock_f) &ega_clock;

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
  ini_get_uint (sct, "mode_640x480_w", &ega->mode_640x480_w, 640);
  ini_get_uint (sct, "mode_640x480_h", &ega->mode_640x480_h, 480);

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

  ega->dirty = 0;

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

  fprintf (fp, "EGA: MODE=%u  OFS=%04X  POS=%04X[%u/%u]  CRS=[%u-%u]  LATCH=[%02x %02x %02x %02x]\n",
    ega->mode, ega->crtc_ofs, ega->crtc_pos, x, y,
    ega->crtc_reg[0x0a], ega->crtc_reg[0x0b],
    ega->latch[0], ega->latch[1], ega->latch[2], ega->latch[3]
  );

  fprintf (fp, "REGS: 3C2=%02x  3CC=%02x  3DA=%02x  BASE=%05lX  SIZE=%05lX\n",
    ega->reg->data[0x12], ega->reg->data[0x1c], ega->reg->data[0x2a],
    0xa0000 + ega->base, ega->size
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

int ega_dump (ega_t *ega, FILE *fp)
{
  fprintf (fp, "# EGA dump\n");

  fprintf (fp, "\n# REGS:\n");
  pce_dump_hex (fp, ega->reg->data, ega->reg->size, ega->reg->base, 16, "# ", 0);

  fprintf (fp, "\n# CRTC:\n");
  pce_dump_hex (fp, ega->crtc_reg, 24, 0, 16, "# ", 0);

  fprintf (fp, "\n# TS:\n");
  pce_dump_hex (fp, ega->ts_reg, 5, 0, 16, "# ", 0);

  fprintf (fp, "\n# GDC:\n");
  pce_dump_hex (fp, ega->gdc_reg, 9, 0, 16, "# ", 0);

  fprintf (fp, "\n# ATC:\n");
  pce_dump_hex (fp, ega->atc_reg, 21, 0, 16, "# ", 0);

  fputs ("\n\n# RAM:\n", fp);
  pce_dump_hex (fp, ega->data, 256 * 1024, 0, 16, "", 1);

  return (0);
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
 * CGA text modes
 *****************************************************************************/

static
int ega_screenshot_cga_txt (ega_t *ega, FILE *fp)
{
  unsigned i;
  unsigned x, y;

  i = (ega->crtc_ofs << 1) & 0xfffe;

  if (ega->reg->data[0x1c] & 0x20) {
    i |= 1;
  }

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
void ega_update_cga_txt (ega_t *ega)
{
  unsigned i, j;
  unsigned x, y;
  unsigned fg, bg;
  unsigned rofs;

  i = (ega->crtc_ofs << 1) & 0xfffe;

  rofs = 4 * ega->crtc_reg[0x13];

  if (ega->reg->data[0x1c] & 0x20) {
    i |= 1;
  }

  for (y = 0; y < 25; y++) {
    j = i;
    for (x = 0; x < 80; x++) {
      fg = ega->data[j + 0x10000];
      bg = (fg >> 4) & 0x0f;
      fg = fg & 0x0f;

      trm_set_col (ega->trm, fg, bg);
      trm_set_chr (ega->trm, x, y, ega->data[j]);

      j = (j + 2) & 0xffff;
    }

    i += rofs;
  }
}

static
void ega_set_latches_cga_txt (ega_t *ega, unsigned long addr, unsigned char val[4])
{
  unsigned char wrpl, rofs;

  addr &= 0xffff;

  wrpl = ega->ts_reg[TS_WRPL];

  if (wrpl & 0x03) {
    unsigned      x, y;
    unsigned char c, a;

    if ((wrpl & 0x01) && (val[0] != ega->data[addr])) {
      ega->data[addr] = val[0];
    }

    if ((wrpl & 0x02) && (val[1] != ega->data[addr + 0x10000])) {
      ega->data[addr + 0x10000] = val[1];
    }

    addr &= ~0x0001;

    if (addr < (ega->crtc_ofs << 1)) {
      return;
    }

    addr -= (ega->crtc_ofs << 1);

    if (addr >= 4000) {
      return;
    }

    rofs = 2 * ega->crtc_reg[0x13];
    if (rofs < ega->mode_w) {
      rofs = ega->mode_w;
    }

    c = ega->data[addr];
    a = ega->data[addr + 0x10000];

    x = (addr >> 1) % rofs;
    y = (addr >> 1) / rofs;

    trm_set_col (ega->trm, a & 0x0f, (a & 0xf0) >> 4);
    trm_set_chr (ega->trm, x, y, c);
  }
}


/*****************************************************************************
 * CGA 4 color graphic mode
 *****************************************************************************/

static
int ega_screenshot_cga4 (ega_t *ega, FILE *fp)
{
  unsigned      i, x, y, w;
  unsigned      idx;
  unsigned char rgb[3];
  unsigned long addr, rofs;
  unsigned char msk;

  fprintf (fp, "P6\n%u %u\n255 ", ega->mode_w, ega->mode_h);

  addr = ega->crtc_ofs;
  rofs = 2 * ega->crtc_reg[0x13];

  w = ega->mode_w / 4;

  for (y = 0; y < ega->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;

      for (i = 0; i < 4; i++) {
        idx = (ega->data[addr + x + 0x00000] & (msk >> 1)) ? 0x01 : 0x00;
        idx |= (ega->data[addr + x + 0x10000] & msk) ? 0x02 : 0x00;
        idx |= (ega->data[addr + x + 0x20000] & msk) ? 0x04 : 0x00;
        idx |= (ega->data[addr + x + 0x30000] & msk) ? 0x08 : 0x00;

        idx &= ega->atc_reg[0x12];

        ega_get_rgb (ega, idx, rgb);
        fwrite (rgb, 1, 3, fp);

        msk = msk >> 2;
      }
    }

    if (addr & 0x2000) {
      addr = (addr + rofs) & 0x1fff;
    }
    else {
      addr |= 0x2000;
    }
  }

  return (0);
}

static
void ega_update_cga4 (ega_t *ega)
{
  unsigned      i, x, y, w;
  unsigned      sx, sy, sw, sh;
  unsigned      col1, col2;
  unsigned      rofs;
  unsigned long addr;
  unsigned char msk;

  addr = ega->crtc_ofs & 0xffff;
  rofs = 2 * ega->crtc_reg[0x13];

  w = ega->mode_w / 4;

  col1 = 0;
  trm_set_col (ega->trm, 0, 0);

  for (y = 0; y < ega->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;
      for (i = 0; i < 4; i++) {
        col2 = (ega->data[addr + x + 0x00000] & (msk >> 1)) ? 0x01 : 0x00;
        col2 |= (ega->data[addr + x + 0x10000] & msk) ? 0x02 : 0x00;
        col2 |= (ega->data[addr + x + 0x20000] & msk) ? 0x04 : 0x00;
        col2 |= (ega->data[addr + x + 0x30000] & msk) ? 0x08 : 0x00;

        col2 &= ega->atc_reg[0x12];

        if (col1 != col2) {
          trm_set_col (ega->trm, col2, 0);
          col1 = col2;
        }

        pce_smap_get_pixel (&ega->smap, 4 * x + i, y, &sx, &sy, &sw, &sh);
        trm_set_pxl (ega->trm, sx, sy, sw, sh);

        msk = msk >> 2;
      }
    }

    if (addr & 0x2000) {
      addr = (addr + rofs) & 0x1fff;
    }
    else {
      addr |= 0x2000;
    }
  }

  ega->dirty = 0;
}

static
void ega_set_latches_cga4 (ega_t *ega, unsigned long addr, unsigned char latch[4])
{
  unsigned      i;
  unsigned      sx, sy, sw, sh;
  unsigned      x, y, c;
  unsigned      rofs;
  unsigned char msk;

  addr &= 0xffff;

  msk = 0;

  if (ega->ts_reg[2] & 0x01) {
    msk |= ega->data[addr + 0x00000] ^ latch[0];
    ega->data[addr + 0x00000] = latch[0];
  }

  if (ega->ts_reg[2] & 0x02) {
    msk |= ega->data[addr + 0x10000] ^ latch[1];
    ega->data[addr + 0x10000] = latch[1];
  }

  if (ega->ts_reg[2] & 0x04) {
    msk |= ega->data[addr + 0x20000] ^ latch[2];
    ega->data[addr + 0x20000] = latch[2];
  }

  if (ega->ts_reg[2] & 0x08) {
    msk |= ega->data[addr + 0x30000] ^ latch[3];
    ega->data[addr + 0x30000] = latch[3];
  }

  if (msk == 0) {
    return;
  }

  msk = (msk & 0xaa) | ((msk << 1) & 0xaa);

  if (addr < ega->crtc_ofs) {
    return;
  }

  if (ega->dirty) {
    ega->update (ega);
    ega->dirty = 0;
    return;
  }

  rofs = 2 * ega->crtc_reg[0x13];

  if ((4 * rofs) < ega->mode_w) {
    rofs = ega->mode_w / 4;
  }

  y = ((addr - ega->crtc_ofs) & ~0x2000) / rofs;
  y = (addr & 0x2000) ? (2 * y + 1) : (2 * y);
  x = 4 * (((addr - ega->crtc_ofs) & 0x1fff) % rofs);

  if ((x >= ega->mode_w) || (y >= ega->mode_h)) {
    return;
  }

  for (i = 0; i < 4; i++) {
    unsigned m;

    m = 0x80 >> (2 * i);

    if (msk & m) {
      c = (ega->data[addr + 0x00000] & (m >> 1)) ? 0x01 : 0x00;
      c |= (ega->data[addr + 0x10000] & m) ? 0x02 : 0x00;
      c |= (ega->data[addr + 0x20000] & m) ? 0x04 : 0x00;
      c |= (ega->data[addr + 0x30000] & m) ? 0x08 : 0x00;

      c &= ega->atc_reg[0x12];

      trm_set_col (ega->trm, c, 0);
      pce_smap_get_pixel (&ega->smap, x + i, y, &sx, &sy, &sw, &sh);
      trm_set_pxl (ega->trm, sx, sy, sw, sh);
    }
  }
}


/*****************************************************************************
 * EGA 16 color graphic modes and CGA 2 color graphic mode
 *****************************************************************************/

static
int ega_screenshot_ega16 (ega_t *ega, FILE *fp)
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

        idx &= ega->atc_reg[0x12];

        ega_get_rgb (ega, idx, rgb);
        fwrite (rgb, 1, 3, fp);

        msk = msk >> 1;
      }
    }

    if (ega->crtc_reg[0x17] & 0x01) {
      if (addr & 0x2000) {
        addr = (addr + rofs) & 0x1fff;
      }
      else {
        addr |= 0x2000;
      }
    }
    else {
      addr = (addr + rofs) & 0xffff;
    }
  }

  return (0);
}

static
void ega_update_ega16 (ega_t *ega)
{
  unsigned      i, x, y, w;
  unsigned      sx, sy, sw, sh;
  unsigned      col1, col2;
  unsigned      rofs;
  unsigned long addr;
  unsigned char msk;

  addr = ega->crtc_ofs & 0xffff;
  rofs = 2 * ega->crtc_reg[0x13];

  w = ega->mode_w / 8;

  col1 = 0;
  trm_set_col (ega->trm, 0, 0);

  for (y = 0; y < ega->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;
      for (i = 0; i < 8; i++) {
        col2 = (ega->data[addr + x + 0 * 65536] & msk) ? 0x01 : 0x00;
        col2 |= (ega->data[addr + x + 1 * 65536] & msk) ? 0x02 : 0x00;
        col2 |= (ega->data[addr + x + 2 * 65536] & msk) ? 0x04 : 0x00;
        col2 |= (ega->data[addr + x + 3 * 65536] & msk) ? 0x08 : 0x00;

        col2 &= ega->atc_reg[0x12];

        if (col1 != col2) {
          trm_set_col (ega->trm, col2, 0);
          col1 = col2;
        }

        pce_smap_get_pixel (&ega->smap, 8 * x + i, y, &sx, &sy, &sw, &sh);
        trm_set_pxl (ega->trm, sx, sy, sw, sh);

        msk = msk >> 1;
      }
    }

    if (ega->crtc_reg[0x17] & 0x01) {
      if (addr & 0x2000) {
        addr = (addr + rofs) & 0x1fff;
      }
      else {
        addr |= 0x2000;
      }
    }
    else {
      addr = (addr + rofs) & 0xffff;
    }
  }

  ega->dirty = 0;
}

static
void ega_set_latches_ega16 (ega_t *ega, unsigned long addr, unsigned char latch[4])
{
  unsigned      i;
  unsigned      sx, sy, sw, sh;
  unsigned      x, y, c;
  unsigned      rofs;
  unsigned char msk;

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

  if (msk == 0) {
    return;
  }

  if (addr < ega->crtc_ofs) {
    return;
  }

  if (ega->dirty) {
    ega->update (ega);
    ega->dirty = 0;
    return;
  }

  rofs = 2 * ega->crtc_reg[0x13];

  if ((8 * rofs) < ega->mode_w) {
    rofs = ega->mode_w / 8;
  }

  if (ega->crtc_reg[0x17] & 0x01) {
    y = ((addr - ega->crtc_ofs) & ~0x2000) / rofs;
    y = (addr & 0x2000) ? (2 * y + 1) : (2 * y);
    x = 8 * (((addr - ega->crtc_ofs) & 0x1fff) % rofs);
  }
  else {
    y = (addr - ega->crtc_ofs) / rofs;
    x = 8 * ((addr - ega->crtc_ofs) % rofs);
  }

  if ((x >= ega->mode_w) || (y >= ega->mode_h)) {
    return;
  }

  for (i = 0; i < 8; i++) {
    if (msk & (0x80 >> i)) {
      c = (ega->data[addr + 0 * 65536] & (0x80 >> i)) ? 0x01 : 0x00;
      c |= (ega->data[addr + 1 * 65536] & (0x80 >> i)) ? 0x02 : 0x00;
      c |= (ega->data[addr + 2 * 65536] & (0x80 >> i)) ? 0x04 : 0x00;
      c |= (ega->data[addr + 3 * 65536] & (0x80 >> i)) ? 0x08 : 0x00;

      c &= ega->atc_reg[0x12];

      trm_set_col (ega->trm, c, 0);
      pce_smap_get_pixel (&ega->smap, x + i, y, &sx, &sy, &sw, &sh);
      trm_set_pxl (ega->trm, sx, sy, sw, sh);
    }
  }
}


int ega_screenshot (ega_t *ega, FILE *fp, unsigned mode)
{
  return (ega->screenshot (ega, fp));
}

static
void ega_set_uint8_odd_even (ega_t *ega, unsigned long addr, unsigned char val)
{
  unsigned char latches[4];
  unsigned long tmp;

  tmp = addr;

  addr = (addr & 0xfffe);

  if (ega->reg->data[0x1c] & 0x20) {
    addr |= 1;
  }

  if (tmp & 1) {
    latches[0] = ega->data[addr];
    latches[1] = val;
    latches[2] = ega->data[addr + 0x20000];
    latches[3] = val;
  }
  else {
    latches[0] = val;
    latches[1] = ega->data[addr + 0x10000];
    latches[2] = val;
    latches[3] = ega->data[addr + 0x30000];
  }

  ega->set_latches (ega, addr, latches);
}

static
unsigned char ega_get_uint8_odd_even (ega_t *ega, unsigned long addr)
{
  if (addr & 1) {
    addr = (addr & 0xfffe) + 0x10000;
  }
  else {
    addr = addr & 0xfffe;
  }

  if (ega->reg->data[0x1c] & 0x20) {
    addr |= 1;
  }

  return (ega->data[addr]);
}

void ega_set_uint8_gdc (ega_t *ega, unsigned long addr, unsigned char val)
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

  ega->set_latches (ega, addr, col);
}

static
unsigned char ega_get_uint8_gdc (ega_t *ega, unsigned long addr)
{
  addr &= 0xffff;

  ega->latch[0] = ega->data[addr + 0x00000];
  ega->latch[1] = ega->data[addr + 0x10000];
  ega->latch[2] = ega->data[addr + 0x20000];
  ega->latch[3] = ega->data[addr + 0x30000];

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

int ega_eval_mode (ega_t *ega)
{
  if (ega->ts_reg[0x04] & 0x04) {
    ega->set_uint8 = &ega_set_uint8_gdc;
    ega->get_uint8 = &ega_get_uint8_gdc;
  }
  else {
    ega->set_uint8 = &ega_set_uint8_odd_even;
    ega->get_uint8 = &ega_get_uint8_odd_even;
  }

  if (ega->atc_reg[0x10] & 0x01) {
    if (ega->gdc_reg[0x05] & 0x20) {
      ega->update = &ega_update_cga4;
      ega->screenshot = &ega_screenshot_cga4;
      ega->set_latches = &ega_set_latches_cga4;
    }
    else {
      ega->update = &ega_update_ega16;
      ega->screenshot = &ega_screenshot_ega16;
      ega->set_latches = &ega_set_latches_ega16;
    }
  }
  else {
    ega->update = &ega_update_cga_txt;
    ega->screenshot = &ega_screenshot_cga_txt;
    ega->set_latches = &ega_set_latches_cga_txt;
  }

  return (0);
}

static
void ega_update (ega_t *ega)
{
  ega->update (ega);
}

void ega_set_mode (ega_t *ega, unsigned mode, unsigned w, unsigned h)
{
  unsigned sw, sh;

  pce_log (MSG_DEB, "ega:\tset mode %u (%u, %u)\n", mode, w, h);

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
      else if ((w == 640) && (h == 480)) {
        sw = ega->mode_640x480_w;
        sh = ega->mode_640x480_h;
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

  ega->dirty = 1;
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
    if (y1 > 13) {
      trm_set_crs (ega->trm, 0, 0, 0);
      return;
    }

    if ((y2 < y1) || (y2 > 13)) {
      y2 = 13;
    }

    y1 = (255 * y1 + 6) / 13;
    y2 = (255 * y2 + 6) / 13;

    trm_set_crs (ega->trm, y1, y2, 1);
  }
}

void ega_set_page_ofs (ega_t *ega, unsigned ofs)
{
  if (ega->crtc_ofs == ofs) {
    return;
  }

  ega->crtc_ofs = ofs;

//  ega_update (ega);
}

void ega_mem_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
//  pce_log (MSG_DEB, "ega: set mem %04lx = %02x\n", addr, val);

  if (addr < ega->base) {
    return;
  }

  addr -= ega->base;

  if (addr >= ega->size) {
    return;
  }

  ega->set_uint8 (ega, addr, val);
}

void ega_mem_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
  ega_mem_set_uint8 (ega, addr, val & 0xff);
  ega_mem_set_uint8 (ega, addr + 1, val >> 8);
}

unsigned char ega_mem_get_uint8 (ega_t *ega, unsigned long addr)
{
//  pce_log (MSG_DEB, "ega: get mem %04lx\n", addr);

  if (addr < ega->base) {
    return (0xff);
  }

  addr -= ega->base;

  if (addr >= ega->size) {
    return (0xff);
  }

  return (ega->get_uint8 (ega, addr));
}

unsigned short ega_mem_get_uint16 (ega_t *ega, unsigned long addr)
{
  unsigned short ret;

  ret = ega_mem_get_uint8 (ega, addr) & 0xff;
  ret |= (ega_mem_get_uint8 (ega, addr + 1) & 0xff) << 8;

  return (ret);
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
      ega_eval_mode (ega);
      break;

    case 0x06: /* misc */
      ega->gdc_reg[0x06] = val & 0x0f;
      switch ((val >> 2) & 0x03) {
        case 0x00:
          ega->base = 0x00000;
          ega->size = 0x20000;
          break;
        case 0x01:
          ega->base = 0x00000;
          ega->size = 0x10000;
          break;
        case 0x02:
          ega->base = 0x10000;
          ega->size = 0x08000;
          break;
        case 0x03:
          ega->base = 0x18000;
          ega->size = 0x08000;
          break;
      }
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

      ega->dirty = 1;
    }
    return;
  }

  switch (reg) {
    case 0x10: /* mode control */
      ega->atc_reg[reg] = val;
      ega_eval_mode (ega);
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
      ega_eval_mode (ega);
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
      ega_set_crs (ega, ega->crtc_reg[0x0a], ega->crtc_reg[0x0b]);
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

unsigned char ega_get_input_state_1 (ega_t *ega)
{
  static unsigned cnt = 0;
  unsigned char   val;

  ega->atc_index = 1;

  cnt += 1;

  val = ega->reg->data[0x2a];

  if (val & 0x08) {
    if (cnt >= 64) {
      cnt = 0;
      val &= ~(0x80 | 0x08 | 0x01);
    }
  }
  else {
    if ((cnt & 3) == 0) {
      val ^= 0x01;
    }
    if (cnt >= (8 * ega->mode_h)) {
      cnt = 0;
      val |= (0x80 | 0x08);
      val &= ~0x01;

      if (ega->dirty) {
        ega->update (ega);
        ega->dirty = 0;
      }
    }
  }

  ega->reg->data[0x2a] = val;

  return (val);
}

void ega_reg_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
//  pce_log (MSG_DEB, "ega: set reg %04lx = %02x\n", addr, val);

  switch (addr) {
    case 0x10: /* 0x3c0: ATC index/data */
      if (ega->atc_index) {
        ega->atc_index = 0;
        ega->reg->data[0x10] = val;
      }
      else {
        ega->atc_index = 1;
        ega_atc_set_reg (ega, ega->reg->data[0x10], val);
      }
      break;

    case 0x12: /* 0x3c2: misc output register */
    case 0x1c:
      ega->reg->data[0x1c] = val;
      break;

    case 0x14: /* 0x3c4: TS index */
      ega->reg->data[0x14] = val;
      break;

    case 0x15: /* 0x3c5: TS data */
      ega_ts_set_reg (ega, ega->reg->data[0x14], val);
      break;

    case 0x1e: /* 0x3ce: GDC index */
      ega->reg->data[0x1e] = val;
      break;

    case 0x1f: /* 0x3cf: GDC data */
      ega_gdc_set_reg (ega, ega->reg->data[0x1e], val);
      break;

    case 0x20: /* 0x3d0: pce extension */
      switch (val & 0x7f) {
        case 0x02:
        case 0x03:
          ega_set_mode (ega, 0, 80, 25);
          break;

        case 0x04:
        case 0x05:
          ega_set_mode (ega, 16, 320, 200);
          break;

        case 0x06:
          ega_set_mode (ega, 16, 640, 200);
          break;

        case 0x07:
          ega_set_mode (ega, 0, 80, 25);
          break;

        case 0x0d:
          ega_set_mode (ega, 16, 320, 200);
          break;

        case 0x0e:
          ega_set_mode (ega, 16, 640, 200);
          break;

        case 0x0f:
        case 0x10:
          ega_set_mode (ega, 16, 640, 350);
          break;

        case 0x11:
        case 0x12:
          ega_set_mode (ega, 16, 640, 480);
          break;

        default:
          pce_log (MSG_INF, "ega: unknown mode (%u)\n", val);
          break;
      }
      break;

//    case 0x04:
    case 0x24: /* 0x3d4: CRTC index */
      ega->reg->data[0x24] = val;
      break;

//    case 0x05:
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
//  pce_log (MSG_DEB, "ega: get reg %04lx\n", addr);

  switch (addr) {
    case 0x12: /* 0x3c2: input state 0  */
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

//    case 0x04:
    case 0x24: /* 0x3d4 CRTC index */
      return (ega->reg->data[0x24]);

//    case 0x05:
    case 0x25: /* 0x3d5 CRTC data */
      return (ega_crtc_get_reg (ega, ega->reg->data[0x24]));

//    case 0x0a:
    case 0x2a: /* 0x3da: input state 1 */
      return (ega_get_input_state_1 (ega));

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

void ega_clock (ega_t *ega, unsigned long cnt)
{
  static unsigned upd_cnt = 0;

  if (ega->dirty) {
    upd_cnt += 1;

    if (upd_cnt > 16) {
      upd_cnt = 0;
      ega->update (ega);
      ega->dirty = 0;
    }
  }
}
