/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-pfi.c                                    *
 * Created:     2013-12-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2017 Hampa Hug <hampa@hampa.ch>                     *
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
#include "pfi-io.h"
#include "pfi-pfi.h"


#define PFI_MAGIC_PFI  0x50464920
#define PFI_MAGIC_TEXT 0x54455854
#define PFI_MAGIC_TRAK 0x5452414b
#define PFI_MAGIC_INDX 0x494e4458
#define PFI_MAGIC_DATA 0x44415441
#define PFI_MAGIC_END  0x454e4420

#define PFI_CRC_POLY   0x1edc6f41


static
unsigned long pfi_crc (unsigned long crc, const void *buf, unsigned cnt)
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
					reg = (reg << 1) ^ PFI_CRC_POLY;
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
int pfi_skip_chunk (FILE *fp, unsigned long size, unsigned long crc)
{
	unsigned      cnt;
	unsigned char buf[256];

	while (size > 0) {
		cnt = (size < 256) ? size : 256;

		if (pfi_read (fp, buf, cnt)) {
			return (1);
		}

		crc = pfi_crc (crc, buf, cnt);

		size -= cnt;
	}

	if (pfi_read (fp, buf, 4)) {
		return (1);
	}

	if (pfi_get_uint32_be (buf, 0) != crc) {
		fprintf (stderr, "pfi: crc error\n");
		return (1);
	}

	return (0);
}

static
int pfi_load_header (FILE *fp, pfi_img_t *img, unsigned long size, unsigned long crc)
{
	unsigned char buf[4];
	unsigned long vers;

	if (size < 4) {
		return (1);
	}

	if (pfi_read (fp, buf, 4)) {
		return (1);
	}

	crc = pfi_crc (crc, buf, 4);

	vers = pfi_get_uint32_be (buf, 0);

	if (vers != 0) {
		fprintf (stderr, "pfi: unknown version number (%lu)\n", vers);
		return (1);
	}

	if (pfi_skip_chunk (fp, size - 4, crc)) {
		return (1);
	}

	return (0);
}

