/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/new.c                                          *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
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


static
int pri_new_tracks (pri_img_t *img, pri_cyl_t *cyl, unsigned c)
{
	unsigned  h, h0, h1;
	pri_trk_t *trk;

	if (par_trk_all) {
		h0 = 0;
		h1 = cyl->trk_cnt;
	}
	else {
		h0 = par_trk[0];
		h1 = par_trk[1] + 1;
	}

	for (h = h0; h < h1; h++) {
		trk = pri_img_get_track (img, c, h, 0);

		if (trk != NULL) {
			continue;
		}

		trk = pri_img_get_track (img, c, h, 1);

		if (trk == NULL) {
			return (1);
		}

		pri_trk_set_clock (trk, par_data_rate);
	}

	return (0);
}

static
int pri_new_cylinders (pri_img_t *img)
{
	unsigned  c, c0, c1;
	pri_cyl_t *cyl;

	if (par_cyl_all) {
		c0 = 0;
		c1 = img->cyl_cnt;
	}
	else {
		c0 = par_cyl[0];
		c1 = par_cyl[1] + 1;
	}

	for (c = c0; c < c1; c++) {
		cyl = pri_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		if (pri_new_tracks (img, cyl, c)) {
			return (1);
		}
	}

	return (0);
}

int pri_new (pri_img_t *img)
{
	if (pri_new_cylinders (img)) {
		fprintf (stderr, "%s: creating failed\n", arg0);
	}

	return (0);
}
