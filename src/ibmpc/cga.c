/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/cga.c                                            *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: cga.c,v 1.8 2003/08/30 03:08:53 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


video_t *cga_new (terminal_t *trm, ini_sct_t *sct)
{
  unsigned i;
  unsigned iobase, membase, memsize;
  cga_t    *cga;

  cga = (cga_t *) malloc (sizeof (cga_t));
  if (cga == NULL) {
    return (NULL);
  }

  pce_video_init (&cga->vid);

  cga->vid.ext = cga;
  cga->vid.del = (pce_video_del_f) &cga_del;
  cga->vid.get_mem = (pce_video_get_mem_f) &cga_get_mem;
  cga->vid.get_reg = (pce_video_get_reg_f) &cga_get_reg;
  cga->vid.prt_state = (pce_video_prt_state_f) &cga_prt_state;

  for (i = 0; i < 16; i++) {
    cga->crtc_reg[i] = 0;
  }

  ini_get_uint (sct, "io", &iobase, 0x3d4);
  ini_get_uint (sct, "membase", &membase, 0xb8000);
  ini_get_uint (sct, "memsize", &memsize, 16384);

  memsize = (memsize < 16384) ? 16384 : memsize;

  pce_log (MSG_INF, "video:\tCGA io=0x%04x membase=0x%05x memsize=0x%05x\n",
    iobase, membase, memsize
  );

  cga->mem = mem_blk_new (membase, memsize, 1);
  cga->mem->ext = cga;
  cga->mem->set_uint8 = (seta_uint8_f) &cga_mem_set_uint8;
  cga->mem->set_uint16 = (seta_uint16_f) &cga_mem_set_uint16;
  mem_blk_init (cga->mem, 0x00);

  cga->reg = mem_blk_new (iobase, 16, 1);
  cga->reg->ext = cga;
  cga->reg->set_uint8 = (seta_uint8_f) &cga_reg_set_uint8;
  cga->reg->set_uint16 = (seta_uint16_f) &cga_reg_set_uint16;
  cga->reg->get_uint8 = (geta_uint8_f) &cga_reg_get_uint8;
  cga->reg->get_uint16 = (geta_uint16_f) &cga_reg_get_uint16;
  mem_blk_init (cga->reg, 0x00);

  cga->trm = trm;

  cga->crtc_pos = 0;
  cga->crtc_ofs = 0;

  cga->crs_on = 1;

  cga->pal = 0;
  cga->palette[0] = 0;
  cga->palette[1] = 3;
  cga->palette[2] = 5;
  cga->palette[3] = 15;

  cga->mode = 0;
  trm_set_size (trm, TERM_MODE_TEXT, 80, 25);

  return (&cga->vid);
}

void cga_del (cga_t *cga)
{
  if (cga != NULL) {
    mem_blk_del (cga->mem);
    mem_blk_del (cga->reg);
    free (cga);
  }
}

void cga_clock (cga_t *cga)
{
}

void cga_prt_state (cga_t *cga, FILE *fp)
{
  unsigned i;
  unsigned x, y;

  if (cga->crtc_pos < cga->crtc_ofs) {
    x = 0;
    y = 0;
  }
  else {
    x = (cga->crtc_pos - cga->crtc_ofs) % 80;
    y = (cga->crtc_pos - cga->crtc_ofs) / 80;
  }

  fprintf (fp, "CGA: MODE=%u  OFS=%04X  POS=%04X[%u/%u]  CRS=%s  BG=%02X  PAL=%u\n",
    cga->mode, cga->crtc_ofs, cga->crtc_pos, x, y,
    (cga->crs_on) ? "ON" : "OFF",
    cga->reg->data[5] & 0x0f, (cga->reg->data[5] >> 5) & 1
  );

  fprintf (fp, "REG: 3D8=%02X  3D9=%02X  3DA=%02X  PAL=%u:[%02X %02X %02X %02X]\n",
    cga->reg->data[4], cga->reg->data[5], cga->reg->data[6],
    cga->pal, cga->palette[0], cga->palette[1], cga->palette[2], cga->palette[3]
  );

  fprintf (fp, "CRTC=[%02X", cga->crtc_reg[0]);
  for (i = 1; i < 15; i++) {
    if (i == 8) {
      fputs ("-", fp);
    }
    else {
      fputs (" ", fp);
    }
    fprintf (fp, "%02X", cga->crtc_reg[i]);
  }
  fputs ("]\n", fp);

  fflush (fp);
}

