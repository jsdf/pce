/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi.c                                        *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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

#include "psi.h"


psi_sct_t *psi_sct_new (unsigned c, unsigned h, unsigned s, unsigned n)
{
	psi_sct_t *sct;

	sct = malloc (sizeof (psi_sct_t));

	if (sct == NULL) {
		return (NULL);
	}

	sct->next = NULL;

	sct->c = c;
	sct->h = h;
	sct->s = s;
	sct->n = n;

	sct->flags = 0;

	sct->encoding = PSI_ENC_MFM;

	sct->cur_alt = 0;

	if (n > 0) {
		sct->data = malloc (n);

		if (sct->data == NULL) {
			free (sct);
			return (NULL);
		}
	}
	else {
		sct->data = NULL;
	}

	sct->tag_cnt = 0;

	sct->position = 0xffffffff;
	sct->read_time = 0;

	sct->have_mfm_size = 0;
	sct->have_gcr_format = 0;

	return (sct);
}

void psi_sct_del (psi_sct_t *sct)
{
	psi_sct_t *tmp;

	while (sct != NULL) {
		tmp = sct;
		sct = sct->next;

		free (tmp->data);
		free (tmp);
	}
}

psi_sct_t *psi_sct_clone (const psi_sct_t *sct, int deep)
{
	psi_sct_t *dst, *tmp;

	dst = psi_sct_new (sct->c, sct->h, sct->s, sct->n);

	if (dst == NULL) {
		return (NULL);
	}

	dst->flags = sct->flags;
	dst->encoding = sct->encoding;

	memcpy (dst->data, sct->data, dst->n);

	dst->tag_cnt = sct->tag_cnt;

	if (sct->tag_cnt > 0) {
		memcpy (dst->tag, sct->tag, sct->tag_cnt);
	}

	dst->position = sct->position;
	dst->read_time = sct->read_time;

	if (sct->have_mfm_size) {
		dst->have_mfm_size = 1;
		dst->mfm_size = sct->mfm_size;
	}

	if (sct->have_gcr_format) {
		dst->have_gcr_format = 1;
		dst->gcr_format = sct->gcr_format;
	}

	if (deep == 0) {
		dst->cur_alt = 0;
	}
	else {
		dst->cur_alt = sct->cur_alt;

		tmp = dst;
		sct = sct->next;

		while (sct != NULL) {
			tmp->next = psi_sct_clone (sct, 0);

			if (tmp->next == NULL) {
				psi_sct_del (dst);
				return (NULL);
			}

			sct = sct->next;
		}
	}

	return (dst);
}

void psi_sct_add_alternate (psi_sct_t *sct, psi_sct_t *alt)
{
	while (sct->next != NULL) {
		sct = sct->next;
	}

	sct->next = alt;
}

psi_sct_t *psi_sct_get_alternate (psi_sct_t *sct, unsigned idx)
{
	while (idx > 0) {
		if (sct == NULL) {
			return (NULL);
		}

		sct = sct->next;
		idx -= 1;
	}

	return (sct);
}

int psi_sct_set_size (psi_sct_t *sct, unsigned size, unsigned filler)
{
	unsigned char *tmp;

	if (size <= sct->n) {
		sct->n = size;
		return (0);
	}

	tmp = realloc (sct->data, size);

	if (tmp == NULL) {
		return (1);
	}

	if (size > sct->n) {
		memset (tmp + sct->n, filler, size - sct->n);
	}

	sct->n = size;
	sct->data = tmp;

	return (0);
}

unsigned psi_sct_get_size (const psi_sct_t *sct)
{
	return (sct->n);
}

void psi_sct_fill (psi_sct_t *sct, unsigned val)
{
	unsigned i;

	for (i = 0; i < sct->n; i++) {
		sct->data[i] = val;
	}
}

int psi_sct_uniform (const psi_sct_t *sct)
{
	unsigned i;

	for (i = 1; i < sct->n; i++) {
		if (sct->data[0] != sct->data[i]) {
			return (0);
		}
	}

	return (1);
}

void psi_sct_set_flags (psi_sct_t *sct, unsigned long flags, int set)
{
	if (set) {
		sct->flags |= flags;
	}
	else {
		sct->flags &= ~flags;
	}
}

void psi_sct_set_encoding (psi_sct_t *sct, unsigned enc)
{
	while (sct != NULL) {
		sct->encoding = enc;
		sct = sct->next;
	}
}

void psi_sct_set_position (psi_sct_t *sct, unsigned long val)
{
	sct->position = val;
}

unsigned long psi_sct_get_position (const psi_sct_t *sct)
{
	return (sct->position);
}

void psi_sct_set_read_time (psi_sct_t *sct, unsigned long val)
{
	sct->read_time = val;
}

