/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkpce.c                                       *
 * Created:       2004-11-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#include "blkpce.h"

#include <stdlib.h>
#include <string.h>


#define DSK_PCE_MAGIC 0x50494d47UL


/*
PCE image file format

0    4    Magic (PIMG)
4    4    version (0)
8    8    block count
16   8    directory offset
24   8    next offset
32   4    cylinders
36   4    heads
40   4    sectors
44   4    block size
*/


static
int dsk_pce_get_dir (disk_pce_t *img, uint32_t idx)
{
  uint32_t i;
  uint64_t dofs;
  uint64_t *dir;

  if (img->dir[idx] != NULL) {
    return (0);
  }

  if (img->dir_alloc < 256) {
    img->dir[idx] = malloc (img->dir_size * sizeof (uint64_t));
    if (img->dir[idx] != NULL) {
      img->dir_alloc += 1;
    }
  }

  if (img->dir[idx] == NULL) {
    if (img->dir_alloc == 0) {
      return (1);
    }

    while (img->dir[img->dir_next] == NULL) {
      img->dir_next += 1;
      if (img->dir_next >= img->dir_cnt) {
        img->dir_next = 0;
      }
    }

    img->dir[idx] = img->dir[img->dir_next];
    img->dir[img->dir_next] = NULL;
  }

  dofs = img->dir_base + (uint64_t) 8 * img->dir_size * idx;

  if (dsk_read (img->fp, img->dir_buf, dofs, 8 * img->dir_size)) {
    return (1);
  }

  dir = img->dir[idx];

  for (i = 0; i < img->dir_size; i++) {
    dir[i] = dsk_get_uint64_be (img->dir_buf + 8 * i, 0);
  }

  return (0);
}

static
int dsk_pce_get_blk_ofs (disk_pce_t *img, uint64_t blk, uint64_t *ofs)
{
  uint32_t i, j;

  j = blk / img->dir_size;
  i = blk % img->dir_size;

  if (img->dir[j] == NULL) {
    if (dsk_pce_get_dir (img, j)) {
      return (1);
    }
  }

  *ofs = img->dir[j][i];

  return (0);
}

static
int dsk_pce_set_blk_ofs (disk_pce_t *img, uint64_t blk, uint64_t ofs)
{
  uint32_t      i, j;
  uint64_t      dofs;
  unsigned char buf[8];

  j = blk / img->dir_size;
  i = blk % img->dir_size;

  if (img->dir[j] == NULL) {
    if (dsk_pce_get_dir (img, j)) {
      return (1);
    }
  }

  img->dir[j][i] = ofs;

  dsk_set_uint64_be (buf, 0, ofs);

  dofs = img->dir_base + 8 * blk;

  if (dsk_write (img->fp, buf, dofs, 8)) {
    return (1);
  }

  return (0);
}

static
int dsk_pce_set_blk_next (disk_pce_t *img, uint64_t ofs)
{
  unsigned char buf[8];

  img->blk_next = ofs;

  dsk_set_uint64_be (buf, 0, ofs);

  if (dsk_write (img->fp, buf, 24, 8)) {
    return (1);
  }

  return (0);
}

static
int dsk_pce_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
  disk_pce_t    *img;
  uint64_t      ofs;
  unsigned char *t;

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = dsk->ext;

  t = buf;

  while (n > 0) {
    if (dsk_pce_get_blk_ofs (img, i, &ofs)) {
      return (1);
    }

    if (ofs == 0) {
      memset (t, 0, 512);
    }
    else {
      if (dsk_read (img->fp, t, ofs, 512)) {
        return (1);
      }
    }

    t += 512;
    i += 1;
    n -= 1;
  }

  return (0);
}

static
int dsk_pce_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
  disk_pce_t          *img;
  uint64_t            ofs;
  const unsigned char *t;

  if (dsk->readonly) {
    return (1);
  }

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = dsk->ext;

  t = buf;

  while (n > 0) {
    if (dsk_pce_get_blk_ofs (img, i, &ofs)) {
      return (1);
    }

    if (ofs == 0) {
      ofs = img->blk_next;

      if (dsk_pce_set_blk_next (img, img->blk_next + img->blk_size)) {
        return (1);
      }

      if (dsk_pce_set_blk_ofs (img, i, ofs)) {
        return (1);
      }
    }

    if (dsk_write (img->fp, t, ofs, 512)) {
      return (1);
    }

    t += 512;
    i += 1;
    n -= 1;
  }

  fflush (img->fp);

  return (0);
}

