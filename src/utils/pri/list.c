/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/list.c                                         *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2018 Hampa Hug <hampa@hampa.ch>                     *
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
int pri_list_track_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned   i;
	double     rpm;
	pri_evt_t  *evt;
	const char *str;

	if ((trk->clock > 0) && (trk->size > 0)) {
		rpm = (60.0 * trk->clock) / trk->size;
	}
	else {
		rpm = 0.0;
	}

	printf ("%2lu/%lu: CLK: %lu  BITS: %lu  RPM: %.4f\n",
		c, h,
		pri_trk_get_clock (trk),
		pri_trk_get_size (trk),
		rpm
	);

	if (par_list_long) {
		evt = trk->evt;

		i = 0;

		while (evt != NULL) {
			switch (evt->type) {
			case PRI_EVENT_WEAK:
				str = "WEAK";
				break;

			case PRI_EVENT_CLOCK:
				str = "CLOCK";
				break;

			default:
				str = "UNK";
				break;
			}

			printf ("\t%u: EVT(%lu): %-6s %6lu  0x%08lx\n",
				i, evt->type, str, evt->pos, evt->val
			);

			i += 1;
			evt = evt->next;
		}
	}

	return (0);
}

int pri_list_tracks (pri_img_t *img)
{
	return (pri_for_all_tracks (img, pri_list_track_cb, NULL));
}
