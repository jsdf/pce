/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pbit/pbit-io-pbit.c                              *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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

#include "pbit.h"
#include "pbit-io.h"
#include "pbit-io-pbit.h"


#define PBIT_CHUNK_PBIT 0x50424954
#define PBIT_CHUNK_TEXT 0x54455854
#define PBIT_CHUNK_TRAK 0x5452414b
#define PBIT_CHUNK_DATA 0x44415441
#define PBIT_CHUNK_END  0x454e4420

#define PBIT_CRC_POLY   0x1edc6f41


static
unsigned long pbit_crc (unsigned long crc, const void *buf, unsigned cnt)
{
	unsigned             i, j;
	unsigned             val;
	unsigned long        reg;
	const unsigned char  *src;
	static int           tab_ok = 0;
	static unsigned long tab[256];

	if (tab_ok == 0) {
		for (i = 0; i < 256; i++) {
			reg = (unsigned long) i << 24;

			for (j = 0; j < 8; j++) {
				if (reg & 0x80000000) {
					reg = (reg << 1) ^ PBIT_CRC_POLY;
				}
				else {
					reg = reg << 1;
				}
			}

			tab[i] = reg;
		}

		tab_ok = 1;
	}

	src = buf;

	while (cnt > 0) {
		val = (crc >> 24) ^ *(src++);
		crc = (crc << 8) ^ tab[val & 0xff];
		cnt -= 1;
	}

	return (crc & 0xffffffff);
}

static
int pbit_skip_chunk (FILE *fp, unsigned long size, unsigned long crc)
{
	unsigned      cnt;
	unsigned char buf[256];

	while (size > 0) {
		cnt = (size < 256) ? size : 256;

		if (pbit_read (fp, buf, cnt)) {
			return (1);
		}

		crc = pbit_crc (crc, buf, cnt);

		size -= cnt;
	}

	if (pbit_read (fp, buf, 4)) {
		return (1);
	}

	if (pbit_get_uint32_be (buf, 0) != crc) {
		fprintf (stderr, "pbit: crc error\n");
		return (1);
	}

	return (0);
}

static
int pbit_load_header (FILE *fp, pbit_img_t *img, unsigned long size, unsigned long crc)
{
	unsigned char buf[8];
	unsigned long vers;

	if (size < 8) {
		return (1);
	}

	if (pbit_read (fp, buf, 8)) {
		return (1);
	}

	crc = pbit_crc (crc, buf, 8);

	vers = pbit_get_uint32_be (buf, 0);

	if (vers != 0) {
		fprintf (stderr, "pbit: unknown version number (%lu)\n", vers);
		return (1);
	}

	if (pbit_skip_chunk (fp, size - 8, crc)) {
		return (1);
	}

	return (0);
}

