/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-pfdc4.c                              *
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


#define PFDC4_FLAG_CRC_ID     0x0001
#define PFDC4_FLAG_CRC_DATA   0x0002
#define PFDC4_FLAG_DEL_DAM    0x0004
#define PFDC4_FLAG_NO_DAM     0x0008
#define PFDC4_FLAG_ALTERNATE  0x4000
#define PFDC4_FLAG_COMPRESSED 0x8000

#define PFDC4_CHUNK_PFDC      0x50464443
#define PFDC4_CHUNK_TEXT      0x54455854
#define PFDC4_CHUNK_SECT      0x53454354
#define PFDC4_CHUNK_TAGS      0x54414753
#define PFDC4_CHUNK_DATA      0x44415441
#define PFDC4_CHUNK_END       0x454e4420

#define PFDC4_CRC_POLY        0x1edc6f41


static
unsigned long pfdc4_crc (unsigned long crc, const void *buf, unsigned cnt)
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
					reg = (reg << 1) ^ PFDC4_CRC_POLY;
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
int pfdc4_read (FILE *fp, void *buf, unsigned cnt, unsigned long *crc)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	if (crc != NULL) {
		*crc = pfdc4_crc (*crc, buf, cnt);
	}

	return (0);
}

static
int pfdc4_write (FILE *fp, const void *buf, unsigned cnt, unsigned long *crc)
{
	if (crc != NULL) {
		*crc = pfdc4_crc (*crc, buf, cnt);
	}

	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}


static
void pfdc4_sct_set_flags (psi_sct_t *sct, unsigned flg)
{
	if (flg & PFDC4_FLAG_CRC_ID) {
		sct->flags |= PSI_FLAG_CRC_ID;
	}

	if (flg & PFDC4_FLAG_CRC_DATA) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	if (flg & PFDC4_FLAG_DEL_DAM) {
		sct->flags |= PSI_FLAG_DEL_DAM;
	}

	if (flg & PFDC4_FLAG_NO_DAM) {
		sct->flags |= PSI_FLAG_NO_DAM;
	}
}

static
int pfdc4_skip_chunk (FILE *fp, unsigned long cnt, unsigned long crc)
{
	unsigned      n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfdc4_read (fp, buf, n, &crc)) {
			return (1);
		}

		cnt -= n;
	}

	if (pfdc4_read (fp, buf, 4, NULL)) {
		return (1);
	}

	if (psi_get_uint32_be (buf, 0) != crc) {
		fprintf (stderr, "pfdc: crc error\n");
		return (1);
	}

	return (0);
}

static
int pfdc4_load_header (FILE *fp, unsigned long size, unsigned long crc)
{
	unsigned long vers;
	unsigned char buf[8];

	if (size < 4) {
		return (1);
	}

	if (pfdc4_read (fp, buf, 4, &crc)) {
		return (1);
	}

	vers = psi_get_uint32_be (buf, 0);

	if (vers != 0x00040000) {
		fprintf (stderr, "pfdc: unknown version (%lu)\n", vers);
		return (1);
	}

	if (pfdc4_skip_chunk (fp, size - 4, crc)) {
		return (1);
	}

	return (0);
}

