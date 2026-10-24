/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-pfi.c                                    *
 * Created:     2013-12-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2022 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <config.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pfi.h"
#include "pfi-pfi.h"

#include <lib/ciff.h>
#include <lib/endian.h>


#define CKID_PFI  0x50464920
#define CKID_TEXT 0x54455854
#define CKID_TRAK 0x5452414b
#define CKID_INDX 0x494e4458
#define CKID_DATA 0x44415441
#define CKID_END  0x454e4420


typedef struct {
	ciff_t        ciff;

	pfi_img_t     *img;
	pfi_trk_t     *trk;

	unsigned long bufmax;
	unsigned char *buf;
} pfi_load_t;


static
unsigned char *pfi_alloc (pfi_load_t *pfi, unsigned long size)
{
	unsigned char *tmp;

	if (pfi->bufmax < size) {
		if ((tmp = realloc (pfi->buf, size)) == NULL) {
			return (NULL);
		}

		pfi->buf = tmp;
		pfi->bufmax = size;
	}

	return (pfi->buf);
}

static
void pfi_free (pfi_load_t *pfi)
{
	free (pfi->buf);

	pfi->buf = NULL;
	pfi->bufmax = 0;
}

static
int pfi_load_header (pfi_load_t *pfi)
{
	unsigned char buf[4];
	unsigned long vers;

	if (ciff_read (&pfi->ciff, buf, 4)) {
		return (1);
	}

	vers = get_uint32_be (buf, 0);

	if (vers != 0) {
		fprintf (stderr, "pfi: unknown version number (%lu)\n", vers);
		return (1);
	}

	return (0);
}

static
int pfi_load_comment (pfi_load_t *pfi)
{
	unsigned      i, j, k, d;
	unsigned long size;
	unsigned char *buf;

	size = pfi->ciff.size;

	if (size == 0) {
		return (0);
	}

	if ((buf = pfi_alloc (pfi, size)) == NULL) {
		return (1);
	}

	if (ciff_read (&pfi->ciff, buf, size)) {
		return (1);
	}

	i = 0;
	j = size;

	while (i < j) {
		if ((buf[i] == 0x0d) || (buf[i] == 0x0a)) {
			i += 1;
		}
		else if (buf[i] == 0x00) {
			i += 1;
		}
		else {
			break;
		}
	}

	while (j > i) {
		if ((buf[j - 1] == 0x0d) || (buf[j - 1] == 0x0a)) {
			j -= 1;
		}
		else if (buf[j - 1] == 0x00) {
			j += 1;
		}
		else {
			break;
		}
	}

	if (i == j) {
		return (0);
	}

	k = i;
	d = i;

	while (k < j) {
		if (buf[k] == 0x0d) {
			if (((k + 1) < j) && (buf[k + 1] == 0x0a)) {
				k += 1;
			}
			else {
				buf[d++] = 0x0a;
			}
		}
		else {
			buf[d++] = buf[k];
		}

		k += 1;
	}

	j = d;

	if (pfi->img->comment_size > 0) {
		unsigned char c;

		c = 0x0a;

		if (pfi_img_add_comment (pfi->img, &c, 1)) {
			return (1);
		}
	}

	if (pfi_img_add_comment (pfi->img, buf + i, j - i)) {
		return (1);
	}

	return (0);
}

static
int pfi_load_track_header (pfi_load_t *pfi)
{
	unsigned char buf[12];
	unsigned long c, h, clock;

	if (ciff_read (&pfi->ciff, buf, 12)) {
		return (1);
	}

	c = get_uint32_be (buf, 0);
	h = get_uint32_be (buf, 4);
	clock = get_uint32_be (buf, 8);

	pfi->trk = pfi_img_get_track (pfi->img, c, h, 1);

	if (pfi->trk == NULL) {
		return (1);
	}

	pfi_trk_set_clock (pfi->trk, clock);

	return (0);
}

static
int pfi_load_indx (pfi_load_t *pfi)
{
	unsigned char buf[4];

	while (pfi->ciff.size >= 4) {
		if (ciff_read (&pfi->ciff, buf, 4)) {
			return (1);
		}

		if (pfi_trk_add_index (pfi->trk, get_uint32_be (buf, 0))) {
			return (1);
		}
	}

	return (0);
}

