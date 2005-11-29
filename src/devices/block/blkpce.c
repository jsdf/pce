/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/blkpce.c                                 *
 * Created:       2004-11-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-11-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
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
8    4    image offset
12   4    block count
16   4    cylinders
20   4    heads
24   4    sectors
28   4    block size
*/


static
int dsk_pce_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
  disk_pce_t *img;
  uint64_t   ofs, cnt;

  img = dsk->ext;

  if ((i + n) > img->blk_cnt) {
    return (1);
  }

  ofs = img->blk_ofs + 512 * (uint64_t) i;
  cnt = 512 * (uint64_t) n;

  if (dsk_read (img->fp, buf, ofs, cnt)) {
    return (1);
  }

  return (0);
}

static
int dsk_pce_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
  disk_pce_t *img;
  uint64_t   ofs, cnt;

  img = dsk->ext;

  if (dsk->readonly) {
    return (1);
  }

  if ((i + n) > img->blk_cnt) {
    return (1);
  }

  ofs = img->blk_ofs + 512 * (uint64_t) i;
  cnt = 512 * (uint64_t) n;

  if (dsk_write (img->fp, buf, ofs, cnt)) {
    return (1);
  }

  fflush (img->fp);

  return (0);
}

static
void dsk_pce_del (disk_t *dsk)
{
  disk_pce_t *img;

  img = dsk->ext;

  if (img->fp != NULL) {
    fclose (img->fp);
  }

  free (img);
}

disk_t *dsk_pce_open_fp (FILE *fp, int ro)
{
  disk_pce_t    *img;
  uint32_t      c, h, s;
  unsigned char buf[32];

  if (fread (buf, 1, 32, fp) != 32) {
    return (NULL);
  }

  if (dsk_get_uint32_be (buf, 0) != DSK_PCE_MAGIC) {
    return (NULL);
  }

  if (dsk_get_uint32_be (buf, 4) != 0) {
    return (NULL);
  }

  if (dsk_get_uint32_be (buf, 28) != 512) {
    return (NULL);
  }

  c = dsk_get_uint32_be (buf, 16);
  h = dsk_get_uint32_be (buf, 20);
  s = dsk_get_uint32_be (buf, 24);

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

  img->blk_ofs = dsk_get_uint32_be (buf, 8);
  img->blk_cnt = dsk_get_uint32_be (buf, 12);
  img->blk_size = dsk_get_uint32_be (buf, 28);

  img->c = c;
  img->h = h;
  img->s = s;

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
  uint32_t      n;
  unsigned char buf[32];

  n = c * h * s;
  if (n == 0) {
    return (1);
  }

  dsk_set_uint32_be (buf, 0, DSK_PCE_MAGIC);
  dsk_set_uint32_be (buf, 4, 0);
  dsk_set_uint32_be (buf, 8, 128);
  dsk_set_uint32_be (buf, 12, n);
  dsk_set_uint32_be (buf, 16, c);
  dsk_set_uint32_be (buf, 20, h);
  dsk_set_uint32_be (buf, 24, s);
  dsk_set_uint32_be (buf, 28, 512);

  if (dsk_write (fp, buf, 0, 32)) {
    return (1);
  }

  buf[0] = 0;
  if (dsk_write (fp, buf, 128 + 512 * (uint64_t) n - 1, 1)) {
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
