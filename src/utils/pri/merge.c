/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/merge.c                                        *
 * Created:     2013-12-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#include "main.h"

#include <stdio.h>
#include <string.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>


typedef struct {
	pri_img_t *img;
	pri_img_t *src;
	char      overwrite;
} pri_merge_t;


static
int pri_merge_track_cb (pri_img_t *src, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_merge_t *par;

	par = opaque;

	if (par->overwrite == 0) {
		if (pri_img_get_track (par->img, c, h, 0) != NULL) {
			return (0);
		}
	}

	if ((trk = pri_img_get_track (par->src, c, h, 0)) == NULL) {
		return (1);
	}

	if ((trk = pri_trk_clone (trk)) == NULL) {
		return (1);
	}

	if (pri_img_set_track (par->img, trk, c, h)) {
		pri_trk_del (trk);
		return (1);
	}

	return (0);
}

int pri_merge_image (pri_img_t *img, const char *fname, int overwrite)
{
	int         r;
	pri_merge_t par;

	par.img = img;
	par.src = pri_img_load (fname, PRI_FORMAT_NONE);

	if (par.src == NULL) {
		return (1);
	}

	par.overwrite = (overwrite != 0);

	r = pri_for_all_tracks (par.src, pri_merge_track_cb, &par);

	pri_img_del (par.src);

	return (r);
}
