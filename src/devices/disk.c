/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/disk.c                                         *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "disk.h"


#define buf_get_uint32(buf) \
  (((((((buf)[3] << 8) | (buf)[2]) << 8) | (buf)[1]) << 8) | (buf)[0])


void dsk_init_chs (disk_t *dsk, unsigned d, unsigned c, unsigned h, unsigned s, int ro)
{
  dsk->del = NULL;
  dsk->read = NULL;
  dsk->write = NULL;

  dsk->drive = d;

  dsk->c = c;
  dsk->h = h;
  dsk->s = s;

  dsk->blocks = c * h * s;

  dsk->visible_c = c;
  dsk->visible_h = h;
  dsk->visible_s = s;

  dsk->readonly = ro;

  dsk->ext = NULL;
}

void dsk_del (disk_t *dsk)
{
  if ((dsk != NULL) && (dsk->del != NULL)) {
    dsk->del (dsk);
  }
}


void dsk_set_visible_geometry (disk_t *dsk, unsigned c, unsigned h, unsigned s)
{
  dsk->visible_c = c;
  dsk->visible_h = h;
  dsk->visible_s = s;
}

static
int dsk_ram_load (disk_ram_t *ram, const char *fname)
{
  FILE *fp;

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return (1);
  }

  fread (ram->data, 512, ram->dsk.blocks, fp);

  fclose (fp);

  return (0);
}

static
int dsk_ram_read (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  disk_ram_t *ram;

  ram = (disk_ram_t *) dsk->ext;

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  memcpy (buf, ram->data + 512UL * i, 512UL * n);

  return (0);
}

static
int dsk_ram_write (disk_t *dsk, const void *buf, unsigned long i, unsigned long n)
{
  disk_ram_t *ram;

  if (dsk->readonly) {
    return (1);
  }

  ram = (disk_ram_t *) dsk->ext;

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  memcpy (ram->data + 512UL * i, buf, 512UL * n);

  return (0);
}

static
void dsk_ram_del (disk_t *dsk)
{
  disk_ram_t *ram;

  ram = (disk_ram_t *) dsk->ext;

  free (ram->data);
  free (ram);
}

disk_t *dsk_ram_new (unsigned d, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro)
{
  disk_ram_t *ram;

  ram = (disk_ram_t *) malloc (sizeof (disk_ram_t));
  if (ram == NULL) {
    return (NULL);
  }

  dsk_init_chs (&ram->dsk, d, c, h, s, ro);

  ram->dsk.del = &dsk_ram_del;
  ram->dsk.read = &dsk_ram_read;
  ram->dsk.write = &dsk_ram_write;
  ram->dsk.ext = ram;

  ram->data = (unsigned char *) malloc (512UL * ram->dsk.blocks);
  if (ram->data == NULL) {
    free (ram);
    return (NULL);
  }

  memset (ram->data, 0, 512UL * ram->dsk.blocks);

  if (fname != NULL) {
    if (dsk_ram_load (ram, fname)) {
      free (ram->data);
      free (ram);
      return (NULL);
    }
  }

  return (&ram->dsk);
}


static
int dsk_img_read (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  disk_img_t *img;

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = (disk_img_t *) dsk->ext;

  if (fseek (img->fp, img->start + 512UL * i, SEEK_SET)) {
    return (1);
  }

  if (fread (buf, 512, n, img->fp) != n) {
    return (1);
  }

  return (0);
}

static
int dsk_img_write (disk_t *dsk, const void *buf, unsigned long i, unsigned long n)
{
  disk_img_t *img;

  if (dsk->readonly) {
    return (1);
  }

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = (disk_img_t *) dsk->ext;

  if (fseek (img->fp, img->start + 512UL * i, SEEK_SET)) {
    return (1);
  }

  if (fwrite (buf, 512, n, img->fp) != n) {
    return (1);
  }

  fflush (img->fp);

  return (0);
}

static
void dsk_img_del (disk_t *dsk)
{
  disk_img_t *img;

  img = (disk_img_t *) dsk->ext;

  fclose (img->fp);
  free (img);
}

