/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkcow.c                                       *
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


#include "blkcow.h"

#include <stdlib.h>
#include <string.h>


static
cow_hash_t *cow_new_block (unsigned long blk, unsigned long offs)
{
  cow_hash_t *ret;

  ret = malloc (sizeof (cow_hash_t));
  if (ret == NULL) {
    return (NULL);
  }

  ret->next = NULL;
  ret->block = blk;
  ret->offset = offs;

  return (ret);
}

static
void cow_del_block (cow_hash_t *h)
{
  cow_hash_t *tmp;

  while (h != NULL) {
    tmp = h->next;
    free (h);
    h = tmp;
  }
}

static
cow_hash_t *cow_get_block (cow_hash_t **hash, unsigned cnt, unsigned long blk)
{
  cow_hash_t *h;

  h = hash[blk & (cnt - 1)];

  while (h != NULL) {
    if (h->block == blk) {
      return (h);
    }

    h = h->next;
  }

  return (NULL);
}

static
void cow_set_block (cow_hash_t **hash, unsigned cnt, cow_hash_t *h)
{
  unsigned i;

  i = h->block & (cnt - 1);

  h->next = hash[i];
  hash[i] = h;
}


static
int dsk_cow_read (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  unsigned char *tmp;
  disk_cow_t    *cow;
  cow_hash_t    *blk;

  cow = dsk->ext;
  tmp = buf;

  while (n > 0) {
    if (i >= dsk->blocks) {
      return (1);
    }

    blk = cow_get_block (cow->hash, cow->hash_cnt, i);

    if (blk == NULL) {
      if (dsk_read_lba (cow->orig, tmp, i, 1)) {
        return (1);
      }
    }
    else {
      if (fseek (cow->fp, blk->offset, SEEK_SET)) {
        return (1);
      }

      dsk_fread_zero (tmp, 512, 1, cow->fp);
    }

    i += 1;
    n -= 1;
    tmp += 512;
  }

  return (0);
}

static
int dsk_cow_write (disk_t *dsk, const void *buf, unsigned long i, unsigned long n)
{
  const unsigned char *tmp;
  disk_cow_t          *cow;
  cow_hash_t          *h;

  if (dsk->readonly) {
    return (1);
  }

  cow = dsk->ext;
  tmp = buf;

  while (n > 0) {
    if (i >= dsk->blocks) {
      return (1);
    }

    h = cow_get_block (cow->hash, cow->hash_cnt, i);

    if (h == NULL) {
      h = cow_new_block (i, cow->offset);
      cow->offset += 512;
      cow_set_block (cow->hash, cow->hash_cnt, h);
    }

    if (fseek (cow->fp, h->offset, SEEK_SET)) {
      return (1);
    }

    if (fwrite (tmp, 512, 1, cow->fp) != 1) {
      return (1);
    }

    i += 1;
    n -= 1;
    tmp += 512;
  }

  fflush (cow->fp);

  return (0);
}

static
int dsk_cow_commit (disk_t *dsk)
{
  unsigned      i;
  disk_cow_t    *cow;
  cow_hash_t    *h;
  unsigned char buf[512];

  cow = dsk->ext;

  for (i = 0; i < cow->hash_cnt; i++) {
    while (cow->hash[i] != NULL) {
      h = cow->hash[i];

      if (fseek (cow->fp, h->offset, SEEK_SET)) {
        return (1);
      }

      dsk_fread_zero (buf, 512, 1, cow->fp);

      if (dsk_write_lba (cow->orig, buf, h->block, 1)) {
        return (1);
      }

      cow->hash[i] = h->next;
      h->next = NULL;
      cow_del_block (h);
    }
  }

  cow->offset = 0;

  return (0);
}

static
void dsk_cow_del (disk_t *dsk)
{
  unsigned   i;
  disk_cow_t *cow;

  cow = (disk_cow_t *) dsk->ext;

  for (i = 0; i < cow->hash_cnt; i++) {
    cow_del_block (cow->hash[i]);
  }

  free (cow->hash);

  fclose (cow->fp);

  dsk_del (cow->orig);

  free (cow);
}

disk_t *dsk_cow_new (disk_t *dsk, const char *fname)
{
  unsigned   i;
  disk_cow_t *cow;

  cow = (disk_cow_t *) malloc (sizeof (disk_cow_t));
  if (cow == NULL) {
    return (NULL);
  }

  cow->dsk = *dsk;

  cow->dsk.del = &dsk_cow_del;
  cow->dsk.read = &dsk_cow_read;
  cow->dsk.write = &dsk_cow_write;
  cow->dsk.commit = &dsk_cow_commit;
  cow->dsk.ext = cow;

  cow->orig = dsk;

  cow->fp = fopen (fname, "w+b");
#if 0
  if (cow->fp == NULL) {
    cow->fp = fopen (fname, "w+b");
  }
#endif

  if (cow->fp == NULL) {
    free (cow);
    return (NULL);
  }

  cow->offset = 0;

  cow->hash_cnt = 1024;
  cow->hash = malloc (1024 * sizeof (cow_hash_t *));

  for (i = 0; i < cow->hash_cnt; i++) {
    cow->hash[i] = NULL;
  }

  return (&cow->dsk);
}