mem_blk_t *cga_get_mem (cga_t *cga)
{
  return (cga->mem);
}

mem_blk_t *cga_get_reg (cga_t *cga)
{
  return (cga->reg);
}

static
void cga_mode0_update (cga_t *cga)
{
  unsigned i;
  unsigned x, y;
  unsigned fg, bg;

  i = (cga->crtc_ofs << 1) & 0x3fff;

  for (y = 0; y < 25; y++) {
    for (x = 0; x < 80; x++) {
      fg = cga->mem->data[i + 1] & 0x0f;
      bg = (cga->mem->data[i + 1] & 0xf0) >> 4;

      trm_set_col (cga->trm, fg, bg);
      trm_set_chr (cga->trm, x, y, cga->mem->data[i]);

      i = (i + 2) & 0x3fff;
    }
  }
}

static
void cga_mode1_update (cga_t *cga)
{
  unsigned      x, y, i;
  unsigned      sx, sy;
  unsigned      val0, val1;
  unsigned char *mem0, *mem1;

  mem0 = cga->mem->data;
  mem1 = cga->mem->data + 8192;

  sy = 0;
  for (y = 0; y < 100; y++) {
    sx = 0;

    for (x = 0; x < 80; x++) {
      val0 = mem0[x];
      val1 = mem1[x];

      for (i = 0; i < 4; i++) {
        trm_set_col (cga->trm, cga->palette[(val0 >> 6) & 0x03], 0);
        trm_set_pxl (cga->trm, sx, sy);
        trm_set_pxl (cga->trm, sx + 1, sy);
        trm_set_pxl (cga->trm, sx, sy + 1);
        trm_set_pxl (cga->trm, sx + 1, sy + 1);

        trm_set_col (cga->trm, cga->palette[(val1 >> 6) & 0x03], 0);
        trm_set_pxl (cga->trm, sx, sy + 2);
        trm_set_pxl (cga->trm, sx + 1, sy + 2);
        trm_set_pxl (cga->trm, sx, sy + 3);
        trm_set_pxl (cga->trm, sx + 1, sy + 3);

        val0 <<= 2;
        val1 <<= 2;
        sx += 2;
      }
    }

    mem0 += 80;
    mem1 += 80;
    sy += 4;
  }
}

void cga_update (cga_t *cga)
{
  switch (cga->mode) {
    case 0:
      cga_mode0_update (cga);
      break;

    case 1:
      cga_mode1_update (cga);
      break;
  }
}

void cga_set_pos (cga_t *cga, unsigned pos)
{
  cga->crtc_pos = pos;

  if (cga->mode == 0) {
    if (pos < cga->crtc_ofs) {
      return;
    }

    pos -= cga->crtc_ofs;

    if (pos >= 2000) {
      return;
    }

    trm_set_pos (cga->trm, pos % 80, pos / 80);
  }
}

void cga_set_crs (cga_t *cga, unsigned y1, unsigned y2)
{
  if (cga->mode == 0) {
    if (y1 < y2) {
      y1 = 0;
      y2 = 7;
    }

    y1 = (y1 <= 7) ? (7 - y1) : 0;
    y2 = (y2 <= 7) ? (7 - y2) : 0;

    y1 = (y1 << 5) | (y1 << 2) | (y1 >> 1);
    y2 = (y2 << 5) | (y2 << 2) | (y2 >> 1);

    trm_set_crs (cga->trm, y1, y2);
  }
}

void cga_set_page_ofs (cga_t *cga, unsigned ofs)
{
  if (cga->crtc_ofs == ofs) {
    return;
  }

  cga->crtc_ofs = ofs;

  if (cga->mode == 0) {
    cga_update (cga);
  }
}

void cga_set_palette (cga_t *cga, unsigned pal, unsigned char bg)
{
  pal &= 1;

  if (pal == cga->pal) {
    return;
  }

  cga->pal = pal;

  switch (pal & 1) {
    case 0:
      cga->palette[0] = bg;
      cga->palette[1] = 3;
      cga->palette[2] = 5;
      cga->palette[3] = 15;
      break;

    case 1:
      cga->palette[0] = bg;
      cga->palette[1] = 2;
      cga->palette[2] = 4;
      cga->palette[3] = 14;
      break;
  }

  cga_update (cga);
}

