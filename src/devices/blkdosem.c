/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkdosem.c                                     *
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


#include "blkdosem.h"

#include <stdlib.h>
#include <string.h>


static
unsigned long buf_get_uint32 (const void *buf, unsigned i)
{
  const unsigned char *tmp;

  tmp = (const unsigned char *) buf + i;

  return ((tmp[3] << 24) | (tmp[2] << 16) | (tmp[1] << 8) | tmp[0]);
}

static
void buf_set_uint32 (void *buf, unsigned i, unsigned long v)
{
  unsigned char *tmp;

  tmp = (unsigned char *) buf + i;

  tmp[0] = v & 0xff;
  tmp[1] = (v >> 8) & 0xff;
  tmp[2] = (v >> 16) & 0xff;
  tmp[3] = (v >> 24) & 0xff;
}


static
int dsk_dosemu_read (disk_t *dsk, void *buf, unsigned long i, unsigned long n)
{
  disk_dosemu_t *img;

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
int dsk_dosemu_write (disk_t *dsk, const void *buf, unsigned long i, unsigned long n)
{
  disk_dosemu_t *img;

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
void dsk_dosemu_del (disk_t *dsk)
{
  disk_dosemu_t *img;

  img = dsk->ext;

  fclose (img->fp);
  free (img);
}

disk_t *dsk_dosemu_new (unsigned d, const char *fname, int ro)
{
  disk_dosemu_t *img;
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

  c = buf_get_uint32 (buf, 15);
  h = buf_get_uint32 (buf, 7);
  s = buf_get_uint32 (buf, 11);
  start = buf_get_uint32 (buf, 19);

  if (start < 128) {
    return (NULL);
  }

  img = (disk_dosemu_t *) malloc (sizeof (disk_dosemu_t));
  if (img == NULL) {
    return (NULL);
  }

  dsk_init (&img->dsk, img, d, c, h, s, ro);

  img->dsk.del = &dsk_dosemu_del;
  img->dsk.read = &dsk_dosemu_read;
  img->dsk.write = &dsk_dosemu_write;

  img->start = start;

  if (ro) {
    img->fp = fopen (fname, "rb");
  }
  else {
    img->fp = fopen (fname, "r+b");
  }

  if (img->fp == NULL) {
    free (img);
    return (NULL);
  }

  return (&img->dsk);
}

disk_t *dsk_dosemu_create (unsigned d, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro)
{
  unsigned long n;
  unsigned char buf[512];
  FILE          *fp;

  /* make sure the file doesn't exist */
  fp = fopen (fname, "rb");
  if (fp != NULL) {
    fclose (fp);
    return (NULL);
  }

  fp = fopen (fname, "wb");
  if (fp == NULL) {
    return (NULL);
  }

  memset (buf, 0, 128);

  memcpy (buf, "DOSEMU\x00", 7);

  buf_set_uint32 (buf, 7, h);
  buf_set_uint32 (buf, 11, s);
  buf_set_uint32 (buf, 15, c);
  buf_set_uint32 (buf, 19, 128);

  fwrite (buf, 1, 128, fp);

  memset (buf, 0, 512);

  n = (unsigned long) c * (unsigned long) h * (unsigned long) s;

  while (n > 0) {
    fwrite (buf, 1, 512, fp);
    n -= 1;
  }

  fclose (fp);

  return (dsk_dosemu_new (d, fname, ro));
}
