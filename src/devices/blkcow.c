/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkcow.c                                       *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-10-05 by Hampa Hug <hampa@hampa.ch>                   *
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


#define COW_MAGIC 0x434f5720

/*
0   4  magic
4   4  version
8   4  block count
12  4  bitmap start
16  4  data start
*/


static
unsigned long cow_get_uint32 (const void *buf, unsigned i)
{
  const unsigned char *tmp;

  tmp = (const unsigned char *) buf + i;

  return ((tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3]);
}

static
void cow_set_uint32 (void *buf, unsigned i, unsigned long v)
{
  unsigned char *tmp;

  tmp = (unsigned char *) buf + i;

  tmp[0] = (v >> 24) & 0xff;
  tmp[1] = (v >> 16) & 0xff;
  tmp[2] = (v >> 8) & 0xff;
  tmp[3] = v & 0xff;
}

static
int cow_read_bitmap (disk_cow_t *cow)
{
  if (fseek (cow->fp, cow->bitmap_offset, SEEK_SET)) {
    return (1);
  }

  if (fread (cow->bitmap, 1, cow->bitmap_size, cow->fp) != cow->bitmap_size) {
    return (1);
  }

  return (0);
}

static
int cow_write_bitmap (disk_cow_t *cow)
{
  if (fseek (cow->fp, cow->bitmap_offset, SEEK_SET)) {
    return (1);
  }

  if (fwrite (cow->bitmap, 1, cow->bitmap_size, cow->fp) != cow->bitmap_size) {
    return (1);
  }

  fflush (cow->fp);

  return (0);
}

static
int cow_get_block (disk_cow_t *cow, unsigned long blk, unsigned long *cnt)
{
  int           r;
  unsigned long i, n;
  unsigned char m, v;

  i = blk / 8;
  n = *cnt;
  m = 0x80 >> (blk & 7);

  r = (cow->bitmap[i] & m) != 0;

  *cnt = 1;

  if (n <= 1) {
    return (r);
  }

  n = n - 1;
  v = r ? 0xff : 0x00;

  while (n > 0) {
    if (m == 0x01) {
      i += 1;
      m = 0x80;

      while ((n >= 8) && (cow->bitmap[i] == v)) {
        i += 1;
        n -= 8;
        *cnt += 8;
      }
    }
    else {
      m = m >> 1;
    }

    if (((cow->bitmap[i] & m) != 0) != r) {
      return (r);
    }

    n -= 1;
    *cnt += 1;
  }

  return (r);
}

static
int cow_set_block (disk_cow_t *cow, unsigned long blk, unsigned long cnt, int val)
{
  unsigned long i;
  unsigned long i0, i1;
  unsigned char m0, m1;

  cnt = (blk + cnt - 1);

  i0 = blk / 8;
  i1 = cnt / 8;

  m0 = 0x80 >> (blk & 7);
  m1 = 0x80 >> (cnt & 7);

  m0 |= (m0 - 1);
  m1 |= (m1 - 1) ^ 0xff;

  if (val) {
    if (i0 == i1) {
      cow->bitmap[i0] |= (m0 & m1);
    }
    else {
      cow->bitmap[i0] |= m0;
      cow->bitmap[i1] |= m1;
      for (i = i0 + 1; i < i1; i++) {
        cow->bitmap[i] = 0xff;
      }
    }
  }
  else {
    if (i0 == i1) {
      cow->bitmap[i0] &= ~(m0 & m1);
    }
    else {
      cow->bitmap[i0] &= ~m0;
      cow->bitmap[i1] &= ~m1;
      for (i = i0 + 1; i < i1; i++) {
        cow->bitmap[i] = 0x00;
      }
    }
  }

  i1 = i1 - i0 + 1;

  if (fseek (cow->fp, cow->bitmap_offset + i0, SEEK_SET)) {
    return (1);
  }

  if (fwrite (cow->bitmap + i0, 1, i1, cow->fp) != i1) {
    return (1);
  }

  return (0);
}

static
int dsk_cow_read (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  unsigned char *tmp;
  unsigned long cnt;
  disk_cow_t    *cow;

  cow = dsk->ext;
  tmp = buf;

  while (n > 0) {
    if (i >= dsk->blocks) {
      return (1);
    }

    cnt = n;
    if (cow_get_block (cow, i, &cnt)) {
      if (fseek (cow->fp, cow->data_offset + 512UL * i, SEEK_SET)) {
        return (1);
      }

      dsk_fread_zero (tmp, 512, cnt, cow->fp);
    }
    else {
      if (dsk_read_lba (cow->orig, tmp, i, cnt)) {
        return (1);
      }
    }

    i += cnt;
    n -= cnt;
    tmp += 512 * cnt;
  }

  return (0);
}

