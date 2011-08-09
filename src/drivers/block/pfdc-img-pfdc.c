/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-pfdc.c                            *
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
#include "pfdc-img-pfdc.h"


/* version 0 sector flags */
#define PFDC_FLAG0_FM         0x01
#define PFDC_FLAG0_MFM        0x02
#define PFDC_FLAG0_CRC_ID     0x04
#define PFDC_FLAG0_CRC_DATA   0x08
#define PFDC_FLAG0_DEL_DAM    0x10
#define PFDC_FLAG0_COMPRESSED 0x80
#define PFDC_FLAG0_MASK       0x1f

/* version 1 sector flags */
#define PFDC_FLAG1_ALTERNATE  0x01
#define PFDC_FLAG1_CRC_ID     0x04
#define PFDC_FLAG1_CRC_DATA   0x08
#define PFDC_FLAG1_DEL_DAM    0x10
#define PFDC_FLAG1_COMPRESSED 0x80
#define PFDC_FLAG1_MASK       0x1d

/* version 2 sector flags */
#define PFDC_FLAG2_CRC_ID     0x01
#define PFDC_FLAG2_CRC_DATA   0x02
#define PFDC_FLAG2_DEL_DAM    0x04
#define PFDC_FLAG2_ALTERNATE  0x40
#define PFDC_FLAG2_COMPRESSED 0x80

#define PFDC_CHUNK_SC 0x5343
#define PFDC_CHUNK_EN 0x454e
#define PFDC_CHUNK_CM 0x434d
#define PFDC_CHUNK_TG 0x5447

#define PFDC_CRC_POLY         0x04c11db7


static unsigned long par_file_crc;


static
unsigned long pfdc_crc (unsigned long crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;

	tmp = buf;

	while (cnt > 0) {
		crc ^= (*tmp & 0xff) << 24;

		for (i = 0; i < 8; i++) {
			if (crc & 0x80000000) {
				crc = (crc << 1) ^ PFDC_CRC_POLY;
			}
			else {
				crc = crc << 1;
			}
		}

		tmp += 1;
		cnt -= 1;
	}

	return (crc &= 0xffffffff);
}

static
int pfdc_read (FILE *fp, void *buf, unsigned cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	par_file_crc = pfdc_crc (par_file_crc, buf, cnt);

	return (0);
}

static
int pfdc_write (FILE *fp, const void *buf, unsigned cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	par_file_crc = pfdc_crc (par_file_crc, buf, cnt);

	return (0);
}

static
int pfdc_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfdc_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}

static
void pfdc_sct_set_flags_v0 (pfdc_sct_t *sct, unsigned flg, unsigned long dr)
{
	unsigned enc;

	if (flg & PFDC_FLAG0_FM) {
		enc = PFDC_ENC_FM;
	}
	else if (flg & PFDC_FLAG0_MFM) {
		enc = PFDC_ENC_MFM;
	}
	else {
		enc = PFDC_ENC_UNKNOWN;
	}

	pfdc_sct_set_encoding (sct, enc, dr);

	if (flg & PFDC_FLAG0_CRC_ID) {
		sct->flags |= PFDC_FLAG_CRC_ID;
	}

	if (flg & PFDC_FLAG0_CRC_DATA) {
		sct->flags |= PFDC_FLAG_CRC_DATA;
	}

	if (flg & PFDC_FLAG0_DEL_DAM) {
		sct->flags |= PFDC_FLAG_DEL_DAM;
	}
}

static
int pfdc_load_sector_v0 (FILE *fp, pfdc_img_t *img)
{
	unsigned      c, h, n, f;
	unsigned char buf[16];
	pfdc_sct_t    *sct;

	if (pfdc_read (fp, buf, 12)) {
		return (1);
	}

	c = buf[0];
	h = buf[1];
	f = buf[5];
	n = pfdc_get_uint16_be (buf, 6);

	sct = pfdc_sct_new (buf[2], buf[3], buf[4], n);

	if (sct == NULL) {
		return (1);
	}

	if (pfdc_img_add_sector (img, sct, c, h)) {
		pfdc_sct_del (sct);
		return (1);
	}

	pfdc_sct_set_flags_v0 (sct, f, pfdc_get_uint32_be (buf, 8));

	if (f & PFDC_FLAG0_COMPRESSED) {
		if (pfdc_read (fp, buf + 12, 1)) {
			return (1);
		}

		pfdc_sct_fill (sct, buf[12]);
	}
	else {
		if (pfdc_read (fp, sct->data, n)) {
			return (1);
		}
	}

	return (0);
}

