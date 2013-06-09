/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-pfdc2.c                              *
 * Created:     2012-01-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "psi-io.h"


#define PFDC2_MAGIC           0x50464443

#define PFDC2_FLAG_CRC_ID     0x01
#define PFDC2_FLAG_CRC_DATA   0x02
#define PFDC2_FLAG_DEL_DAM    0x04
#define PFDC2_FLAG_NO_DAM     0x08
#define PFDC2_FLAG_ALTERNATE  0x40
#define PFDC2_FLAG_COMPRESSED 0x80

#define PFDC2_ENC_UNKNOWN     0
#define PFDC2_ENC_FM          1
#define PFDC2_ENC_MFM         2
#define PFDC2_ENC_GCR         3

#define PFDC2_CHUNK_SC        0x5343
#define PFDC2_CHUNK_EN        0x454e
#define PFDC2_CHUNK_CM        0x434d
#define PFDC2_CHUNK_TG        0x5447

#define PFDC2_CRC_POLY        0x04c11db7


static unsigned long par_file_crc;


static
unsigned long pfdc2_crc (unsigned long crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;

	tmp = buf;

	while (cnt > 0) {
		crc ^= (*tmp & 0xff) << 24;

		for (i = 0; i < 8; i++) {
			if (crc & 0x80000000) {
				crc = (crc << 1) ^ PFDC2_CRC_POLY;
			}
			else {
				crc = crc << 1;
			}
		}

		tmp += 1;
		cnt -= 1;
	}

	return (crc & 0xffffffff);
}

static
int pfdc2_read (FILE *fp, void *buf, unsigned cnt)
{
	if (psi_read (fp, buf, cnt)) {
		return (1);
	}

	par_file_crc = pfdc2_crc (par_file_crc, buf, cnt);

	return (0);
}

static
int pfdc2_write (FILE *fp, const void *buf, unsigned cnt)
{
	if (psi_write (fp, buf, cnt)) {
		return (1);
	}

	par_file_crc = pfdc2_crc (par_file_crc, buf, cnt);

	return (0);
}

static
int pfdc2_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfdc2_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}

static
void pfdc2_sct_set_encoding (psi_sct_t *sct, unsigned enc, unsigned long dr)
{
	switch (enc) {
	case PFDC2_ENC_FM:
		enc = (dr < 375000) ? PSI_ENC_FM_DD : PSI_ENC_FM_HD;
		break;

	case PFDC2_ENC_MFM:
		enc = (dr < 375000) ? PSI_ENC_MFM_DD : PSI_ENC_MFM_HD;
		break;

	case PFDC2_ENC_GCR:
		enc = PSI_ENC_GCR;
		break;

	default:
		enc = 0;
		break;
	}

	psi_sct_set_encoding (sct, enc);
}

static
void pfdc2_sct_set_flags (psi_sct_t *sct, unsigned long flg)
{
	if (flg & PFDC2_FLAG_CRC_ID) {
		sct->flags |= PSI_FLAG_CRC_ID;
	}

	if (flg & PFDC2_FLAG_CRC_DATA) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	if (flg & PFDC2_FLAG_DEL_DAM) {
		sct->flags |= PSI_FLAG_DEL_DAM;
	}

	if (flg & PFDC2_FLAG_NO_DAM) {
		sct->flags |= PSI_FLAG_NO_DAM;
	}
}

static
int pfdc2_skip_chunk (FILE *fp, unsigned cnt, unsigned long crc)
{
	unsigned      n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfdc2_read (fp, buf, n)) {
			return (1);
		}

		crc = pfdc2_crc (crc, buf, n);

		cnt -= n;
	}

	if (pfdc2_read (fp, buf, 4)) {
		return (1);
	}

	if (psi_get_uint32_be (buf, 0) != crc) {
		return (1);
	}

	return (0);
}

