/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/video.c                                          *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: video.c,v 1.4 2003/09/14 21:27:38 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


void pce_video_init (video_t *vid)
{
  vid->ext = NULL;

  vid->type = PCE_VIDEO_NONE;

  vid->del = NULL;
  vid->get_mem = NULL;
  vid->get_reg = NULL;
  vid->prt_state = NULL;
}

void pce_video_del (video_t *vid)
{
  if (vid->del != NULL) {
    vid->del (vid->ext);
  }
}

mem_blk_t *pce_video_get_mem (video_t *vid)
{
  if (vid->get_mem != NULL) {
    return (vid->get_mem (vid->ext));
  }

  return (NULL);
}

mem_blk_t *pce_video_get_reg (video_t *vid)
{
  if (vid->get_reg != NULL) {
    return (vid->get_reg (vid->ext));
  }

  return (NULL);
}

void pce_video_prt_state (video_t *vid, FILE *fp)
{
  if (vid->prt_state != NULL) {
    vid->prt_state (vid->ext, fp);
  }
}

int pce_video_screenshot (video_t *vid, FILE *fp, unsigned mode)
{
  if (vid->screenshot != NULL) {
    return (vid->screenshot (vid->ext, fp, mode));
  }

  return (1);
}

void pce_smap_init (scrmap_t *smap,
  unsigned w, unsigned h, unsigned sw, unsigned sh)
{
  unsigned long i, n;
  unsigned      x, y;
  unsigned      px, pw, tx;
  unsigned      py, ph, ty;

  smap->w = w;
  smap->h = h;
  smap->sw = sw;
  smap->sh = sh;

  n = (unsigned long) w * (unsigned long) h;

  smap->pxl = (scrmap_pixel_t *) malloc (n * sizeof (scrmap_pixel_t));
  if (smap->pxl == NULL) {
    return;
  }

  i = 0;
  py = 0;
  ty = 0;

  for (y = 0; y < h; y++) {
    ty += sh;
    ph = ty / h;
    ty = ty % h;

    px = 0;
    tx = 0;

    for (x = 0; x < w; x++) {
      tx += sw;
      pw = tx / w;
      tx = tx % w;

      smap->pxl[i].x = px;
      smap->pxl[i].y = py;
      smap->pxl[i].w = pw;
      smap->pxl[i].h = ph;

      i += 1;
      px += pw;
    }

    py += ph;
  }
}

void pce_smap_free (scrmap_t *smap)
{
  if (smap != NULL) {
    free (smap->pxl);
    smap->pxl = NULL;
  }
}

void pce_smap_get_pixel (scrmap_t *smap, unsigned x, unsigned y,
  unsigned *sx, unsigned *sy, unsigned *sw, unsigned *sh)
{
  unsigned long i;

  if ((x >= smap->w) || (y >= smap->h)) {
    *sx = 0;
    *sy = 0;
    *sw = 0;
    *sh = 0;
    return;
  }

  i = (unsigned long) smap->w * y + x;

  *sx = smap->pxl[i].x;
  *sy = smap->pxl[i].y;
  *sw = smap->pxl[i].w;
  *sh = smap->pxl[i].h;
}
