/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/blkraw.c                                 *
 * Created:       2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-02-26 by Hampa Hug <hampa@hampa.ch>                   *
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


#include "blkraw.h"

#include <stdlib.h>


static
int dsk_img_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
  disk_img_t *img;
  uint64_t   ofs, cnt;

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = dsk->ext;

  ofs = img->start + 512 * (uint64_t) i;
  cnt = 512 * (uint64_t) n;

  if (dsk_read (img->fp, buf, ofs, cnt)) {
    return (1);
  }

  return (0);
}

static
int dsk_img_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
  disk_img_t *img;
  uint64_t   ofs, cnt;

  if (dsk->readonly) {
    return (1);
  }

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = dsk->ext;

  ofs = img->start + 512 * (uint64_t) i;
  cnt = 512 * (uint64_t) n;

  if (dsk_write (img->fp, buf, ofs, cnt)) {
    return (1);
  }

  fflush (img->fp);

  return (0);
}

static
void dsk_img_del (disk_t *dsk)
{
  disk_img_t *img;

  img = dsk->ext;

  fclose (img->fp);
  free (img);
}

disk_t *dsk_img_open_fp (FILE *fp, uint32_t c, uint32_t h, uint32_t s, int ro)
{
  disk_img_t *img;

  img = malloc (sizeof (disk_img_t));
  if (img == NULL) {
    return (NULL);
  }

  dsk_init (&img->dsk, img, c, h, s);

  dsk_set_readonly (&img->dsk, ro);

  img->dsk.del = dsk_img_del;
  img->dsk.read = dsk_img_read;
  img->dsk.write = dsk_img_write;

  img->start = 0;

  img->fp = fp;

  return (&img->dsk);
}

disk_t *dsk_img_open (const char *fname, uint32_t c, uint32_t h, uint32_t s, int ro)
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

  dsk = dsk_img_open_fp (fp, c, h, s, ro);

  if (dsk == NULL) {
    fclose (fp);
    return (NULL);
  }

  return (dsk);
}

disk_t *dsk_dosimg_open_fp (FILE *fp, int ro)
{
  unsigned char buf[512];
  uint32_t      c, h, s, n;
  disk_t        *dsk;

  if (dsk_read (fp, buf, 0, 512)) {
    return (NULL);
  }

  if ((buf[510] != 0x55) || (buf[511] != 0xaa)) {
    return (NULL);
  }

  if (dsk_get_uint16_le (buf, 11) != 512) {
    return (NULL);
  }

  n = dsk_get_uint16_le (buf, 19);
  if (n == 0) {
    n = dsk_get_uint32_le (buf, 32);
  }

  h = dsk_get_uint16_le (buf, 26);
  s = dsk_get_uint16_le (buf, 24);
  c = n / (h * s);

  dsk = dsk_img_open_fp (fp, c, h, s, ro);

  return (dsk);
}

disk_t *dsk_dosimg_open (const char *fname, int ro)
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

  dsk = dsk_dosimg_open_fp (fp, ro);

  if (dsk == NULL) {
    fclose (fp);
    return (NULL);
  }

  return (dsk);
}

void dsk_img_set_offset (disk_t *dsk, uint64_t ofs)
{
  disk_img_t *img;

  img = dsk->ext;

  img->start = ofs;
}

int dsk_img_create_fp (FILE *fp, uint32_t c, uint32_t h, uint32_t s)
{
  uint64_t      cnt;
  unsigned char buf;

  cnt = 512 * (uint64_t) (c * h * s);
  if (cnt == 0) {
    return (1);
  }

  buf = 0;

  if (dsk_write (fp, &buf, cnt - 1, 1)) {
    return (1);
  }

  return (0);
}

int dsk_img_create (const char *fname, uint32_t c, uint32_t h, uint32_t s)
{
  int  r;
  FILE *fp;

  fp = fopen (fname, "w+b");
  if (fp == NULL) {
    return (1);
  }

  r = dsk_img_create_fp (fp, c, h, s);

  fclose (fp);

  return (r);
}
