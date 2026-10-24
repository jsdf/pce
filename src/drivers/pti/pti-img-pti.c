/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-pti.c                                *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2024 Hampa Hug <hampa@hampa.ch>                     *
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

#include "pti.h"
#include "pti-img-pti.h"

#include <lib/ciff.h>
#include <lib/endian.h>


#define CKID_PTI  0x50544920
#define CKID_TEXT 0x54455854
#define CKID_DATA 0x44415441
#define CKID_END  0x454e4420

#define PTI_FLAG_INVERT 1
#define PTI_FLAG_KNOWN  (PTI_FLAG_INVERT)


typedef struct {
	ciff_t        ciff;

	pti_img_t     *img;

	unsigned long bufmax;
	unsigned char *buf;
} pti_load_t;


static
unsigned char *pti_alloc (pti_load_t *pti, unsigned long size)
{
	unsigned char *tmp;

	if (pti->bufmax < size) {
		if ((tmp = realloc (pti->buf, size)) == NULL) {
			return (NULL);
		}

		pti->buf = tmp;
		pti->bufmax = size;
	}

	return (pti->buf);
}

static
void pti_free (pti_load_t *pti)
{
	free (pti->buf);

	pti->buf = NULL;
	pti->bufmax = 0;
}

static
int pti_load_header (pti_load_t *pti)
{
	unsigned      n;
	unsigned char buf[16];
	unsigned long vers, clock, flags;

	n = (pti->ciff.size < 12) ? 8 : 12;

	if (ciff_read (&pti->ciff, buf, n)) {
		return (1);
	}

	vers = get_uint32_be (buf, 0);

	if (vers != 0) {
		fprintf (stderr, "pti: unknown version number (%lu)\n", vers);
		return (1);
	}

	clock = get_uint32_be (buf, 4);

	pti_img_set_clock (pti->img, clock);

	flags = (n >= 12) ? get_uint32_be (buf, 8) : 0;

	if (flags & ~PTI_FLAG_KNOWN) {
		fprintf (stderr, "pti: unknown flags (0x%08lx)\n", flags);
	}

	pti_img_set_inverted (pti->img, flags & PTI_FLAG_INVERT);

	return (0);
}

