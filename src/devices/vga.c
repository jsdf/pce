/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/vga.c                                          *
 * Created:       2003-09-06 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-03-27 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: vga.c 390 2004-02-23 14:38:02Z hampa $ */


#include <stdio.h>
#include <stdlib.h>

#include <lib/log.h>
#include <lib/hexdump.h>

#include "vga.h"


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


static void vga_update (vga_t *vga);

static void vga_update_cga_txt (vga_t *vga);
static int vga_screenshot_cga_txt (vga_t *vga, FILE *fp);
static void vga_set_latches_cga_txt (vga_t *cga, unsigned long addr, unsigned char val[4]);

static void vga_set_uint8_odd_even (vga_t *vga, unsigned long addr, unsigned char val);
static unsigned char vga_get_uint8_odd_even (vga_t *vga, unsigned long addr);

static void vga_set_uint8_chain4 (vga_t *vga, unsigned long addr, unsigned char val);
static unsigned char vga_get_uint8_chain4 (vga_t *vga, unsigned long addr);


static
void vga_dac_map_entry (vga_t *vga, unsigned idx)
{
  unsigned r, g, b;

  idx &= 0xff;

  r = vga->dac_reg[3 * idx + 0] & 0x3f;
  g = vga->dac_reg[3 * idx + 1] & 0x3f;
  b = vga->dac_reg[3 * idx + 2] & 0x3f;

  r = (r << 2) | (r >> 6);
  g = (g << 2) | (g >> 6);
  b = (b << 2) | (b >> 6);

  r = (r << 8) | r;
  g = (g << 8) | g;
  b = (b << 8) | b;

  trm_set_map (vga->trm, idx, r, g, b);
}

static
void vga_set_ega_dac (vga_t *vga)
{
  unsigned i;
  unsigned r, g, b;

  for (i = 0; i < 64; i++) {
    r = ((i & 0x04) ? 0x2a : 0x00) + ((i & 0x20) ? 0x15 : 0x00);
    g = ((i & 0x02) ? 0x2a : 0x00) + ((i & 0x10) ? 0x15 : 0x00);
    b = ((i & 0x01) ? 0x2a : 0x00) + ((i & 0x08) ? 0x15 : 0x00);

    vga->dac_reg[3 * i + 0] = r;
    vga->dac_reg[3 * i + 1] = g;
    vga->dac_reg[3 * i + 2] = b;
  }

  for (i = 64; i < 256; i++) {
    vga->dac_reg[3 * i + 0] = i >> 2;
    vga->dac_reg[3 * i + 1] = i >> 2;
    vga->dac_reg[3 * i + 2] = i >> 2;
  }

  for (i = 0; i < 256; i++) {
    vga_dac_map_entry (vga, i);
  }
}


video_t *vga_new (terminal_t *trm, ini_sct_t *sct)
{
  vga_t    *vga;

  vga = (vga_t *) malloc (sizeof (vga_t));
  if (vga == NULL) {
    return (NULL);
  }

  vga->data = (unsigned char *) malloc (256UL * 1024);
  if (vga->data == NULL) {
    free (vga);
    return (NULL);
  }

  vga->trm = trm;

  vga->base = 0x18000;
  vga->size = 0x08000;

  vga->update = &vga_update_cga_txt;
  vga->screenshot = &vga_screenshot_cga_txt;
  vga->set_latches = &vga_set_latches_cga_txt;
  vga->set_uint8 = &vga_set_uint8_odd_even;
  vga->get_uint8 = &vga_get_uint8_odd_even;

  pce_video_init (&vga->vid);

  vga->vid.type = PCE_VIDEO_EGA;
  vga->vid.ext = vga;
  vga->vid.del = (pce_video_del_f) &vga_del;
  vga->vid.get_mem = (pce_video_get_mem_f) &vga_get_mem;
  vga->vid.get_reg = (pce_video_get_reg_f) &vga_get_reg;
  vga->vid.prt_state = (pce_video_prt_state_f) &vga_prt_state;
  vga->vid.update = (pce_video_update_f) &vga_update;
  vga->vid.dump = (pce_video_dump_f) &vga_dump;
  vga->vid.screenshot = (pce_video_screenshot_f) &vga_screenshot;
  vga->vid.clock = (pce_video_clock_f) &vga_clock;

  pce_smap_init (&vga->smap, 320, 200, 320, 200);

  memset (vga->crtc_reg, 0xff, 24 * sizeof (unsigned char));
  memset (vga->ts_reg, 0, 5 * sizeof (unsigned char));
  memset (vga->gdc_reg, 0, 9 * sizeof (unsigned char));
  memset (vga->atc_reg, 0, 21 * sizeof (unsigned char));
  memset (vga->dac_reg, 0, 768 * sizeof (unsigned char));

  vga->dac_idx = 0;
  vga->dac_col_msk = 0xff;
  vga->dac_state = 0;

  vga_set_ega_dac (vga);

  vga->mode_320x200_w = ini_get_lng_def (sct, "mode_320x200_w", 640);
  vga->mode_320x200_h = ini_get_lng_def (sct, "mode_320x200_h", 400);
  vga->mode_640x200_w = ini_get_lng_def (sct, "mode_640x200_w", 640);
  vga->mode_640x200_h = ini_get_lng_def (sct, "mode_640x200_h", 400);
  vga->mode_640x350_w = ini_get_lng_def (sct, "mode_640x350_w", 640);
  vga->mode_640x350_h = ini_get_lng_def (sct, "mode_640x350_h", 480);
  vga->mode_640x480_w = ini_get_lng_def (sct, "mode_640x480_w", 640);
  vga->mode_640x480_h = ini_get_lng_def (sct, "mode_640x480_h", 480);

  pce_log (MSG_INF, "video:\tVGA io=0x03b0 membase=0xa000 memsize=262144\n");

  vga->mem = mem_blk_new (0xa0000, 128UL * 1024, 0);
  vga->mem->ext = vga;
  vga->mem->set_uint8 = (mem_set_uint8_f) &vga_mem_set_uint8;
  vga->mem->get_uint8 = (mem_get_uint8_f) &vga_mem_get_uint8;
  vga->mem->set_uint16 = (mem_set_uint16_f) &vga_mem_set_uint16;
  vga->mem->get_uint16 = (mem_get_uint16_f) &vga_mem_get_uint16;

  vga->reg = mem_blk_new (0x3b0, 64, 1);
  vga->reg->ext = vga;
  vga->reg->set_uint8 = (mem_set_uint8_f) &vga_reg_set_uint8;
  vga->reg->set_uint16 = (mem_set_uint16_f) &vga_reg_set_uint16;
  vga->reg->get_uint8 = (mem_get_uint8_f) &vga_reg_get_uint8;
  vga->reg->get_uint16 = (mem_get_uint16_f) &vga_reg_get_uint16;
  mem_blk_clear (vga->reg, 0x00);

  vga->crtc_pos = 0;
  vga->crtc_ofs = 0;

  vga->crs_on = 1;

  vga->dirty = 0;

  vga->mode = 0;

  trm_set_size (trm, TERM_MODE_TEXT, 80, 25);

  return (&vga->vid);
}

