/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/disk.c                                           *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: disk.c,v 1.7 2003/09/01 18:07:12 hampa Exp $ */


#include <stdio.h>

#include "pce.h"

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>


#define buf_get_uint32(buf) \
  (((((((buf)[3] << 8) | (buf)[2]) << 8) | (buf)[1]) << 8) | (buf)[0])


disk_t *dsk_new (unsigned drive)
{
  disk_t *dsk;

  dsk = (disk_t *) malloc (sizeof (disk_t));
  if (dsk == NULL) {
    return (NULL);
  }

  dsk->drive = drive;
  dsk->geom.c = 0;
  dsk->geom.h = 0;
  dsk->geom.s = 0;
  dsk->start = 0;
  dsk->blocks = 0;

  dsk->readonly = 1;

  dsk->data = NULL;
  dsk->fp = NULL;
  dsk->fp_close = 0;

  return (dsk);
}

void dsk_free (disk_t *dsk)
{
  free (dsk->data);
  dsk->data = NULL;

  if ((dsk->fp_close) && (dsk->fp != NULL)) {
    fclose (dsk->fp);
  }

  dsk->fp = NULL;
}

void dsk_del (disk_t *dsk)
{
  if (dsk != NULL) {
    dsk_free (dsk);
    free (dsk);
  }
}

int dsk_set_mem (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro)
{
  dsk_free (dsk);

  dsk->geom.c = c;
  dsk->geom.h = h;
  dsk->geom.s = s;
  dsk->start = 0;
  dsk->blocks = c * h * s;

  dsk->readonly = ro;

  dsk->data = (unsigned char *) malloc (512 * dsk->blocks);
  if (dsk->data == NULL) {
    return (1);
  }

  memset (dsk->data, 0, 512UL * dsk->blocks);

  if (fname != NULL) {
    FILE *fp;

    fp = fopen (fname, "rb");
    if (fp == NULL) {
      return (1);
    }

    if (fread (dsk->data, 512, dsk->blocks, fp) != dsk->blocks) {
      fclose (fp);
      return (1);
    }

    fclose (fp);
  }

  return (0);
}

int dsk_set_image (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro)
{
  char *mode;

  dsk_free (dsk);

  if (ro) {
    mode = "rb";
  }
  else {
    mode = "r+b";
  }

  dsk->fp = fopen (fname, mode);
  if ((dsk->fp == NULL) && (ro == 0)) {
    dsk->fp = fopen (fname, "w+b");
  }

  if (dsk->fp == NULL) {
    return (1);
  }

  dsk->fp_close = 1;

  dsk->geom.c = c;
  dsk->geom.h = h;
  dsk->geom.s = s;
  dsk->start = 0;
  dsk->blocks = c * h * s;

  dsk->readonly = ro;

  return (0);
}

int dsk_set_hdimage (disk_t *dsk, const char *fname, int ro)
{
  FILE          *fp;
  char          *mode;
  unsigned char buf[128];

  dsk_free (dsk);

  mode = (ro) ? "rb" : "r+b";

  fp = fopen (fname, mode);
  if (fp == NULL) {
    return (1);
  }

  if (fread (buf, 1, 128, fp) != 128) {
    fclose (fp);
    return (1);
  }

  if (memcmp (buf, "DOSEMU\x00", 7) != 0) {
    fclose (fp);
    return (1);
  }

  dsk->geom.c = buf_get_uint32 (buf + 15);
  dsk->geom.h = buf_get_uint32 (buf + 7);
  dsk->geom.s = buf_get_uint32 (buf + 11);
  dsk->start = 128;
  dsk->blocks = dsk->geom.c * dsk->geom.h * dsk->geom.s;

  dsk->readonly = ro;

  dsk->fp = fp;
  dsk->fp_close = 1;

  return (0);
}

int dsk_get_lba (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  unsigned long *lba)
{
  if ((s < 1) || (s > dsk->geom.s)) {
    return (1);
  }

  if ((h >= dsk->geom.h) || (c >= dsk->geom.c)) {
    return (1);
  }

  *lba = ((c * dsk->geom.h + h) * dsk->geom.s + s - 1);

  return (0);
}

