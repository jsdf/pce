/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkimg.c                                       *
 * Created:       2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
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


#include "blkimg.h"

#include <stdlib.h>


static
int dsk_img_read (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  disk_img_t *img;

  if ((i + n) > dsk->blocks) {
    return (1);
  }

  img = dsk->ext;

  if (fseek (img->fp, img->start + 512UL * i, SEEK_SET)) {
    return (1);
  }

  dsk_fread_zero (buf, 512, n, img->fp);

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

  img = dsk->ext;

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

  dsk_init (&img->dsk, img, d, c, h, s, ro);

  img->dsk.del = &dsk_img_del;
  img->dsk.read = &dsk_img_read;
  img->dsk.write = &dsk_img_write;

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