void vga_del (vga_t *vga)
{
  if (vga != NULL) {
    mem_blk_del (vga->mem);
    mem_blk_del (vga->reg);
    pce_smap_free (&vga->smap);
    free (vga->data);
    free (vga);
  }
}

void vga_prt_state (vga_t *vga, FILE *fp)
{
  unsigned i;
  unsigned x, y;

  if (vga->crtc_pos < vga->crtc_ofs) {
    x = 0;
    y = 0;
  }
  else {
    x = (vga->crtc_pos - vga->crtc_ofs) % 80;
    y = (vga->crtc_pos - vga->crtc_ofs) / 80;
  }

  fprintf (fp, "VGA: MODE=%u  OFS=%04X  POS=%04X[%u/%u]  CRS=[%u-%u]  LATCH=[%02x %02x %02x %02x]\n",
    vga->mode, vga->crtc_ofs, vga->crtc_pos, x, y,
    vga->crtc_reg[0x0a], vga->crtc_reg[0x0b],
    vga->latch[0], vga->latch[1], vga->latch[2], vga->latch[3]
  );

  fprintf (fp, "REGS: 3C2=%02x  3CC=%02x  3DA=%02x  BASE=%05lX  SIZE=%05lX\n",
    vga->reg->data[0x12], vga->reg->data[0x1c], vga->reg->data[0x2a],
    0xa0000 + vga->base, vga->size
  );

  fprintf (fp, "CRTC: [%02X", vga->crtc_reg[0]);
  for (i = 1; i < 24; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", vga->crtc_reg[i]);
  }
  fputs ("]\n", fp);

  fprintf (fp, "TS:   [%02X", vga->ts_reg[0]);
  for (i = 1; i < 5; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", vga->ts_reg[i]);
  }
  fputs ("]\n", fp);

  fprintf (fp, "GDC:  [%02X", vga->gdc_reg[0]);
  for (i = 1; i < 9; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", vga->gdc_reg[i]);
  }
  fputs ("]\n", fp);

  fprintf (fp, "ATC:  [%02X", vga->atc_reg[0]);
  for (i = 1; i < 21; i++) {
    fputs ((i & 7) ? " " : "-", fp);
    fprintf (fp, "%02X", vga->atc_reg[i]);
  }
  fputs ("]\n", fp);

  fflush (fp);
}

int vga_dump (vga_t *vga, FILE *fp)
{
  fprintf (fp, "# VGA dump\n");

  fprintf (fp, "\n# REGS:\n");
  pce_dump_hex (fp, vga->reg->data, vga->reg->size, vga->reg->base, 16, "# ", 0);

  fprintf (fp, "\n# CRTC:\n");
  pce_dump_hex (fp, vga->crtc_reg, 24, 0, 16, "# ", 0);

  fprintf (fp, "\n# TS:\n");
  pce_dump_hex (fp, vga->ts_reg, 5, 0, 16, "# ", 0);

  fprintf (fp, "\n# GDC:\n");
  pce_dump_hex (fp, vga->gdc_reg, 9, 0, 16, "# ", 0);

  fprintf (fp, "\n# ATC:\n");
  pce_dump_hex (fp, vga->atc_reg, 21, 0, 16, "# ", 0);

  fputs ("\n\n# RAM:\n", fp);
  pce_dump_hex (fp, vga->data, 256 * 1024, 0, 16, "", 1);

  return (0);
}

mem_blk_t *vga_get_mem (vga_t *vga)
{
  return (vga->mem);
}

mem_blk_t *vga_get_reg (vga_t *vga)
{
  return (vga->reg);
}


static
void vga_get_rgb_dac (vga_t *vga, unsigned idx, unsigned char rgb[3])
{
  idx = 3 * (idx & vga->dac_col_msk);

  rgb[0] = vga->dac_reg[idx] & 0x3f;
  rgb[1] = vga->dac_reg[idx + 1] & 0x3f;
  rgb[2] = vga->dac_reg[idx + 2] & 0x3f;

  rgb[0] = (rgb[0] << 2) | (rgb[0] >> 6);
  rgb[1] = (rgb[1] << 2) | (rgb[1] >> 6);
  rgb[2] = (rgb[2] << 2) | (rgb[2] >> 6);
}

static
void vga_get_rgb_atc (vga_t *vga, unsigned idx, unsigned char rgb[3])
{
  vga_get_rgb_dac (vga, vga->atc_reg[idx], rgb);
}


/*****************************************************************************
 * CGA text modes
 *****************************************************************************/

static
int vga_screenshot_cga_txt (vga_t *vga, FILE *fp)
{
  unsigned i;
  unsigned x, y;
  unsigned rofs;

  i = (vga->crtc_ofs << 1) & 0xfffe;

  if (vga->reg->data[0x1c] & 0x20) {
    i |= 1;
  }

  rofs = 4 * vga->crtc_reg[0x13];

  for (y = 0; y < vga->mode_h; y++) {
    for (x = 0; x < vga->mode_w; x++) {
      fputc (vga->data[(i + 2 * x) & 0xffff], fp);
    }

    i = (i + rofs) & 0xffff;

    fputs ("\n", fp);
  }

  return (0);
}

static
void vga_update_cga_txt (vga_t *vga)
{
  unsigned i, j;
  unsigned x, y;
  unsigned fg, bg;
  unsigned rofs;

  i = (vga->crtc_ofs << 1) & 0xfffe;

  if (vga->reg->data[0x1c] & 0x20) {
    i |= 1;
  }

  rofs = 4 * vga->crtc_reg[0x13];

  for (y = 0; y < vga->mode_h; y++) {
    j = i;
    for (x = 0; x < vga->mode_w; x++) {
      fg = vga->data[j + 0x10000];
      bg = (fg >> 4) & 0x0f;
      fg = fg & 0x0f;

      trm_set_col (vga->trm, fg, bg);
      trm_set_chr (vga->trm, x, y, vga->data[j]);

      j = (j + 2) & 0xffff;
    }

    i += rofs;
  }
}