int dsk_read_lba (disk_t *dsk, void *buf,
  unsigned long blk_i, unsigned long blk_n)
{
  unsigned long i, n;

  if ((blk_i + blk_n) > dsk->blocks) {
    return (1);
  }

  i = 512UL * blk_i;
  n = 512UL * blk_n;

  if (dsk->data != NULL) {
    memcpy (buf, dsk->data + i, n);
  }
  else if (dsk->fp != NULL) {
    if (fseek (dsk->fp, dsk->start + i, SEEK_SET)) {
      return (1);
    }

    memset (buf, 0, n);

    if (fread (buf, 512, blk_n, dsk->fp) != blk_n) {
      return (1);
    }
  }
  else {
    return (1);
  }

  return (0);
}

int dsk_read_chs (disk_t *dsk, void *buf,
  unsigned c, unsigned h, unsigned s, unsigned long blk_n)
{
  unsigned long blk_i;

  if (dsk_get_lba (dsk, c, h, s, &blk_i)) {
    return (1);
  }

  return (dsk_read_lba (dsk, buf, blk_i, blk_n));
}

int dsk_write_lba (disk_t *dsk, const void *buf,
  unsigned long blk_i, unsigned long blk_n)
{
  unsigned long       i, n;
  const unsigned char *tmp;

  tmp = (const unsigned char *) buf;

  if ((blk_i + blk_n) > dsk->blocks) {
    return (1);
  }

  i = 512UL * blk_i;
  n = 512UL * blk_n;

  if (dsk->data != NULL) {
    memcpy (dsk->data + i, buf, n);
  }
  else if (dsk->fp != NULL) {
    if (fseek (dsk->fp, dsk->start + i, SEEK_SET)) {
      return (1);
    }

    if (fwrite (buf, 512, blk_n, dsk->fp) != blk_n) {
      return (1);
    }

    fflush (dsk->fp);
  }
  else {
    return (1);
  }

  return (0);
}

int dsk_write_chs (disk_t *dsk, const void *buf,
  unsigned c, unsigned h, unsigned s, unsigned long blk_n)
{
  unsigned long blk_i;

  if (dsk_get_lba (dsk, c, h, s, &blk_i)) {
    return (1);
  }

  return (dsk_write_lba (dsk, buf, blk_i, blk_n));
}

void dsk_set_drive (disk_t *dsk, unsigned drive)
{
  dsk->drive = drive;
}

disks_t *dsks_new (void)
{
  disks_t *dsks;

  dsks = (disks_t *) malloc (sizeof (disks_t));
  if (dsks == NULL) {
    return (NULL);
  }

  dsks->cnt = 0;
  dsks->last_status = 0;

  return (dsks);
}

void dsks_del (disks_t *dsks)
{
  unsigned i;

  if (dsks == NULL) {
    return;
  }

  for (i = 0; i < dsks->cnt; i++) {
    dsk_del (dsks->dsk[i]);
  }

  free (dsks);
}

int dsks_add_disk (disks_t *dsks, disk_t *dsk)
{
  unsigned i;

  if (dsks->cnt >= PCE_MAX_DISKS) {
    return (1);
  }

  for (i = 0; i < dsks->cnt; i++) {
    if (dsks->dsk[i]->drive == dsk->drive) {
      return (1);
    }
  }

  dsks->dsk[dsks->cnt] = dsk;
  dsks->cnt += 1;

  return (0);
}

void dsks_rmv_disk (disks_t *dsks, disk_t *dsk)
{
  unsigned i, j;

  j = 0;
  for (i = 0; i < dsks->cnt; i++) {
    if (dsks->dsk[i] != dsk) {
      dsks->dsk[j] = dsks->dsk[i];
      j += 1;
    }
  }

  dsks->cnt = j;
}

disk_t *dsks_get_disk (disks_t *dsks, unsigned drive)
{
  unsigned i;

  for (i = 0; i < dsks->cnt; i++) {
    if (dsks->dsk[i]->drive == drive) {
      return (dsks->dsk[i]);
    }
  }

  return (NULL);
}

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