unsigned long psi_sct_get_read_time (const psi_sct_t *sct)
{
	return (sct->read_time);
}

void psi_sct_set_mfm_size (psi_sct_t *sct, unsigned char val)
{
	sct->have_mfm_size = 1;
	sct->mfm_size = val;
}

unsigned psi_sct_get_mfm_size (const psi_sct_t *sct)
{
	unsigned n;

	if (sct->have_mfm_size) {
		return (sct->mfm_size);
	}

	n = 0;

	while ((128 << n) < sct->n) {
		n += 1;
	}

	return (n);
}

void psi_sct_set_gcr_format (psi_sct_t *sct, unsigned char val)
{
	sct->have_gcr_format = 1;
	sct->gcr_format = val;
}

unsigned psi_sct_get_gcr_format (const psi_sct_t *sct)
{
	if (sct->have_gcr_format) {
		return (sct->gcr_format);
	}

	return (0);
}

unsigned psi_sct_set_tags (psi_sct_t *sct, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *src;

	src = buf;

	if (cnt > PSI_TAGS_MAX) {
		cnt = PSI_TAGS_MAX;
	}

	sct->tag_cnt = cnt;

	for (i = 0; i < cnt; i++) {
		sct->tag[i] = src[i];
	}

	return (cnt);
}

unsigned psi_sct_get_tags (const psi_sct_t *sct, void *buf, unsigned cnt)
{
	unsigned      i, n;
	unsigned char *dst;

	dst = buf;

	n = (cnt < sct->tag_cnt) ? cnt : sct->tag_cnt;

	for (i = 0; i < n; i++) {
		dst[i] = sct->tag[i];
	}

	for (i = n; i < cnt; i++) {
		dst[i] = 0;
	}

	return (n);
}


psi_trk_t *psi_trk_new (unsigned h)
{
	psi_trk_t *trk;

	trk = malloc (sizeof (psi_trk_t));

	if (trk == NULL) {
		return (NULL);
	}

	trk->h = h;
	trk->sct_cnt = 0;
	trk->sct = NULL;

	return (trk);
}

void psi_trk_free (psi_trk_t *trk)
{
	unsigned i;

	for (i = 0; i < trk->sct_cnt; i++) {
		psi_sct_del (trk->sct[i]);
	}

	free (trk->sct);

	trk->sct_cnt = 0;
	trk->sct = NULL;
}

void psi_trk_del (psi_trk_t *trk)
{
	if (trk != NULL) {
		psi_trk_free (trk);
		free (trk);
	}
}

int psi_trk_add_sector (psi_trk_t *trk, psi_sct_t *sct)
{
	psi_sct_t **tmp;

	tmp = realloc (trk->sct, (trk->sct_cnt + 1) * sizeof (psi_sct_t **));

	if (tmp == NULL) {
		return (1);
	}

	trk->sct = tmp;

	trk->sct[trk->sct_cnt] = sct;
	trk->sct_cnt += 1;

	return (0);
}

psi_sct_t *psi_trk_get_indexed_sector (psi_trk_t *trk, unsigned idx, int phy)
{
	unsigned i;
	unsigned min_sct, min_idx, min_cnt;

	if (trk->sct_cnt == 0) {
		return (NULL);
	}

	if (phy) {
		if (idx < trk->sct_cnt) {
			return (trk->sct[idx]);
		}

		return (NULL);
	}

	min_sct = 0;

	while (1) {
		min_idx = 0;
		min_cnt = 0;

		for (i = 0; i < trk->sct_cnt; i++) {
			if (trk->sct[i]->s < min_sct) {
				;
			}
			else if (min_cnt == 0) {
				min_idx = i;
				min_cnt = 1;
			}
			else if (trk->sct[i]->s < trk->sct[min_idx]->s) {
				min_idx = i;
				min_cnt = 1;
			}
			else if (trk->sct[i]->s == trk->sct[min_idx]->s) {
				min_cnt += 1;
			}
		}

		if (min_cnt == 0) {
			return (NULL);
		}

		if (idx < min_cnt) {
			for (i = 0; i < trk->sct_cnt; i++) {
				if (trk->sct[i]->s == trk->sct[min_idx]->s) {
					if (idx == 0) {
						return (trk->sct[i]);
					}

					idx -= 1;
				}
			}
		}
		else {
			idx -= min_cnt;
		}

		min_sct = trk->sct[min_idx]->s + 1;
	}

	return (NULL);
}

