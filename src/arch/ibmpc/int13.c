/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/int13.c                                     *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-01-31 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#include "pce.h"


static
unsigned dsks_get_hd_cnt (disks_t *dsks)
{
  unsigned i, n;

  n = 0;

  for (i = 0; i < dsks->cnt; i++) {
    if (dsks->dsk[i]->drive & 0x80) {
      n += 1;
    }
  }

  return (n);
}

#if 0
static
void dsk_int13_log (disks_t *dsks, e8086_t *cpu, FILE *fp)
{
  pce_log (MSG_DEB,
    "int 13 func %02X: %04X:%04X  AX=%04X  BX=%04X  CX=%04X  DX=%04X  ES=%04X\n",
    e86_get_reg8 (cpu, E86_REG_AH),
    e86_get_mem16 (cpu, e86_get_ss (cpu), e86_get_sp (cpu) + 2),
    e86_get_mem16 (cpu, e86_get_ss (cpu), e86_get_sp (cpu)),
    e86_get_reg16 (cpu, E86_REG_AX),
    e86_get_reg16 (cpu, E86_REG_BX),
    e86_get_reg16 (cpu, E86_REG_CX),
    e86_get_reg16 (cpu, E86_REG_DX),
    e86_get_sreg (cpu, E86_REG_ES)
  );
}
#endif

static
void dsk_int13_set_status (disks_t *dsks, e8086_t *cpu, unsigned val)
{
  e86_set_ah (cpu, val);
  e86_set_mem8 (cpu, 0x40, 0x41, val);
  e86_set_cf (cpu, val != 0);
}

static
void dsk_int13_02 (disks_t *dsks, e8086_t *cpu)
{
  unsigned       i;
  unsigned long  blk_i, blk_n;
  unsigned       c, h, s;
  unsigned short seg, ofs;
  unsigned char  buf[512];
  disk_t         *dsk;

  dsk = dsks_get_disk (dsks, e86_get_dl (cpu));
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 0x01);
    return;
  }

  blk_n = e86_get_al (cpu);

  c = e86_get_ch (cpu) | ((e86_get_cl (cpu) & 0xc0) << 2);
  h = e86_get_dh (cpu);
  s = e86_get_cl (cpu) & 0x3f;

  seg = e86_get_es (cpu);
  ofs = e86_get_bx (cpu);

  if (dsk_get_lba (dsk, c, h, s, &blk_i)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  while (blk_n > 0) {
    if (dsk_read_lba (dsk, buf, blk_i, 1)) {
      dsk_int13_set_status (dsks, cpu, 0x01);
      return;
    }

    for (i = 0; i < 512; i++) {
      e86_set_mem8 (cpu, seg, ofs, buf[i]);
      ofs = (ofs + 1) & 0xffff;
    }

    blk_n -= 1;
    blk_i += 1;
  }

  dsk_int13_set_status (dsks, cpu, 0x00);
}

static
void dsk_int13_03 (disks_t *dsks, e8086_t *cpu)
{
  unsigned       i;
  unsigned long  blk_i, blk_n;
  unsigned       c, h, s;
  unsigned short seg, ofs;
  unsigned char  buf[512];
  disk_t         *dsk;

  dsk = dsks_get_disk (dsks, e86_get_dl (cpu));
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 0x01);
    return;
  }

  if (dsk->readonly) {
    dsk_int13_set_status (dsks, cpu, 0x03);
    return;
  }

  blk_n = e86_get_al (cpu);

  c = e86_get_ch (cpu) | ((e86_get_cl (cpu) & 0xc0) << 2);
  h = e86_get_dh (cpu);
  s = e86_get_cl (cpu) & 0x3f;

  seg = e86_get_es (cpu);
  ofs = e86_get_bx (cpu);

  if (dsk_get_lba (dsk, c, h, s, &blk_i)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  while (blk_n > 0) {
    for (i = 0; i < 512; i++) {
      buf[i] = e86_get_mem8 (cpu, seg, ofs);
      ofs = (ofs + 1) & 0xffff;
    }

    if (dsk_write_lba (dsk, buf, blk_i, 1)) {
      dsk_int13_set_status (dsks, cpu, 0x01);
      return;
    }

    blk_n -= 1;
    blk_i += 1;
  }

  dsk_int13_set_status (dsks, cpu, 0x00);
}

