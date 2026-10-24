/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/delete.c                                       *
 * Created:     2013-12-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pfi/pfi.h>


static
int pfi_delete_track_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pfi_img_del_track (img, c, h);

	return (0);
}

int pfi_delete_tracks (pfi_img_t *img)
{
	return (pfi_for_all_tracks (img, pfi_delete_track_cb, NULL));
}


int pfi_double_step (pfi_img_t *img, int even)
{
	unsigned  c, cn;
	pfi_cyl_t *cyl;

	cn = pfi_img_get_cyl_cnt (img);

	for (c = 0; c < cn; c++) {
		cyl = pfi_img_rmv_cylinder (img, c);

		if (((c & 1) && even) || (((c & 1) == 0) && !even)) {
			pfi_cyl_del (cyl);
		}
		else {
			pfi_img_set_cylinder (img, cyl, c / 2);
		}
	}

	return (0);
}
