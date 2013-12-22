/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/edit.c                                         *
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
#include <stdlib.h>

#include <drivers/pri/pri.h>


static
int pri_edit_clock_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *p)
{
	pri_trk_set_clock (trk, *(unsigned long *) p);
	return (0);
}

static
int pri_edit_data_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *p)
{
	pri_trk_clear (trk, *(unsigned long *) p);
	return (0);
}

static
int pri_edit_size_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *p)
{
	pri_trk_set_size (trk, *(unsigned long *) p);
	return (0);
}

int pri_edit_tracks (pri_img_t *img, const char *what, const char *val)
{
	int           r;
	unsigned long v;
	pri_trk_cb    fct;

	v = strtoul (val, NULL, 0);

	if (strcmp (what, "clock") == 0) {
		fct = pri_edit_clock_cb;
	}
	else if (strcmp (what, "data") == 0) {
		fct = pri_edit_data_cb;
	}
	else if (strcmp (what, "size") == 0) {
		fct = pri_edit_size_cb;
	}
	else {
		fprintf (stderr, "%s: unknown field (%s)\n", arg0, what);
		return (1);
	}

	r = pri_for_all_tracks (img, fct, &v);

	if (r) {
		fprintf (stderr, "%s: editing failed (%s = %lu)\n",
			arg0, what, v
		);
	}

	return (r);
}