static
void pfdc_sct_set_flags_v1 (pfdc_sct_t *sct, unsigned long flg)
{
	if (flg & PFDC_FLAG1_CRC_ID) {
		sct->flags |= PFDC_FLAG_CRC_ID;
	}

	if (flg & PFDC_FLAG1_CRC_DATA) {
		sct->flags |= PFDC_FLAG_CRC_DATA;
	}

	if (flg & PFDC_FLAG1_DEL_DAM) {
		sct->flags |= PFDC_FLAG_DEL_DAM;
	}
}

static
int pfdc_load_sector_v1 (FILE *fp, pfdc_img_t *img, pfdc_sct_t **last)
{
	unsigned      c, h, n;
	unsigned long f;
	unsigned char buf[32];
	pfdc_sct_t    *sct;

	if (pfdc_read (fp, buf, 16)) {
		return (1);
	}

	if (buf[0] != 'S') {
		return (1);
	}

	c = buf[1];
	h = buf[2];
	n = pfdc_get_uint16_be (buf, 6);

	sct = pfdc_sct_new (buf[3], buf[4], buf[5], n);

	if (sct == NULL) {
		return (1);
	}

	f = pfdc_get_uint32_be (buf, 12);

	pfdc_sct_set_flags_v1 (sct, f);
	pfdc_sct_set_encoding (sct, buf[8], pfdc_get_uint32_be (buf, 8) & 0x00ffffff);

	if (f & PFDC_FLAG1_ALTERNATE) {
		if (*last == NULL) {
			pfdc_sct_del (sct);
			return (1);
		}

		pfdc_sct_add_alternate (*last, sct);
	}
	else {
		if (pfdc_img_add_sector (img, sct, c, h)) {
			pfdc_sct_del (sct);
			return (1);
		}
	}

	if (f & PFDC_FLAG1_COMPRESSED) {
		if (pfdc_read (fp, buf + 16, 1)) {
			return (1);
		}

		pfdc_sct_fill (sct, buf[16]);
	}
	else {
		if (pfdc_read (fp, sct->data, n)) {
			return (1);
		}
	}

	*last = sct;

	return (0);
}

static
void pfdc_sct_set_flags_v2 (pfdc_sct_t *sct, unsigned long flg)
{
	if (flg & PFDC_FLAG2_CRC_ID) {
		sct->flags |= PFDC_FLAG_CRC_ID;
	}

	if (flg & PFDC_FLAG2_CRC_DATA) {
		sct->flags |= PFDC_FLAG_CRC_DATA;
	}

	if (flg & PFDC_FLAG2_DEL_DAM) {
		sct->flags |= PFDC_FLAG_DEL_DAM;
	}
}

static
int pfdc_skip_chunk_v2 (FILE *fp, unsigned cnt, unsigned long crc)
{
	unsigned      n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfdc_read (fp, buf, n)) {
			return (1);
		}

		crc = pfdc_crc (crc, buf, n);

		cnt -= n;
	}

	if (pfdc_read (fp, buf, 4)) {
		return (1);
	}

	if (pfdc_get_uint32_be (buf, 0) != crc) {
		return (1);
	}

	return (0);
}