void cga_set_mode (cga_t *cga, unsigned char mode)
{
  unsigned newmode;

  if ((mode & 0x02) == 0) {
    newmode = 0;
  }
  else if ((mode & 0x10) == 0) {
    newmode = 1;
  }
  else {
    newmode = 2;
  }

//  fprintf (stderr, "cga: set mode %u -> %u\n", cga->mode, newmode);

  if (newmode == cga->mode) {
    return;
  }

  cga->mode = newmode;

  switch (newmode) {
    case 0:
      trm_set_size (cga->trm, TERM_MODE_TEXT, 80, 25);
      break;

    case 1:
      trm_set_size (cga->trm, TERM_MODE_GRAPH, 640, 400);
      break;

    case 2:
      trm_set_size (cga->trm, TERM_MODE_GRAPH, 640, 400);
      break;
  }

  cga_update (cga);
}

void cga_mode0_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
  unsigned      x, y;
  unsigned char c, a;

  if (cga->mem->data[addr] == val) {
    return;
  }

  cga->mem->data[addr] = val;

  if (addr & 1) {
    c = cga->mem->data[addr - 1];
    a = val;
  }
  else {
    c = val;
    a = cga->mem->data[addr + 1];
  }

  if (addr < (cga->crtc_ofs << 1)) {
    return;
  }

  addr -= (cga->crtc_ofs << 1);

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

  trm_set_col (cga->trm, a & 0x0f, (a & 0xf0) >> 4);
  trm_set_chr (cga->trm, x, y, c);
}

void cga_mode0_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
  unsigned      x, y;
  unsigned char c, a;

  if (addr & 1) {
    cga_mem_set_uint8 (cga, addr, val & 0xff);

    if (addr < cga->mem->end) {
      cga_mem_set_uint8 (cga, addr + 1, val >> 8);
    }

    return;
  }

  c = val & 0xff;
  a = (val >> 8) & 0xff;

  if ((cga->mem->data[addr] == c) && (cga->mem->data[addr + 1] == a)) {
    return;
  }

  cga->mem->data[addr] = c;
  cga->mem->data[addr + 1] = a;

  if (addr < (cga->crtc_ofs << 1)) {
    return;
  }

  addr -= (cga->crtc_ofs << 1);

  if (addr >= 4000) {
    return;
  }

  x = (addr >> 1) % 80;
  y = (addr >> 1) / 80;

  trm_set_col (cga->trm, a & 0x0f, (a & 0xf0) >> 4);
  trm_set_chr (cga->trm, x, y, c);
}

void cga_mode1_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
  unsigned      i;
  unsigned      x, y, sx, sy;
  unsigned char old;

  old = cga->mem->data[addr];

  if (old == val) {
    return;
  }

  cga->mem->data[addr] = val;

  if (addr < 8192) {
    x = 4 * (addr % 80);
    y = 2 * (addr / 80);
  }
  else {
    x = 4 * ((addr - 8192) % 80);
    y = 2 * ((addr - 8192) / 80) + 1;
  }

  if (y >= 200) {
    return;
  }

  sx = 2 * x;
  sy = 2 * y;

  for (i = 0; i < 4; i++) {
    unsigned col;

    if ((old ^ val) & 0xc0) {
      col = (val >> 6) & 0x03;
      trm_set_col (cga->trm, cga->palette[col], 0);

      trm_set_pxl (cga->trm, sx, sy);
      trm_set_pxl (cga->trm, sx + 1, sy);
      trm_set_pxl (cga->trm, sx, sy + 1);
      trm_set_pxl (cga->trm, sx + 1, sy + 1);
    }

    old <<= 2;
    val <<= 2;
    sx += 2;
  }
}

