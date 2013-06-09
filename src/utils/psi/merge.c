/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/merge.c                                        *
 * Created:     2013-06-09 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "merge.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>
#include <drivers/psi/psi-img.h>


static
int psi_merge_sectors_equal (const psi_sct_t *s1, const psi_sct_t *s2)
{
	if ((s1->c != s2->c) || (s1->h != s2->h) || (s1->s != s2->s)) {
		return (0);
	}

	if (s1->n != s2->n) {
		return (0);
	}

	if (memcmp (s1->data, s2->data, s1->n) != 0) {
		return (0);
	}

	return (1);
}

static
int psi_merge_sectors_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque)
{
	psi_img_t *dimg;
	psi_sct_t *dsct, *dtmp;

	dimg = opaque;

	dsct = psi_img_get_sector (dimg, c, h, sct->s, 0);

	if (dsct == NULL) {
		dsct = psi_sct_clone (sct, 1);

		if (dsct == NULL) {
			return (1);
		}

		if (psi_img_add_sector (dimg, dsct, c, h)) {
			psi_sct_del (dsct);
			return (1);
		}

		while (dsct != NULL) {
			par_cnt += 1;
			dsct = dsct->next;
		}

		return (0);
	}

	dtmp = dsct;

	while (dtmp != NULL) {
		if (psi_merge_sectors_equal (dtmp, sct)) {
			break;
		}

		dtmp = dtmp->next;
	}

	if (dtmp == NULL) {
		dtmp = psi_sct_clone (sct, 0);

		if (dtmp == NULL) {
			return (1);
		}

		psi_sct_add_alternate (dsct, dtmp);

		par_cnt += 1;
	}

	return (0);
}

int psi_merge_image (psi_img_t *img, const char *fname)
{
	int       r;
	psi_img_t *src;

	src = psi_load (fname, PSI_FORMAT_NONE);

	if (src == NULL) {
		fprintf (stderr, "%s: loading image failed (%s)\n",
			arg0, fname
		);
		return (1);
	}

	par_cnt = 0;

	r = psi_for_all_sectors (src, psi_merge_sectors_cb, img);

	psi_img_del (src);

	if (par_verbose) {
		fprintf (stderr, "%s: merge %lu sectors from %s\n",
			arg0, par_cnt, fname
		);
	}

	if (r) {
		fprintf (stderr, "%s: merging failed\n", arg0);
	}

	return (r);
}
