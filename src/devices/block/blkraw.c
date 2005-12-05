/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/blkraw.c                                 *
 * Created:       2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-05 by Hampa Hug <hampa@hampa.ch>                   *
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

  /* boot sector id */
  if ((buf[510] != 0x55) || (buf[511] != 0xaa)) {
    return (NULL);
  }

  /* sector size */
  if (dsk_get_uint16_le (buf, 11) != 512) {
    return (NULL);
  }

  /* total sectors */
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

disk_t *dsk_mbrimg_open_fp (FILE *fp, int ro)
{
  unsigned      i;
  unsigned char *p;
  unsigned char buf[512];
  uint32_t      c, h, s, n;
  uint32_t      tc1, th1, ts1, tn;
  uint32_t      tc2, th2, ts2;
  disk_t        *dsk;

  if (dsk_read (fp, buf, 0, 512)) {
    return (NULL);
  }

  /* mbr id */
  if ((buf[510] != 0x55) || (buf[511] != 0xaa)) {
    return (NULL);
  }

  c = 0;
  h = 0;
  s = 0;
  n = 0;

  p = buf + 0x1be;

  for (i = 0; i < 4; i++) {
    if (p[0] & 0x7f) {
      return (NULL);
    }

    /* partition end, in blocks */
    tn = dsk_get_uint32_le (p, 8);
    tn += dsk_get_uint32_le (p, 12);
    n = (tn > n) ? tn : n;

    /* partition start */
    tc1 = p[3] | ((p[2] & 0xc0) << 2);
    th1 = p[1];
    ts1 = p[2] & 0x3f;
    h = (th1 > h) ? th1 : h;
    s = (ts1 > s) ? ts1 : s;

    /* partition end */
    tc2 = p[7] | ((p[6] & 0xc0) << 2);
    th2 = p[5];
    ts2 = p[6] & 0x3f;
    h = (th2 > h) ? th2 : h;
    s = (ts2 > s) ? ts2 : s;

    /* check if start is before end */
    if (tc2 < tc1) {
      return (NULL);
    }
    else if (tc2 == tc1) {
      if (th2 < th1) {
        return (NULL);
      }
      else if (th2 == th1) {
        if (ts2 < ts1) {
          return (NULL);
        }
      }
    }
  }

  if (s == 0) {
    return (NULL);
  }

  h = h + 1;
  c = n / (h * s);

  if (c == 0) {
    return (NULL);
  }

  dsk = dsk_img_open_fp (fp, c, h, s, ro);

  return (dsk);
}

disk_t *dsk_mbrimg_open (const char *fname, int ro)
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

  dsk = dsk_mbrimg_open_fp (fp, ro);

  if (dsk == NULL) {
    fclose (fp);
    return (NULL);
  }

  return (dsk);
}

disk_t *dsk_fdimg_open_fp (FILE *fp, int ro)
{
  uint64_t cnt;

  if (dsk_get_filesize (fp, &cnt)) {
    return (NULL);
  }

  switch (cnt) {
    case 160UL * 1024UL:
      return (dsk_img_open_fp (fp, 40, 1, 8, ro));

    case 180UL * 1024UL:
      return (dsk_img_open_fp (fp, 40, 1, 9, ro));

    case 320UL * 1024UL:
      return (dsk_img_open_fp (fp, 40, 2, 8, ro));

    case 360UL * 1024UL:
      return (dsk_img_open_fp (fp, 40, 2, 9, ro));

    case 720UL * 1024UL:
      return (dsk_img_open_fp (fp, 80, 2, 9, ro));

    case 1200UL * 1024UL:
      return (dsk_img_open_fp (fp, 80, 2, 15, ro));

    case 1440UL * 1024UL:
      return (dsk_img_open_fp (fp, 80, 2, 18, ro));

    case 2880UL * 1024UL:
      return (dsk_img_open_fp (fp, 80, 2, 36, ro));
  }

  return (NULL);
}

disk_t *dsk_fdimg_open (const char *fname, int ro)
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

  dsk = dsk_fdimg_open_fp (fp, ro);

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
