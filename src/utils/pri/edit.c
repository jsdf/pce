/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/edit.c                                         *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2019 Hampa Hug <hampa@hampa.ch>                     *
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

static
int pri_edit_image (pri_img_t *img, const char *what, unsigned long val)
{
	if (strcmp (what, "readonly") == 0) {
		img->readonly = (val != 0);
		return (0);
	}
	else if (strcmp (what, "woz-cleaned") == 0) {
		img->woz_cleaned = (val != 0);
		return (0);
	}
	else if (strcmp (what, "woz-track-sync") == 0) {
		img->woz_track_sync = (val != 0);
		return (0);
	}
	else {
		return (-1);
	}
}

static
int pri_edit_tracks (pri_img_t *img, const char *what, unsigned long val)
{
	int           r;
	pri_trk_cb    fct;

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
		return (-1);
	}

	r = pri_for_all_tracks (img, fct, &val);

	if (r) {
		fprintf (stderr, "%s: editing failed (%s = %lu)\n",
			arg0, what, val
		);
	}

	return (r);
}

int pri_edit (pri_img_t *img, const char *what, const char *val)
{
	int           r;
	unsigned long v;

	v = strtoul (val, NULL, 0);

	if ((r = pri_edit_image (img, what, v)) >= 0) {
		return (r);
	}

	if ((r = pri_edit_tracks (img, what, v)) >= 0) {
		return (r);
	}

	fprintf (stderr, "%s: unknown field (%s)\n", arg0, what);

	return (1);
}