static
int pbit_load_comment (FILE *fp, pbit_img_t *img, unsigned size, unsigned long crc)
{
	unsigned      i, j, k, d;
	unsigned char *buf;

	if (size == 0) {
		return (pbit_skip_chunk (fp, size, crc));
	}

	buf = malloc (size);

	if (buf == NULL) {
		return (1);
	}

	if (pbit_read (fp, buf, size)) {
		free (buf);
		return (1);
	}

	crc = pbit_crc (crc, buf, size);

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
		return (pbit_skip_chunk (fp, 0, crc));
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

	if (img->comment_size > 0) {
		unsigned char c;

		c = 0x0a;

		if (pbit_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	if (pbit_img_add_comment (img, buf + i, j - i)) {
		free (buf);
		return (1);
	}

	free (buf);

	if (pbit_skip_chunk (fp, 0, crc)) {
		return (1);
	}

	return (0);
}

static
pbit_trk_t *pbit_load_track_header (FILE *fp, pbit_img_t *img, unsigned long size, unsigned long crc)
{
	unsigned char buf[20];
	unsigned long c, h, n, clock;
	pbit_trk_t    *trk;

	if (size < 20) {
		return (NULL);
	}

	if (pbit_read (fp, buf, 20)) {
		return (NULL);
	}

	crc = pbit_crc (crc, buf, 20);

	size -= 20;

	c = pbit_get_uint32_be (buf, 0);
	h = pbit_get_uint32_be (buf, 4);
	n = pbit_get_uint32_be (buf, 8);
	clock = pbit_get_uint32_be (buf, 12);
	//flags = pbit_get_uint32_be (buf, 16);

	trk = pbit_img_get_track (img, c, h, 1);

	if (trk == NULL) {
		return (NULL);
	}

	if (pbit_trk_set_size (trk, n)) {
		return (NULL);
	}

	pbit_trk_set_clock (trk, clock);

	if (pbit_skip_chunk (fp, size, crc)) {
		return (NULL);
	}

	return (trk);
}

static
int pbit_load_track_data (FILE *fp, pbit_img_t *img, pbit_trk_t *trk, unsigned long size, unsigned long crc)
{
	unsigned long cnt;

	cnt = (trk->size + 7) / 8;

	if (cnt < size) {
		cnt = size;
	}

	if (pbit_read (fp, trk->data, cnt)) {
		return (1);
	}

	crc = pbit_crc (crc, trk->data, cnt);

	if (pbit_skip_chunk (fp, size - cnt, crc)) {
		return (1);
	}

	return (0);
}

static
int pbit_load_image (FILE *fp, pbit_img_t *img)
{
	unsigned long type, size;
	unsigned long crc;
	pbit_trk_t    *trk;
	unsigned char buf[8];

	if (pbit_read (fp, buf, 4)) {
		return (0);
	}

	type = pbit_get_uint32_be (buf, 0);

	if (type != PBIT_CHUNK_PBIT) {
		return (1);
	}

	if (pbit_read (fp, buf + 4, 4)) {
		return (1);
	}

	size = pbit_get_uint32_be (buf, 4);

	crc = pbit_crc (0, buf, 8);

	if (pbit_load_header (fp, img, size, crc)) {
		return (1);
	}

	trk = NULL;

	while (1) {
		if (pbit_read (fp, buf, 8)) {
			return (1);
		}

		type = pbit_get_uint32_be (buf, 0);
		size = pbit_get_uint32_be (buf, 4);

		crc = pbit_crc (0, buf, 8);

		if (type == PBIT_CHUNK_END) {
			if (pbit_skip_chunk (fp, size, crc)) {
				return (1);
			}

			return (0);
		}
		else if (type == PBIT_CHUNK_TEXT) {
			if (pbit_load_comment (fp, img, size, crc)) {
				return (1);
			}
		}
		else if (type == PBIT_CHUNK_TRAK) {
			trk = pbit_load_track_header (fp, img, size, crc);

			if (trk == NULL) {
				return (1);
			}
		}
		else if (type == PBIT_CHUNK_DATA) {
			if (trk == NULL) {
				return (1);
			}

			if (pbit_load_track_data (fp, img, trk, size, crc)) {
				return (1);
			}
		}
		else {
			if (pbit_skip_chunk (fp, size, crc)) {
				return (1);
			}
		}
	}

	return (1);
}

pbit_img_t *pbit_load_pbit (FILE *fp)
{
	pbit_img_t *img;

	img = pbit_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (pbit_load_image (fp, img)) {
		pbit_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int pbit_save_header (FILE *fp, const pbit_img_t *img)
{
	unsigned char buf[32];

	pbit_set_uint32_be (buf, 0, PBIT_CHUNK_PBIT);
	pbit_set_uint32_be (buf, 4, 8);
	pbit_set_uint32_be (buf, 8, 0);
	pbit_set_uint32_be (buf, 12, 0);
	pbit_set_uint32_be (buf, 16, pbit_crc (0, buf, 16));

	if (pbit_write (fp, buf, 20)) {
		return (1);
	}

	return (0);
}

static
int pbit_save_end (FILE *fp, const pbit_img_t *img)
{
	unsigned char buf[16];

	pbit_set_uint32_be (buf, 0, PBIT_CHUNK_END);
	pbit_set_uint32_be (buf, 4, 0);
	pbit_set_uint32_be (buf, 8, pbit_crc (0, buf, 8));

	if (pbit_write (fp, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int pbit_save_comment (FILE *fp, const pbit_img_t *img)
{
	unsigned long       i, j;
	unsigned long       crc;
	const unsigned char *src;
	unsigned char       *buf;
	unsigned char       hdr[8];

	if (img->comment_size == 0) {
		return (0);
	}

	buf = malloc (img->comment_size + 2);

	if (buf == NULL) {
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
		free (buf);
		return (0);
	}

	buf[j++] = 0x0a;

	pbit_set_uint32_be (hdr, 0, PBIT_CHUNK_TEXT);
	pbit_set_uint32_be (hdr, 4, j);

	crc = pbit_crc (0, hdr, 8);

	if (pbit_write (fp, hdr, 8)) {
		return (1);
	}

	crc = pbit_crc (crc, buf, j);

	if (pbit_write (fp, buf, j)) {
		return (1);
	}

	pbit_set_uint32_be (hdr, 0, crc);

	if (pbit_write (fp, hdr, 4)) {
		return (1);
	}

	return (0);
}

static
int pbit_save_track (FILE *fp, const pbit_trk_t *trk, unsigned long c, unsigned long h)
{
	unsigned long cnt;
	unsigned char buf[32];
	unsigned long crc;

	pbit_set_uint32_be (buf, 0, PBIT_CHUNK_TRAK);
	pbit_set_uint32_be (buf, 4, 20);
	pbit_set_uint32_be (buf, 8, c);
	pbit_set_uint32_be (buf, 12, h);
	pbit_set_uint32_be (buf, 16, trk->size);
	pbit_set_uint32_be (buf, 20, trk->clock);
	pbit_set_uint32_be (buf, 24, 0);
	pbit_set_uint32_be (buf, 28, pbit_crc (0, buf, 28));

	if (pbit_write (fp, buf, 32)) {
		return (1);
	}

	if (trk->size == 0) {
		return (0);
	}

	cnt = (trk->size + 7) / 8;

	pbit_set_uint32_be (buf, 0, PBIT_CHUNK_DATA);
	pbit_set_uint32_be (buf, 4, cnt);

	crc = pbit_crc (0, buf, 8);

	if (pbit_write (fp, buf, 8)) {
		return (1);
	}

	crc = pbit_crc (crc, trk->data, cnt);

	if (pbit_write (fp, trk->data, cnt)) {
		return (1);
	}

	pbit_set_uint32_be (buf, 0, crc);

	if (pbit_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

int pbit_save_pbit (FILE *fp, const pbit_img_t *img)
{
	unsigned long c, h;
	pbit_cyl_t    *cyl;
	pbit_trk_t    *trk;

	if (pbit_save_header (fp, img)) {
		return (1);
	}

	if (pbit_save_comment (fp, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (cyl == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk == NULL) {
				continue;
			}

			if (pbit_save_track (fp, trk, c, h)) {
				return (1);
			}
		}
	}

	if (pbit_save_end (fp, img)) {
		return (1);
	}

	return (0);
}


int pbit_probe_pbit_fp (FILE *fp)
{
	unsigned char buf[4];

	if (pbit_read (fp, buf, 4)) {
		return (0);
	}

	if (pbit_get_uint32_be (buf, 0) != PBIT_CHUNK_PBIT) {
		return (0);
	}

	return (1);
}

int pbit_probe_pbit (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = pbit_probe_pbit_fp (fp);

	fclose (fp);

	return (r);
}
