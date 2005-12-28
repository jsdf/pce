/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/scrmap.h                                      *
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


#ifndef PCE_TERMINAL_SCRMAP_H
#define PCE_TERMINAL_SCRMAP_H 1


#include <stdio.h>


typedef struct {
	unsigned x;
	unsigned w;
} trm_scrmap_pixel_t;

typedef struct {
	unsigned w;
	unsigned h;

	unsigned sw;
	unsigned sh;

	unsigned *mapx;
	unsigned *mapw;
	unsigned *mapy;
	unsigned *maph;
} trm_scrmap_t;


void trm_smap_init (trm_scrmap_t *smap);

void trm_smap_free (trm_scrmap_t *smap);


void trm_smap_set_map (trm_scrmap_t *smap,
	unsigned w, unsigned h, unsigned sw, unsigned sh
);

void trm_smap_get_pixel (trm_scrmap_t *smap, unsigned x, unsigned y,
	unsigned *sx, unsigned *sy, unsigned *sw, unsigned *sh
);


#endif