static
void dsk_pce_del (disk_t *dsk)
{
  disk_pce_t *img;
  uint32_t   i;

  img = dsk->ext;

  for (i = 0; i < img->dir_cnt; i++) {
    if (img->dir[i] != NULL) {
      free (img->dir[i]);
    }
  }

  free (img->dir);

  fclose (img->fp);

  free (img);
}

disk_t *dsk_pce_open_fp (FILE *fp, int ro)
{
  disk_pce_t    *img;
  uint32_t      c, h, s;
  uint32_t      i;
  unsigned char buf[64];

  if (fread (buf, 1, 64, fp) != 64) {
    return (NULL);
  }

  if (dsk_get_uint32_be (buf, 0) != DSK_PCE_MAGIC) {
    return (NULL);
  }

  if (dsk_get_uint32_be (buf, 4) != 0) {
    return (NULL);
  }

  if (dsk_get_uint32_be (buf, 44) != 512) {
    return (NULL);
  }

  c = dsk_get_uint32_be (buf, 32);
  h = dsk_get_uint32_be (buf, 36);
  s = dsk_get_uint32_be (buf, 40);

  img = malloc (sizeof (disk_pce_t));
  if (img == NULL) {
    return (NULL);
  }

  dsk_init (&img->dsk, img, c, h, s);

  dsk_set_readonly (&img->dsk, ro);

  img->fp = fp;

  img->dsk.del = dsk_pce_del;
  img->dsk.read = dsk_pce_read;
  img->dsk.write = dsk_pce_write;

  img->blk_cnt = dsk_get_uint64_be (buf, 8);
  img->dir_base = dsk_get_uint64_be (buf, 16);
  img->blk_next = dsk_get_uint64_be (buf, 24);
  img->blk_size = dsk_get_uint32_be (buf, 44);

  img->dir_size = 512;
  img->dir_cnt = (img->blk_cnt + img->dir_size - 1) / img->dir_size;
  img->dir_next = 0;
  img->dir_alloc = 0;

  img->dir = malloc (img->dir_cnt * sizeof (uint64_t *));
  img->dir_buf = malloc (8 * img->dir_size);

  if ((img->dir == NULL) || (img->dir_buf == NULL)) {
    free (img->dir_buf);
    free (img->dir);
    free (img);
    return (NULL);
  }

  for (i = 0; i < img->dir_cnt; i++) {
    img->dir[i] = NULL;
  }

  return (&img->dsk);
}

disk_t *dsk_pce_open (const char *fname, int ro)
{
  disk_t *dsk;
  FILE   *fp;

  if (ro) {
    fp = fopen (fname, "rb");
  }
  else {
    fp = fopen (fname, "r+b");
  }

  if (fp == NULL) {
    return (NULL);
  }

  dsk = dsk_pce_open_fp (fp, ro);

  if (dsk == NULL) {
    fclose (fp);
    return (NULL);
  }

  return (dsk);
}

int dsk_pce_create_fp (FILE *fp, uint32_t c, uint32_t h, uint32_t s)
{
  uint64_t      n;
  uint64_t      dir_base, blk_next;
  unsigned char buf[64];

  n = (uint64_t) c * (uint64_t) h * (uint64_t) s;
  if (n == 0) {
    return (1);
  }

  dir_base = 64;
  blk_next = dir_base + 8 * n;
  blk_next = (blk_next + 511) & ~((uint64_t) 511);

  dsk_set_uint32_be (buf, 0, DSK_PCE_MAGIC);
  dsk_set_uint32_be (buf, 4, 0);
  dsk_set_uint64_be (buf, 8, n);
  dsk_set_uint64_be (buf, 16, dir_base);
  dsk_set_uint64_be (buf, 24, blk_next);
  dsk_set_uint32_be (buf, 32, c);
  dsk_set_uint32_be (buf, 36, h);
  dsk_set_uint32_be (buf, 40, s);
  dsk_set_uint32_be (buf, 44, 512);

  if (dsk_write (fp, buf, 0, 48)) {
    return (1);
  }

  dsk_set_uint64_be (buf, 0, 0);

  if (dsk_write (fp, buf, dir_base + 8 * n, 8)) {
    return (1);
  }

  return (0);
}

int dsk_pce_create (const char *fname, uint32_t c, uint32_t h, uint32_t s)
{
  int  r;
  FILE *fp;

  fp = fopen (fname, "w+b");
  if (fp == NULL) {
    return (1);
  }

  r = dsk_pce_create_fp (fp, c, h, s);

  fclose (fp);

  return (r);
}