void cga_mode2_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
  unsigned      i;
  unsigned      x, y, sx, sy;
  unsigned char old;

  old = cga->mem->data[addr];

  if (old == val) {
    return;
  }

  cga->mem->data[addr] = val;

  if (addr < 8192) {
    x = 8 * (addr % 80);
    y = 2 * (addr / 80);
  }
  else {
    x = 8 * ((addr - 8192) % 80);
    y = 2 * ((addr - 8192) / 80) + 1;
  }

  sx = x;
  sy = 2 * y;

  for (i = 0; i < 8; i++) {
    unsigned col;

    if ((old ^ val) & 0x80) {
      col = (val >> 7) & 0x01;
      trm_set_col (cga->trm, col ? 15 : 0, 0);
      trm_set_pxl (cga->trm, sx, sy);
      trm_set_pxl (cga->trm, sx, sy + 1);
    }

    old <<= 1;
    val <<= 1;
    sx += 1;
  }
}

void cga_mem_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
  switch (cga->mode) {
    case 0:
      cga_mode0_set_uint8 (cga, addr, val);
      break;

    case 1:
      cga_mode1_set_uint8 (cga, addr, val);
      break;

    case 2:
      cga_mode2_set_uint8 (cga, addr, val);
      break;
  }
}

void cga_mem_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
  switch (cga->mode) {
    case 0:
      cga_mode0_set_uint16 (cga, addr, val);
      break;

    case 1:
      cga_mode1_set_uint8 (cga, addr, val);
      if (addr < cga->mem->end) {
        cga_mode1_set_uint8 (cga, addr + 1, val >> 8);
      }
      break;

    case 2:
      cga_mode2_set_uint8 (cga, addr, val);
      if (addr < cga->mem->end) {
        cga_mode2_set_uint8 (cga, addr + 1, val >> 8);
      }
      break;
  }
}

void cga_crtc_set_reg (cga_t *cga, unsigned reg, unsigned char val)
{
  if (reg > 15) {
    return;
  }

  cga->crtc_reg[reg] = val;

  switch (reg) {
    case 0x0a:
    case 0x0b:
      cga_set_crs (cga, cga->crtc_reg[0x0b], cga->crtc_reg[0x0a]);
      break;

    case 0x0c:
      cga_set_page_ofs (cga, (cga->crtc_reg[0x0c] << 8) | val);
      break;

    case 0x0d:
      cga_set_page_ofs (cga, (cga->crtc_reg[0x0c] << 8) | val);
      break;

    case 0x0e:
//      cga_set_pos (cga, (val << 8) | (cga->crtc_reg[0x0f] & 0xff));
      break;

    case 0x0f:
      cga_set_pos (cga, (cga->crtc_reg[0x0e] << 8) | val);
      break;
  }
}

unsigned char cga_crtc_get_reg (cga_t *cga, unsigned reg)
{
  if (reg > 15) {
    return (0xff);
  }

  return (cga->crtc_reg[reg]);
}

void cga_reg_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
  cga->reg->data[addr] = val;

  switch (addr) {
    case 0x01:
      cga_crtc_set_reg (cga, cga->reg->data[0], val);
      break;

    case 0x04:
      cga_set_mode (cga, val);
      break;

    case 0x05:
      cga_set_palette (cga, (val >> 5) ^ 1, val & 0x0f);
      break;
  }
}

void cga_reg_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
  cga_reg_set_uint8 (cga, addr, val & 0xff);

  if (addr < cga->reg->end) {
    cga_reg_set_uint8 (cga, addr + 1, val >> 8);
  }
}

unsigned char cga_reg_get_uint8 (cga_t *cga, unsigned long addr)
{
  static unsigned cnt = 0;

  switch (addr) {
    case 0x00:
      return (cga->reg->data[0]);

    case 0x01:
      return (cga_crtc_get_reg (cga, cga->reg->data[0]));

    case 0x06:
      cnt += 1;
      if ((cnt & 7) == 0) {
        cga->reg->data[6] ^= 1;
      }
      if (cnt >= 64) {
        cnt = 0;
        cga->reg->data[6] ^= 8;
      }

      return (cga->reg->data[6]);

    default:
      return (0xff);
  }
}

unsigned short cga_reg_get_uint16 (cga_t *cga, unsigned long addr)
{
  unsigned short ret;

  ret = cga_reg_get_uint8 (cga, addr);

  if (addr < cga->reg->end) {
    ret |= cga_reg_get_uint8 (cga, addr + 1) << 8;
  }

  return (ret);
}