static
void vga_set_latches_cga_txt (vga_t *vga, unsigned long addr, unsigned char val[4])
{
  unsigned char wrpl, rofs;

  addr &= 0xffff;

  wrpl = vga->ts_reg[TS_WRPL];

  if (wrpl & 0x03) {
    unsigned      x, y;
    unsigned char c, a;

    if ((wrpl & 0x01) && (val[0] != vga->data[addr])) {
      vga->data[addr] = val[0];
    }

    if ((wrpl & 0x02) && (val[1] != vga->data[addr + 0x10000])) {
      vga->data[addr + 0x10000] = val[1];
    }

    addr &= ~0x0001;

    if (addr < (vga->crtc_ofs << 1)) {
      return;
    }

    addr -= (vga->crtc_ofs << 1);

    if (addr >= 4000) {
      return;
    }

    rofs = 2 * vga->crtc_reg[0x13];
    if (rofs < vga->mode_w) {
      rofs = vga->mode_w;
    }

    c = vga->data[addr];
    a = vga->data[addr + 0x10000];

    x = (addr >> 1) % rofs;
    y = (addr >> 1) / rofs;

    trm_set_col (vga->trm, a & 0x0f, (a & 0xf0) >> 4);
    trm_set_chr (vga->trm, x, y, c);
  }
}


/*****************************************************************************
 * CGA 4 color graphic mode
 *****************************************************************************/