int psi_trk_interleave (psi_trk_t *trk, unsigned il)
{
	unsigned  i, j, n;
	psi_sct_t **sct, *tmp;

	n = trk->sct_cnt;

	if (n < 2) {
		return (0);
	}

	sct = malloc (n * sizeof (psi_sct_t *));

	if (sct == NULL) {
		return (1);
	}

	for (i = 0; i < n; i++) {
		tmp = trk->sct[i];
		trk->sct[i] = NULL;

		j = i;
		while ((j > 0) && (tmp->s < sct[j - 1]->s)) {
			sct[j] = sct[j - 1];
			j -= 1;
		}

		sct[j] = tmp;
	}

	j = 0;

	for (i = 0; i < n; i++) {
		while (trk->sct[j] != NULL) {
			j = (j + 1) % n;
		}

		trk->sct[j] = sct[i];

		j = (j + il) % n;
	}

	free (sct);

	return (0);
}


psi_cyl_t *psi_cyl_new (unsigned c)
{
	psi_cyl_t *cyl;

	cyl = malloc (sizeof (psi_cyl_t));

	if (cyl == NULL) {
		return (NULL);
	}

	cyl->c = c;
	cyl->trk_cnt = 0;
	cyl->trk = NULL;

	return (cyl);
}

void psi_cyl_free (psi_cyl_t *cyl)
{
	unsigned i;

	for (i = 0; i < cyl->trk_cnt; i++) {
		psi_trk_del (cyl->trk[i]);
	}

	free (cyl->trk);

	cyl->trk_cnt = 0;
	cyl->trk = 0;
}

void psi_cyl_del (psi_cyl_t *cyl)
{
	if (cyl != NULL) {
		psi_cyl_free (cyl);
		free (cyl);
	}
}

int psi_cyl_add_track (psi_cyl_t *cyl, psi_trk_t *trk)
{
	psi_trk_t **tmp;

	tmp = realloc (cyl->trk, (cyl->trk_cnt + 1) * sizeof (psi_trk_t **));

	if (tmp == NULL) {
		return (1);
	}

	cyl->trk = tmp;

	if (trk == NULL) {
		trk = psi_trk_new (cyl->trk_cnt);

		if (trk == NULL) {
			return (1);
		}
	}

	cyl->trk[cyl->trk_cnt] = trk;
	cyl->trk_cnt += 1;

	return (0);
}

psi_trk_t *psi_cyl_get_track (psi_cyl_t *cyl, unsigned h, int alloc)
{
	if (h < cyl->trk_cnt) {
		return (cyl->trk[h]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	while (cyl->trk_cnt <= h) {
		if (psi_cyl_add_track (cyl, NULL)) {
			return (NULL);
		}
	}

	return (cyl->trk[h]);
}


psi_img_t *psi_img_new (void)
{
	psi_img_t *img;

	img = malloc (sizeof (psi_img_t));

	if (img == NULL) {
		return (NULL);
	}

	img->cyl_cnt = 0;
	img->cyl = NULL;

	img->comment_size = 0;
	img->comment = NULL;

	return (img);
}

void psi_img_free (psi_img_t *img)
{
	unsigned i;

	for (i = 0; i < img->cyl_cnt; i++) {
		psi_cyl_del (img->cyl[i]);
	}

	free (img->cyl);

	img->cyl_cnt = 0;
	img->cyl = NULL;

	free (img->comment);
}

void psi_img_del (psi_img_t *img)
{
	if (img != NULL) {
		psi_img_free (img);
		free (img);
	}
}

void psi_img_erase (psi_img_t *img)
{
	unsigned i;

	for (i = 0; i < img->cyl_cnt; i++) {
		psi_cyl_del (img->cyl[i]);
	}

	free (img->cyl);

	img->cyl_cnt = 0;
	img->cyl = NULL;

	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;
}

int psi_img_add_cylinder (psi_img_t *img, psi_cyl_t *cyl)
{
	psi_cyl_t **tmp;

	tmp = realloc (img->cyl, (img->cyl_cnt + 1) * sizeof (psi_cyl_t **));

	if (tmp == NULL) {
		return (1);
	}

	img->cyl = tmp;

	if (cyl == NULL) {
		cyl = psi_cyl_new (img->cyl_cnt);

		if (cyl == NULL) {
			return (1);
		}
	}

	img->cyl[img->cyl_cnt] = cyl;
	img->cyl_cnt += 1;

	return (0);
}

int psi_img_add_track (psi_img_t *img, psi_trk_t *trk, unsigned c)
{
	psi_cyl_t *cyl;

	cyl = psi_img_get_cylinder (img, c, 1);

	if (cyl == NULL) {
		return (1);
	}

	if (psi_cyl_add_track (cyl, trk)) {
		return (1);
	}

	return (0);
}

int psi_img_add_sector (psi_img_t *img, psi_sct_t *sct, unsigned c, unsigned h)
{
	psi_trk_t *trk;

	trk = psi_img_get_track (img, c, h, 1);

	if (trk == NULL) {
		return (1);
	}

	if (psi_trk_add_sector (trk, sct)) {
		return (1);
	}

	return (0);
}

void psi_img_remove_sector (psi_img_t *img, const psi_sct_t *sct)
{
	unsigned  c, h, s, d;
	psi_cyl_t *cyl;
	psi_trk_t *trk;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			d = 0;

			for (s = 0; s < trk->sct_cnt; s++) {
				if (trk->sct[s] != sct) {
					trk->sct[d++] = trk->sct[s];
				}
			}

			trk->sct_cnt = d;
		}
	}
}

