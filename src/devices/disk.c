/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/disk.c                                         *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
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


#include "disk.h"

#include <stdlib.h>
#include <string.h>


void dsk_fread_zero (void *buf, size_t size, size_t cnt, FILE *fp)
{
  size_t r;

  r = fread (buf, size, cnt, fp);

  if (r < cnt) {
    memset ((unsigned char *) buf + size * r, 0x00, size * (cnt - r));
  }
}

void dsk_init (disk_t *dsk, void *ext, unsigned d,
  unsigned c, unsigned h, unsigned s, int ro)
{
  dsk->del = NULL;
  dsk->read = NULL;
  dsk->write = NULL;
  dsk->commit = NULL;

  dsk->drive = d;

  dsk->c = c;
  dsk->h = h;
  dsk->s = s;

  dsk->blocks = (unsigned long) c * h * s;

  dsk->visible_c = c;
  dsk->visible_h = h;
  dsk->visible_s = s;

  dsk->readonly = (ro != 0);

  dsk->ext = ext;
}

void dsk_del (disk_t *dsk)
{
  if ((dsk != NULL) && (dsk->del != NULL)) {
    dsk->del (dsk);
  }
}


void dsk_set_visible_chs (disk_t *dsk, unsigned c, unsigned h, unsigned s)
{
  dsk->visible_c = c;
  dsk->visible_h = h;
  dsk->visible_s = s;
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

int dsk_commit (disk_t *dsk)
{
  if (dsk->commit != NULL) {
    return (dsk->commit (dsk));
  }

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

int dsks_commit (disks_t *dsks)
{
  unsigned i;
  int      r;

  r = 0;

  for (i = 0; i < dsks->cnt; i++) {
    if (dsk_commit (dsks->dsk[i])) {
      r = 1;
    }
  }

  return (r);
}
