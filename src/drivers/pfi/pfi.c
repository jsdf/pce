/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi.c                                        *
 * Created:     2012-01-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pfi.h"
#include "track.h"


pfi_cyl_t *pfi_cyl_new (void)
{
	pfi_cyl_t *cyl;

	if ((cyl = malloc (sizeof (pfi_cyl_t))) == NULL) {
		return (NULL);
	}

	cyl->trk_cnt = 0;
	cyl->trk = NULL;

	return (cyl);
}

void pfi_cyl_del (pfi_cyl_t *cyl)
{
	unsigned long i;

	if (cyl != NULL) {
		for (i = 0; i < cyl->trk_cnt; i++) {
			pfi_trk_del (cyl->trk[i]);
		}

		free (cyl->trk);
		free (cyl);
	}
}

pfi_cyl_t *pfi_cyl_clone (const pfi_cyl_t *cyl)
{
	unsigned long i;
	pfi_trk_t     *trk;
	pfi_cyl_t     *ret;

	if ((ret = pfi_cyl_new()) == NULL) {
		return (NULL);
	}

	for (i = 0; i < cyl->trk_cnt; i++) {
		if (cyl->trk[i] == NULL) {
			continue;
		}

		if ((trk = pfi_trk_clone (cyl->trk[i], 1, 1)) == NULL) {
			pfi_cyl_del (ret);
			return (NULL);
		}

		pfi_cyl_set_track (ret, trk, i);
	}

	return (ret);
}

unsigned long pfi_cyl_get_trk_cnt (const pfi_cyl_t *cyl)
{
	return (cyl->trk_cnt);
}