static
int pti_load_comment (pti_load_t *pti)
{
	unsigned      i, j, k, d;
	unsigned long size;
	unsigned char *buf;

	size = pti->ciff.size;

	if (size == 0) {
		return (0);
	}

	if ((buf = pti_alloc (pti, size)) == NULL) {
		return (1);
	}

	if (ciff_read (&pti->ciff, buf, size)) {
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

	if (pti->img->comment_size > 0) {
		unsigned char c;

		c = 0x0a;

		if (pti_img_add_comment (pti->img, &c, 1)) {
			return (1);
		}
	}

	if (pti_img_add_comment (pti->img, buf + i, j - i)) {
		return (1);
	}

	return (0);
}

static
int pti_decode_data (pti_load_t *pti, unsigned char *buf, unsigned long size)
{
	int           inv;
	unsigned      tag, cnt;
	unsigned long clk;
	int           level;

	inv = pti_img_get_inverted (pti->img);

	if (ciff_read (&pti->ciff, buf, size)) {
		return (1);
	}

	while (size > 0) {
		size -= 1;

		tag = *(buf++);

		cnt = ((tag >> 4) & 3) + 1;
		clk = tag & 0x0f;

		if (cnt > size) {
			return (1);
		}

		size -= cnt;

		while (cnt-- > 0) {
			clk = (clk << 8) | *(buf++);
		}

		switch (tag & 0xc0) {
		case 0x00:
			level = 1;
			break;

		case 0x40:
			level = 0;
			break;

		case 0x80:
			level = -1;
			break;

		case 0xc0:
			level = -1;
			break;

		default:
			return (1);
		}

		if (inv) {
			level = -level;
		}

		if ((tag & 0xc0) == 0x80) {
			if (pti_img_add_pulse (pti->img, clk / 2, level)) {
				return (1);
			}

			if (pti_img_add_pulse (pti->img, clk - (clk / 2), -level)) {
				return (1);
			}
		}
		else {
			if (pti_img_add_pulse (pti->img, clk, level)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int pti_load_data (pti_load_t *pti)
{
	unsigned long size;
	unsigned char *buf;

	size = pti->ciff.size;

	if ((buf = pti_alloc (pti, size)) == NULL) {
		return (1);
	}

	if (pti_decode_data (pti, buf, size)) {
		return (1);
	}

	return (0);
}

static
int pti_load_image (pti_load_t *pti)
{
	if (ciff_read_id (&pti->ciff)) {
		return (1);
	}

	if (pti->ciff.ckid != CKID_PTI) {
		return (1);
	}

	if (pti_load_header (pti)) {
		return (1);
	}

	while (ciff_read_id (&pti->ciff) == 0) {
		switch (pti->ciff.ckid) {
		case CKID_END:
			if (ciff_read_crc (&pti->ciff)) {
				return (1);
			}
			return (0);

		case CKID_TEXT:
			if (pti_load_comment (pti)) {
				return (1);
			}
			break;

		case CKID_DATA:
			if (pti_load_data (pti)) {
				return (1);
			}
			break;
		}

	}

	return (1);
}

pti_img_t *pti_load_pti (FILE *fp)
{
	int        r;
	pti_load_t pti;

	memset (&pti, 0, sizeof (pti));

	if ((pti.img = pti_img_new()) == NULL) {
		return (NULL);
	}

	ciff_init (&pti.ciff, fp, 1);

	pti.buf = NULL;
	pti.bufmax = 0;

	r = pti_load_image (&pti);

	if (pti.ciff.crc_error) {
		fprintf (stderr, "pti: crc error\n");
	}

	ciff_free (&pti.ciff);
	pti_free (&pti);

	if (r) {
		pti_img_del (pti.img);
		return (NULL);
	}

	pti_img_clean (pti.img);

	return (pti.img);
}


static
int pti_save_header (ciff_t *ciff, const pti_img_t *img)
{
	unsigned char buf[24];
	unsigned long flags;

	flags = 0;

	if (pti_img_get_inverted (img)) {
		flags |= PTI_FLAG_INVERT;
	}

	set_uint32_be (buf, 0, 0);
	set_uint32_be (buf, 4, pti_img_get_clock (img));
	set_uint32_be (buf, 8, flags);

	if (ciff_write_chunk (ciff, CKID_PTI, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int pti_save_end (ciff_t *ciff, const pti_img_t *img)
{
	if (ciff_write_chunk (ciff, CKID_END, NULL, 0)) {
		return (1);
	}

	return (0);
}

static
int pti_save_comment (ciff_t *ciff, const pti_img_t *img)
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
int pti_save_data (ciff_t *ciff, const pti_img_t *img)
{
	int           r, inv;
	unsigned long clk, clk2;
	int           level, level2;
	unsigned      tag;
	unsigned long i, n, max;
	unsigned char *buf, *tmp;

	if (img->pulse_cnt == 0) {
		return (0);
	}

	max = 4096;

	if ((buf = malloc (max)) == NULL) {
		return (1);
	}

	inv = pti_img_get_inverted (img);

	i = 0;
	n = 0;

	while (i < img->pulse_cnt) {
		pti_pulse_get (img->pulse[i++], &clk, &level);

		if (inv) {
			level = -level;
		}

		if (clk == 0) {
			continue;
		}

		if ((n + 16) > max) {
			max *= 2;

			if ((tmp = realloc (buf, max)) == 0) {
				free (buf);
				return (1);
			}

			buf = tmp;
		}

		clk2 = 0;
		level2 = 0;

		if (i < img->pulse_cnt) {
			pti_pulse_get (img->pulse[i], &clk2, &level2);

			if (inv) {
				level2 = -level2;
			}
		}

		if ((level < 0) && (level2 > 0) && (clk == ((clk + clk2) / 2))) {
			i += 1;
			tag = 0x80;
			clk += clk2;
		}
		else if (level < 0) {
			tag = 0xc0;
		}
		else if (level > 0) {
			tag = 0x00;
		}
		else {
			tag = 0x40;
		}

		if (clk < (1UL << 12)) {
			buf[n++] = tag | 0x00 | ((clk >> 8) & 0x0f);

		}
		else if (clk < (1UL << 20)) {
			buf[n++] = tag | 0x10 | ((clk >> 16) & 0x0f);
			buf[n++] = (clk >> 8) & 0xff;
		}
		else if (clk < (1UL << 28)) {
			buf[n++] = tag | 0x20 | ((clk >> 24) & 0x0f);
			buf[n++] = (clk >> 16) & 0xff;
			buf[n++] = (clk >> 8) & 0xff;
		}
		else {
			buf[n++] = tag | 0x30;
			buf[n++] = (clk >> 24) & 0xff;
			buf[n++] = (clk >> 16) & 0xff;
			buf[n++] = (clk >> 8) & 0xff;
		}

		buf[n++] = clk & 0xff;
	}

	r = ciff_write_chunk (ciff, CKID_DATA, buf, n);

	free (buf);

	return (r);
}

int pti_save_pti (FILE *fp, const pti_img_t *img)
{
	ciff_t ciff;

	ciff_init (&ciff, fp, 1);

	if (pti_save_header (&ciff, img)) {
		return (1);
	}

	if (pti_save_comment (&ciff, img)) {
		return (1);
	}

	if (pti_save_data (&ciff, img)) {
		return (1);
	}

	if (pti_save_end (&ciff, img)) {
		return (1);
	}

	return (0);
}


int pti_probe_pti_fp (FILE *fp)
{
	unsigned char buf[4];

	if (fread (buf, 1, 4, fp) != 4) {
		return (0);
	}

	if (get_uint32_be (buf, 0) != CKID_PTI) {
		return (0);
	}

	return (1);
}

int pti_probe_pti (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pti_probe_pti_fp (fp);

	fclose (fp);

	return (r);
}