static
int pfi_load_comment (FILE *fp, pfi_img_t *img, unsigned size, unsigned long crc)
{
	unsigned      i, j, k, d;
	unsigned char *buf;

	if (size == 0) {
		return (pfi_skip_chunk (fp, size, crc));
	}

	if ((buf = malloc (size)) == NULL) {
		return (1);
	}

	if (pfi_read (fp, buf, size)) {
		free (buf);
		return (1);
	}

	crc = pfi_crc (crc, buf, size);

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
		return (pfi_skip_chunk (fp, 0, crc));
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

		if (pfi_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	if (pfi_img_add_comment (img, buf + i, j - i)) {
		free (buf);
		return (1);
	}

	free (buf);

	if (pfi_skip_chunk (fp, 0, crc)) {
		return (1);
	}

	return (0);
}

static
pfi_trk_t *pfi_load_track_header (FILE *fp, pfi_img_t *img, unsigned long size, unsigned long crc)
{
	unsigned char buf[12];
	unsigned long c, h, clock;
	pfi_trk_t     *trk;

	if (size < 12) {
		return (NULL);
	}

	if (pfi_read (fp, buf, 12)) {
		return (NULL);
	}

	crc = pfi_crc (crc, buf, 12);

	size -= 12;

	c = pfi_get_uint32_be (buf, 0);
	h = pfi_get_uint32_be (buf, 4);
	clock = pfi_get_uint32_be (buf, 8);

	trk = pfi_img_get_track (img, c, h, 1);

	if (trk == NULL) {
		return (NULL);
	}

	pfi_trk_set_clock (trk, clock);

	if (pfi_skip_chunk (fp, size, crc)) {
		return (NULL);
	}

	return (trk);
}

static
int pfi_load_indx (FILE *fp, pfi_img_t *img, pfi_trk_t *trk, unsigned long size, unsigned long crc)
{
	unsigned char buf[4];

	while (size >= 4) {
		if (pfi_read (fp, buf, 4)) {
			return (1);
		}

		crc = pfi_crc (crc, buf, 4);

		if (pfi_trk_add_index (trk, pfi_get_uint32_be (buf, 0))) {
			return (1);
		}

		size -= 4;
	}

	if (pfi_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pfi_load_track_data (FILE *fp, pfi_img_t *img, pfi_trk_t *trk, unsigned long size, unsigned long crc)
{
	if (pfi_trk_set_size (trk, size)) {
		return (1);
	}

	if (pfi_read (fp, trk->data, size)) {
		return (1);
	}

	crc = pfi_crc (crc, trk->data, size);

	if (pfi_skip_chunk (fp, 0, crc)) {
		return (1);
	}

	return (0);
}

static
int pfi_load_image (FILE *fp, pfi_img_t *img)
{
	int           have_header;
	unsigned long type, size;
	unsigned long crc;
	pfi_trk_t     *trk;
	unsigned char buf[8];

	have_header = 0;
	trk = NULL;

	while (1) {
		if (pfi_read (fp, buf, 8)) {
			return (1);
		}

		type = pfi_get_uint32_be (buf, 0);
		size = pfi_get_uint32_be (buf, 4);

		crc = pfi_crc (0, buf, 8);

		if (type == PFI_MAGIC_END) {
			if (pfi_skip_chunk (fp, size, crc)) {
				return (1);
			}

			return (0);
		}
		else if (type == PFI_MAGIC_PFI) {
			if (have_header) {
				return (1);
			}

			if (pfi_load_header (fp, img, size, crc)) {
				return (1);
			}

			have_header = 1;
		}
		else if (type == PFI_MAGIC_TEXT) {
			if (have_header == 0) {
				return (1);
			}

			if (pfi_load_comment (fp, img, size, crc)) {
				return (1);
			}
		}
		else if (type == PFI_MAGIC_TRAK) {
			if (have_header == 0) {
				return (1);
			}

			trk = pfi_load_track_header (fp, img, size, crc);

			if (trk == NULL) {
				return (1);
			}
		}
		else if (type == PFI_MAGIC_INDX) {
			if (trk == NULL) {
				return (1);
			}

			if (pfi_load_indx (fp, img, trk, size, crc)) {
				return (1);
			}
		}
		else if (type == PFI_MAGIC_DATA) {
			if (trk == NULL) {
				return (1);
			}

			if (pfi_load_track_data (fp, img, trk, size, crc)) {
				return (1);
			}
		}
		else {
			if (have_header == 0) {
				return (1);
			}

			if (pfi_skip_chunk (fp, size, crc)) {
				return (1);
			}
		}
	}

	return (1);
}

pfi_img_t *pfi_load_pfi (FILE *fp)
{
	pfi_img_t *img;

	img = pfi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (pfi_load_image (fp, img)) {
		pfi_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int pfi_save_header (FILE *fp, const pfi_img_t *img)
{
	unsigned char buf[16];

	pfi_set_uint32_be (buf, 0, PFI_MAGIC_PFI);
	pfi_set_uint32_be (buf, 4, 4);
	pfi_set_uint32_be (buf, 8, 0);
	pfi_set_uint32_be (buf, 12, pfi_crc (0, buf, 12));

	if (pfi_write (fp, buf, 16)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_end (FILE *fp, const pfi_img_t *img)
{
	unsigned char buf[16];

	pfi_set_uint32_be (buf, 0, PFI_MAGIC_END);
	pfi_set_uint32_be (buf, 4, 0);
	pfi_set_uint32_be (buf, 8, pfi_crc (0, buf, 8));

	if (pfi_write (fp, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_comment (FILE *fp, const pfi_img_t *img)
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

	pfi_set_uint32_be (hdr, 0, PFI_MAGIC_TEXT);
	pfi_set_uint32_be (hdr, 4, j);

	crc = pfi_crc (0, hdr, 8);

	if (pfi_write (fp, hdr, 8)) {
		return (1);
	}

	crc = pfi_crc (crc, buf, j);

	if (pfi_write (fp, buf, j)) {
		return (1);
	}

	pfi_set_uint32_be (hdr, 0, crc);

	if (pfi_write (fp, hdr, 4)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_indx (FILE *fp, const pfi_trk_t *trk)
{
	unsigned      i;
	unsigned long crc;
	unsigned char buf[8];

	pfi_set_uint32_be (buf, 0, PFI_MAGIC_INDX);
	pfi_set_uint32_be (buf, 4, 4 * trk->idx_cnt);

	crc = pfi_crc (0, buf, 8);

	if (pfi_write (fp, buf, 8)) {
		return (1);
	}

	for (i = 0; i < trk->idx_cnt; i++) {
		pfi_set_uint32_be (buf, 0, trk->idx[i]);

		crc = pfi_crc (crc, buf, 4);

		if (pfi_write (fp, buf, 4)) {
			return (1);
		}
	}

	pfi_set_uint32_be (buf, 0, crc);

	if (pfi_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_data (FILE *fp, const pfi_trk_t *trk)
{
	unsigned long crc;
	unsigned char buf[32];

	if (trk->size == 0) {
		return (0);
	}

	pfi_set_uint32_be (buf, 0, PFI_MAGIC_DATA);
	pfi_set_uint32_be (buf, 4, trk->size);

	crc = pfi_crc (0, buf, 8);

	if (pfi_write (fp, buf, 8)) {
		return (1);
	}

	crc = pfi_crc (crc, trk->data, trk->size);

	if (pfi_write (fp, trk->data, trk->size)) {
		return (1);
	}

	pfi_set_uint32_be (buf, 0, crc);

	if (pfi_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int pfi_save_track (FILE *fp, const pfi_trk_t *trk, unsigned long c, unsigned long h)
{
	unsigned char buf[32];

	pfi_set_uint32_be (buf, 0, PFI_MAGIC_TRAK);
	pfi_set_uint32_be (buf, 4, 12);
	pfi_set_uint32_be (buf, 8, c);
	pfi_set_uint32_be (buf, 12, h);
	pfi_set_uint32_be (buf, 16, trk->clock);
	pfi_set_uint32_be (buf, 20, pfi_crc (0, buf, 20));

	if (pfi_write (fp, buf, 24)) {
		return (1);
	}

	if (pfi_save_indx (fp, trk)) {
		return (1);
	}

	if (pfi_save_data (fp, trk)) {
		return (1);
	}

	return (0);
}

int pfi_save_pfi (FILE *fp, const pfi_img_t *img)
{
	unsigned long c, h;
	pfi_cyl_t     *cyl;
	pfi_trk_t     *trk;

	if (pfi_save_header (fp, img)) {
		return (1);
	}

	if (pfi_save_comment (fp, img)) {
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

			if (pfi_save_track (fp, trk, c, h)) {
				return (1);
			}
		}
	}

	if (pfi_save_end (fp, img)) {
		return (1);
	}

	return (0);
}


int pfi_probe_pfi_fp (FILE *fp)
{
	return (0);
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