static
int pfi_decode_track_data (pfi_load_t *pfi, unsigned char *buf, unsigned long size)
{
	unsigned      cnt;
	unsigned long pos;
	uint32_t      val;

	if (ciff_read (&pfi->ciff, buf, size)) {
		return (1);
	}

	pos = 0;

	while (pos < size) {
		cnt = buf[pos++];

		if (cnt < 4) {
			if ((pos + cnt + 1) > size) {
				return (1);
			}

			val = buf[pos++];

			while (cnt > 0) {
				val = (val << 8) | buf[pos++];
				cnt -= 1;
			}
		}
		else if (cnt < 8) {
			if ((pos + 1) > size) {
				return (1);
			}

			val = ((cnt << 8) | buf[pos++]) & 0x03ff;
		}
		else {
			val = cnt;
		}

		if (pfi_trk_add_pulse (pfi->trk, val)) {
			return (1);
		}
	}

	pfi_trk_clean (pfi->trk);

	return (0);
}

static
int pfi_load_track_data (pfi_load_t *pfi)
{
	unsigned long size;
	unsigned char *buf;

	size = pfi->ciff.size;

	if ((buf = pfi_alloc (pfi, size)) == NULL) {
		return (1);
	}

	if (pfi_decode_track_data (pfi, buf, size)) {
		return (1);
	}

	return (0);
}