static
int pfdc4_load_sect (FILE *fp, psi_img_t *img, psi_sct_t **last, unsigned long size, unsigned long crc)
{
	unsigned      en, pc, ph, lc, lh, ls, fl, n, ex;
	unsigned char buf[32];
	psi_sct_t     *sct;

	if (size < 18) {
		return (1);
	}

	if (pfdc4_read (fp, buf, 18, &crc)) {
		return (1);
	}

	pc = psi_get_uint16_be (buf, 0);
	ph = psi_get_uint16_be (buf, 2);
	lc = psi_get_uint16_be (buf, 4);
	lh = psi_get_uint16_be (buf, 6);
	ls = psi_get_uint16_be (buf, 8);
	n = psi_get_uint16_be (buf, 10);
	ex = buf[12];
	fl = psi_get_uint16_be (buf, 14);
	en = psi_get_uint16_be (buf, 16);

	sct = psi_sct_new (lc, lh, ls, n);

	if (sct == NULL) {
		return (1);
	}

	pfdc4_sct_set_flags (sct, fl);
	psi_sct_set_encoding (sct, en);

	switch (en & PSI_ENC_MASK) {
	case PSI_ENC_FM:
	case PSI_ENC_MFM:
		psi_sct_set_mfm_size (sct, ex);
		break;

	case PSI_ENC_GCR:
		psi_sct_set_gcr_format (sct, ex);
		break;
	}

	if (fl & PFDC4_FLAG_ALTERNATE) {
		if (*last == NULL) {
			fprintf (stderr, "pfdc: orphaned alternate sector\n");
			psi_sct_del (sct);
			return (1);
		}

		psi_sct_add_alternate (*last, sct);
	}
	else {
		if (psi_img_add_sector (img, sct, pc, ph)) {
			psi_sct_del (sct);
			return (1);
		}
	}

	size -= 18;

	if (fl & PFDC4_FLAG_COMPRESSED) {
		psi_sct_fill (sct, buf[13]);
	}

	*last = sct;

	if (pfdc4_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pfdc4_load_data (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	unsigned cnt;

	if (sct == NULL) {
		return (1);
	}

	cnt = (sct->n < size) ? sct->n : size;

	if (cnt > 0) {
		if (pfdc4_read (fp, sct->data, cnt, &crc)) {
			return (1);
		}
	}

	if (pfdc4_skip_chunk (fp, size - cnt, crc)) {
		return (1);
	}

	return (0);
}

static
int pfdc4_load_tags (FILE *fp, psi_sct_t *last, unsigned long size, unsigned long crc)
{
	unsigned      cnt;
	unsigned char buf[256];

	if (last == NULL) {
		return (1);
	}

	cnt = (size < 256) ? size : 256;

	if (pfdc4_read (fp, buf, cnt, &crc)) {
		return (1);
	}

	psi_sct_set_tags (last, buf, cnt);

	return (pfdc4_skip_chunk (fp, size - cnt, crc));
}

static
int pfdc4_load_text (FILE *fp, psi_img_t *img, unsigned long size, unsigned long crc)
{
	int           r;
	unsigned long i, n;
	unsigned char *buf;

	if (size == 0) {
		return (pfdc4_skip_chunk (fp, size, crc));
	}

	buf = malloc (size);

	if (buf == NULL) {
		return (1);
	}

	if (pfdc4_read (fp, buf, size, &crc)) {
		free (buf);
		return (1);
	}

	i = 0;
	n = size;

	if (buf[0] == 0x0a) {
		i = 1;
		n -= 1;
	}

	if ((n > 0) && (buf[n - 1] == 0x0a)) {
		n -= 1;
	}

	r = psi_img_add_comment (img, buf + i, n);

	free (buf);

	r |= pfdc4_skip_chunk (fp, 0, crc);

	return (r);
}

static
int pfdc4_load_end (FILE *fp, psi_img_t *img, unsigned long size, unsigned long crc)
{
	if (pfdc4_skip_chunk (fp, size, crc)) {
		return (1);
	}

	psi_img_clean_comment (img);

	return (0);
}

int pfdc4_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz)
{
	unsigned long ckid, size, crc;
	psi_sct_t     *last;
	unsigned char buf[8];

	psi_set_uint32_be (buf, 0, id);
	psi_set_uint32_be (buf, 4, sz);

	crc = pfdc4_crc (0, buf, 8);

	if (pfdc4_load_header (fp, sz, crc)) {
		return (1);
	}

	last = NULL;

	while (1) {
		crc = 0;

		if (pfdc4_read (fp, buf, 8, &crc)) {
			return (1);
		}

		ckid = psi_get_uint32_be (buf, 0);
		size = psi_get_uint32_be (buf, 4);

		switch (ckid) {
		case PFDC4_CHUNK_END:
			if (pfdc4_load_end (fp, img, size, crc)) {
				return (1);
			}
			return (0);

		case PFDC4_CHUNK_SECT:
			if (pfdc4_load_sect (fp, img, &last, size, crc)) {
				return (1);
			}
			break;

		case PFDC4_CHUNK_DATA:
			if (pfdc4_load_data (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PFDC4_CHUNK_TAGS:
			if (pfdc4_load_tags (fp, last, size, crc)) {
				return (1);
			}
			break;

		case PFDC4_CHUNK_TEXT:
			if (pfdc4_load_text (fp, img, size, crc)) {
				return (1);
			}
			break;

		default:
			if (pfdc4_skip_chunk (fp, size, crc)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}


static
int pfdc4_save_chunk (FILE *fp, unsigned ckid, unsigned size, const void *data)
{
	unsigned long crc;
	unsigned char buf[8];

	psi_set_uint32_be (buf, 0, ckid);
	psi_set_uint32_be (buf, 4, size);

	crc = 0;

	if (pfdc4_write (fp, buf, 8, &crc)) {
		return (1);
	}

	if (size > 0) {
		if (pfdc4_write (fp, data, size, &crc)) {
			return (1);
		}
	}

	psi_set_uint32_be (buf, 0, crc);

	if (pfdc4_write (fp, buf, 4, NULL)) {
		return (1);
	}

	return (0);
}

static
int pfdc4_save_header (FILE *fp)
{
	unsigned char buf[4];

	psi_set_uint16_be (buf, 0, 4);
	psi_set_uint16_be (buf, 2, 0);

	if (pfdc4_save_chunk (fp, PFDC4_CHUNK_PFDC, 4, buf)) {
		return (1);
	}

	return (0);
}

static
unsigned long pfdc4_sct_get_flags (const psi_sct_t *sct)
{
	unsigned long f;

	f = 0;

	if (sct->flags & PSI_FLAG_CRC_ID) {
		f |= PFDC4_FLAG_CRC_ID;
	}

	if (sct->flags & PSI_FLAG_CRC_DATA) {
		f |= PFDC4_FLAG_CRC_DATA;
	}

	if (sct->flags & PSI_FLAG_DEL_DAM) {
		f |= PFDC4_FLAG_DEL_DAM;
	}

	if (sct->flags & PSI_FLAG_NO_DAM) {
		f |= PFDC4_FLAG_NO_DAM;
	}

	return (f);
}

static
int pfdc4_save_sector (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, int alt)
{
	unsigned      i;
	unsigned      flg, cnt;
	unsigned char buf[256];

	flg = pfdc4_sct_get_flags (sct);

	if (psi_sct_uniform (sct) && (sct->n > 0)) {
		flg |= PFDC4_FLAG_COMPRESSED;
	}

	if (alt) {
		flg |= PFDC4_FLAG_ALTERNATE;
	}

	psi_set_uint16_be (buf, 0, c);
	psi_set_uint16_be (buf, 2, h);
	psi_set_uint16_be (buf, 4, sct->c);
	psi_set_uint16_be (buf, 6, sct->h);
	psi_set_uint16_be (buf, 8, sct->s);
	psi_set_uint16_be (buf, 10, sct->n);
	psi_set_uint16_be (buf, 12, 0);
	psi_set_uint16_be (buf, 14, flg);
	psi_set_uint16_be (buf, 16, sct->encoding);

	if (flg & PFDC4_FLAG_COMPRESSED) {
		buf[13] = sct->data[0];
	}

	switch (sct->encoding & PSI_ENC_MASK) {
	case PSI_ENC_FM:
	case PSI_ENC_MFM:
		buf[12] = psi_sct_get_mfm_size (sct);
		break;

	case PSI_ENC_GCR:
		buf[12] = psi_sct_get_gcr_format (sct);
		break;
	}

	if (pfdc4_save_chunk (fp, PFDC4_CHUNK_SECT, 18, buf)) {
		return (1);
	}

	cnt = psi_sct_get_tags (sct, buf, 256);

	if (cnt > 0) {
		for (i = 0; i < cnt; i++) {
			if (buf[i] != 0) {
				break;
			}
		}

		if (i < cnt) {
			if (pfdc4_save_chunk (fp, PFDC4_CHUNK_TAGS, cnt, buf)) {
				return (1);
			}
		}
	}

	if ((flg & (PFDC4_FLAG_COMPRESSED | PFDC4_FLAG_NO_DAM)) == 0) {
		if (sct->n > 0) {
			if (pfdc4_save_chunk (fp, PFDC4_CHUNK_DATA, sct->n, sct->data)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int pfdc4_save_alternates (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h)
{
	if (pfdc4_save_sector (fp, sct, c, h, 0)) {
		return (1);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (pfdc4_save_sector (fp, sct, c, h, 1)) {
			return (1);
		}

		sct = sct->next;
	}

	return (0);
}

static
int pfdc4_save_text (FILE *fp, const psi_img_t *img)
{
	int           r;
	unsigned long crc;
	unsigned char buf[16];

	if (img->comment_size == 0) {
		return (0);
	}

	psi_set_uint32_be (buf, 0, PFDC4_CHUNK_TEXT);
	psi_set_uint32_be (buf, 4, img->comment_size + 2);

	buf[8] = 0x0a;

	crc = 0;

	r = pfdc4_write (fp, buf, 9, &crc);
	r |= pfdc4_write (fp, img->comment, img->comment_size, &crc);
	r |= pfdc4_write (fp, buf + 8, 1, &crc);
	psi_set_uint32_be (buf, 0, crc);
	r |= pfdc4_write (fp, buf, 4, NULL);

	return (r);
}

int pfdc4_save_fp (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h, s;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	if (pfdc4_save_header (fp)) {
		return (1);
	}

	if (pfdc4_save_text (fp, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (pfdc4_save_alternates (fp, sct, c, h)) {
					return (1);
				}
			}
		}
	}

	if (pfdc4_save_chunk (fp, PFDC4_CHUNK_END, 0, NULL)) {
		return (1);
	}

	fflush (fp);

	return (0);
}
