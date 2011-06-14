/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-imd.c                             *
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
#include <time.h>

#include "pfdc.h"
#include "pfdc-img-imd.h"


static
int imd_sct_set_encoding (pfdc_sct_t *sct, unsigned val)
{
	unsigned      enc;
	unsigned long dr;

	switch (val) {
	case 0:
		enc = PFDC_ENC_FM;
		dr = 500000;
		break;

	case 1:
		enc = PFDC_ENC_FM;
		dr = 300000;
		break;

	case 2:
		enc = PFDC_ENC_FM;
		dr = 250000;
		break;

	case 3:
		enc = PFDC_ENC_MFM;
		dr = 500000;
		break;

	case 4:
		enc = PFDC_ENC_MFM;
		dr = 300000;
		break;

	case 5:
		enc = PFDC_ENC_MFM;
		dr = 250000;
		break;

	default:
		return (1);
	}

	pfdc_sct_set_encoding (sct, enc, dr);

	return (0);
}

static
int imd_read_track (FILE *fp, pfdc_img_t *img, const unsigned char *hdr)
{
	unsigned      i;
	unsigned      c, h, sc, n, cnt;
	unsigned      lc, lh, ls;
	unsigned char dat;
	unsigned char smap[256];
	unsigned char cmap[256];
	unsigned char hmap[256];
	pfdc_sct_t    *sct;

	c = hdr[1];
	h = hdr[2] & 1;
	sc = hdr[3];
	n = hdr[4];

	if (sc == 0) {
		return (1);
	}

	if (n > 6) {
		return (1);
	}

	cnt = 128 << n;

	if (fread (smap, 1, sc, fp) != sc) {
		return (1);
	}

	if (hdr[2] & 0x80) {
		if (fread (cmap, 1, sc, fp) != sc) {
			return (1);
		}
	}

	if (hdr[2] & 0x40) {
		if (fread (hmap, 1, sc, fp) != sc) {
			return (1);
		}
	}

	for (i = 0; i < sc; i++) {
		lc = (hdr[2] & 0x80) ? cmap[i] : c;
		lh = (hdr[2] & 0x40) ? hmap[i] : h;
		ls = smap[i];

		if (fread (&dat, 1, 1, fp) != 1) {
			return (1);
		}

		sct = pfdc_sct_new (lc, lh, ls, cnt);

		if (sct == NULL) {
			return (1);
		}

		if (pfdc_img_add_sector (img, sct, c, h)) {
			pfdc_sct_del (sct);
			return (1);
		}

		if (imd_sct_set_encoding (sct, hdr[0])) {
			return (1);
		}

		if ((dat == 5) || (dat == 6) || (dat == 7) || (dat == 8)) {
			pfdc_sct_set_flags (sct, PFDC_FLAG_CRC_DATA, 1);
		}

		if ((dat == 3) || (dat == 4) || (dat == 7) || (dat == 8)) {
			pfdc_sct_set_flags (sct, PFDC_FLAG_DEL_DAM, 1);
		}

		if (dat == 0) {
			pfdc_sct_set_flags (sct, PFDC_FLAG_CRC_DATA, 1);
			pfdc_sct_fill (sct, 0);
		}
		else if ((dat == 2) || (dat == 4) || (dat == 6) || (dat == 8)) {
			if (fread (&dat, 1, 1, fp) != 1) {
				return (1);
			}

			pfdc_sct_fill (sct, dat);
		}
		else if ((dat == 1) || (dat == 3) || (dat == 5) || (dat == 7)) {
			if (fread (sct->data, 1, sct->n, fp) != sct->n) {
				return (1);
			}
		}
		else {
			return (1);
		}
	}

	return (0);
}

static
int imd_load_fp (FILE *fp, pfdc_img_t *img)
{
	int           c;
	int           cmt;
	size_t        r;
	unsigned char buf[8];

	if (fread (buf, 1, 4, fp) != 4) {
		return (1);
	}

	if (pfdc_get_uint32_be (buf, 0) != PFDC_MAGIC_IMD) {
		return (1);
	}

	cmt = 0;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			return (1);
		}

		if (c == 0x1a) {
			break;
		}

		if (c == 0x0d) {
			c = fgetc (fp);

			if ((c != EOF) && (c != 0x0a)) {
				ungetc (c, fp);
			}

			c = 0x0a;
		}

		if ((cmt == 0) && (c == 0x0a)) {
			cmt = 1;
		}
		else if (cmt) {
			buf[0] = c;

			pfdc_img_add_comment (img, buf, 1);
		}
	}

	while (1) {
		r = fread (buf, 1, 5, fp);

		if (r == 0) {
			return (0);
		}

		if (r != 5) {
			return (1);
		}

		if (imd_read_track (fp, img, buf)) {
			return (1);
		}
	}
}