static
int vga_screenshot_cga4 (vga_t *vga, FILE *fp)
{
  unsigned      i, x, y, w;
  unsigned      idx;
  unsigned char rgb[3];
  unsigned long addr, rofs;
  unsigned char msk;

  fprintf (fp, "P6\n%u %u\n255 ", vga->mode_w, vga->mode_h);

  addr = vga->crtc_ofs;
  rofs = 2 * vga->crtc_reg[0x13];

  w = vga->mode_w / 4;

  for (y = 0; y < vga->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;

      for (i = 0; i < 4; i++) {
        idx = (vga->data[addr + x + 0x00000] & (msk >> 1)) ? 0x01 : 0x00;
        idx |= (vga->data[addr + x + 0x10000] & msk) ? 0x02 : 0x00;
        idx |= (vga->data[addr + x + 0x20000] & msk) ? 0x04 : 0x00;
        idx |= (vga->data[addr + x + 0x30000] & msk) ? 0x08 : 0x00;

        idx &= vga->atc_reg[0x12];

        vga_get_rgb_atc (vga, idx, rgb);
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
void vga_update_cga4 (vga_t *vga)
{
  unsigned      i, x, y, w;
  unsigned      sx, sy, sw, sh;
  unsigned      col1, col2;
  unsigned      rofs;
  unsigned long addr;
  unsigned char msk;

  addr = vga->crtc_ofs & 0xffff;
  rofs = 2 * vga->crtc_reg[0x13];

  w = vga->mode_w / 4;

  col1 = 0;
  trm_set_col (vga->trm, 0, 0);

  for (y = 0; y < vga->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;
      for (i = 0; i < 4; i++) {
        col2 = (vga->data[addr + x + 0x00000] & (msk >> 1)) ? 0x01 : 0x00;
        col2 |= (vga->data[addr + x + 0x10000] & msk) ? 0x02 : 0x00;
        col2 |= (vga->data[addr + x + 0x20000] & msk) ? 0x04 : 0x00;
        col2 |= (vga->data[addr + x + 0x30000] & msk) ? 0x08 : 0x00;

        col2 &= vga->atc_reg[0x12];

        if (col1 != col2) {
          trm_set_col (vga->trm, col2, 0);
          col1 = col2;
        }

        pce_smap_get_pixel (&vga->smap, 4 * x + i, y, &sx, &sy, &sw, &sh);
        trm_set_pxl (vga->trm, sx, sy, sw, sh);

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

  vga->dirty = 0;
}

static
void vga_set_latches_cga4 (vga_t *vga, unsigned long addr, unsigned char latch[4])
{
  unsigned      i;
  unsigned      sx, sy, sw, sh;
  unsigned      x, y, c;
  unsigned      rofs;
  unsigned char msk;

  addr &= 0xffff;

  msk = 0;

  if (vga->ts_reg[2] & 0x01) {
    msk |= vga->data[addr + 0x00000] ^ latch[0];
    vga->data[addr + 0x00000] = latch[0];
  }

  if (vga->ts_reg[2] & 0x02) {
    msk |= vga->data[addr + 0x10000] ^ latch[1];
    vga->data[addr + 0x10000] = latch[1];
  }

  if (vga->ts_reg[2] & 0x04) {
    msk |= vga->data[addr + 0x20000] ^ latch[2];
    vga->data[addr + 0x20000] = latch[2];
  }

  if (vga->ts_reg[2] & 0x08) {
    msk |= vga->data[addr + 0x30000] ^ latch[3];
    vga->data[addr + 0x30000] = latch[3];
  }

  if (msk == 0) {
    return;
  }

  msk = (msk & 0xaa) | ((msk << 1) & 0xaa);

  if (addr < vga->crtc_ofs) {
    return;
  }

  if (vga->dirty) {
    vga->update (vga);
    vga->dirty = 0;
    return;
  }

  rofs = 2 * vga->crtc_reg[0x13];

  if ((4 * rofs) < vga->mode_w) {
    rofs = vga->mode_w / 4;
  }

  y = ((addr - vga->crtc_ofs) & ~0x2000) / rofs;
  y = (addr & 0x2000) ? (2 * y + 1) : (2 * y);
  x = 4 * (((addr - vga->crtc_ofs) & 0x1fff) % rofs);

  if ((x >= vga->mode_w) || (y >= vga->mode_h)) {
    return;
  }

  for (i = 0; i < 4; i++) {
    unsigned m;

    m = 0x80 >> (2 * i);

    if (msk & m) {
      c = (vga->data[addr + 0x00000] & (m >> 1)) ? 0x01 : 0x00;
      c |= (vga->data[addr + 0x10000] & m) ? 0x02 : 0x00;
      c |= (vga->data[addr + 0x20000] & m) ? 0x04 : 0x00;
      c |= (vga->data[addr + 0x30000] & m) ? 0x08 : 0x00;

      c &= vga->atc_reg[0x12];

      trm_set_col (vga->trm, c, 0);
      pce_smap_get_pixel (&vga->smap, x + i, y, &sx, &sy, &sw, &sh);
      trm_set_pxl (vga->trm, sx, sy, sw, sh);
    }
  }
}


/*****************************************************************************
 * EGA 16 color graphic modes and CGA 2 color graphic mode
 *****************************************************************************/

static
int vga_screenshot_ega16 (vga_t *vga, FILE *fp)
{
  unsigned      i, x, y, w;
  unsigned      idx;
  unsigned char rgb[3];
  unsigned long addr, rofs;
  unsigned char msk;

  fprintf (fp, "P6\n%u %u\n255 ", vga->mode_w, vga->mode_h);

  addr = vga->crtc_ofs;
  rofs = 2 * vga->crtc_reg[0x13];

  w = vga->mode_w / 8;

  for (y = 0; y < vga->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;

      for (i = 0; i < 8; i++) {
        idx = (vga->data[addr + x + 0x00000] & msk) ? 0x01 : 0x00;
        idx |= (vga->data[addr + x + 0x10000] & msk) ? 0x02 : 0x00;
        idx |= (vga->data[addr + x + 0x20000] & msk) ? 0x04 : 0x00;
        idx |= (vga->data[addr + x + 0x30000] & msk) ? 0x08 : 0x00;

        idx &= vga->atc_reg[0x12];

        vga_get_rgb_atc (vga, idx, rgb);
        fwrite (rgb, 1, 3, fp);

        msk = msk >> 1;
      }
    }

    if (vga->crtc_reg[0x17] & 0x01) {
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
void vga_update_ega16 (vga_t *vga)
{
  unsigned      i, x, y, w;
  unsigned      sx, sy, sw, sh;
  unsigned      col1, col2;
  unsigned      rofs;
  unsigned long addr;
  unsigned char msk;

  addr = vga->crtc_ofs & 0xffff;
  rofs = 2 * vga->crtc_reg[0x13];

  w = vga->mode_w / 8;

  col1 = 0;
  trm_set_col (vga->trm, 0, 0);

  for (y = 0; y < vga->mode_h; y++) {
    for (x = 0; x < w; x++) {
      msk = 0x80;
      for (i = 0; i < 8; i++) {
        col2 = (vga->data[addr + x + 0x00000] & msk) ? 0x01 : 0x00;
        col2 |= (vga->data[addr + x + 0x10000] & msk) ? 0x02 : 0x00;
        col2 |= (vga->data[addr + x + 0x20000] & msk) ? 0x04 : 0x00;
        col2 |= (vga->data[addr + x + 0x30000] & msk) ? 0x08 : 0x00;

        col2 &= vga->atc_reg[0x12];

        if (col1 != col2) {
          trm_set_col (vga->trm, col2, 0);
          col1 = col2;
        }

        pce_smap_get_pixel (&vga->smap, 8 * x + i, y, &sx, &sy, &sw, &sh);
        trm_set_pxl (vga->trm, sx, sy, sw, sh);

        msk = msk >> 1;
      }
    }

    if (vga->crtc_reg[0x17] & 0x01) {
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

  vga->dirty = 0;
}

static
void vga_set_latches_ega16 (vga_t *vga, unsigned long addr, unsigned char latch[4])
{
  unsigned      i;
  unsigned      sx, sy, sw, sh;
  unsigned      x, y, c, m;
  unsigned      rofs;
  unsigned char msk;

  addr &= 0xffff;

  msk = 0;

  if (vga->ts_reg[2] & 0x01) {
    msk |= vga->data[addr] ^ latch[0];
    vga->data[addr] = latch[0];
  }

  if (vga->ts_reg[2] & 0x02) {
    msk |= vga->data[addr + 0x10000] ^ latch[1];
    vga->data[addr + 0x10000] = latch[1];
  }

  if (vga->ts_reg[2] & 0x04) {
    msk |= vga->data[addr + 0x20000] ^ latch[2];
    vga->data[addr + 0x20000] = latch[2];
  }

  if (vga->ts_reg[2] & 0x08) {
    msk |= vga->data[addr + 0x30000] ^ latch[3];
    vga->data[addr + 0x30000] = latch[3];
  }

  if (msk == 0) {
    return;
  }

  if (addr < vga->crtc_ofs) {
    return;
  }

  if (vga->dirty) {
    vga->update (vga);
    vga->dirty = 0;
    return;
  }

  rofs = 2 * vga->crtc_reg[0x13];

  if ((8 * rofs) < vga->mode_w) {
    rofs = vga->mode_w / 8;
  }

  if (vga->crtc_reg[0x17] & 0x01) {
    y = ((addr - vga->crtc_ofs) & ~0x2000) / rofs;
    y = (addr & 0x2000) ? (2 * y + 1) : (2 * y);
    x = 8 * (((addr - vga->crtc_ofs) & 0x1fff) % rofs);
  }
  else {
    y = (addr - vga->crtc_ofs) / rofs;
    x = 8 * ((addr - vga->crtc_ofs) % rofs);
  }

  if ((x >= vga->mode_w) || (y >= vga->mode_h)) {
    return;
  }

  m = 0x80;

  for (i = 0; i < 8; i++) {
    if (msk & m) {
      c = (vga->data[addr + 0x00000] & m) ? 0x01 : 0x00;
      c |= (vga->data[addr + 0x10000] & m) ? 0x02 : 0x00;
      c |= (vga->data[addr + 0x20000] & m) ? 0x04 : 0x00;
      c |= (vga->data[addr + 0x30000] & m) ? 0x08 : 0x00;

      c &= vga->atc_reg[0x12];

      trm_set_col (vga->trm, c, 0);
      pce_smap_get_pixel (&vga->smap, x + i, y, &sx, &sy, &sw, &sh);
      trm_set_pxl (vga->trm, sx, sy, sw, sh);
    }

    m = m >> 1;
  }
}


/*****************************************************************************
 * VGA 256 color graphic modes
 *****************************************************************************/

static
int vga_screenshot_vga256 (vga_t *vga, FILE *fp)
{
  return (1);
}

static
void vga_update_vga256 (vga_t *vga)
{
  unsigned      x, y;
  unsigned      sx, sy, sw, sh;
  unsigned      col;
  unsigned      rofs;
  unsigned long addr;

  addr = vga->crtc_ofs & 0xffff;
  rofs = 8 * vga->crtc_reg[0x13];

  for (y = 0; y < vga->mode_h; y++) {
    for (x = 0; x < vga->mode_w; x++) {
      if (vga->crtc_reg[0x14] & 0x20) {
        col = vga->data[((addr + x) & 0xfffc) + ((addr + x) & 0x03) * 0x10000UL];
      }
      else {
        col = vga->data[((addr + x) >> 2) + ((addr + x) & 0x03) * 0x10000UL];
      }

      trm_set_col (vga->trm, col, 0);
      pce_smap_get_pixel (&vga->smap, x, y, &sx, &sy, &sw, &sh);
      trm_set_pxl (vga->trm, sx, sy, sw, sh);
    }

    addr = (addr + rofs) & 0xffff;
  }

  vga->dirty = 0;
}

static
void vga_set_latches_vga256 (vga_t *vga, unsigned long addr, unsigned char latch[4])
{
  unsigned i;
  unsigned x, y;
  unsigned sx, sy, sw, sh;

  if (vga->crtc_reg[0x14] & 0x20) {
    addr = addr & 0xfffc;
  }
  else {
    addr = addr >> 2;
  }

  x = addr % 320;
  y = addr / 320;

  for (i = 0; i < 4; i++) {
    vga->data[addr] = latch[i];

    trm_set_col (vga->trm, latch[i], 0);
    pce_smap_get_pixel (&vga->smap, x + i, y, &sx, &sy, &sw, &sh);
    trm_set_pxl (vga->trm, sx, sy, sw, sh);

    addr += 0x10000UL;
  }
}


/*****************************************************************************
 * odd / even mode
 *****************************************************************************/

static
void vga_set_uint8_odd_even (vga_t *vga, unsigned long addr, unsigned char val)
{
  unsigned char latches[4];
  unsigned long tmp;

  tmp = addr;

  addr = (addr & 0xfffe);

  if (vga->reg->data[0x1c] & 0x20) {
    addr |= 1;
  }

  if (tmp & 1) {
    latches[0] = vga->data[addr];
    latches[1] = val;
    latches[2] = vga->data[addr + 0x20000];
    latches[3] = val;
  }
  else {
    latches[0] = val;
    latches[1] = vga->data[addr + 0x10000];
    latches[2] = val;
    latches[3] = vga->data[addr + 0x30000];
  }

  vga->set_latches (vga, addr, latches);
}

static
unsigned char vga_get_uint8_odd_even (vga_t *vga, unsigned long addr)
{
  if (addr & 1) {
    addr = (addr & 0xfffe) + 0x10000;
  }
  else {
    addr = addr & 0xfffe;
  }

  if (vga->reg->data[0x1c] & 0x20) {
    addr |= 1;
  }

  return (vga->data[addr]);
}


/*****************************************************************************
 * chain4 mode
 *****************************************************************************/

static
void vga_set_uint8_chain4 (vga_t *vga, unsigned long addr, unsigned char val)
{
  unsigned char latches[4];
  unsigned      plane;

  plane = addr & 0x03;

  addr = (addr & 0xfffc);

  latches[0] = vga->data[addr];
  latches[1] = vga->data[addr + 0x10000];
  latches[2] = vga->data[addr + 0x20000];
  latches[3] = vga->data[addr + 0x30000];

  latches[plane] = val;

  vga->set_latches (vga, addr, latches);
}

static
unsigned char vga_get_uint8_chain4 (vga_t *vga, unsigned long addr)
{
  addr = (addr & 0xfffc) + (addr & 0x03) * 0x10000UL;

  return (vga->data[addr]);
}


/*****************************************************************************
 * gdc byte mode
 *****************************************************************************/

void vga_set_uint8_gdc (vga_t *vga, unsigned long addr, unsigned char val)
{
  unsigned char col[4];

  switch (vga->gdc_reg[5] & 0x03) {
    case 0x00: { /* write mode 0 */
      unsigned char ena, set, msk;

      ena = vga->gdc_reg[0x01];
      set = vga->gdc_reg[0x00];
      msk = vga->gdc_reg[0x08];

      col[0] = (ena & 0x01) ? ((set & 0x01) ? 0xff : 0x00) : val;
      col[1] = (ena & 0x02) ? ((set & 0x02) ? 0xff : 0x00) : val;
      col[2] = (ena & 0x04) ? ((set & 0x04) ? 0xff : 0x00) : val;
      col[3] = (ena & 0x08) ? ((set & 0x08) ? 0xff : 0x00) : val;

      switch (vga->gdc_reg[3] & 0x18) {
        case 0x00: /* copy */
          break;

        case 0x08: /* and */
          col[0] &= vga->latch[0];
          col[1] &= vga->latch[1];
          col[2] &= vga->latch[2];
          col[3] &= vga->latch[3];
          break;

        case 0x10: /* or */
          col[0] |= vga->latch[0];
          col[1] |= vga->latch[1];
          col[2] |= vga->latch[2];
          col[3] |= vga->latch[3];
          break;

        case 0x18: /* xor */
          col[0] ^= vga->latch[0];
          col[1] ^= vga->latch[1];
          col[2] ^= vga->latch[2];
          col[3] ^= vga->latch[3];
          break;
      }

      col[0] = (col[0] & msk) | (vga->latch[0] & ~msk);
      col[1] = (col[1] & msk) | (vga->latch[1] & ~msk);
      col[2] = (col[2] & msk) | (vga->latch[2] & ~msk);
      col[3] = (col[3] & msk) | (vga->latch[3] & ~msk);
    }
    break;

    case 0x01: /* write mode 1 */
      col[0] = vga->latch[0];
      col[1] = vga->latch[1];
      col[2] = vga->latch[2];
      col[3] = vga->latch[3];
      break;

    case 0x02: { /* write mode 2 */
      unsigned char msk;

      msk = vga->gdc_reg[0x08];

      col[0] = (val & 0x01) ? 0xff : 0x00;
      col[1] = (val & 0x02) ? 0xff : 0x00;
      col[2] = (val & 0x04) ? 0xff : 0x00;
      col[3] = (val & 0x08) ? 0xff : 0x00;

      switch (vga->gdc_reg[3] & 0x18) {
        case 0x00: /* copy */
          break;

        case 0x08: /* and */
          col[0] &= vga->latch[0];
          col[1] &= vga->latch[1];
          col[2] &= vga->latch[2];
          col[3] &= vga->latch[3];
          break;

        case 0x10: /* or */
          col[0] |= vga->latch[0];
          col[1] |= vga->latch[1];
          col[2] |= vga->latch[2];
          col[3] |= vga->latch[3];
          break;

        case 0x18: /* xor */
          col[0] ^= vga->latch[0];
          col[1] ^= vga->latch[1];
          col[2] ^= vga->latch[2];
          col[3] ^= vga->latch[3];
          break;
      }

      col[0] = (col[0] & msk) | (vga->latch[0] & ~msk);
      col[1] = (col[1] & msk) | (vga->latch[1] & ~msk);
      col[2] = (col[2] & msk) | (vga->latch[2] & ~msk);
      col[3] = (col[3] & msk) | (vga->latch[3] & ~msk);
    }
    break;

    default:
      col[0] = vga->latch[0];
      col[1] = vga->latch[1];
      col[2] = vga->latch[2];
      col[3] = vga->latch[3];
      break;
  }

  vga->set_latches (vga, addr, col);
}

static
unsigned char vga_get_uint8_gdc (vga_t *vga, unsigned long addr)
{
  addr &= 0xffff;

  vga->latch[0] = vga->data[addr + 0x00000];
  vga->latch[1] = vga->data[addr + 0x10000];
  vga->latch[2] = vga->data[addr + 0x20000];
  vga->latch[3] = vga->data[addr + 0x30000];

  switch (vga->gdc_reg[5] & 0x08) {
    case 0x00: /* read mode 0 */
      return (vga->latch[vga->gdc_reg[4] & 0x03]);

    case 0x08: { /* read mode 1 */
      unsigned char ccare, ccmpr, cmp[4], ret;

      ccare = vga->gdc_reg[0x07];
      ccmpr = vga->gdc_reg[0x02];

      cmp[0] = (ccare & 0x01) ? ((ccmpr & 0x01) ? 0xff : 0x00) : vga->latch[0];
      cmp[1] = (ccare & 0x02) ? ((ccmpr & 0x02) ? 0xff : 0x00) : vga->latch[1];
      cmp[2] = (ccare & 0x04) ? ((ccmpr & 0x04) ? 0xff : 0x00) : vga->latch[2];
      cmp[3] = (ccare & 0x08) ? ((ccmpr & 0x08) ? 0xff : 0x00) : vga->latch[3];

      ret = cmp[0] ^ vga->latch[0];
      ret |= cmp[1] ^ vga->latch[1];
      ret |= cmp[2] ^ vga->latch[2];
      ret |= cmp[3] ^ vga->latch[3];

      return (~ret & 0xff);
    }
  }

  return (0xff);
}


int vga_screenshot (vga_t *vga, FILE *fp, unsigned mode)
{
  return (vga->screenshot (vga, fp));
}

static
void vga_update (vga_t *vga)
{
  vga->update (vga);
}

int vga_eval_mode (vga_t *vga)
{
  if (vga->gdc_reg[0x05] & 0x40) {
    vga->set_uint8 = &vga_set_uint8_chain4;
    vga->get_uint8 = &vga_get_uint8_chain4;
    pce_log (MSG_DEB, "vga: cpu mode chain4\n");
  }
  else if (vga->gdc_reg[0x05] & 0x10) {
    vga->set_uint8 = &vga_set_uint8_odd_even;
    vga->get_uint8 = &vga_get_uint8_odd_even;
    pce_log (MSG_DEB, "vga: cpu mode odd/even\n");
  }
  else {
    vga->set_uint8 = &vga_set_uint8_gdc;
    vga->get_uint8 = &vga_get_uint8_gdc;
    pce_log (MSG_DEB, "vga: cpu mode gdc\n");
  }

  if (vga->atc_reg[0x10] & 0x01) {
    if (vga->gdc_reg[0x05] & 0x40) {
      vga->update = &vga_update_vga256;
      vga->screenshot = &vga_screenshot_vga256;
      vga->set_latches = &vga_set_latches_vga256;
      pce_log (MSG_DEB, "vga: crt mode vga256\n");
    }
    else if (vga->gdc_reg[0x05] & 0x20) {
      vga->update = &vga_update_cga4;
      vga->screenshot = &vga_screenshot_cga4;
      vga->set_latches = &vga_set_latches_cga4;
      pce_log (MSG_DEB, "vga: crt mode cga4\n");
    }
    else {
      vga->update = &vga_update_ega16;
      vga->screenshot = &vga_screenshot_ega16;
      vga->set_latches = &vga_set_latches_ega16;
      pce_log (MSG_DEB, "vga: crt mode ega16\n");
    }
  }
  else {
    vga->update = &vga_update_cga_txt;
    vga->screenshot = &vga_screenshot_cga_txt;
    vga->set_latches = &vga_set_latches_cga_txt;
    pce_log (MSG_DEB, "vga: crt mode txt\n");
  }

  return (0);
}

void vga_set_mode (vga_t *vga, unsigned mode, unsigned w, unsigned h)
{
  unsigned sw, sh;

  pce_log (MSG_DEB, "vga:\tset mode %u (%u, %u)\n", mode, w, h);

  switch (mode) {
    case 0:
      trm_set_size (vga->trm, TERM_MODE_TEXT, w, h);
      break;

    case 16:
    case 256:
      if ((w == 320) && (h == 200)) {
        sw = vga->mode_320x200_w;
        sh = vga->mode_320x200_h;
      }
      else if ((w == 640) && (h == 200)) {
        sw = vga->mode_640x200_w;
        sh = vga->mode_640x200_h;
      }
      else if ((w == 640) && (h == 350)) {
        sw = vga->mode_640x350_w;
        sh = vga->mode_640x350_h;
      }
      else if ((w == 640) && (h == 480)) {
        sw = vga->mode_640x480_w;
        sh = vga->mode_640x480_h;
      }
      else {
        sw = w;
        sh = h;
      }

      trm_set_size (vga->trm, TERM_MODE_GRAPH, sw, sh);
      pce_smap_free (&vga->smap);
      pce_smap_init (&vga->smap, w, h, sw, sh);
      break;
  }

  vga->mode = mode;
  vga->mode_w = w;
  vga->mode_h = h;

  vga->dirty = 1;
}

void vga_set_pos (vga_t *vga, unsigned pos)
{
  unsigned x, y;
  unsigned rofs;

  vga->crtc_pos = pos;

  if (pos < vga->crtc_ofs) {
    return;
  }

  pos -= vga->crtc_ofs;

  rofs = 2 * vga->crtc_reg[0x13];
  if (rofs == 0) {
    rofs = 80;
  }

  y = pos / rofs;
  x = pos % rofs;

  if ((x >= vga->mode_w) || (y >= vga->mode_h)) {
    return;
  }

  trm_set_pos (vga->trm, x, y);
}

void vga_set_crs (vga_t *vga, unsigned y1, unsigned y2)
{
  if (vga->mode == 0) {
    if (y1 > 13) {
      trm_set_crs (vga->trm, 0, 0, 0);
      return;
    }

    if ((y2 < y1) || (y2 > 13)) {
      y2 = 13;
    }

    y1 = (255 * y1 + 6) / 13;
    y2 = (255 * y2 + 6) / 13;

    trm_set_crs (vga->trm, y1, y2, 1);
  }
}

void vga_set_page_ofs (vga_t *vga, unsigned ofs)
{
  if (vga->crtc_ofs == ofs) {
    return;
  }

  vga->crtc_ofs = ofs;

  vga->dirty = 1;
}

void vga_mem_set_uint8 (vga_t *vga, unsigned long addr, unsigned char val)
{
  if (addr < vga->base) {
    return;
  }

  addr -= vga->base;

  if (addr >= vga->size) {
    return;
  }

  vga->set_uint8 (vga, addr, val);
}

void vga_mem_set_uint16 (vga_t *vga, unsigned long addr, unsigned short val)
{
  vga_mem_set_uint8 (vga, addr, val & 0xff);
  vga_mem_set_uint8 (vga, addr + 1, val >> 8);
}

unsigned char vga_mem_get_uint8 (vga_t *vga, unsigned long addr)
{
  if (addr < vga->base) {
    return (0xff);
  }

  addr -= vga->base;

  if (addr >= vga->size) {
    return (0xff);
  }

  return (vga->get_uint8 (vga, addr));
}

unsigned short vga_mem_get_uint16 (vga_t *vga, unsigned long addr)
{
  unsigned short ret;

  ret = vga_mem_get_uint8 (vga, addr) & 0xff;
  ret |= (vga_mem_get_uint8 (vga, addr + 1) & 0xff) << 8;

  return (ret);
}


void vga_gdc_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
  switch (reg) {
    case 0x00: /* set / reset */
      vga->gdc_reg[0x00] = val & 0x0f;
      break;

    case 0x01: /* enable set / reset */
      vga->gdc_reg[0x01] = val & 0x0f;
      break;

    case 0x02: /* color compare */
      vga->gdc_reg[0x02] = val & 0x0f;
      break;

    case 0x03: /* function select */
      vga->gdc_reg[0x03] = val & 0x1f;
      break;

    case 0x04: /* read plane select */
      vga->gdc_reg[0x04] = val & 0x0f;
      break;

    case 0x05: /* gdc mode */
      vga->gdc_reg[0x05] = val & 0x7f;
      vga_eval_mode (vga);
      break;

    case 0x06: /* misc */
      vga->gdc_reg[0x06] = val & 0x0f;
      switch ((val >> 2) & 0x03) {
        case 0x00:
          vga->base = 0x00000;
          vga->size = 0x20000;
          break;
        case 0x01:
          vga->base = 0x00000;
          vga->size = 0x10000;
          break;
        case 0x02:
          vga->base = 0x10000;
          vga->size = 0x08000;
          break;
        case 0x03:
          vga->base = 0x18000;
          vga->size = 0x08000;
          break;
      }
      break;

    case 0x07: /* color don't care */
      vga->gdc_reg[0x07] = val & 0x0f;
      break;

    case 0x08: /* bit mask */
      vga->gdc_reg[0x08] = val & 0xff;
      break;
  }
}

unsigned char vga_gdc_get_reg (vga_t *vga, unsigned reg)
{
  return (0xff);
}

void vga_atc_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
  if (reg >= 0x15) {
    return;
  }

  if (reg < 16) {
    if ((vga->reg->data[0x10] & 0x20) == 0) {
      unsigned char rgb[3];

      vga->atc_reg[reg] = val & 0x3f;

      vga_get_rgb_atc (vga, reg, rgb);
      trm_set_map (vga->trm, reg, rgb[0] << 8, rgb[1] << 8, rgb[2] << 8);

      vga->dirty = 1;
    }
    return;
  }

  switch (reg) {
    case 0x10: /* mode control */
      vga->atc_reg[reg] = val;
      vga_eval_mode (vga);
      break;

    case 0x11: /* overscan color */
      vga->atc_reg[reg] = val;
      break;

    case 0x12: /* color plane enable */
      vga->atc_reg[reg] = val & 0x0f;
      break;

    case 0x13: /* horizontal pixel panning */
      vga->atc_reg[reg] = val & 0x0f;
      break;

    case 0x14: /* color select */
      vga->atc_reg[reg] = val & 0x0f;
      break;

  }
}

unsigned char vga_atc_get_reg (vga_t *vga, unsigned reg)
{
  if (reg <= 0x14) {
    return (vga->atc_reg[reg]);
  }

  return (0xff);
}

static
void vga_dac_set_col (vga_t *vga, unsigned char val)
{
  unsigned idx;

  idx = vga->dac_idx % 768;

  vga->dac_reg[idx] = val;
  vga->dac_idx = (idx + 1) % 768;

  vga_dac_map_entry (vga, idx / 3);

  vga->dirty = 1;
}

static
unsigned char vga_dac_get_col (vga_t *vga)
{
  unsigned      idx;
  unsigned char val;

  idx = vga->dac_idx % 768;

  val = vga->dac_reg[idx];
  vga->dac_idx = (idx + 1) % 768;

  return (val);
}

void vga_ts_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
  if (reg >= 5) {
    return;
  }

  vga->ts_reg[reg] = val;

  switch (reg) {
    case 0x00: /* reset */
      vga->ts_reg[reg] = val & 0x03;

      if (val & 0x02) {
        ; /* sync reset */
      }
      else if (val & 0x01) {
        ; /* async reset */
      }
      break;

    case 0x01: /* TS mode */
      vga->ts_reg[reg] = val;
      break;

    case 0x02: /* write plane mask */
      vga->ts_reg[reg] = val & 0x0f;
      break;

    case 0x03: /* font select */
      vga->ts_reg[reg] = val & 0x3f;
      break;

    case 0x04: /* memory mode */
      vga->ts_reg[reg] = val & 0x0f;
      vga_eval_mode (vga);
      break;
  }
}

unsigned char vga_ts_get_reg (vga_t *vga, unsigned reg)
{
  if (reg < 5) {
    return (vga->ts_reg[reg]);
  }

  return (0xff);
}

void vga_crtc_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
  if (reg > 24) {
    return;
  }

  vga->crtc_reg[reg] = val;

  switch (reg) {
    case 0x0a:
    case 0x0b:
      vga_set_crs (vga, vga->crtc_reg[0x0a], vga->crtc_reg[0x0b]);
      break;

    case 0x0c:
/*      vga_set_page_ofs (vga, (vga->crtc_reg[0x0c] << 8) | val); */
      break;

    case 0x0d:
      vga_set_page_ofs (vga, (vga->crtc_reg[0x0c] << 8) | val);
      break;

    case 0x0e:
/*      vga_set_pos (vga, (val << 8) | (vga->crtc_reg[0x0f] & 0xff)); */
      break;

    case 0x0f:
      vga_set_pos (vga, (vga->crtc_reg[0x0e] << 8) | val);
      break;
  }
}

unsigned char vga_crtc_get_reg (vga_t *vga, unsigned reg)
{
  if (reg > 24) {
    return (0xff);
  }

  return (vga->crtc_reg[reg]);
}

unsigned char vga_get_input_state_1 (vga_t *vga)
{
  static unsigned cnt = 0;
  unsigned char   val;

  vga->atc_index = 1;

  cnt += 1;

  val = vga->reg->data[0x2a];

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
    if (cnt >= (8 * vga->mode_h)) {
      cnt = 0;
      val |= (0x80 | 0x08);
      val &= ~0x01;

      if (vga->dirty) {
        vga->update (vga);
        vga->dirty = 0;
      }
    }
  }

  vga->reg->data[0x2a] = val;

  return (val);
}

void vga_reg_set_uint8 (vga_t *vga, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0x10: /* 0x3c0: ATC index/data */
      if (vga->atc_index) {
        vga->atc_index = 0;
        vga->reg->data[0x10] = val;
      }
      else {
        vga->atc_index = 1;
        vga_atc_set_reg (vga, vga->reg->data[0x10], val);
      }
      break;

    case 0x12: /* 0x3c2: misc output register */
    case 0x1c:
      vga->reg->data[0x1c] = val;
      break;

    case 0x14: /* 0x3c4: TS index */
      vga->reg->data[0x14] = val;
      break;

    case 0x15: /* 0x3c5: TS data */
      vga_ts_set_reg (vga, vga->reg->data[0x14], val);
      break;

    case 0x16: /* 0x3c6: DAC color mask */
      vga->dac_col_msk = val & 0xff;
      break;

    case 0x17: /* 0x3c7: DAC write address */
      vga->dac_idx = 3 * (val & 0xff);
      vga->dac_state = 0x00;
      break;

    case 0x18: /* 0x3c8: DAC read address */
      vga->dac_idx = 3 * (val & 0xff);
      vga->dac_state = 0x03;
      break;

    case 0x19: /* 0x3c9: DAC color value */
      vga_dac_set_col (vga, val);
      break;

    case 0x1e: /* 0x3ce: GDC index */
      vga->reg->data[0x1e] = val;
      break;

    case 0x1f: /* 0x3cf: GDC data */
      vga_gdc_set_reg (vga, vga->reg->data[0x1e], val);
      break;

    case 0x20: /* 0x3d0: pce extension */
      switch (val & 0x7f) {
        case 0x00:
        case 0x01:
          vga_set_mode (vga, 0, 40, 25);
          break;

        case 0x02:
        case 0x03:
          vga_set_mode (vga, 0, 80, 25);
          vga_set_ega_dac (vga);
          break;

        case 0x04:
        case 0x05:
          vga_set_mode (vga, 16, 320, 200);
          break;

        case 0x06:
          vga_set_mode (vga, 16, 640, 200);
          break;

        case 0x07:
          vga_set_mode (vga, 0, 80, 25);
          break;

        case 0x0d:
          vga_set_mode (vga, 16, 320, 200);
          break;

        case 0x0e:
          vga_set_mode (vga, 16, 640, 200);
          break;

        case 0x0f:
        case 0x10:
          vga_set_mode (vga, 16, 640, 350);
          break;

        case 0x11:
        case 0x12:
          vga_set_mode (vga, 16, 640, 480);
          break;

        case 0x13:
          vga_set_mode (vga, 256, 320, 200);
          break;

        default:
          pce_log (MSG_INF, "vga: unknown mode (%u)\n", val);
          break;
      }
      break;

    case 0x24: /* 0x3d4: CRTC index */
      vga->reg->data[0x24] = val;
      break;

    case 0x25: /* 0x3d5: CRTC data */
      vga_crtc_set_reg (vga, vga->reg->data[0x24], val);
      break;
  }
}

void vga_reg_set_uint16 (vga_t *vga, unsigned long addr, unsigned short val)
{
  vga_reg_set_uint8 (vga, addr, val & 0xff);
  vga_reg_set_uint8 (vga, addr + 1, val >> 8);
}

unsigned char vga_reg_get_uint8 (vga_t *vga, unsigned long addr)
{
  switch (addr) {
    case 0x11: /* 0x3c1: ATC data */
      if (vga->atc_index == 0) {
        return (vga_atc_get_reg (vga, vga->reg->data[0x10]));
      }
      return (0xff);

    case 0x12: /* 0x3c2: input state 0  */
      return (0x00);

    case 0x14: /* 0x3c4 */
      return (vga->reg->data[addr]);

    case 0x15: /* 0x3c5 */
      return (vga_ts_get_reg (vga, vga->reg->data[0x14]));

    case 0x17: /* 0x3c7: DAC state */
      return (vga->dac_state);

    case 0x19: /* 0x3c9: DAC color value */
      return (vga_dac_get_col (vga));
      break;

    case 0x1c: /* 0x3cc: misc output register */
      return (vga->reg->data[0x1c]);

    case 0x1e: /* 0x3ce: GDC index */
      return (vga->reg->data[addr]);

    case 0x1f: /* 0x3cf: GDC data */
      return (vga_gdc_get_reg (vga, vga->reg->data[0x1e]));

    case 0x24: /* 0x3d4 CRTC index */
      return (vga->reg->data[0x24]);

    case 0x25: /* 0x3d5 CRTC data */
      return (vga_crtc_get_reg (vga, vga->reg->data[0x24]));

    case 0x2a: /* 0x3da: input state 1 */
      return (vga_get_input_state_1 (vga));

    default:
      return (0xff);
  }
}

unsigned short vga_reg_get_uint16 (vga_t *vga, unsigned long addr)
{
  unsigned short ret;

  ret = vga_reg_get_uint8 (vga, addr);
  ret |= vga_reg_get_uint8 (vga, addr + 1) << 8;

  return (ret);
}

void vga_clock (vga_t *vga, unsigned long cnt)
{
  static unsigned upd_cnt = 0;

  if (vga->dirty) {
    upd_cnt += 1;

    if (upd_cnt > 16) {
      upd_cnt = 0;
      vga->update (vga);
      vga->dirty = 0;
    }
  }
}
