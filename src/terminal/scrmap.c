/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/scrmap.c                                      *
 * Created:       2004-05-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-08-01 by Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>

#include "scrmap.h"


void trm_smap_init (trm_scrmap_t *smap)
{
  smap->w = 0;
  smap->h = 0;
  smap->sw = 0;
  smap->sh = 0;

  smap->mapx = NULL;
  smap->mapy = NULL;
  smap->mapw = NULL;
  smap->maph = NULL;
}

void trm_smap_set_map (trm_scrmap_t *smap,
  unsigned w, unsigned h, unsigned sw, unsigned sh)
{
  unsigned long i;
  unsigned      pos, cnt, tmp;

  if (smap->mapx != NULL) {
    free (smap->mapx);
  }

  smap->w = w;
  smap->h = h;
  smap->sw = sw;
  smap->sh = sh;

  smap->mapx = (unsigned *) malloc (2 * (w + h) * sizeof (unsigned));
  if (smap->mapx == NULL) {
    return;
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
}

void trm_smap_free (trm_scrmap_t *smap)
{
  if (smap != NULL) {
    free (smap->mapx);
    smap->mapx = NULL;
  }
}

void trm_smap_get_pixel (trm_scrmap_t *smap, unsigned x, unsigned y,
  unsigned *sx, unsigned *sy, unsigned *sw, unsigned *sh)
{
  if ((x >= smap->w) || (y >= smap->h)) {
    *sx = 0;
    *sy = 0;
    *sw = 0;
    *sh = 0;
    return;
  }

  *sx = smap->mapx[x];
  *sy = smap->mapy[y];
  *sw = smap->mapw[x];
  *sh = smap->maph[y];
}