pfi_trk_t *pfi_cyl_get_track (pfi_cyl_t *cyl, unsigned long idx, int alloc)
{
	pfi_trk_t *trk;

	if ((idx < cyl->trk_cnt) && (cyl->trk[idx] != NULL)) {
		return (cyl->trk[idx]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	if ((trk = pfi_trk_new()) == NULL) {
		return (NULL);
	}

	if (pfi_cyl_set_track (cyl, trk, idx)) {
		pfi_trk_del (trk);
		return (NULL);
	}

	return (trk);
}

const pfi_trk_t *pfi_cyl_get_track_const (const pfi_cyl_t *cyl, unsigned long idx)
{
	if (idx < cyl->trk_cnt) {
		return (cyl->trk[idx]);
	}

	return (NULL);
}

int pfi_cyl_set_track (pfi_cyl_t *cyl, pfi_trk_t *trk, unsigned long idx)
{
	unsigned long i;
	pfi_trk_t     **tmp;

	if (idx < cyl->trk_cnt) {
		pfi_trk_del (cyl->trk[idx]);

		cyl->trk[idx] = trk;

		return (0);
	}

	tmp = realloc (cyl->trk, (idx + 1) * sizeof (pfi_trk_t *));

	if (tmp == NULL) {
		return (1);
	}

	for (i = cyl->trk_cnt; i < idx; i++) {
		tmp[i] = NULL;
	}

	tmp[idx] = trk;

	cyl->trk = tmp;
	cyl->trk_cnt = idx + 1;

	return (0);
}

int pfi_cyl_add_track (pfi_cyl_t *cyl, pfi_trk_t *trk)
{
	return (pfi_cyl_set_track (cyl, trk, cyl->trk_cnt));
}

int pfi_cyl_del_track (pfi_cyl_t *cyl, unsigned long idx)
{
	if (idx >= cyl->trk_cnt) {
		return (1);
	}

	if (cyl->trk[idx] == NULL) {
		return (1);
	}

	pfi_trk_del (cyl->trk[idx]);

	cyl->trk[idx] = NULL;

	while ((cyl->trk_cnt > 0) && (cyl->trk[cyl->trk_cnt - 1] == NULL)) {
		cyl->trk_cnt -= 1;
	}

	return (0);
}


pfi_img_t *pfi_img_new (void)
{
	pfi_img_t *img;

	if ((img = malloc (sizeof (pfi_img_t))) == NULL) {
		return (NULL);
	}

	img->cyl_cnt = 0;
	img->cyl = NULL;

	img->comment_size = 0;
	img->comment = NULL;

	return (img);
}

void pfi_img_del (pfi_img_t *img)
{
	unsigned long i;

	if (img != NULL) {
		for (i = 0; i < img->cyl_cnt; i++) {
			pfi_cyl_del (img->cyl[i]);
		}

		free (img->comment);
		free (img->cyl);
		free (img);
	}
}

unsigned long pfi_img_get_cyl_cnt (const pfi_img_t *img)
{
	return (img->cyl_cnt);
}

unsigned long pfi_img_get_trk_cnt (const pfi_img_t *img, unsigned long c)
{
	if (c >= img->cyl_cnt) {
		return (0);
	}

	if (img->cyl[c] == NULL) {
		return (0);
	}

	return (img->cyl[c]->trk_cnt);
}

unsigned long pfi_img_get_trk_max (const pfi_img_t *img)
{
	unsigned long c, n;

	n = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		if (img->cyl[c] != NULL) {
			if (img->cyl[c]->trk_cnt > n) {
				n = img->cyl[c]->trk_cnt;
			}
		}
	}

	return (n);
}

static
void pfi_img_fix_cyl (pfi_img_t *img)
{
	while ((img->cyl_cnt > 0) && (img->cyl[img->cyl_cnt - 1] == NULL)) {
		img->cyl_cnt -= 1;
	}
}

pfi_cyl_t *pfi_img_get_cylinder (pfi_img_t *img, unsigned long idx, int alloc)
{
	pfi_cyl_t *cyl;

	if ((idx < img->cyl_cnt) && (img->cyl[idx] != NULL)) {
		return (img->cyl[idx]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	if ((cyl = pfi_cyl_new()) == NULL) {
		return (NULL);
	}

	if (pfi_img_set_cylinder (img, cyl, idx)) {
		pfi_cyl_del (cyl);
		return (NULL);
	}

	return (cyl);
}

const pfi_cyl_t *pfi_img_get_cylinder_const (const pfi_img_t *img, unsigned long idx)
{
	if (idx < img->cyl_cnt) {
		return (img->cyl[idx]);
	}

	return (0);
}

pfi_cyl_t *pfi_img_rmv_cylinder (pfi_img_t *img, unsigned long idx)
{
	pfi_cyl_t *cyl;

	if (idx >= img->cyl_cnt) {
		return (NULL);
	}

	if (img->cyl[idx] == NULL) {
		return (NULL);
	}

	cyl = img->cyl[idx];

	img->cyl[idx] = NULL;

	pfi_img_fix_cyl (img);

	return (cyl);
}

pfi_trk_t *pfi_img_rmv_track (pfi_img_t *img, unsigned long c, unsigned long h)
{
	pfi_cyl_t *cyl;
	pfi_trk_t *trk;

	cyl = pfi_img_get_cylinder (img, c, 0);

	if (cyl == NULL) {
		return (NULL);
	}

	if (h >= cyl->trk_cnt) {
		return (NULL);
	}

	trk = cyl->trk[h];

	cyl->trk[h] = NULL;

	return (trk);
}

int pfi_img_set_cylinder (pfi_img_t *img, pfi_cyl_t *cyl, unsigned long idx)
{
	unsigned long i;
	pfi_cyl_t     **tmp;

	if (idx < img->cyl_cnt) {
		pfi_cyl_del (img->cyl[idx]);
		img->cyl[idx] = cyl;
		pfi_img_fix_cyl (img);

		return (0);
	}

	tmp = realloc (img->cyl, (idx + 1) * sizeof (pfi_cyl_t *));

	if (tmp == NULL) {
		return (1);
	}

	for (i = img->cyl_cnt; i < idx; i++) {
		tmp[i] = NULL;
	}

	tmp[idx] = cyl;

	img->cyl = tmp;
	img->cyl_cnt = idx + 1;

	pfi_img_fix_cyl (img);

	return (0);
}

int pfi_img_add_cylinder (pfi_img_t *img, pfi_cyl_t *cyl)
{
	return (pfi_img_set_cylinder (img, cyl, img->cyl_cnt));
}

int pfi_img_del_cylinder (pfi_img_t *img, unsigned long idx)
{
	if (idx >= img->cyl_cnt) {
		return (1);
	}

	if (img->cyl[idx] == NULL) {
		return (1);
	}

	pfi_cyl_del (img->cyl[idx]);

	img->cyl[idx] = NULL;

	pfi_img_fix_cyl (img);

	return (0);
}

pfi_trk_t *pfi_img_get_track (pfi_img_t *img, unsigned long c, unsigned long h, int alloc)
{
	pfi_cyl_t *cyl;
	pfi_trk_t *trk;

	cyl = pfi_img_get_cylinder (img, c, alloc);

	if (cyl == NULL) {
		return (NULL);
	}

	trk = pfi_cyl_get_track (cyl, h, alloc);

	if (trk == NULL) {
		return (NULL);
	}

	return (trk);
}

const pfi_trk_t *pfi_img_get_track_const (const pfi_img_t *img, unsigned long c, unsigned long h)
{
	const pfi_cyl_t *cyl;
	const pfi_trk_t *trk;

	cyl = pfi_img_get_cylinder_const (img, c);

	if (cyl == NULL) {
		return (NULL);
	}

	trk = pfi_cyl_get_track_const (cyl, h);

	if (trk == NULL) {
		return (NULL);
	}

	return (trk);
}

int pfi_img_set_track (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h)
{
	pfi_cyl_t *cyl;

	cyl = pfi_img_get_cylinder (img, c, 1);

	if (cyl == NULL) {
		return (1);
	}

	if (pfi_cyl_set_track (cyl, trk, h)) {
		return (1);
	}

	return (0);
}

int pfi_img_del_track (pfi_img_t *img, unsigned long c, unsigned long h)
{
	if (c >= img->cyl_cnt) {
		return (1);
	}

	if (img->cyl[c] == NULL) {
		return (1);
	}

	if (pfi_cyl_del_track (img->cyl[c], h)) {
		return (1);
	}

	return (0);
}

int pfi_img_add_comment (pfi_img_t *img, const unsigned char *buf, unsigned cnt)
{
	unsigned char *tmp;

	tmp = realloc (img->comment, img->comment_size + cnt);

	if (tmp == NULL) {
		return (1);
	}

	memcpy (tmp + img->comment_size, buf, cnt);

	img->comment_size += cnt;
	img->comment = tmp;

	return (0);
}

int pfi_img_set_comment (pfi_img_t *img, const unsigned char *buf, unsigned cnt)
{
	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;

	if ((buf == NULL) || (cnt == 0)) {
		return (0);
	}

	if (pfi_img_add_comment (img, buf, cnt)) {
		return (1);
	}

	return (0);
}