static
int pfdc2_load_sector (FILE *fp, psi_img_t *img, psi_sct_t **last, unsigned size, unsigned long crc)
{
	unsigned      c, h, n;
	unsigned long f, dr;
	unsigned char buf[32];
	psi_sct_t     *sct;

	if (size < 12) {
		return (1);
	}

	if (pfdc2_read (fp, buf, 12)) {
		return (1);
	}

	f = buf[0];
	c = buf[1];
	h = buf[2];
	n = psi_get_uint16_be (buf, 6);

	sct = psi_sct_new (buf[3], buf[4], buf[5], n);

	if (sct == NULL) {
		return (1);
	}

	dr = psi_get_uint32_be (buf, 8) & 0x00ffffff;

	pfdc2_sct_set_flags (sct, f);
	pfdc2_sct_set_encoding (sct, buf[8], dr);

	if (f & PFDC2_FLAG_ALTERNATE) {
		if (*last == NULL) {
			psi_sct_del (sct);
			return (1);
		}

		psi_sct_add_alternate (*last, sct);
	}
	else {
		if (psi_img_add_sector (img, sct, c, h)) {
			psi_sct_del (sct);
			return (1);
		}
	}

	size -= 12;

	crc = pfdc2_crc (crc, buf, 12);

	if (f & PFDC2_FLAG_COMPRESSED) {
		if (size < 1) {
			return (1);
		}

		if (pfdc2_read (fp, buf, 1)) {
			return (1);
		}

		crc = pfdc2_crc (crc, buf, 1);

		psi_sct_fill (sct, buf[0]);

		size -= 1;
	}
	else {
		if (size < sct->n) {
			return (1);
		}

		if (pfdc2_read (fp, sct->data, n)) {
			return (1);
		}

		crc = pfdc2_crc (crc, sct->data, n);

		size -= sct->n;
	}

	*last = sct;

	if (pfdc2_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pfdc2_load_tags (FILE *fp, psi_sct_t *last, unsigned size, unsigned long crc)
{
	unsigned      cnt;
	unsigned char buf[256];

	if (last == NULL) {
		return (1);
	}

	cnt = (size < 256) ? size : 256;

	if (pfdc2_read (fp, buf, cnt)) {
		return (1);
	}

	crc = pfdc2_crc (crc, buf, cnt);

	psi_sct_set_tags (last, buf, cnt);

	return (pfdc2_skip_chunk (fp, size - cnt, crc));
}

static
int pfdc2_load_comment (FILE *fp, psi_img_t *img, unsigned size, unsigned long crc)
{
	int           r;
	unsigned      i, n;
	unsigned char *buf;

	if (size == 0) {
		return (pfdc2_skip_chunk (fp, size, crc));
	}

	buf = malloc (size);

	if (buf == NULL) {
		return (1);
	}

	if (pfdc2_read (fp, buf, size)) {
		free (buf);
		return (1);
	}

	crc = pfdc2_crc (crc, buf, size);

	i = 0;
	n = size;

	if (buf[0] == 0x0a) {
		i += 1;
		n -= 1;
	}

	if ((n > 0) && (buf[n - 1] == 0x0a)) {
		n -= 1;
	}

	r = psi_img_add_comment (img, buf + i, n);

	free (buf);

	r |= pfdc2_skip_chunk (fp, 0, crc);

	return (r);
}

static
int pfdc2_load_end (FILE *fp, psi_img_t *img, unsigned size, unsigned long crc)
{
	if (pfdc2_skip_chunk (fp, size, crc)) {
		return (1);
	}

	psi_img_clean_comment (img);

	return (0);
}

static
int pfdc2_load_chunks (FILE *fp, psi_img_t *img)
{
	unsigned      ckid, size;
	unsigned long crc;
	psi_sct_t     *last;
	unsigned char buf[4];

	last = NULL;

	while (1) {
		if (pfdc2_read (fp, buf, 4)) {
			return (1);
		}

		ckid = psi_get_uint16_be (buf, 0);
		size = psi_get_uint16_be (buf, 2);

		crc = pfdc2_crc (0, buf, 4);

		switch (ckid) {
		case PFDC2_CHUNK_EN:
			if (pfdc2_load_end (fp, img, size, crc)) {
				return (1);
			}
			return (0);

		case PFDC2_CHUNK_SC:
			if (pfdc2_load_sector (fp, img, &last, size, crc)) {
				return (1);
			}
			break;

		case PFDC2_CHUNK_TG:
			if (pfdc2_load_tags (fp, last, size, crc)) {
				return (1);
			}
			break;

		case PFDC2_CHUNK_CM:
			if (pfdc2_load_comment (fp, img, size, crc)) {
				return (1);
			}
			break;

		default:
			if (pfdc2_skip_chunk (fp, size, crc)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}

int pfdc2_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz)
{
	unsigned long crc;
	unsigned long ofs;
	unsigned char buf[16];

	fprintf (stderr, "pfdc: warning: loading deprecated version 2 file\n");

	psi_set_uint32_be (buf, 0, id);
	psi_set_uint32_be (buf, 4, sz);

	par_file_crc = pfdc2_crc (0xffffffff, buf, 8);

	if (pfdc2_read (fp, buf + 8, 8)) {
		return (1);
	}

	ofs = psi_get_uint32_be (buf, 12);

	if (ofs < 16) {
		return (1);
	}

	if (pfdc2_skip (fp, ofs - 16)) {
		return (1);
	}

	if (pfdc2_load_chunks (fp, img)) {
		return (1);
	}

	crc = par_file_crc;

	if (pfdc2_read (fp, buf, 4)) {
		return (1);
	}

	if (psi_get_uint32_be (buf, 0) != crc) {
		return (1);
	}

	return (0);
}


static
int pfdc2_save_chunk (FILE *fp, unsigned ckid, unsigned size, const void *data)
{
	unsigned long crc;
	unsigned char buf[4];

	psi_set_uint16_be (buf, 0, ckid);
	psi_set_uint16_be (buf, 2, size);

	crc = pfdc2_crc (0, buf, 4);

	if (pfdc2_write (fp, buf, 4)) {
		return (1);
	}

	if (size > 0) {
		crc = pfdc2_crc (crc, data, size);

		if (pfdc2_write (fp, data, size)) {
			return (1);
		}
	}

	psi_set_uint32_be (buf, 0, crc);

	if (pfdc2_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
void pfdc2_sct_get_encoding (const psi_sct_t *sct, unsigned *enc, unsigned long *dr)
{
	switch (sct->encoding) {
	case PSI_ENC_FM_DD:
		*enc = PFDC2_ENC_FM;
		*dr = 125000;
		break;

	case PSI_ENC_FM_HD:
		*enc = PFDC2_ENC_FM;
		*dr = 250000;
		break;

	case PSI_ENC_MFM_DD:
		*enc = PFDC2_ENC_MFM;
		*dr = 250000;
		break;

	case PSI_ENC_MFM_HD:
		*enc = PFDC2_ENC_MFM;
		*dr = 500000;
		break;

	case PSI_ENC_MFM_ED:
		*enc = PFDC2_ENC_MFM;
		*dr = 1000000;
		break;

	case PSI_ENC_GCR:
		*enc = PFDC2_ENC_GCR;
		*dr = 500000;
		break;

	default:
		*enc = 0;
		*dr = 0;
		break;
	}
}

static
unsigned long pfdc2_sct_get_flags (const psi_sct_t *sct)
{
	unsigned long f;

	f = 0;

	if (sct->flags & PSI_FLAG_CRC_ID) {
		f |= PFDC2_FLAG_CRC_ID;
	}

	if (sct->flags & PSI_FLAG_CRC_DATA) {
		f |= PFDC2_FLAG_CRC_DATA;
	}

	if (sct->flags & PSI_FLAG_DEL_DAM) {
		f |= PFDC2_FLAG_DEL_DAM;
	}

	if (sct->flags & PSI_FLAG_NO_DAM) {
		f |= PFDC2_FLAG_NO_DAM;
	}

	return (f);
}

static
int pfdc2_save_sector (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, int alt)
{
	unsigned      cnt;
	unsigned char flg;
	unsigned      enc;
	unsigned long dr;
	unsigned long crc;
	unsigned char buf[256];

	flg = pfdc2_sct_get_flags (sct);

	cnt = 12;

	if (psi_sct_uniform (sct) && (sct->n > 0)) {
		flg |= PFDC2_FLAG_COMPRESSED;
		cnt += 1;
	}
	else {
		cnt += sct->n;
	}

	if (alt) {
		flg |= PFDC2_FLAG_ALTERNATE;
	}

	psi_set_uint16_be (buf, 0, PFDC2_CHUNK_SC);
	psi_set_uint16_be (buf, 2, cnt);

	pfdc2_sct_get_encoding (sct, &enc, &dr);

	psi_set_uint32_be (buf, 12, dr);

	buf[4] = flg;
	buf[5] = c;
	buf[6] = h;
	buf[7] = sct->c;
	buf[8] = sct->h;
	buf[9] = sct->s;
	buf[10] = (sct->n >> 8) & 0xff;
	buf[11] = sct->n & 0xff;
	buf[12] = enc;

	crc = pfdc2_crc (0, buf, 16);

	if (pfdc2_write (fp, buf, 16)) {
		return (1);
	}

	if (flg & PFDC2_FLAG_COMPRESSED) {
		buf[0] = sct->data[0];

		crc = pfdc2_crc (crc, buf, 1);

		if (pfdc2_write (fp, buf, 1)) {
			return (1);
		}
	}
	else {
		crc = pfdc2_crc (crc, sct->data, sct->n);

		if (pfdc2_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	psi_set_uint32_be (buf, 0, crc);

	if (pfdc2_write (fp, buf, 4)) {
		return (1);
	}

	cnt = psi_sct_get_tags (sct, buf, 256);

	if (cnt > 0) {
		if (pfdc2_save_chunk (fp, PFDC2_CHUNK_TG, cnt, buf)) {
			return (1);
		}
	}

	return (0);
}

static
int pfdc2_save_alternates (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h)
{
	if (pfdc2_save_sector (fp, sct, c, h, 0)) {
		return (1);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (pfdc2_save_sector (fp, sct, c, h, 1)) {
			return (1);
		}

		sct = sct->next;
	}

	return (0);
}

static
int pfdc2_save_comment (FILE *fp, const psi_img_t *img)
{
	int           r;
	unsigned long crc;
	unsigned char buf[16];

	if (img->comment_size == 0) {
		return (0);
	}

	psi_set_uint16_be (buf, 0, PFDC2_CHUNK_CM);
	psi_set_uint16_be (buf, 2, img->comment_size + 2);

	buf[4] = 0x0a;

	crc = pfdc2_crc (0, buf, 5);
	crc = pfdc2_crc (crc, img->comment, img->comment_size);
	crc = pfdc2_crc (crc, buf + 4, 1);

	r = pfdc2_write (fp, buf, 5);
	r |= pfdc2_write (fp, img->comment, img->comment_size);
	r |= pfdc2_write (fp, buf + 4, 1);
	psi_set_uint32_be (buf, 0, crc);
	r |= pfdc2_write (fp, buf, 4);

	return (r);
}

int pfdc2_save_fp (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h, s;
	unsigned char   buf[16];
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	par_file_crc = 0xffffffff;

	psi_set_uint32_be (buf, 0, PFDC2_MAGIC);
	psi_set_uint16_be (buf, 4, 2);
	psi_set_uint16_be (buf, 6, 0);
	psi_set_uint32_be (buf, 8, 0);
	psi_set_uint32_be (buf, 12, 16);

	if (pfdc2_write (fp, buf, 16)) {
		return (1);
	}

	if (pfdc2_save_comment (fp, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (pfdc2_save_alternates (fp, sct, c, h)) {
					return (1);
				}
			}
		}
	}

	if (pfdc2_save_chunk (fp, PFDC2_CHUNK_EN, 0, NULL)) {
		return (1);
	}

	psi_set_uint32_be (buf, 0, par_file_crc);

	if (pfdc2_write (fp, buf, 4)) {
		return (1);
	}

	fflush (fp);

	return (0);
}