static
int dsk_cow_write (disk_t *dsk, const void *buf, unsigned long i, unsigned long n)
{
  const unsigned char *tmp;
  unsigned long       cnt;
  disk_cow_t          *cow;

  if (dsk->readonly) {
    return (1);
  }

  cow = dsk->ext;
  tmp = buf;

  while (n > 0) {
    if (i >= dsk->blocks) {
      return (1);
    }

    cnt = n;
    if (cow_get_block (cow, i, &cnt) == 0) {
      cow_set_block (cow, i, cnt, 1);
    }

    if (fseek (cow->fp, cow->data_offset + 512UL * i, SEEK_SET)) {
      return (1);
    }

    if (fwrite (tmp, 512, cnt, cow->fp) != cnt) {
      return (1);
    }

    i += cnt;
    n -= cnt;
    tmp += 512 * cnt;
  }

  fflush (cow->fp);

  return (0);
}

static
int cow_commit_block (disk_cow_t *cow, unsigned long blk)
{
  unsigned char buf[512];

  if (fseek (cow->fp, cow->data_offset + 512UL * blk, SEEK_SET)) {
    return (1);
  }

  dsk_fread_zero (buf, 512, 1, cow->fp);

  if (dsk_write_lba (cow->orig, buf, blk, 1)) {
    return (1);
  }

  if (fseek (cow->fp, cow->data_offset + 512UL * blk, SEEK_SET)) {
    return (1);
  }

  memset (buf, 0, 512);

  if (fwrite (buf, 512, 1, cow->fp) != 1) {
    return (1);
  }

  return (0);
}

static
int dsk_cow_commit (disk_t *dsk)
{
  unsigned long i, n;
  unsigned long blk;
  unsigned char msk;
  disk_cow_t    *cow;

  cow = dsk->ext;

  n = 0;

  for (i = 0; i < cow->bitmap_size; i++) {
    if (cow->bitmap[i] != 0) {
      msk = cow->bitmap[i];
      blk = 8UL * i;

      while (msk != 0) {
        if (msk & 0x80) {
          cow_commit_block (cow, blk);
          n += 1;
        }

        msk = (msk << 1) & 0xff;
        blk += 1;
      }

      cow->bitmap[i] = 0;
    }
  }

  if (n > 0) {
    if (cow_write_bitmap (cow)) {
      return (1);
    }
  }

  fflush (cow->fp);

  return (0);
}

static
void dsk_cow_del (disk_t *dsk)
{
  disk_cow_t *cow;

  cow = dsk->ext;

  dsk_del (cow->orig);

  fclose (cow->fp);

  free (cow->bitmap);
  free (cow);
}

static
int dsk_cow_create (disk_cow_t *cow)
{
  unsigned char buf[32];

  cow->bitmap_size = (cow->dsk.blocks + 7) / 8;
  cow->bitmap_offset = 20;
  cow->data_offset = (20 + cow->bitmap_size + 511) & (~511UL);

  cow_set_uint32 (buf, 0, COW_MAGIC);
  cow_set_uint32 (buf, 4, 0);
  cow_set_uint32 (buf, 8, cow->dsk.blocks);
  cow_set_uint32 (buf, 12, cow->bitmap_offset);
  cow_set_uint32 (buf, 16, cow->data_offset);

  if (fwrite (buf, 1, 20, cow->fp) != 20) {
    return (1);
  }

  memset (cow->bitmap, 0, cow->bitmap_size);

  if (cow_write_bitmap (cow)) {
    return (1);
  }

  return (0);
}

static
int dsk_cow_open (disk_cow_t *cow)
{
  unsigned char buf[32];

  if (fread (buf, 1, 20, cow->fp) != 20) {
    return (1);
  }

  if (cow_get_uint32 (buf, 0) != COW_MAGIC) {
    return (1);
  }

  if (cow_get_uint32 (buf, 4) != 0) {
    return (1);
  }

  if (cow_get_uint32 (buf, 8) != cow->dsk.blocks) {
    return (1);
  }

  cow->bitmap_offset = cow_get_uint32 (buf, 12);
  cow->data_offset = cow_get_uint32 (buf, 16);

  if (cow_read_bitmap (cow)) {
    return (1);
  }

  return (0);
}

static
int dsk_cow_open_file (disk_cow_t *cow, const char *fname)
{
  cow->fp = fopen (fname, "r+b");
  if (cow->fp != NULL) {
    if (dsk_cow_open (cow)) {
      fclose (cow->fp);
      return (1);
    }

    return (0);
  }

  cow->fp = fopen (fname, "w+b");
  if (cow->fp == NULL) {
    return (1);
  }

  if (dsk_cow_create (cow)) {
    fclose (cow->fp);
    return (1);
  }

  return (0);
}

disk_t *dsk_cow_new (disk_t *dsk, const char *fname)
{
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

  cow->bitmap_size = (cow->dsk.blocks + 7) / 8;
  cow->bitmap = (unsigned char *) malloc (cow->bitmap_size);
  if (cow->bitmap == NULL) {
    free (cow);
    return (NULL);
  }

  if (dsk_cow_open_file (cow, fname)) {
    free (cow->bitmap);
    free (cow);
    return (NULL);
  }

  return (&cow->dsk);
}
