/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc.c                                     *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include "pfdc.h"


unsigned pfdc_get_uint16_be (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);

	return (val);
}

unsigned long pfdc_get_uint32_be (const void *buf, unsigned idx)
{
	unsigned long       val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);
	val = (val << 8) | (tmp[2] & 0xff);
	val = (val << 8) | (tmp[3] & 0xff);

	return (val);
}

void pfdc_set_uint16_be (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void pfdc_set_uint32_be (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

unsigned pfdc_get_uint16_le (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[1] & 0xff;
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

void pfdc_set_uint16_le (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}


pfdc_sct_t *pfdc_sct_new (unsigned c, unsigned h, unsigned s, unsigned n)
{
	pfdc_sct_t *sct;

	sct = malloc (sizeof (pfdc_sct_t));

	if (sct == NULL) {
		return (NULL);
	}

	sct->next = NULL;

	sct->c = c;
	sct->h = h;
	sct->s = s;
	sct->n = n;

	sct->flags = 0;

	sct->encoding = PFDC_ENC_MFM;
	sct->data_rate = 250000;

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

	return (sct);
}

void pfdc_sct_del (pfdc_sct_t *sct)
{
	pfdc_sct_t *tmp;

	while (sct != NULL) {
		tmp = sct;
		sct = sct->next;

		free (tmp->data);
		free (tmp);
	}
}

pfdc_sct_t *pfdc_sct_clone (const pfdc_sct_t *sct, int deep)
{
	pfdc_sct_t *dst, *tmp;

	dst = pfdc_sct_new (sct->c, sct->h, sct->s, sct->n);

	if (dst == NULL) {
		return (NULL);
	}

	dst->flags = sct->flags;
	dst->encoding = sct->encoding;
	dst->data_rate = sct->data_rate;

	memcpy (dst->data, sct->data, dst->n);

	dst->tag_cnt = sct->tag_cnt;

	if (sct->tag_cnt > 0) {
		memcpy (dst->tag, sct->tag, sct->tag_cnt);
	}

	if (deep == 0) {
		dst->cur_alt = 0;
	}
	else {
		tmp = dst;
		sct = sct->next;

		while (sct != NULL) {
			tmp->next = pfdc_sct_clone (sct, 0);

			if (tmp->next == NULL) {
				pfdc_sct_del (dst);
				return (NULL);
			}

			sct = sct->next;
		}

		dst->cur_alt = sct->cur_alt;
	}

	return (dst);
}

void pfdc_sct_add_alternate (pfdc_sct_t *sct, pfdc_sct_t *alt)
{
	while (sct->next != NULL) {
		sct = sct->next;
	}

	sct->next = alt;
}

pfdc_sct_t *pfdc_sct_get_alternate (pfdc_sct_t *sct, unsigned idx)
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

int pfdc_sct_set_size (pfdc_sct_t *sct, unsigned size, unsigned filler)
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

void pfdc_sct_fill (pfdc_sct_t *sct, unsigned val)
{
	unsigned i;

	for (i = 0; i < sct->n; i++) {
		sct->data[i] = val;
	}
}

int pfdc_sct_uniform (const pfdc_sct_t *sct)
{
	unsigned i;

	for (i = 1; i < sct->n; i++) {
		if (sct->data[0] != sct->data[i]) {
			return (0);
		}
	}

	return (1);
}

void pfdc_sct_set_flags (pfdc_sct_t *sct, unsigned long flags, int set)
{
	if (set) {
		sct->flags |= flags;
	}
	else {
		sct->flags &= ~flags;
	}
}

void pfdc_sct_set_encoding (pfdc_sct_t *sct, unsigned enc, unsigned long rate)
{
	while (sct != NULL) {
		sct->encoding = enc;
		sct->data_rate = rate;

		sct = sct->next;
	}
}

unsigned pfdc_sct_set_tags (pfdc_sct_t *sct, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *src;

	src = buf;

	if (cnt > PFDC_TAGS_MAX) {
		cnt = PFDC_TAGS_MAX;
	}

	sct->tag_cnt = cnt;

	for (i = 0; i < cnt; i++) {
		sct->tag[i] = src[i];
	}

	return (cnt);
}

unsigned pfdc_sct_get_tags (const pfdc_sct_t *sct, void *buf, unsigned cnt)
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


pfdc_trk_t *pfdc_trk_new (unsigned h)
{
	pfdc_trk_t *trk;

	trk = malloc (sizeof (pfdc_trk_t));

	if (trk == NULL) {
		return (NULL);
	}

	trk->h = h;
	trk->sct_cnt = 0;
	trk->sct = NULL;

	return (trk);
}

void pfdc_trk_free (pfdc_trk_t *trk)
{
	unsigned i;

	for (i = 0; i < trk->sct_cnt; i++) {
		pfdc_sct_del (trk->sct[i]);
	}

	free (trk->sct);

	trk->sct_cnt = 0;
	trk->sct = NULL;
}

void pfdc_trk_del (pfdc_trk_t *trk)
{
	if (trk != NULL) {
		pfdc_trk_free (trk);
		free (trk);
	}
}

int pfdc_trk_add_sector (pfdc_trk_t *trk, pfdc_sct_t *sct)
{
	pfdc_sct_t **tmp;

	tmp = realloc (trk->sct, (trk->sct_cnt + 1) * sizeof (pfdc_sct_t **));

	if (tmp == NULL) {
		return (1);
	}

	trk->sct = tmp;

	trk->sct[trk->sct_cnt] = sct;
	trk->sct_cnt += 1;

	return (0);
}

pfdc_sct_t *pfdc_trk_get_indexed_sector (pfdc_trk_t *trk, unsigned idx, int phy)
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


pfdc_cyl_t *pfdc_cyl_new (unsigned c)
{
	pfdc_cyl_t *cyl;

	cyl = malloc (sizeof (pfdc_cyl_t));

	if (cyl == NULL) {
		return (NULL);
	}

	cyl->c = c;
	cyl->trk_cnt = 0;
	cyl->trk = NULL;

	return (cyl);
}

void pfdc_cyl_free (pfdc_cyl_t *cyl)
{
	unsigned i;

	for (i = 0; i < cyl->trk_cnt; i++) {
		pfdc_trk_del (cyl->trk[i]);
	}

	free (cyl->trk);

	cyl->trk_cnt = 0;
	cyl->trk = 0;
}

void pfdc_cyl_del (pfdc_cyl_t *cyl)
{
	if (cyl != NULL) {
		pfdc_cyl_free (cyl);
		free (cyl);
	}
}

int pfdc_cyl_add_track (pfdc_cyl_t *cyl, pfdc_trk_t *trk)
{
	pfdc_trk_t **tmp;

	tmp = realloc (cyl->trk, (cyl->trk_cnt + 1) * sizeof (pfdc_trk_t **));

	if (tmp == NULL) {
		return (1);
	}

	cyl->trk = tmp;

	if (trk == NULL) {
		trk = pfdc_trk_new (cyl->trk_cnt);

		if (trk == NULL) {
			return (1);
		}
	}

	cyl->trk[cyl->trk_cnt] = trk;
	cyl->trk_cnt += 1;

	return (0);
}

pfdc_trk_t *pfdc_cyl_get_track (pfdc_cyl_t *cyl, unsigned h, int alloc)
{
	if (h < cyl->trk_cnt) {
		return (cyl->trk[h]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	while (cyl->trk_cnt <= h) {
		if (pfdc_cyl_add_track (cyl, NULL)) {
			return (NULL);
		}
	}

	return (cyl->trk[h]);
}


pfdc_img_t *pfdc_img_new (void)
{
	pfdc_img_t *img;

	img = malloc (sizeof (pfdc_img_t));

	if (img == NULL) {
		return (NULL);
	}

	img->cyl_cnt = 0;
	img->cyl = NULL;

	img->comment_size = 0;
	img->comment = NULL;

	return (img);
}

void pfdc_img_free (pfdc_img_t *img)
{
	unsigned i;

	for (i = 0; i < img->cyl_cnt; i++) {
		pfdc_cyl_del (img->cyl[i]);
	}

	free (img->cyl);

	img->cyl_cnt = 0;
	img->cyl = NULL;

	free (img->comment);
}

void pfdc_img_del (pfdc_img_t *img)
{
	if (img != NULL) {
		pfdc_img_free (img);
		free (img);
	}
}

void pfdc_img_erase (pfdc_img_t *img)
{
	unsigned i;

	for (i = 0; i < img->cyl_cnt; i++) {
		pfdc_cyl_del (img->cyl[i]);
	}

	free (img->cyl);

	img->cyl_cnt = 0;
	img->cyl = NULL;

	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;
}

int pfdc_img_add_cylinder (pfdc_img_t *img, pfdc_cyl_t *cyl)
{
	pfdc_cyl_t **tmp;

	tmp = realloc (img->cyl, (img->cyl_cnt + 1) * sizeof (pfdc_cyl_t **));

	if (tmp == NULL) {
		return (1);
	}

	img->cyl = tmp;

	if (cyl == NULL) {
		cyl = pfdc_cyl_new (img->cyl_cnt);

		if (cyl == NULL) {
			return (1);
		}
	}

	img->cyl[img->cyl_cnt] = cyl;
	img->cyl_cnt += 1;

	return (0);
}

int pfdc_img_add_sector (pfdc_img_t *img, pfdc_sct_t *sct, unsigned c, unsigned h)
{
	pfdc_trk_t *trk;

	trk = pfdc_img_get_track (img, c, h, 1);

	if (trk == NULL) {
		return (1);
	}

	if (pfdc_trk_add_sector (trk, sct)) {
		return (1);
	}

	return (0);
}

void pfdc_img_remove_sector (pfdc_img_t *img, const pfdc_sct_t *sct)
{
	unsigned   c, h, s, d;
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;

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

pfdc_cyl_t *pfdc_img_get_cylinder (pfdc_img_t *img, unsigned c, int alloc)
{
	if (c < img->cyl_cnt) {
		return (img->cyl[c]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	while (img->cyl_cnt <= c) {
		if (pfdc_img_add_cylinder (img, NULL)) {
			return (NULL);
		}
	}

	return (img->cyl[c]);
}

pfdc_trk_t *pfdc_img_get_track (pfdc_img_t *img, unsigned c, unsigned h, int alloc)
{
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;

	cyl = pfdc_img_get_cylinder (img, c, alloc);

	if (cyl == NULL) {
		return (NULL);
	}

	trk = pfdc_cyl_get_track (cyl, h, alloc);

	if (trk == NULL) {
		return (NULL);
	}

	return (trk);
}

pfdc_sct_t *pfdc_img_get_sector (pfdc_img_t *img, unsigned c, unsigned h, unsigned s, int phy)
{
	unsigned   i;
	pfdc_trk_t *trk;

	trk = pfdc_img_get_track (img, c, h, 0);

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

int pfdc_img_map_sector (pfdc_img_t *img, unsigned long idx, unsigned *pc, unsigned *ph, unsigned *ps)
{
	unsigned   i, j, k;
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;
	pfdc_sct_t *sct;

	for (i = 0; i < img->cyl_cnt; i++) {
		cyl = img->cyl[i];

		for (j = 0; j < cyl->trk_cnt; j++) {
			trk = cyl->trk[j];

			if (idx < trk->sct_cnt) {
				sct = pfdc_trk_get_indexed_sector (trk, idx, 0);

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

int pfdc_img_add_comment (pfdc_img_t *img, const unsigned char *buf, unsigned cnt)
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

int pfdc_img_set_comment (pfdc_img_t *img, const unsigned char *buf, unsigned cnt)
{
	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;

	if ((buf == NULL) || (cnt == 0)) {
		return (0);
	}

	if (pfdc_img_add_comment (img, buf, cnt)) {
		return (1);
	}

	return (0);
}

unsigned long pfdc_img_get_sector_count (const pfdc_img_t *img)
{
	unsigned long cnt;
	unsigned      c, h, s;
	pfdc_cyl_t    *cyl;
	pfdc_trk_t    *trk;
	pfdc_sct_t    *sct;

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