psi_cyl_t *psi_img_get_cylinder (psi_img_t *img, unsigned c, int alloc)
{
	if (c < img->cyl_cnt) {
		return (img->cyl[c]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	while (img->cyl_cnt <= c) {
		if (psi_img_add_cylinder (img, NULL)) {
			return (NULL);
		}
	}

	return (img->cyl[c]);
}

psi_trk_t *psi_img_get_track (psi_img_t *img, unsigned c, unsigned h, int alloc)
{
	psi_cyl_t *cyl;
	psi_trk_t *trk;

	cyl = psi_img_get_cylinder (img, c, alloc);

	if (cyl == NULL) {
		return (NULL);
	}

	trk = psi_cyl_get_track (cyl, h, alloc);

	if (trk == NULL) {
		return (NULL);
	}

	return (trk);
}

psi_sct_t *psi_img_get_sector (psi_img_t *img, unsigned c, unsigned h, unsigned s, int phy)
{
	unsigned  i;
	psi_trk_t *trk;

	trk = psi_img_get_track (img, c, h, 0);

	if (trk == NULL) {
		return (NULL);
	}

	if (phy) {
		if (s < trk->sct_cnt) {
			return (trk->sct[s]);
		}

		return (NULL);
	}

	for (i = 0; i < trk->sct_cnt; i++) {
		if (trk->sct[i]->s == s) {
			return (trk->sct[i]);
		}
	}

	return (NULL);
}

int psi_img_map_sector (psi_img_t *img, unsigned long idx, unsigned *pc, unsigned *ph, unsigned *ps)
{
	unsigned  i, j, k;
	psi_cyl_t *cyl;
	psi_trk_t *trk;
	psi_sct_t *sct;

	for (i = 0; i < img->cyl_cnt; i++) {
		cyl = img->cyl[i];

		for (j = 0; j < cyl->trk_cnt; j++) {
			trk = cyl->trk[j];

			if (idx < trk->sct_cnt) {
				sct = psi_trk_get_indexed_sector (trk, idx, 0);

				if (sct == NULL) {
					return (1);
				}

				*pc = i;
				*ph = j;
				*ps = 0;

				for (k = 0; k < trk->sct_cnt; k++) {
					if (trk->sct[k] == sct) {
						return (0);
					}

					*ps += 1;
				}

				return (1);
			}

			idx -= trk->sct_cnt;
		}
	}

	return (1);
}

int psi_img_add_comment (psi_img_t *img, const unsigned char *buf, unsigned cnt)
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

int psi_img_set_comment (psi_img_t *img, const unsigned char *buf, unsigned cnt)
{
	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;

	if ((buf == NULL) || (cnt == 0)) {
		return (0);
	}

	if (psi_img_add_comment (img, buf, cnt)) {
		return (1);
	}

	return (0);
}

void psi_img_clean_comment (psi_img_t *img)
{
	unsigned      i, j, n;
	unsigned char *p;

	i = 0;
	j = 0;
	n = img->comment_size;
	p = img->comment;

	while ((i < n) && (p[i] == 0x0a)) {
		i += 1;
	}

	while (i < n) {
		if (p[i] == 0x0d) {
			p[j++] = 0x0a;

			if (((i + 1) < n) && (p[i + 1] == 0x0a)) {
				i += 1;
			}
		}
		else if (p[i] == 0) {
			p[j++] = 0x0a;
		}
		else {
			p[j++] = p[i];
		}

		i += 1;
	}

	while ((j > 0) && (p[j - 1] == 0x0a)) {
		j -= 1;
	}

	img->comment_size = j;

	if (j == 0) {
		free (img->comment);
		img->comment = NULL;
	}
}

unsigned long psi_img_get_sector_count (const psi_img_t *img)
{
	unsigned long cnt;
	unsigned      c, h, s;
	psi_cyl_t     *cyl;
	psi_trk_t     *trk;
	psi_sct_t     *sct;

	cnt = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				while (sct != NULL) {
					sct = sct->next;
					cnt += 1;
				}
			}
		}
	}

	return (cnt);
}