pfdc_img_t *pfdc_load_imd (FILE *fp)
{
	pfdc_img_t *img;

	img = pfdc_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (imd_load_fp (fp, img)) {
		pfdc_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int imd_save_header (FILE *fp, const pfdc_img_t *img)
{
	unsigned  i;
	time_t    t;
	struct tm *tm;

	t = time (NULL);

	tm = gmtime (&t);

	fprintf (fp, "IMD 1.17: %2d/%2d/%4d %02d:%02d:%02d\x0d\x0a",
		tm->tm_mday, tm->tm_mon + 1, 1900 + tm->tm_year,
		tm->tm_hour, tm->tm_min, tm->tm_sec
	);

	if (img->comment_size > 0) {
		i = 0;
		while (i < img->comment_size) {
			if (img->comment[i] == 0x0a) {
				fputc (0x0d, fp);
				fputc (0x0a, fp);
			}
			else {
				fputc (img->comment[i], fp);
			}

			i += 1;
		}
	}

	fputs ("\x0d\x0a\x1a", fp);

	return (0);
}

static
int imd_save_sector_data (FILE *fp, const pfdc_sct_t *sct)
{
	unsigned mode;
	int      compr;

	compr = pfdc_sct_uniform (sct) && (sct->n > 0);

	if ((sct->flags & PFDC_FLAG_CRC_DATA) && (sct->flags & PFDC_FLAG_DEL_DAM)) {
		mode = 7;
	}
	else if (sct->flags & PFDC_FLAG_CRC_DATA) {
		mode = 5;
	}
	else if (sct->flags & PFDC_FLAG_DEL_DAM) {
		mode = 3;
	}
	else {
		mode = 1;
	}

	if (compr) {
		mode += 1;
	}

	fputc (mode, fp);

	if (compr) {
		fputc (sct->data[0], fp);
	}
	else {
		if (fwrite (sct->data, 1, sct->n, fp) != sct->n) {
			return (1);
		}
	}

	return (0);
}

static
int imd_sct_get_encoding (const pfdc_sct_t *sct, unsigned char *mode)
{
	switch (sct->data_rate) {
	case 250000:
		*mode = 2;
		break;

	case 300000:
		*mode = 1;
		break;

	case 500000:
		*mode = 0;
		break;

	default:
		return (1);
	}

	if (sct->encoding != PFDC_ENC_FM) {
		*mode += 3;
	}

	return (0);
}

static
int imd_sct_get_size (const pfdc_sct_t *sct, unsigned char *n)
{
	unsigned size;

	size = sct->n;

	if (size & (size - 1)) {
		return (1);
	}

	if (size > 8192) {
		return (1);
	}

	*n = 0;

	while (size >= 256) {
		*n += 1;
		size >>= 1;
	}

	return (0);
}

static
int imd_save_track (FILE *fp, const pfdc_trk_t *trk, unsigned c, unsigned h)
{
	unsigned      i;
	unsigned      sc;
	unsigned char buf[256];
	unsigned      bcnt;
	int           need_cmap, need_hmap;
	unsigned char *smap, *cmap, *hmap;
	pfdc_sct_t    *sct;

	sc = trk->sct_cnt;

	if (sc == 0) {
		return (0);
	}

	if (sc > 64) {
		return (1);
	}

	need_cmap = 0;
	need_hmap = 0;

	for (i = 1; i < sc; i++) {
		if (trk->sct[i]->c != c) {
			need_cmap = 1;
		}

		if (trk->sct[i]->h != h) {
			need_hmap = 1;
		}

		if (trk->sct[i]->n != trk->sct[0]->n) {
			return (1);
		}

		if (trk->sct[i]->data_rate != trk->sct[0]->data_rate) {
			return (1);
		}
	}

	sct = trk->sct[0];

	if (imd_sct_get_encoding (sct, buf + 0)) {
		return (1);
	}

	buf[1] = c;
	buf[2] = h;
	buf[3] = sc;

	if (imd_sct_get_size (sct, buf + 4)) {
		return (1);
	}

	smap = buf + 5;
	bcnt = sc + 5;

	if (need_cmap) {
		cmap = buf + bcnt;
		bcnt += sc;
		buf[2] |= 0x80;
	}
	else {
		cmap = NULL;
	}

	if (need_hmap) {
		hmap = buf + bcnt;
		bcnt += sc;
		buf[2] |= 0x40;
	}
	else {
		hmap = NULL;
	}

	for (i = 0; i < sc; i++) {
		sct = trk->sct[i];

		smap[i] = sct->s;

		if (cmap != NULL) {
			cmap[i] = sct->c;
		}

		if (hmap != NULL) {
			hmap[i] = sct->h;
		}
	}

	if (fwrite (buf, 1, bcnt, fp) != bcnt) {
		return (1);
	}

	for (i = 0; i < sc; i++) {
		if (imd_save_sector_data (fp, trk->sct[i])) {
			return (1);
		}
	}

	return (0);
}

int pfdc_save_imd (FILE *fp, const pfdc_img_t *img)
{
	unsigned         c, h;
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;

	if (imd_save_header (fp, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (imd_save_track (fp, trk, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

int pfdc_probe_imd_fp (FILE *fp)
{
	unsigned char buf[16];

	if (fseek (fp, 0, SEEK_SET)) {
		return (0);
	}

	if (fread (buf, 1, 4, fp) != 4) {
		return (0);
	}

	if (pfdc_get_uint32_be (buf, 0) != PFDC_MAGIC_IMD) {
		return (0);
	}

	return (1);
}

int pfdc_probe_imd (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = pfdc_probe_imd_fp (fp);

	fclose (fp);

	return (r);
}
