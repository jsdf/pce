/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/cga.c                                            *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-23 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: cga.c,v 1.1 2003/04/23 12:48:42 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


cga_t *cga_new (FILE *fp)
{
  unsigned i;
  cga_t    *cga;

  cga = (cga_t *) malloc (sizeof (cga_t));
  if (cga == NULL) {
    return (NULL);
  }

  for (i = 0; i < 16; i++) {
    cga->crtc_reg[i] = 0;
  }

  trm_init (&cga->trm, fp);

  cga->mem = mem_blk_new (0xb8000, 16384, 1);
  cga->mem->ext = cga;
  cga->mem->set_uint8 = (seta_uint8_f) &cga_mem_set_uint8;
  cga->mem->set_uint16 = (seta_uint16_f) &cga_mem_set_uint16;

  cga->crtc = mem_blk_new (0x3d4, 16, 1);
  cga->crtc->ext = cga;
  cga->crtc->set_uint8 = (seta_uint8_f) &cga_crtc_set_uint8;
  cga->crtc->set_uint16 = (seta_uint16_f) &cga_crtc_set_uint16;
  cga->crtc->get_uint8 = (geta_uint8_f) &cga_crtc_get_uint8;

  cga->crtc_mode = 0;
  cga->crtc_pos = 0;
  cga->crtc_ofs = 0;

  return (cga);
}

void cga_del (cga_t *cga)
{
  if (cga != NULL) {
    trm_free (&cga->trm);
    mem_blk_del (cga->mem);
    mem_blk_del (cga->crtc);
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

  fprintf (fp, "CGA: MODE=%02X  OFS=%04X  POS=%04X[%u/%u]\n",
    cga->crtc_mode, cga->crtc_ofs, cga->crtc_pos, x, y
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

void cga_update (cga_t *cga)
{
  unsigned i;
  unsigned x, y;

  trm_clr_scn (&cga->trm);

  i = (cga->crtc_ofs << 1) & 0x3ffff;

  for (y = 0; y < 25; y++) {
    for (x = 0; x < 80; x++) {
      trm_set_attr_col (&cga->trm, cga->mem->data[i + 1]);
      trm_set_chr_xy (&cga->trm, x, y, cga->mem->data[i]);
      i = (i + 2) & 0x3fff;
    }
  }
}

void cga_set_pos (cga_t *cga, unsigned pos)
{
  cga->crtc_pos = pos;

  if (pos < cga->crtc_ofs) {
    return;
  }

  pos -= cga->crtc_ofs;

  if (pos >= 2000) {
    return;
  }

  trm_set_pos (&cga->trm, pos % 80, pos / 80);
}

void cga_set_page_ofs (cga_t *cga, unsigned ofs)
{
  if (cga->crtc_ofs == ofs) {
    return;
  }

  fprintf (stderr, "cga: set page offset to %04X\n", ofs);

  cga->crtc_ofs = ofs;

  cga_update (cga);
}

void cga_mem_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
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

  trm_set_attr_col (&cga->trm, a);
  trm_set_chr_xy (&cga->trm, x, y, c);
}

void cga_mem_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
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

  trm_set_attr_col (&cga->trm, a);
  trm_set_chr_xy (&cga->trm, x, y, c);
}

void cga_crtc_set_reg (cga_t *cga, unsigned reg, unsigned char val)
{
  if (reg > 15) {
    return;
  }

  cga->crtc_reg[reg] = val;

  switch (reg) {
    case 0x0c:
      cga_set_page_ofs (cga, (val << 8) | (cga->crtc_reg[0x0d] & 0xff));
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

void cga_crtc_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
  cga->crtc->data[addr] = val;

  switch (addr) {
    case 0x01:
      cga_crtc_set_reg (cga, cga->crtc->data[0], val);
      break;

    case 0x04:
      cga->crtc_mode = val;
      break;
  }
}

void cga_crtc_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
  cga_crtc_set_uint8 (cga, addr, val & 0xff);

  if (addr < cga->crtc->end) {
    cga_crtc_set_uint8 (cga, addr + 1, val >> 8);
  }
}

unsigned char cga_crtc_get_uint8 (cga_t *cga, unsigned long addr)
{
  static unsigned cnt = 0;

  switch (addr) {
    case 0x00:
      return (cga->crtc->data[0]);

    case 0x01:
      return (cga_crtc_get_reg (cga, cga->crtc->data[0]));

    case 0x04:
      return (cga->crtc->data[addr]);
      break;

    case 0x06:
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