static
int pfi_load_image (pfi_load_t *pfi)
{
	if (ciff_read_id (&pfi->ciff)) {
		return (1);
	}

	if (pfi->ciff.ckid != CKID_PFI) {
		return (1);
	}

	if (pfi_load_header (pfi)) {
		return (1);
	}

	pfi->trk = NULL;

	while (ciff_read_id (&pfi->ciff) == 0) {
		switch (pfi->ciff.ckid) {
		case CKID_END:
			if (ciff_read_crc (&pfi->ciff)) {
				return (1);
			}
			return (0);

		case CKID_TEXT:
			if (pfi_load_comment (pfi)) {
				return (1);
			}
			break;


		case CKID_TRAK:
			if (pfi_load_track_header (pfi)) {
				return (1);
			}
			break;

		case CKID_INDX:
			if (pfi->trk == NULL) {
				return (1);
			}

			if (pfi_load_indx (pfi)) {
				return (1);
			}
			break;

		case CKID_DATA:
			if (pfi->trk == NULL) {
				return (1);
			}

			if (pfi_load_track_data (pfi)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}

pfi_img_t *pfi_load_pfi (FILE *fp)
{
	int        r;
	pfi_load_t pfi;

	if ((pfi.img = pfi_img_new()) == NULL) {
		return (NULL);
	}

	ciff_init (&pfi.ciff, fp, 1);

	pfi.trk = NULL;
	pfi.buf = NULL;
	pfi.bufmax = 0;

	r = pfi_load_image (&pfi);

	if (pfi.ciff.crc_error) {
		fprintf (stderr, "pfi: crc error\n");
	}

	ciff_free (&pfi.ciff);
	pfi_free (&pfi);

	if (r) {
		pfi_img_del (pfi.img);
		return (NULL);
	}

	return (pfi.img);
}


static
int pfi_save_header (ciff_t *ciff, const pfi_img_t *img)
{
	unsigned char buf[4];

	set_uint32_be (buf, 0, 0);

	if (ciff_write_chunk (ciff, CKID_PFI, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_end (ciff_t *ciff, const pfi_img_t *img)
{
	if (ciff_write_chunk (ciff, CKID_END, NULL, 0)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_comment (ciff_t *ciff, const pfi_img_t *img)
{
	int                 r;
	unsigned long       i, j;
	const unsigned char *src;
	unsigned char       *buf;

	if (img->comment_size == 0) {
		return (0);
	}

	if ((buf = malloc (img->comment_size + 2)) == NULL) {
		return (1);
	}

	src = img->comment;

	buf[0] = 0x0a;

	i = 0;
	j = 1;

	while (i < img->comment_size) {
		if ((src[i] == 0x0d) || (src[i] == 0x0a)) {
			i += 1;
		}
		else if (src[i] == 0x00) {
			i += 1;
		}
		else {
			break;
		}
	}

	while (i < img->comment_size) {
		if (src[i] == 0x0d) {
			if (((i + 1) < img->comment_size) && (src[i + 1] == 0x0a)) {
				i += 1;
			}
			else {
				buf[j++] = 0x0a;
			}
		}
		else {
			buf[j++] = src[i];
		}

		i += 1;
	}

	while (j > 1) {
		if ((buf[j - 1] == 0x0a) || (buf[j - 1] == 0x00)) {
			j -= 1;
		}
		else {
			break;
		}
	}

	if (j == 1) {
		r = 0;
	}
	else {
		buf[j++] = 0x0a;

		r = ciff_write_chunk (ciff, CKID_TEXT, buf, j);
	}

	free (buf);

	return (r);
}

static
int pfi_save_track_header (ciff_t *ciff, const pfi_trk_t *trk, unsigned long c, unsigned long h)
{
	unsigned char buf[16];

	set_uint32_be (buf, 0, c);
	set_uint32_be (buf, 4, h);
	set_uint32_be (buf, 8, trk->clock);

	if (ciff_write_chunk (ciff, CKID_TRAK, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_indx (ciff_t *ciff, const pfi_trk_t *trk)
{
	unsigned      i;
	unsigned char buf[4];

	if (trk->index_cnt == 0) {
		return (0);
	}

	if (ciff_write_id (ciff, CKID_INDX, 4 * trk->index_cnt)) {
		return (1);
	}

	for (i = 0; i < trk->index_cnt; i++) {
		set_uint32_be (buf, 0, trk->index[i]);

		if (ciff_write (ciff, buf, 4)) {
			return (1);
		}
	}

	if (ciff_write_crc (ciff)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_data (ciff_t *ciff, const pfi_trk_t *trk)
{
	int           r;
	uint32_t      val;
	unsigned long i, n, max;
	unsigned char *buf, *tmp;

	if (trk->pulse_cnt == 0) {
		return (0);
	}

	max = 4096;

	if ((buf = malloc (max)) == NULL) {
		return (1);
	}

	n = 0;

	for (i = 0; i < trk->pulse_cnt; i++) {
		val = trk->pulse[i];

		if (val == 0) {
			continue;
		}

		if ((max - n) < 8) {
			max *= 2;

			if ((tmp = realloc (buf, max)) == 0) {
				free (buf);
				return (1);
			}

			buf = tmp;
		}

		if (val < 8) {
			buf[n++] = 4;
			buf[n++] = val;
		}
		else if (val < (1UL << 8)) {
			buf[n++] = val;
		}
		else if (val < (1UL << 10)) {
			buf[n++] = ((val >> 8) & 3) | 4;
			buf[n++] = val & 0xff;
		}
		else if (val < (1UL << 16)) {
			buf[n++] = 1;
			buf[n++] = (val >> 8) & 0xff;
			buf[n++] = val & 0xff;
		}
		else if (val < (1UL << 24)) {
			buf[n++] = 2;
			buf[n++] = (val >> 16) & 0xff;
			buf[n++] = (val >> 8) & 0xff;
			buf[n++] = val & 0xff;
		}
		else {
			buf[n++] = 3;
			buf[n++] = (val >> 24) & 0xff;
			buf[n++] = (val >> 16) & 0xff;
			buf[n++] = (val >> 8) & 0xff;
			buf[n++] = val & 0xff;
		}
	}

	r = ciff_write_chunk (ciff, CKID_DATA, buf, n);

	free (buf);

	return (r);
}

static
int pfi_save_track (ciff_t *ciff, const pfi_trk_t *trk, unsigned long c, unsigned long h)
{
	if (pfi_save_track_header (ciff, trk, c, h)) {
		return (1);
	}

	if (pfi_save_indx (ciff, trk)) {
		return (1);
	}

	if (pfi_save_data (ciff, trk)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_ciff (ciff_t *ciff, const pfi_img_t *img)
{
	unsigned long c, h;
	pfi_cyl_t     *cyl;
	pfi_trk_t     *trk;

	if (pfi_save_header (ciff, img)) {
		return (1);
	}

	if (pfi_save_comment (ciff, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		if ((cyl = img->cyl[c]) == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			if ((trk = cyl->trk[h]) == NULL) {
				continue;
			}

			if (pfi_save_track (ciff, trk, c, h)) {
				return (1);
			}
		}
	}

	if (pfi_save_end (ciff, img)) {
		return (1);
	}

	return (0);
}

int pfi_save_pfi (FILE *fp, const pfi_img_t *img)
{
	int    r;
	ciff_t ciff;

	ciff_init (&ciff, fp, 1);

	r = pfi_save_ciff (&ciff, img);

	ciff_free (&ciff);

	return (r);
}


int pfi_probe_pfi_fp (FILE *fp)
{
	unsigned char buf[4];

	if (fread (buf, 1, 4, fp) != 4) {
		return (0);
	}

	if (get_uint32_be (buf, 0) != CKID_PFI) {
		return (0);
	}

	return (1);
}

int pfi_probe_pfi (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pfi_probe_pfi_fp (fp);

	fclose (fp);

	return (r);
}
