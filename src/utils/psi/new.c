/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/new.c                                          *
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
#include "new.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>
#include <drivers/psi/psi-img-raw.h>


static
int psi_new_alternates (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h)
{
	unsigned a;

	if (par_alt_all) {
		return (0);
	}

	if (par_alt[1] == 0) {
		return (0);
	}

	for (a = 1; a <= par_alt[1]; a++) {
		if (sct->next == NULL) {
			sct->next = psi_sct_clone (sct, 0);

			if (sct->next == NULL) {
				return (1);
			}

			par_cnt += 1;
		}

		sct = sct->next;
	}

	return (0);
}

static
int psi_new_sectors (psi_img_t *img, psi_trk_t *trk, unsigned c, unsigned h)
{
	unsigned  s;
	psi_sct_t *sct;

	if (par_sct_all) {
		for (s = 0; s < trk->sct_cnt; s++) {
			if (psi_new_alternates (img, trk->sct[s], c, h)) {
				return (1);
			}
		}
	}
	else {
		for (s = par_sct[0]; s <= par_sct[1]; s++) {
			sct = psi_img_get_sector (img, c, h, s, 0);

			if (sct == NULL) {
				sct = psi_sct_new (c, h, s, 512);

				if (sct == NULL) {
					return (1);
				}

				if (psi_trk_add_sector (trk, sct)) {
					psi_sct_del (sct);
					return (1);
				}

				psi_sct_fill (sct, par_filler);

				par_cnt += 1;
			}

			if (psi_new_alternates (img, sct, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int psi_new_tracks (psi_img_t *img, psi_cyl_t *cyl, unsigned c)
{
	unsigned  h, h0, h1;
	psi_trk_t *trk;

	if (par_trk_all) {
		h0 = 0;
		h1 = cyl->trk_cnt;
	}
	else {
		h0 = par_trk[0];
		h1 = par_trk[1] + 1;
	}

	for (h = h0; h < h1; h++) {
		trk = psi_img_get_track (img, c, h, 1);

		if (trk == NULL) {
			return (1);
		}

		if (psi_new_sectors (img, trk, c, h)) {
			return (1);
		}
	}

	return (0);
}

static
int psi_new_cylinders (psi_img_t *img)
{
	unsigned  c, c0, c1;
	psi_cyl_t *cyl;

	if (par_cyl_all) {
		c0 = 0;
		c1 = img->cyl_cnt;
	}
	else {
		c0 = par_cyl[0];
		c1 = par_cyl[1] + 1;
	}

	for (c = c0; c < c1; c++) {
		cyl = psi_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		if (psi_new_tracks (img, cyl, c)) {
			return (1);
		}
	}

	return (0);
}

int psi_new (psi_img_t **img)
{
	int r;

	if (*img == NULL) {
		*img = psi_img_new();

		if (*img == NULL) {
			return (1);
		}
	}

	par_cnt = 0;

	r = psi_new_cylinders (*img);

	if (par_verbose) {
		fprintf (stderr, "%s: create %lu sectors\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: creating failed\n", arg0);
	}

	return (0);
}


static
int psi_new_dos (psi_img_t *img, unsigned long size, unsigned fill)
{
	unsigned             c, h, s;
	psi_cyl_t            *cyl;
	psi_trk_t            *trk;
	psi_sct_t            *sct;
	const psi_geometry_t *geo;

	psi_img_erase (img);

	if (size == 0) {
		return (0);
	}

	geo = psi_get_geometry_from_size (1024 * size, 1023);

	if (geo == NULL) {
		return (1);
	}

	for (c = 0; c < geo->c; c++) {
		cyl = psi_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		for (h = 0; h < geo->h; h++) {
			trk = psi_img_get_track (img, c, h, 1);

			if (trk == NULL) {
				return (1);
			}

			for (s = 0; s < geo->s; s++) {
				sct = psi_sct_new (c, h, s + 1, geo->ssize);

				if (sct == NULL) {
					return (1);
				}

				if (psi_trk_add_sector (trk, sct)) {
					psi_sct_del (sct);
					return (1);
				}

				psi_sct_set_encoding (sct, geo->encoding);
				psi_sct_fill (sct, fill);
			}
		}
	}

	return (0);
}

static
int psi_new_mac (psi_img_t *img, unsigned long size)
{
	unsigned  c, h, s;
	unsigned  trk_cnt, sct_cnt;
	psi_cyl_t *cyl;
	psi_trk_t *trk;
	psi_sct_t *sct;

	psi_img_erase (img);

	if (size == 400) {
		trk_cnt = 1;
	}
	else if (size == 800) {
		trk_cnt = 2;
	}
	else {
		return (1);
	}

	sct_cnt = 13;

	for (c = 0; c < 80; c++) {
		cyl = psi_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		if ((c & 15) == 0) {
			sct_cnt -= 1;
		}

		for (h = 0; h < trk_cnt; h++) {
			trk = psi_img_get_track (img, c, h, 1);

			if (trk == NULL) {
				return (1);
			}

			for (s = 0; s < sct_cnt; s++) {
				sct = psi_sct_new (c, h, s, 512);

				if (sct == NULL) {
					return (1);
				}

				if (psi_trk_add_sector (trk, sct)) {
					psi_sct_del (sct);
					return (1);
				}

				psi_sct_set_encoding (sct, PSI_ENC_GCR);
				psi_sct_fill (sct, par_filler);
			}
		}
	}

	return (0);
}

psi_img_t *psi_new_image (const char *type, const char *size)
{
	int           r;
	unsigned long n;
	psi_img_t     *img;

	n = strtoul (size, NULL, 0);

	img = psi_img_new();

	if (strcmp (type, "cpm") == 0) {
		r = psi_new_dos (img, n, 0xe5);
	}
	else if (strcmp (type, "dos") == 0) {
		r = psi_new_dos (img, n, 0xf6);
	}
	else if (strcmp (type, "mac") == 0) {
		r = psi_new_mac (img, n);
	}
	else {
		r = 1;
	}

	if (r) {
		fprintf (stderr, "%s: bad image type/size (%s/%s)\n",
			arg0, type, size
		);

		psi_img_del (img);

		return (NULL);
	}

	return (img);
}