disk_t *dsk_img_new (unsigned d, unsigned c, unsigned h, unsigned s,
  unsigned long start, const char *fname, int ro)
{
  disk_img_t *img;

  img = (disk_img_t *) malloc (sizeof (disk_img_t));
  if (img == NULL) {
    return (NULL);
  }

  dsk_init_chs (&img->dsk, d, c, h, s, ro);

  img->dsk.del = &dsk_img_del;
  img->dsk.read = &dsk_img_read;
  img->dsk.write = &dsk_img_write;
  img->dsk.ext = img;

  img->start = start;

  if (ro) {
    img->fp = fopen (fname, "rb");
  }
  else {
    img->fp = fopen (fname, "r+b");
    if (img->fp == NULL) {
      img->fp = fopen (fname, "w+b");
    }
  }

  if (img->fp == NULL) {
    free (img);
    return (NULL);
  }

  return (&img->dsk);
}


disk_t *dsk_dosemu_new (unsigned d, const char *fname, int ro)
{
  FILE          *fp;
  unsigned char buf[128];
  unsigned      c, h, s;
  unsigned long start;

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return (NULL);
  }

  if (fread (buf, 1, 128, fp) != 128) {
    fclose (fp);
    return (NULL);
  }

  fclose (fp);

  if (memcmp (buf, "DOSEMU\x00", 7) != 0) {
    fclose (fp);
    return (NULL);
  }

  c = buf_get_uint32 (buf + 15);
  h = buf_get_uint32 (buf + 7);
  s = buf_get_uint32 (buf + 11);
  start = buf_get_uint32 (buf + 19);

  if (start < 128) {
    return (NULL);
  }

  return (dsk_img_new (d, c, h, s, start, fname, ro));
}


disk_t *dsk_auto_new (unsigned d, const char *fname, int ro)
{
  return (NULL);
}


int dsk_get_lba (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  unsigned long *lba)
{
  if ((s < 1) || (s > dsk->s)) {
    return (1);
  }

  if ((h >= dsk->h) || (c >= dsk->c)) {
    return (1);
  }

  *lba = ((c * dsk->h + h) * dsk->s + s - 1);

  return (0);
}

int dsk_read_lba (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  if (dsk->read != NULL) {
    return (dsk->read (dsk, buf, i, n));
  }

  return (1);
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

int dsk_write_lba (disk_t *dsk, const void *buf, unsigned long i, unsigned long n)
{
  if (dsk->write != NULL) {
    return (dsk->write (dsk, buf, i, n));
  }

  return (1);
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


disks_t *dsks_new (void)
{
  disks_t *dsks;

  dsks = (disks_t *) malloc (sizeof (disks_t));
  if (dsks == NULL) {
    return (NULL);
  }

  dsks->cnt = 0;
  dsks->dsk = NULL;

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

  free (dsks->dsk);
  free (dsks);
}

int dsks_add_disk (disks_t *dsks, disk_t *dsk)
{
  unsigned i, n;
  disk_t   **tmp;

  for (i = 0; i < dsks->cnt; i++) {
    if (dsks->dsk[i]->drive == dsk->drive) {
      return (1);
    }
  }

  n = dsks->cnt + 1;
  tmp = (disk_t **) realloc (dsks->dsk, n * sizeof (disk_t *));
  if (tmp == NULL) {
    return (1);
  }

  tmp[dsks->cnt] = dsk;

  dsks->cnt = n;
  dsks->dsk = tmp;

  return (0);
}

int dsks_rmv_disk (disks_t *dsks, disk_t *dsk)
{
  int      r;
  unsigned i, j;

  r = 0;
  j = 0;
  for (i = 0; i < dsks->cnt; i++) {
    if (dsks->dsk[i] != dsk) {
      dsks->dsk[j] = dsks->dsk[i];
      j += 1;
    }
    else {
      r = 1;
    }
  }

  dsks->cnt = j;

  return (r);
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
