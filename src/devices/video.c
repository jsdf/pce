/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/video.c                                        *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-05-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdio.h>
#include <stdlib.h>

#include "video.h"


void pce_video_init (video_t *vid)
{
  vid->ext = NULL;

  vid->type = PCE_VIDEO_NONE;

  vid->del = NULL;
  vid->get_mem = NULL;
  vid->get_reg = NULL;
  vid->prt_state = NULL;
  vid->update = NULL;
  vid->dump = NULL;
  vid->screenshot = NULL;
  vid->clock = NULL;
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

void pce_video_update (video_t *vid)
{
  if (vid->update != NULL) {
    vid->update (vid->ext);
  }
}

int pce_video_dump (video_t *vid, FILE *fp)
{
  if (vid->dump != NULL) {
    return (vid->dump (vid->ext, fp));
  }

  return (1);
}

int pce_video_screenshot (video_t *vid, FILE *fp, unsigned mode)
{
  if (vid->screenshot != NULL) {
    return (vid->screenshot (vid->ext, fp, mode));
  }

  return (1);
}

void pce_video_clock (video_t *vid, unsigned long cnt)
{
  if (vid->clock != NULL) {
    vid->clock (vid->ext, cnt);
  }
}


int pce_smap_init (scrmap_t *smap, unsigned w, unsigned h, unsigned sw, unsigned sh)
{
  unsigned long i;
  unsigned      pos, cnt, tmp;

  smap->w = w;
  smap->h = h;
  smap->sw = sw;
  smap->sh = sh;

  smap->mapx = (unsigned *) malloc (2 * (w + h) * sizeof (unsigned));
  if (smap->mapx == NULL) {
    return (1);
  }

  smap->mapy = smap->mapx + w;
  smap->mapw = smap->mapy + h;
  smap->maph = smap->mapw + w;

  pos = 0;
  tmp = 0;
  for (i = 0; i < w; i++) {
    tmp += sw;
    cnt = tmp / w;
    tmp = tmp % w;

    smap->mapx[i] = pos;
    smap->mapw[i] = cnt;

    pos += cnt;
  }

  pos = 0;
  tmp = 0;
  for (i = 0; i < h; i++) {
    tmp += sh;
    cnt = tmp / h;
    tmp = tmp % h;

    smap->mapy[i] = pos;
    smap->maph[i] = cnt;

    pos += cnt;
  }

  return (0);
}

void pce_smap_free (scrmap_t *smap)
{
  if (smap != NULL) {
    free (smap->mapx);
    smap->mapx = NULL;
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

  *sx = smap->mapx[x];
  *sy = smap->mapy[y];
  *sw = smap->mapw[x];
  *sh = smap->maph[y];
}
