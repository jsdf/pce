/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     floppy.c                                                   *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-17 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: floppy.c,v 1.6 2003/04/18 20:07:22 hampa Exp $ */


#include <stdio.h>

#include <pce.h>

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>


disk_t *dsk_new (unsigned drive, unsigned c, unsigned h, unsigned s)
{
  disk_t *dsk;

  dsk = (disk_t *) malloc (sizeof (disk_t));
  if (dsk == NULL) {
    return (NULL);
  }

  dsk->drive = drive;
  dsk->geom.c = c;
  dsk->geom.h = h;
  dsk->geom.s = s;
  dsk->start = 0;
  dsk->size = c * h * s;

  dsk->data = NULL;
  dsk->fp = NULL;
  dsk->fp_close = 0;

  return (dsk);
}

void dsk_del (disk_t *dsk)
{
  if (dsk != NULL) {
    free (dsk->data);
    if (dsk->fp_close && (dsk->fp != NULL)) {
      fclose (dsk->fp);
    }
    free (dsk);
  }
}

void dsk_set_drive (disk_t *dsk, unsigned drive)
{
  dsk->drive = drive;
}

int dsk_alloc (disk_t *dsk)
{
  if (dsk->data != NULL) {
    return (0);
  }

  if (dsk->fp_close && (dsk->fp != NULL)) {
    fclose (dsk->fp);
  }

  dsk->fp = NULL;
  dsk->fp_close = 0;

  dsk->data = (unsigned char *) malloc (512UL * dsk->size);
  if (dsk->data == NULL) {
    return (1);
  }

  memset (dsk->data, 0, 512UL * dsk->size);

  return (0);
}

int dsk_set_file (disk_t *dsk, const char *fname)
{
  free (dsk->data);
  dsk->data = NULL;

  dsk->fp = fopen (fname, "r+");
  if (dsk->fp == NULL) {
    dsk->fp = fopen (fname, "w+");
    if (dsk->fp == NULL) {
      return (1);
    }
  }

  dsk->fp_close = 1;

  return (0);
}

int dsk_load_img (disk_t *dsk, const char *fname)
{
  FILE *fp;

  if (dsk_alloc (dsk)) {
    return (1);
  }

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return (1);
  }

  if (fread (dsk->data, 1, 512UL * dsk->size, fp) != 512UL * dsk->size) {
    fclose (fp);
    return (1);
  }

  fclose (fp);

  return (0);
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
  unsigned       drive;
  unsigned       cnt;
  unsigned       c, h, s;
  unsigned short seg, ofs;
  unsigned long  i, j, n;
  disk_t         *dsk;

  drive = e86_get_dl (cpu);

  dsk = dsks_get_disk (dsks, drive);
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  cnt = e86_get_al (cpu);
  c = e86_get_ch (cpu) | ((e86_get_cl (cpu) & 0xc0) << 2);
  h = e86_get_dh (cpu);
  s = e86_get_cl (cpu) & 0x3f;

  if ((s < 1) || (s > dsk->geom.s)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if ((h >= dsk->geom.h) || (c >= dsk->geom.c)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  seg = e86_get_es (cpu);
  ofs = e86_get_bx (cpu);

  i = ((c * dsk->geom.h + h) * dsk->geom.s + s - 1);
  n = cnt;

  if ((i + n) > dsk->size) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if (dsk->data != NULL) {
    while (n > 0) {
      for (j = 0; j < 512; j++) {
        e86_set_mem8 (cpu, seg, ofs, dsk->data[512 * i + j]);
        ofs = (ofs + 1) & 0xffff;
      }
      i += 1;
      n -= 1;
    }
  }
  else if (dsk->fp != NULL) {
    unsigned char buf[512];

    if (fseek (dsk->fp, 512 * i, SEEK_SET)) {
      perror ("seek");
      dsk_int13_set_status (dsks, cpu, 1);
      return;
    }

    while (n > 0) {
      memset (buf, 0, 512);
      fread (buf, 1, 512, dsk->fp);
      for (j = 0; j < 512; j++) {
        e86_set_mem8 (cpu, seg, ofs, buf[j]);
        ofs = (ofs + 1) & 0xffff;
      }
      i += 1;
      n -= 1;
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
  unsigned       drive;
  unsigned       cnt;
  unsigned       c, h, s;
  unsigned short seg, ofs;
  unsigned long  i, j, n;
  disk_t         *dsk;

  drive = e86_get_dl (cpu);

  dsk = dsks_get_disk (dsks, drive);
  if (dsk == NULL) {
    dsk_int13_set_status (dsks, cpu, 1);
    return;
  }

  cnt = e86_get_al (cpu);
  c = e86_get_ch (cpu) | ((e86_get_cl (cpu) & 0xc0) << 2);
  h = e86_get_dh (cpu);
  s = e86_get_cl (cpu) & 0x3f;

  if ((s < 1) || (s > dsk->geom.s)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if ((h >= dsk->geom.h) || (c >= dsk->geom.c)) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  seg = e86_get_es (cpu);
  ofs = e86_get_bx (cpu);

  i = (c * dsk->geom.h + h) * dsk->geom.s + s - 1;
  n = cnt;

  if ((i + n) > dsk->size) {
    dsk_int13_set_status (dsks, cpu, 0x04);
    return;
  }

  if (dsk->data != NULL) {
    while (n > 0) {
      for (j = 0; j < 512; j++) {
        dsk->data[512 * i + j] = e86_get_mem8 (cpu, seg, ofs);
        ofs = (ofs + 1) & 0xffff;
      }
      i += 1;
      n -= 1;
    }
  }
  else if (dsk->fp != NULL) {
    unsigned char buf[512];

    if (fseek (dsk->fp, 512 * i, SEEK_SET)) {
      perror ("seek");
      dsk_int13_set_status (dsks, cpu, 1);
      return;
    }

    while (n > 0) {
      for (j = 0; j < 512; j++) {
        buf[j] = e86_get_mem8 (cpu, seg, ofs);
        ofs = (ofs + 1) & 0xffff;
      }
      fwrite (buf, 1, 512, dsk->fp);
      i += 1;
      n -= 1;
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
  e86_set_cx (cpu, (dsk->size >> 16) & 0xffff);
  e86_set_dx (cpu, dsk->size & 0xffff);
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
