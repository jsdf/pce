/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/delete.c                                       *
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
#include "delete.h"

#include <stdio.h>
#include <string.h>

#include <drivers/pri/pri.h>


static
int pri_delete_track_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_img_del_track (img, c, h);

	return (0);
}

int pri_delete_tracks (pri_img_t *img)
{
	return (pri_for_all_tracks (img, pri_delete_track_cb, NULL));
}


int pri_double_step (pri_img_t *img, int even)
{
	unsigned  c, cn;
	pri_cyl_t *cyl;

	cn = pri_img_get_cyl_cnt (img);

	for (c = 0; c < cn; c++) {
		cyl = pri_img_rmv_cylinder (img, c);

		if (((c & 1) && even) || (((c & 1) == 0) && !even)) {
			pri_cyl_del (cyl);
		}
		else {
			pri_img_set_cylinder (img, cyl, c / 2);
		}
	}

	return (0);
}
