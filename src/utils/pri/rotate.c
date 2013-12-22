/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/rotate.c                                       *
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
int pri_align_gcr_track_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      val, run;
	unsigned long bit;
	unsigned long syn1, syn2, syn_cnt;
	unsigned long max1, max2, max_cnt;

	pri_trk_set_pos (trk, 0);

	syn1 = 0;
	syn2 = 0;
	syn_cnt = 0;

	max1 = 0;
	max2 = 0;
	max_cnt = 0;

	val = 0;
	run = 0;

	while (1) {
		pri_trk_get_bits (trk, &bit, 1);

		val = (val << 1) | (bit & 1);
		run += 1;

		if ((val & 0x3ff) == 0xff) {
			syn2 = trk->idx;
			syn_cnt += 1;

			if (syn_cnt > max_cnt) {
				max1 = syn1;
				max2 = syn2;
				max_cnt = syn_cnt;
			}

			run = 0;
		}
		else if (run >= 18) {
			if (trk->wrap) {
				break;
			}

			syn1 = trk->idx;
			syn_cnt = 0;
		}
	}

	max1 = (max1 + (max2 - max1) / 2) % trk->size;

	pri_trk_rotate (trk, max1);

	return (0);
}

int pri_align_gcr_tracks (pri_img_t *img)
{
	return (pri_for_all_tracks (img, pri_align_gcr_track_cb, NULL));
}


static
int pri_rotate_track_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long *idx;

	idx = opaque;

	if (pri_trk_rotate (trk, *idx)) {
		return (1);
	}

	return (0);
}

int pri_rotate_tracks (pri_img_t *img, unsigned long idx)
{
	return (pri_for_all_tracks (img, pri_rotate_track_cb, &idx));
}
