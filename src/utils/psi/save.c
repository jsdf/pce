/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/save.c                                         *
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
#include "save.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


static
int psi_save_sectors_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque)
{
	FILE *fp;

	fp = opaque;

	if (fwrite (sct->data, 1, sct->n, fp) != sct->n) {
		return (1);
	}

	par_cnt += 1;

	return (0);
}

int psi_save_sectors (psi_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		fprintf (stderr, "%s: can't create file (%s)\n", arg0, fname);
		return (1);
	}

	par_cnt = 0;

	r = psi_for_all_sectors (img, psi_save_sectors_cb, fp);

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: save %lu sectors to %s\n",
			arg0, par_cnt, fname
		);
	}

	if (r) {
		fprintf (stderr, "%s: saving sectors failed\n", arg0);
	}

	return (r);
}


static
int psi_save_tags_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque)
{
	FILE          *fp;
	unsigned char buf[12];

	fp = opaque;

	psi_sct_get_tags (sct, buf, 12);

	if (fwrite (buf, 1, 12, fp) != 12) {
		return (1);
	}

	par_cnt += 1;

	return (0);
}

int psi_save_tags (psi_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		fprintf (stderr, "%s: can't create file (%s)\n", arg0, fname);
		return (1);
	}

	par_cnt = 0;

	r = psi_for_all_sectors (img, psi_save_tags_cb, fp);

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: save %lu sector tags to %s\n",
			arg0, par_cnt, fname
		);
	}

	if (r) {
		fprintf (stderr, "%s: saving tags failed\n", arg0);
	}

	return (r);
}
