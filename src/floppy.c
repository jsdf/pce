/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/floppy.c                                               *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-21 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: floppy.c,v 1.9 2003/04/21 19:10:59 hampa Exp $ */


#include <stdio.h>

#include <pce.h>

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

int dsk_set_mem (disk_t *dsk, unsigned c, unsigned h, unsigned s, const char *fname, int ro)
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

int dsk_set_image (disk_t *dsk, unsigned c, unsigned h, unsigned s, const char *fname, int ro)
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

  fprintf (stderr, "drive %u: image %s c=%u h=%u s=%u b=%lu\n",
    dsk->drive, fname, dsk->geom.c, dsk->geom.h, dsk->geom.s, dsk->blocks
  );

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

  fprintf (stderr, "drive %u: hdimage %s c=%u h=%u s=%u b=%lu\n",
    dsk->drive, fname, dsk->geom.c, dsk->geom.h, dsk->geom.s, dsk->blocks
  );

  dsk->fp = fp;
  dsk->fp_close = 1;

  return (0);
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

void dsk_int13_set_status (disks_t *dsks, e8086_t *cpu, unsigned val)
{
  e86_set_ah (cpu, val);
  e86_set_mem8 (cpu, 0x40, 0x41, val);
  e86_set_cf (cpu, val != 0);

  dsks->last_status = val;
}

void dsk_int13_02 (disks_t *dsks, e8086_t *cpu)
{
  unsigned long  i, j;
  unsigned long  blk_i;
  unsigned       c, h, s, n;
  unsigned short seg, ofs;
  disk_t         *dsk;

  dsk = dsks_get_disk (dsks, e86_get_dl (cpu));
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  n = e86_get_al (cpu);
  c = e86_get_ch (cpu) | ((e86_get_cl (cpu) & 0xc0) << 2);
  h = e86_get_dh (cpu);
  s = e86_get_cl (cpu) & 0x3f;
  seg = e86_get_es (cpu);
  ofs = e86_get_bx (cpu);

  if ((s < 1) || (s > dsk->geom.s)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if ((h >= dsk->geom.h) || (c >= dsk->geom.c)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  blk_i = ((c * dsk->geom.h + h) * dsk->geom.s + s - 1);

  if ((blk_i + n) > dsk->blocks) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if (dsk->data != NULL) {
    unsigned char *src;

    src = dsk->data + 512UL * blk_i;

    for (i = 0; i < n; i++) {
      for (j = 0; j < 512; j++) {
        e86_set_mem8 (cpu, seg, ofs, *(src++));
        ofs = (ofs + 1) & 0xffff;
      }
    }
  }
  else if (dsk->fp != NULL) {
    unsigned char buf[512];

    if (fseek (dsk->fp, dsk->start + 512UL * blk_i, SEEK_SET)) {
      dsk_int13_set_status (dsks, cpu, 1);
      return;
    }

    for (i = 0; i < n; i++) {
      memset (buf, 0, 512);
      fread (buf, 1, 512, dsk->fp);
      for (j = 0; j < 512; j++) {
        e86_set_mem8 (cpu, seg, ofs, buf[j]);
        ofs = (ofs + 1) & 0xffff;
      }
    }
  }
  else {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  dsk_int13_set_status (dsks, cpu, 0);
}

void dsk_int13_03 (disks_t *dsks, e8086_t *cpu)
{
  unsigned long  i, j;
  unsigned       c, h, s, n;
  unsigned short seg, ofs;
  unsigned long  blk_i;
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

  n = e86_get_al (cpu);
  c = e86_get_ch (cpu) | ((e86_get_cl (cpu) & 0xc0) << 2);
  h = e86_get_dh (cpu);
  s = e86_get_cl (cpu) & 0x3f;
  seg = e86_get_es (cpu);
  ofs = e86_get_bx (cpu);

  if ((s < 1) || (s > dsk->geom.s)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if ((h >= dsk->geom.h) || (c >= dsk->geom.c)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  blk_i = (c * dsk->geom.h + h) * dsk->geom.s + s - 1;

  if ((blk_i + n) > dsk->blocks) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if (dsk->data != NULL) {
    unsigned char *dst;

    dst = dsk->data + 512UL * blk_i;

    for (i = 0; i < n; i++) {
      for (j = 0; j < 512; j++) {
        *(dst++) = e86_get_mem8 (cpu, seg, ofs);
        ofs = (ofs + 1) & 0xffff;
      }
    }
  }
  else if (dsk->fp != NULL) {
    unsigned char buf[512];

    if (fseek (dsk->fp, dsk->start + 512UL * blk_i, SEEK_SET)) {
      dsk_int13_set_status (dsks, cpu, 1);
      return;
    }

    for (i = 0; i < n; i++) {
      for (j = 0; j < 512; j++) {
        buf[j] = e86_get_mem8 (cpu, seg, ofs);
        ofs = (ofs + 1) & 0xffff;
      }
      fwrite (buf, 1, 512, dsk->fp);
    }
  }
  else {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
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

void dsk_int13_log (disks_t *dsks, e8086_t *cpu, FILE *fp)
{
  fprintf (fp,
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

void dsk_int13 (disks_t *dsks, e8086_t *cpu)
{
  unsigned func;

  dsk_int13_log (dsks, cpu, stderr);

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
    case 0x05:
    case 0x18:
      dsk_int13_set_status (dsks, cpu, 0);
      break;

    case 0x08:
      dsk_int13_08 (dsks, cpu);
      break;

//    case 0x15:
//      dsk_int13_15 (dsks, cpu);
//      break;

    default:
      dsk_int13_set_status (dsks, cpu, 1);
      break;
  }
}