static
int pfdc_load_sector_v2 (FILE *fp, pfdc_img_t *img, pfdc_sct_t **last, unsigned size, unsigned long crc)
{
	unsigned      c, h, n;
	unsigned long f;
	unsigned char buf[32];
	pfdc_sct_t    *sct;

	if (size < 12) {
		return (1);
	}

	if (pfdc_read (fp, buf, 12)) {
		return (1);
	}

	f = buf[0];
	c = buf[1];
	h = buf[2];
	n = pfdc_get_uint16_be (buf, 6);

	sct = pfdc_sct_new (buf[3], buf[4], buf[5], n);

	if (sct == NULL) {
		return (1);
	}

	pfdc_sct_set_flags_v2 (sct, f);
	pfdc_sct_set_encoding (sct, buf[8], pfdc_get_uint32_be (buf, 8) & 0x00ffffff);

	if (f & PFDC_FLAG2_ALTERNATE) {
		if (*last == NULL) {
			pfdc_sct_del (sct);
			return (1);
		}

		pfdc_sct_add_alternate (*last, sct);
	}
	else {
		if (pfdc_img_add_sector (img, sct, c, h)) {
			pfdc_sct_del (sct);
			return (1);
		}
	}

	size -= 12;

	crc = pfdc_crc (crc, buf, 12);

	if (f & PFDC_FLAG2_COMPRESSED) {
		if (size < 1) {
			return (1);
		}

		if (pfdc_read (fp, buf, 1)) {
			return (1);
		}

		crc = pfdc_crc (crc, buf, 1);

		pfdc_sct_fill (sct, buf[0]);

		size -= 1;
	}
	else {
		if (size < sct->n) {
			return (1);
		}

		if (pfdc_read (fp, sct->data, n)) {
			return (1);
		}

		crc = pfdc_crc (crc, sct->data, n);

		size -= sct->n;
	}

	*last = sct;

	if (pfdc_skip_chunk_v2 (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pfdc_load_tags_v2 (FILE *fp, pfdc_sct_t *last, unsigned size, unsigned long crc)
{
	unsigned      cnt;
	unsigned char buf[256];

	if (last == NULL) {
		return (1);
	}

	cnt = (size < 256) ? size : 256;

	if (pfdc_read (fp, buf, cnt)) {
		return (1);
	}

	crc = pfdc_crc (crc, buf, cnt);

	pfdc_sct_set_tags (last, buf, cnt);

	return (pfdc_skip_chunk_v2 (fp, size - cnt, crc));
}

static
int pfdc_load_comment_v2 (FILE *fp, pfdc_img_t *img, unsigned size, unsigned long crc)
{
	unsigned      i, j, k, d;
	unsigned char *buf;

	if (size == 0) {
		return (pfdc_skip_chunk_v2 (fp, size, crc));
	}

	buf = malloc (size);

	if (buf == NULL) {
		return (1);
	}

	if (pfdc_read (fp, buf, size)) {
		free (buf);
		return (1);
	}

	crc = pfdc_crc (crc, buf, size);

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
		return (pfdc_skip_chunk_v2 (fp, 0, crc));
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

		if (pfdc_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	if (pfdc_img_add_comment (img, buf + i, j - i)) {
		free (buf);
		return (1);
	}

	free (buf);

	if (pfdc_skip_chunk_v2 (fp, 0, crc)) {
		return (1);
	}

	return (0);
}

static
int pfdc_load_chunks_v2 (FILE *fp, pfdc_img_t *img)
{
	unsigned      ckid, size;
	unsigned long crc;
	pfdc_sct_t    *last;
	unsigned char buf[4];

	last = NULL;

	while (1) {
		if (pfdc_read (fp, buf, 4)) {
			return (1);
		}

		ckid = pfdc_get_uint16_be (buf, 0);
		size = pfdc_get_uint16_be (buf, 2);

		crc = pfdc_crc (0, buf, 4);

		switch (ckid) {
		case PFDC_CHUNK_EN:
			if (pfdc_skip_chunk_v2 (fp, size, crc)) {
				return (1);
			}
			return (0);

		case PFDC_CHUNK_SC:
			if (pfdc_load_sector_v2 (fp, img, &last, size, crc)) {
				return (1);
			}
			break;

		case PFDC_CHUNK_TG:
			if (pfdc_load_tags_v2 (fp, last, size, crc)) {
				return (1);
			}
			break;

		case PFDC_CHUNK_CM:
			if (pfdc_load_comment_v2 (fp, img, size, crc)) {
				return (1);
			}
			break;

		default:
			if (pfdc_skip_chunk_v2 (fp, size, crc)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}

static
int pfdc_load_fp (FILE *fp, pfdc_img_t *img)
{
	unsigned long i;
	unsigned      vers;
	unsigned long crc;
	unsigned long cnt, ofs;
	unsigned char buf[16];
	pfdc_sct_t    *last;

	par_file_crc = 0xffffffff;

	if (pfdc_read (fp, buf, 16)) {
		return (1);
	}

	if (pfdc_get_uint32_be (buf, 0) != PFDC_MAGIC_PFDC) {
		return (1);
	}

	vers = pfdc_get_uint16_be (buf, 4);

	if (vers > 2) {
		return (1);
	}

	if (vers < 2) {
		fprintf (stderr,
			"pfdc: warning: loading deprecated version %u file\n",
			vers
		);
	}

	cnt = pfdc_get_uint32_be (buf, 8);
	ofs = pfdc_get_uint32_be (buf, 12);

	if (ofs < 16) {
		return (1);
	}

	if (pfdc_skip (fp, ofs - 16)) {
		return (1);
	}

	if (vers < 2) {
		last = NULL;

		for (i = 0; i < cnt; i++) {
			if (vers == 0) {
				if (pfdc_load_sector_v0 (fp, img)) {
					return (1);
				}
			}
			else if (vers == 1) {
				if (pfdc_load_sector_v1 (fp, img, &last)) {
					return (1);
				}
			}
			else {
				return (1);
			}
		}
	}
	else {
		if (pfdc_load_chunks_v2 (fp, img)) {
			return (1);
		}

		crc = par_file_crc;

		if (pfdc_read (fp, buf, 4)) {
			return (1);
		}

		if (pfdc_get_uint32_be (buf, 0) != crc) {
			return (1);
		}
	}

	return (0);
}

pfdc_img_t *pfdc_load_pfdc (FILE *fp)
{
	pfdc_img_t *img;

	img = pfdc_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (pfdc_load_fp (fp, img)) {
		pfdc_img_del (img);
		return (NULL);
	}

	return (img);
}


static
unsigned pfdc_sct_get_flags_v0 (const pfdc_sct_t *sct)
{
	unsigned f;

	f = 0;

	if (sct->encoding == PFDC_ENC_FM) {
		f |= PFDC_FLAG0_FM;
	}
	else if (sct->encoding == PFDC_ENC_MFM) {
		f |= PFDC_FLAG0_MFM;
	}

	if (sct->flags & PFDC_FLAG_CRC_ID) {
		f |= PFDC_FLAG0_CRC_ID;
	}

	if (sct->flags & PFDC_FLAG_CRC_DATA) {
		f |= PFDC_FLAG0_CRC_DATA;
	}

	if (sct->flags & PFDC_FLAG_DEL_DAM) {
		f |= PFDC_FLAG0_DEL_DAM;
	}

	return (f);
}

static
int pfdc_save_sector_v0 (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h)
{
	int           compr;
	unsigned char buf[16];

	buf[0] = c;
	buf[1] = h;
	buf[2] = sct->c;
	buf[3] = sct->h;
	buf[4] = sct->s;
	buf[5] = pfdc_sct_get_flags_v0 (sct);
	buf[6] = (sct->n >> 8) & 0xff;
	buf[7] = sct->n & 0xff;

	pfdc_set_uint32_be (buf, 8, sct->data_rate);

	compr = pfdc_sct_uniform (sct) && (sct->n > 0);

	if (compr) {
		buf[5] |= PFDC_FLAG0_COMPRESSED;
		buf[12] = sct->data[0];

		if (pfdc_write (fp, buf, 13)) {
			return (1);
		}
	}
	else {
		if (pfdc_write (fp, buf, 12)) {
			return (1);
		}

		if (pfdc_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	return (0);
}

static
unsigned long pfdc_sct_get_flags_v1 (const pfdc_sct_t *sct)
{
	unsigned long f;

	f = 0;

	if (sct->flags & PFDC_FLAG_CRC_ID) {
		f |= PFDC_FLAG1_CRC_ID;
	}

	if (sct->flags & PFDC_FLAG_CRC_DATA) {
		f |= PFDC_FLAG1_CRC_DATA;
	}

	if (sct->flags & PFDC_FLAG_DEL_DAM) {
		f |= PFDC_FLAG1_DEL_DAM;
	}

	return (f);
}

static
int pfdc_save_sector_v1 (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h, int alt)
{
	unsigned long flg;
	unsigned char buf[32];

	flg = pfdc_sct_get_flags_v1 (sct);

	if (pfdc_sct_uniform (sct) && (sct->n > 0)) {
		flg |= PFDC_FLAG1_COMPRESSED;
	}

	if (alt) {
		flg |= PFDC_FLAG1_ALTERNATE;
	}

	pfdc_set_uint32_be (buf, 8, sct->data_rate);

	buf[0] = 'S';
	buf[1] = c;
	buf[2] = h;
	buf[3] = sct->c;
	buf[4] = sct->h;
	buf[5] = sct->s;
	buf[6] = (sct->n >> 8) & 0xff;
	buf[7] = sct->n & 0xff;
	buf[8] = sct->encoding;

	pfdc_set_uint32_be (buf, 12, flg);

	if (flg & PFDC_FLAG1_COMPRESSED) {
		buf[16] = sct->data[0];

		if (pfdc_write (fp, buf, 17)) {
			return (1);
		}
	}
	else {
		if (pfdc_write (fp, buf, 16)) {
			return (1);
		}

		if (pfdc_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	return (0);
}

static
int pfdc_save_alternates_v1 (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h)
{
	if (pfdc_save_sector_v1 (fp, sct, c, h, 0)) {
		return (1);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (pfdc_save_sector_v1 (fp, sct, c, h, 1)) {
			return (1);
		}

		sct = sct->next;
	}

	return (0);
}

static
int pfdc_save_chunk_v2 (FILE *fp, unsigned ckid, unsigned size, const void *data)
{
	unsigned long crc;
	unsigned char buf[4];

	pfdc_set_uint16_be (buf, 0, ckid);
	pfdc_set_uint16_be (buf, 2, size);

	crc = pfdc_crc (0, buf, 4);

	if (pfdc_write (fp, buf, 4)) {
		return (1);
	}

	if (size > 0) {
		crc = pfdc_crc (crc, data, size);

		if (pfdc_write (fp, data, size)) {
			return (1);
		}
	}

	pfdc_set_uint32_be (buf, 0, crc);

	if (pfdc_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
unsigned long pfdc_sct_get_flags_v2 (const pfdc_sct_t *sct)
{
	unsigned long f;

	f = 0;

	if (sct->flags & PFDC_FLAG_CRC_ID) {
		f |= PFDC_FLAG2_CRC_ID;
	}

	if (sct->flags & PFDC_FLAG_CRC_DATA) {
		f |= PFDC_FLAG2_CRC_DATA;
	}

	if (sct->flags & PFDC_FLAG_DEL_DAM) {
		f |= PFDC_FLAG2_DEL_DAM;
	}

	return (f);
}

static
int pfdc_save_sector_v2 (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h, int alt)
{
	unsigned      cnt;
	unsigned char flg;
	unsigned long crc;
	unsigned char buf[256];

	flg = pfdc_sct_get_flags_v2 (sct);

	cnt = 12;

	if (pfdc_sct_uniform (sct) && (sct->n > 0)) {
		flg |= PFDC_FLAG2_COMPRESSED;
		cnt += 1;
	}
	else {
		cnt += sct->n;
	}

	if (alt) {
		flg |= PFDC_FLAG2_ALTERNATE;
	}

	pfdc_set_uint16_be (buf, 0, PFDC_CHUNK_SC);
	pfdc_set_uint16_be (buf, 2, cnt);

	pfdc_set_uint32_be (buf, 12, sct->data_rate);

	buf[4] = flg;
	buf[5] = c;
	buf[6] = h;
	buf[7] = sct->c;
	buf[8] = sct->h;
	buf[9] = sct->s;
	buf[10] = (sct->n >> 8) & 0xff;
	buf[11] = sct->n & 0xff;
	buf[12] = sct->encoding;

	crc = pfdc_crc (0, buf, 16);

	if (pfdc_write (fp, buf, 16)) {
		return (1);
	}

	if (flg & PFDC_FLAG2_COMPRESSED) {
		buf[0] = sct->data[0];

		crc = pfdc_crc (crc, buf, 1);

		if (pfdc_write (fp, buf, 1)) {
			return (1);
		}
	}
	else {
		crc = pfdc_crc (crc, sct->data, sct->n);

		if (pfdc_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	pfdc_set_uint32_be (buf, 0, crc);

	if (pfdc_write (fp, buf, 4)) {
		return (1);
	}

	cnt = pfdc_sct_get_tags (sct, buf, 256);

	if (cnt > 0) {
		if (pfdc_save_chunk_v2 (fp, PFDC_CHUNK_TG, cnt, buf)) {
			return (1);
		}
	}

	return (0);
}

static
int pfdc_save_alternates_v2 (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h)
{
	if (pfdc_save_sector_v2 (fp, sct, c, h, 0)) {
		return (1);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (pfdc_save_sector_v2 (fp, sct, c, h, 1)) {
			return (1);
		}

		sct = sct->next;
	}

	return (0);
}

static
int pfdc_save_comment_v2 (FILE *fp, const pfdc_img_t *img)
{
	unsigned            i, j;
	const unsigned char *src;
	unsigned char       *buf;

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

	if (pfdc_save_chunk_v2 (fp, PFDC_CHUNK_CM, j, buf)) {
		free (buf);
		return (1);
	}

	free (buf);

	return (0);
}

int pfdc_save_pfdc (FILE *fp, const pfdc_img_t *img, unsigned vers)
{
	unsigned         c, h, s;
	unsigned long    scnt;
	unsigned char    buf[16];
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct;

	if (vers > 2) {
		return (1);
	}

	par_file_crc = 0xffffffff;

	scnt = pfdc_img_get_sector_count (img);

	pfdc_set_uint32_be (buf, 0, PFDC_MAGIC_PFDC);
	pfdc_set_uint16_be (buf, 4, vers);
	pfdc_set_uint16_be (buf, 6, 0);
	pfdc_set_uint32_be (buf, 8, (vers < 2) ? scnt : 0);
	pfdc_set_uint32_be (buf, 12, 16);

	if (pfdc_write (fp, buf, 16)) {
		return (1);
	}

	if (vers >= 2) {
		if (pfdc_save_comment_v2 (fp, img)) {
			return (1);
		}
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (vers == 0) {
					if (pfdc_save_sector_v0 (fp, sct, c, h)) {
						return (1);
					}
				}
				else if (vers == 1) {
					if (pfdc_save_alternates_v1 (fp, sct, c, h)) {
						return (1);
					}
				}
				else if (vers == 2) {
					if (pfdc_save_alternates_v2 (fp, sct, c, h)) {
						return (1);
					}
				}
				else {
					return (1);
				}
			}
		}
	}

	if (vers >= 2) {
		if (pfdc_save_chunk_v2 (fp, PFDC_CHUNK_EN, 0, NULL)) {
			return (1);
		}

		pfdc_set_uint32_be (buf, 0, par_file_crc);

		if (pfdc_write (fp, buf, 4)) {
			return (1);
		}
	}

	return (0);
}

int pfdc_probe_pfdc_fp (FILE *fp)
{
	unsigned char buf[16];

	if (fseek (fp, 0, SEEK_SET)) {
		return (0);
	}

	if (pfdc_read (fp, buf, 16)) {
		return (0);
	}

	if (pfdc_get_uint32_be (buf, 0) != PFDC_MAGIC_PFDC) {
		return (0);
	}

	return (1);
}

int pfdc_probe_pfdc (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = pfdc_probe_pfdc_fp (fp);

	fclose (fp);

	return (r);
}