static
void dsk_int13_05 (disks_t *dsks, e8086_t *cpu)
{
  unsigned       i, n;
  unsigned       d, c, h;
  unsigned char  fill;
  unsigned char  buf[512];
  disk_t         *dsk;

  d = e86_get_dl (cpu);

  dsk = dsks_get_disk (dsks, d);
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 0x01);
    return;
  }

  if (dsk->readonly) {
    dsk_int13_set_status (dsks, cpu, 0x03);
    return;
  }

  n = e86_get_al (cpu);
  c = e86_get_ch (cpu);
  if (d > 4) {
    c |= ((e86_get_cl (cpu) & 0xc0) << 2);
  }
  h = e86_get_dh (cpu);

  if (d < 4) {
    unsigned short seg, ofs;

    ofs = e86_get_mem16 (cpu, 0x0000, 4 * 0x001e);
    seg = e86_get_mem16 (cpu, 0x0000, 4 * 0x001e + 2);

    fill = e86_get_mem8 (cpu, seg, ofs + 8);
  }
  else {
    fill = 0xf6;
  }

  memset (buf, fill, 512);

  for (i = 0; i < n; i++) {
    if (dsk_write_chs (dsk, buf, c, h, i + 1, 1)) {
      dsk_int13_set_status (dsks, cpu, 0x04);
      return;
    }
  }

  dsk_int13_set_status (dsks, cpu, 0x00);
}

static
void dsk_int13_08 (disks_t *dsks, e8086_t *cpu)
{
  unsigned drive;
  disk_t   *dsk;

  drive = e86_get_dl (cpu);
  dsk = dsks_get_disk (dsks, drive);

  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  e86_set_dl (cpu, dsks_get_hd_cnt (dsks));
  e86_set_dh (cpu, dsk->visible_h - 1);
  e86_set_ch (cpu, dsk->visible_c - 1);
  e86_set_cl (cpu, dsk->visible_s | (((dsk->visible_c - 1) >> 2) & 0xc0));

  dsk_int13_set_status (dsks, cpu, 0x00);
}

static
void dsk_int13_10 (disks_t *dsks, e8086_t *cpu)
{
  unsigned drive;
  disk_t   *dsk;

  drive = e86_get_dl (cpu);
  dsk = dsks_get_disk (dsks, drive);

  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 0x20);
    return;
  }

  dsk_int13_set_status (dsks, cpu, 0x00);
}

#if 0
static
void dsk_int13_15 (disks_t *dsks, e8086_t *cpu)
{
  unsigned drive;
  disk_t   *dsk;

  drive = e86_get_dl (cpu);
  dsk = dsks_get_disk (dsks, drive);

  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 0x01);
    return;
  }

  dsk_int13_set_status (dsks, cpu, 0);
  e86_set_ah (cpu, 3);
  e86_set_cx (cpu, (dsk->blocks >> 16) & 0xffff);
  e86_set_dx (cpu, dsk->blocks & 0xffff);
  e86_set_cf (cpu, 0);
}
#endif

void dsk_int13 (disks_t *dsks, e8086_t *cpu)
{
  unsigned func;

  /* dsk_int13_log (dsks, cpu, stderr); */

  func = e86_get_ah (cpu);

  switch (func) {
    case 0x00:
      dsk_int13_set_status (dsks, cpu, 0);
      break;

    case 0x01:
      dsk_int13_set_status (dsks, cpu, e86_get_mem8 (cpu, 0x40, 0x41));
      break;

    case 0x02:
      dsk_int13_02 (dsks, cpu);
      break;

    case 0x03:
      dsk_int13_03 (dsks, cpu);
      break;

    case 0x04:
      dsk_int13_set_status (dsks, cpu, 0x00);
      break;

    case 0x05:
      dsk_int13_05 (dsks, cpu);
      break;

    case 0x08:
      dsk_int13_08 (dsks, cpu);
      break;

    case 0x0c:
      dsk_int13_set_status (dsks, cpu, 0x00);
      break;

    case 0x10:
      dsk_int13_10 (dsks, cpu);
      break;

    case 0x18:
      dsk_int13_set_status (dsks, cpu, 0x01);
      break;

    default:
      dsk_int13_set_status (dsks, cpu, 0x01);
      break;
  }
}