void dsk_int13_set_status (disks_t *dsks, e8086_t *cpu, unsigned val)
{
  e86_set_ah (cpu, val);
  e86_set_mem8 (cpu, 0x40, 0x41, val);
  e86_set_cf (cpu, val != 0);

  dsks->last_status = val;
}

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
    dsk_int13_set_status (dsks, cpu, 1);
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

  dsk_int13_set_status (dsks, cpu, 0);
}

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
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  if (dsk->readonly) {
    dsk_int13_set_status (dsks, cpu, 3);
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

  dsk_int13_set_status (dsks, cpu, 0);
}

void dsk_int13_05 (disks_t *dsks, e8086_t *cpu)
{
  unsigned       i, n;
  unsigned       d, c, h;
  unsigned char  fill;
  unsigned char  buf[512];
  unsigned short seg, ofs;
  disk_t         *dsk;

  d = e86_get_dl (cpu);

  dsk = dsks_get_disk (dsks, d);
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  if (dsk->readonly) {
    dsk_int13_set_status (dsks, cpu, 3);
    return;
  }

  n = e86_get_al (cpu);
  c = e86_get_ch (cpu);
  if (d > 4) {
    c |= ((e86_get_cl (cpu) & 0xc0) << 2);
  }
  h = e86_get_dh (cpu);

  ofs = e86_get_mem16 (cpu, 0x0000, 4 * 0x001e);
  seg = e86_get_mem16 (cpu, 0x0000, 4 * 0x001e + 2);
  fill = e86_get_mem8 (cpu, seg, ofs + 8);

  memset (buf, fill, 512);

  for (i = 0; i < n; i++) {
    if (dsk_write_chs (dsk, buf, c, h, i + 1, 1)) {
      dsk_int13_set_status (dsks, cpu, 0x04);
      return;
    }
  }

  dsk_int13_set_status (dsks, cpu, 0);
}

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
  e86_set_dh (cpu, dsk->geom.h - 1);
  e86_set_ch (cpu, dsk->geom.c - 1);
  e86_set_cl (cpu, dsk->geom.s | (((dsk->geom.c - 1) >> 2) & 0xc0));

  dsk_int13_set_status (dsks, cpu, 0);
}

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

  dsk_int13_set_status (dsks, cpu, 0);
}

void dsk_int13_15 (disks_t *dsks, e8086_t *cpu)
{
  unsigned drive;
  disk_t   *dsk;

  drive = e86_get_dl (cpu);
  dsk = dsks_get_disk (dsks, drive);

  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  dsk_int13_set_status (dsks, cpu, 0);
  e86_set_ah (cpu, 3);
  e86_set_cx (cpu, (dsk->blocks >> 16) & 0xffff);
  e86_set_dx (cpu, dsk->blocks & 0xffff);
  e86_set_cf (cpu, 0);
}

void dsk_int13 (disks_t *dsks, e8086_t *cpu)
{
  unsigned func;

//  dsk_int13_log (dsks, cpu, stderr);

  func = e86_get_ah (cpu);

  switch (func) {
    case 0x00:
      dsk_int13_set_status (dsks, cpu, 0);
      break;

    case 0x01:
      dsk_int13_set_status (dsks, cpu, dsks->last_status);
      break;

    case 0x02:
      dsk_int13_02 (dsks, cpu);
      break;

    case 0x03:
      dsk_int13_03 (dsks, cpu);
      break;

    case 0x04:
      dsk_int13_set_status (dsks, cpu, 0);
      break;

    case 0x05:
      dsk_int13_05 (dsks, cpu);
      break;

    case 0x08:
      dsk_int13_08 (dsks, cpu);
      break;

    case 0x0c:
      dsk_int13_set_status (dsks, cpu, 0);
      break;

    case 0x10:
      dsk_int13_10 (dsks, cpu);
      break;

//    case 0x15:
//      dsk_int13_15 (dsks, cpu);
//      break;

    case 0x18:
      dsk_int13_set_status (dsks, cpu, 1);
      break;

    default:
//      dsk_int13_log (dsks, cpu, stderr);
      dsk_int13_set_status (dsks, cpu, 1);
      break;
  }
}
