/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-psi.c                                *
 * Created:     2013-05-29 by Hampa Hug <hampa@hampa.ch>                     *
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

#include "psi.h"
#include "psi-io.h"


#define PSI_CHUNK_PSI  0x50534920
#define PSI_CHUNK_TEXT 0x54455854
#define PSI_CHUNK_SECT 0x53454354
#define PSI_CHUNK_IBMF 0x49424d46
#define PSI_CHUNK_IBMM 0x49424d4d
#define PSI_CHUNK_MACG 0x4d414347
#define PSI_CHUNK_OFFS 0x4f464653
#define PSI_CHUNK_TIME 0x54494d45
#define PSI_CHUNK_DATA 0x44415441
#define PSI_CHUNK_WEAK 0x5745414b
#define PSI_CHUNK_END  0x454e4420

#define PSI_FORMAT_UNK     0x0000
#define PSI_FORMAT_IBMF    0x0100
#define PSI_FORMAT_IBMF_DD 0x0100
#define PSI_FORMAT_IBMF_HD 0x0101
#define PSI_FORMAT_IBMM_DD 0x0200
#define PSI_FORMAT_IBMM_HD 0x0201
#define PSI_FORMAT_IBMM_ED 0x0202
#define PSI_FORMAT_MACG    0x0300

#define PSI_CRC_POLY 0x1edc6f41

#define PSI_FLAG_COMP 0x01
#define PSI_FLAG_ALT  0x02
#define PSI_FLAG_CRC  0x04

#define PSI_IBMF_CRC_ID   1
#define PSI_IBMF_CRC_DATA 2
#define PSI_IBMF_DEL_DAM  4
#define PSI_IBMF_NO_DAM   8

#define PSI_IBMM_CRC_ID   1
#define PSI_IBMM_CRC_DATA 2
#define PSI_IBMM_DEL_DAM  4
#define PSI_IBMM_NO_DAM   8

#define PSI_MACG_CRC_ID   1
#define PSI_MACG_CRC_DATA 2
#define PSI_MACG_NO_DAM   4


static
unsigned long psi_crc (unsigned long crc, const void *buf, unsigned cnt)
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
					reg = (reg << 1) ^ PSI_CRC_POLY;
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
int psi_read_crc (FILE *fp, void *buf, unsigned cnt, unsigned long *crc)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	if (crc != NULL) {
		*crc = psi_crc (*crc, buf, cnt);
	}

	return (0);
}

static
int psi_write_crc (FILE *fp, const void *buf, unsigned cnt, unsigned long *crc)
{
	if (crc != NULL) {
		*crc = psi_crc (*crc, buf, cnt);
	}

	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}


static
int psi_skip_chunk (FILE *fp, unsigned long cnt, unsigned long crc)
{
	unsigned      n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (psi_read_crc (fp, buf, n, &crc)) {
			return (1);
		}

		cnt -= n;
	}

	if (psi_read_crc (fp, buf, 4, NULL)) {
		return (1);
	}

	if (psi_get_uint32_be (buf, 0) != crc) {
		fprintf (stderr, "psi: crc error\n");
		return (1);
	}

	return (0);
}

static
int psi_load_header (FILE *fp, unsigned long size, unsigned long crc, unsigned *enc)
{
	unsigned long vers;
	unsigned char buf[8];

	if (size < 4) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 4, &crc)) {
		return (1);
	}

	vers = psi_get_uint16_be (buf, 0);

	if (vers != 0x0000) {
		fprintf (stderr, "psi: unknown version (%lu)\n", vers);
		return (1);
	}

	switch (psi_get_uint16_be (buf, 2)) {
	case PSI_FORMAT_IBMF_DD:
		*enc = PSI_ENC_FM_DD;
		break;

	case PSI_FORMAT_IBMF_HD:
		*enc = PSI_ENC_FM_HD;
		break;

	case PSI_FORMAT_IBMM_DD:
		*enc = PSI_ENC_MFM_DD;
		break;

	case PSI_FORMAT_IBMM_HD:
		*enc = PSI_ENC_MFM_HD;
		break;

	case PSI_FORMAT_IBMM_ED:
		*enc = PSI_ENC_MFM_ED;
		break;

	case PSI_FORMAT_MACG:
		*enc = PSI_ENC_GCR;
		break;

	default:
		*enc = PSI_ENC_UNKNOWN;
		break;
	}

	if (psi_skip_chunk (fp, size - 4, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_fm (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	unsigned char buf[8];

	if ((sct == NULL) || (size < 6)) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 6, &crc)) {
		return (1);
	}

	sct->c = buf[0];
	sct->h = buf[1];
	sct->s = buf[2];
	psi_sct_set_mfm_size (sct, buf[3]);

	if (buf[4] & PSI_IBMF_CRC_ID) {
		sct->flags |= PSI_FLAG_CRC_ID;
	}

	if (buf[4] & PSI_IBMF_CRC_DATA) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	if (buf[4] & PSI_IBMF_DEL_DAM) {
		sct->flags |= PSI_FLAG_DEL_DAM;
	}

	if (buf[4] & PSI_IBMF_NO_DAM) {
		sct->flags |= PSI_FLAG_NO_DAM;
	}

	switch (buf[5]) {
	case 1:
		psi_sct_set_encoding (sct, PSI_ENC_FM_HD);
		break;

	default:
		psi_sct_set_encoding (sct, PSI_ENC_FM_DD);
		break;
	}

	if (psi_skip_chunk (fp, size - 6, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_mfm (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	unsigned char buf[8];

	if ((sct == NULL) || (size < 6)) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 6, &crc)) {
		return (1);
	}

	sct->c = buf[0];
	sct->h = buf[1];
	sct->s = buf[2];
	psi_sct_set_mfm_size (sct, buf[3]);

	if (buf[4] & PSI_IBMM_CRC_ID) {
		sct->flags |= PSI_FLAG_CRC_ID;
	}

	if (buf[4] & PSI_IBMM_CRC_DATA) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	if (buf[4] & PSI_IBMM_DEL_DAM) {
		sct->flags |= PSI_FLAG_DEL_DAM;
	}

	if (buf[4] & PSI_IBMM_NO_DAM) {
		sct->flags |= PSI_FLAG_NO_DAM;
	}

	switch (buf[5]) {
	case 1:
		psi_sct_set_encoding (sct, PSI_ENC_MFM_HD);
		break;

	case 2:
		psi_sct_set_encoding (sct, PSI_ENC_MFM_ED);
		break;

	default:
		psi_sct_set_encoding (sct, PSI_ENC_MFM_DD);
		break;
	}

	if (psi_skip_chunk (fp, size - 6, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_macg (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	unsigned char buf[18];

	if (sct == NULL) {
		return (1);
	}

	if (size < 18) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 18, &crc)) {
		return (1);
	}

	sct->c = psi_get_uint16_be (buf, 0);
	sct->h = buf[2];
	sct->s = buf[3];
	psi_sct_set_gcr_format (sct, buf[4]);

	if (buf[5] & PSI_MACG_CRC_ID) {
		sct->flags |= PSI_FLAG_CRC_ID;
	}

	if (buf[5] & PSI_MACG_CRC_DATA) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	if (buf[5] & PSI_MACG_NO_DAM) {
		sct->flags |= PSI_FLAG_NO_DAM;
	}

	psi_sct_set_encoding (sct, PSI_ENC_GCR);

	psi_sct_set_tags (sct, buf + 6, 12);

	if (psi_skip_chunk (fp, size - 18, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_offs (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	unsigned char buf[4];

	if ((sct == NULL) || (size < 4)) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 4, &crc)) {
		return (1);
	}

	psi_sct_set_position (sct, psi_get_uint32_be (buf, 0));

	if (psi_skip_chunk (fp, size - 4, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_time (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	unsigned char buf[4];

	if ((sct == NULL) || (size < 4)) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 4, &crc)) {
		return (1);
	}

	psi_sct_set_read_time (sct, psi_get_uint32_be (buf, 0));

	if (psi_skip_chunk (fp, size - 4, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_sect (FILE *fp, psi_img_t *img, psi_sct_t **last, unsigned long size, unsigned long crc, unsigned enc)
{
	unsigned      c, h, s, f, n;
	unsigned char buf[8];
	psi_sct_t     *sct;

	if (size < 8) {
		return (1);
	}

	if (psi_read_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	c = psi_get_uint16_be (buf, 0);
	h = buf[2];
	s = buf[3];
	n = psi_get_uint16_be (buf, 4);
	f = buf[6];

	if ((sct = psi_sct_new (c, h, s, n)) == NULL) {
		return (1);
	}

	if (f & PSI_FLAG_ALT) {
		if (*last == NULL) {
			fprintf (stderr, "psi: orphaned alternate sector\n");
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

	if (f & PSI_FLAG_CRC) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	psi_sct_set_encoding (sct, enc);

	size -= 8;

	if (f & PSI_FLAG_COMP) {
		psi_sct_fill (sct, buf[7]);
	}

	*last = sct;

	if (psi_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_data (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	if (sct == NULL) {
		return (1);
	}

	if (psi_sct_set_size (sct, size, 0)) {
		return (1);
	}

	if (size > 0) {
		if (psi_read_crc (fp, sct->data, size, &crc)) {
			return (1);
		}
	}

	if (psi_skip_chunk (fp, 0, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_weak (FILE *fp, psi_img_t *img, psi_sct_t *sct, unsigned long size, unsigned long crc)
{
	if (sct == NULL) {
		return (1);
	}

	if (psi_sct_set_size (sct, size, 0)) {
		return (1);
	}

	if (size > 0) {
		if (psi_weak_alloc (sct)) {
			return (1);
		}

		if (psi_read_crc (fp, sct->weak, size, &crc)) {
			return (1);
		}

		psi_weak_clean (sct);
	}

	if (psi_skip_chunk (fp, 0, crc)) {
		return (1);
	}

	return (0);
}

static
int psi_load_text (FILE *fp, psi_img_t *img, unsigned long size, unsigned long crc)
{
	int           r;
	unsigned long i, n;
	unsigned char *buf;

	if (size == 0) {
		return (psi_skip_chunk (fp, size, crc));
	}

	if ((buf = malloc (size)) == NULL) {
		return (1);
	}

	if (psi_read_crc (fp, buf, size, &crc)) {
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

	r |= psi_skip_chunk (fp, 0, crc);

	return (r);
}

static
int psi_load_end (FILE *fp, psi_img_t *img, unsigned long size, unsigned long crc)
{
	if (psi_skip_chunk (fp, size, crc)) {
		return (1);
	}

	psi_img_clean_comment (img);

	return (0);
}

static
int psi_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned long ckid, size, crc;
	unsigned      enc;
	psi_sct_t     *last;
	unsigned char buf[8];

	crc = 0;

	if (psi_read_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	ckid = psi_get_uint32_be (buf, 0);
	size = psi_get_uint32_be (buf, 4);

	if (ckid != PSI_CHUNK_PSI) {
		return (1);
	}

	if (psi_load_header (fp, size, crc, &enc)) {
		return (1);
	}

	last = NULL;

	while (1) {
		crc = 0;

		if (psi_read_crc (fp, buf, 8, &crc)) {
			return (1);
		}

		ckid = psi_get_uint32_be (buf, 0);
		size = psi_get_uint32_be (buf, 4);

		switch (ckid) {
		case PSI_CHUNK_END:
			if (psi_load_end (fp, img, size, crc)) {
				return (1);
			}
			return (0);

		case PSI_CHUNK_SECT:
			if (psi_load_sect (fp, img, &last, size, crc, enc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_IBMF:
			if (psi_load_fm (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_IBMM:
			if (psi_load_mfm (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_MACG:
			if (psi_load_macg (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_OFFS:
			if (psi_load_offs (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_TIME:
			if (psi_load_time (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_DATA:
			if (psi_load_data (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_WEAK:
			if (psi_load_weak (fp, img, last, size, crc)) {
				return (1);
			}
			break;

		case PSI_CHUNK_TEXT:
			if (psi_load_text (fp, img, size, crc)) {
				return (1);
			}
			break;

		default:
			if (psi_skip_chunk (fp, size, crc)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}

psi_img_t *psi_load_psi (FILE *fp)
{
	psi_img_t *img;

	if ((img = psi_img_new()) == NULL) {
		return (NULL);
	}

	if (psi_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int psi_save_chunk (FILE *fp, unsigned ckid, unsigned size, const void *data)
{
	unsigned long crc;
	unsigned char buf[8];

	psi_set_uint32_be (buf, 0, ckid);
	psi_set_uint32_be (buf, 4, size);

	crc = 0;

	if (psi_write_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	if (size > 0) {
		if (psi_write_crc (fp, data, size, &crc)) {
			return (1);
		}
	}

	psi_set_uint32_be (buf, 0, crc);

	if (psi_write_crc (fp, buf, 4, NULL)) {
		return (1);
	}

	return (0);
}

static
unsigned psi_get_default_encoding (const psi_img_t *img)
{
	unsigned        c, h, s;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				return (sct->encoding);
			}
		}
	}

	return (PSI_ENC_UNKNOWN);
}

static
int psi_save_header (FILE *fp, const psi_img_t *img, unsigned *enc)
{
	unsigned      val;
	unsigned char buf[4];

	psi_set_uint16_be (buf, 0, 0);

	*enc = psi_get_default_encoding (img);

	switch (*enc) {
	case PSI_ENC_FM_DD:
		val = PSI_FORMAT_IBMF_DD;
		break;

	case PSI_ENC_FM_HD:
		val = PSI_FORMAT_IBMF_HD;
		break;

	case PSI_ENC_MFM_DD:
		val = PSI_FORMAT_IBMM_DD;
		break;

	case PSI_ENC_MFM_HD:
		val = PSI_FORMAT_IBMM_HD;
		break;

	case PSI_ENC_MFM_ED:
		val = PSI_FORMAT_IBMM_ED;
		break;

	case PSI_ENC_GCR:
		val = PSI_FORMAT_MACG;
		break;

	default:
		val = PSI_FORMAT_UNK;
		break;
	}

	psi_set_uint16_be (buf, 2, val);

	if (psi_save_chunk (fp, PSI_CHUNK_PSI, 4, buf)) {
		return (1);
	}

	return (0);
}

static
int psi_save_fm (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, unsigned enc)
{
	unsigned      n;
	int           req;
	unsigned char buf[8];

	if ((sct->encoding & PSI_ENC_MASK) != PSI_ENC_FM) {
		return (0);
	}

	n = psi_sct_get_mfm_size (sct);

	req = (sct->c != c) || (sct->h != h);
	req |= ((n > 8) || (sct->n != (128U << n)));
	req |= (sct->flags & (PSI_FLAG_CRC_ID | PSI_FLAG_DEL_DAM | PSI_FLAG_NO_DAM)) != 0;
	req |= (sct->encoding != enc);

	if (req == 0) {
		return (0);
	}

	buf[0] = sct->c & 0xff;
	buf[1] = sct->h & 0xff;
	buf[2] = sct->s & 0xff;
	buf[3] = n;
	buf[4] = 0;

	buf[4] |= (sct->flags & PSI_FLAG_CRC_ID) ? PSI_IBMF_CRC_ID : 0;
	buf[4] |= (sct->flags & PSI_FLAG_CRC_DATA) ? PSI_IBMF_CRC_DATA : 0;
	buf[4] |= (sct->flags & PSI_FLAG_DEL_DAM) ? PSI_IBMF_DEL_DAM : 0;
	buf[4] |= (sct->flags & PSI_FLAG_NO_DAM) ? PSI_IBMF_NO_DAM : 0;

	switch (sct->encoding) {
	case PSI_ENC_FM_DD:
		buf[5] = 0;
		break;

	case PSI_ENC_FM_HD:
		buf[5] = 1;
		break;

	default:
		buf[5] = 0;
		break;
	}

	if (psi_save_chunk (fp, PSI_CHUNK_IBMF, 6, buf)) {
		return (1);
	}

	return (0);
}

static
int psi_save_mfm (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, unsigned enc)
{
	unsigned      n;
	int           req;
	unsigned char buf[8];

	if ((sct->encoding & PSI_ENC_MASK) != PSI_ENC_MFM) {
		return (0);
	}

	n = psi_sct_get_mfm_size (sct);

	req = (sct->c != c) || (sct->h != h);
	req |= ((n > 8) || (sct->n != (128U << n)));
	req |= (sct->flags & (PSI_FLAG_CRC_ID | PSI_FLAG_DEL_DAM | PSI_FLAG_NO_DAM)) != 0;
	req |= (sct->encoding != enc);

	if (req == 0) {
		return (0);
	}

	buf[0] = sct->c & 0xff;
	buf[1] = sct->h & 0xff;
	buf[2] = sct->s & 0xff;
	buf[3] = n;
	buf[4] = 0;

	buf[4] |= (sct->flags & PSI_FLAG_CRC_ID) ? PSI_IBMM_CRC_ID : 0;
	buf[4] |= (sct->flags & PSI_FLAG_CRC_DATA) ? PSI_IBMM_CRC_DATA : 0;
	buf[4] |= (sct->flags & PSI_FLAG_DEL_DAM) ? PSI_IBMM_DEL_DAM : 0;
	buf[4] |= (sct->flags & PSI_FLAG_NO_DAM) ? PSI_IBMM_NO_DAM : 0;

	switch (sct->encoding) {
	case PSI_ENC_MFM_DD:
		buf[5] = 0;
		break;

	case PSI_ENC_MFM_HD:
		buf[5] = 1;
		break;

	case PSI_ENC_MFM_ED:
		buf[5] = 2;
		break;

	default:
		buf[5] = 0;
		break;
	}

	if (psi_save_chunk (fp, PSI_CHUNK_IBMM, 6, buf)) {
		return (1);
	}

	return (0);
}

static
int psi_save_macg (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, unsigned enc)
{
	unsigned      i;
	int           req;
	unsigned char buf[18];

	if ((sct->encoding & PSI_ENC_MASK) != PSI_ENC_GCR) {
		return (0);
	}

	req = (sct->c != c) || (sct->h != h);
	req |= ((sct->flags & (PSI_FLAG_CRC_ID | PSI_FLAG_NO_DAM)) != 0);
	req |= (sct->encoding != enc);

	if (psi_sct_get_tags (sct, buf + 6, 12) > 0) {
		for (i = 0; i < 12; i++) {
			req |= (buf[6 + i] != 0);
		}
	}

	if (req == 0) {
		return (0);
	}

	psi_set_uint16_be (buf, 0, sct->c);
	buf[2] = sct->h & 0xff;
	buf[3] = sct->s & 0xff;
	buf[4] = psi_sct_get_gcr_format (sct);
	buf[5] = 0;

	buf[5] |= (sct->flags & PSI_FLAG_CRC_ID) ? PSI_MACG_CRC_ID : 0;
	buf[5] |= (sct->flags & PSI_FLAG_CRC_DATA) ? PSI_MACG_CRC_DATA : 0;
	buf[5] |= (sct->flags & PSI_FLAG_NO_DAM) ? PSI_MACG_NO_DAM : 0;

	if (psi_save_chunk (fp, PSI_CHUNK_MACG, 18, buf)) {
		return (1);
	}

	return (0);
}

static
int psi_save_offs (FILE *fp, const psi_sct_t *sct)
{
	unsigned long pos;
	unsigned char buf[4];

	pos = psi_sct_get_position (sct);

	if (pos == 0xffffffff) {
		return (0);
	}

	psi_set_uint32_be (buf, 0, pos);

	if (psi_save_chunk (fp, PSI_CHUNK_OFFS, 4, buf)) {
		return (1);
	}

	return (0);
}

static
int psi_save_time (FILE *fp, const psi_sct_t *sct)
{
	unsigned long val;
	unsigned char buf[4];

	val = psi_sct_get_read_time (sct);

	if ((val == 0) || (val == (8UL * sct->n))) {
		return (0);
	}

	psi_set_uint32_be (buf, 0, val);

	if (psi_save_chunk (fp, PSI_CHUNK_TIME, 4, buf)) {
		return (1);
	}

	return (0);
}

static
int psi_save_sector (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, int alt, unsigned enc)
{
	unsigned      f;
	unsigned char buf[8];

	f = alt ? PSI_FLAG_ALT : 0;

	if (psi_sct_uniform (sct) && (sct->n > 0)) {
		f |= PSI_FLAG_COMP;
	}

	if (sct->flags & PSI_FLAG_CRC_DATA) {
		f |= PSI_FLAG_CRC;
	}

	psi_set_uint16_be (buf, 0, c);
	buf[2] = h & 0xff;
	buf[3] = sct->s & 0xff;
	psi_set_uint16_be (buf, 4, sct->n);
	buf[6] = f;
	buf[7] = 0;

	if (f & PSI_FLAG_COMP) {
		buf[7] = sct->data[0];
	}

	if (psi_save_chunk (fp, PSI_CHUNK_SECT, 8, buf)) {
		return (1);
	}

	if (psi_save_fm (fp, sct, c, h, enc)) {
		return (1);
	}

	if (psi_save_mfm (fp, sct, c, h, enc)) {
		return (1);
	}

	if (psi_save_macg (fp, sct, c, h, enc)) {
		return (1);
	}

	if (psi_save_offs (fp, sct)) {
		return (1);
	}

	if (psi_save_time (fp, sct)) {
		return (1);
	}

	if ((f & PSI_FLAG_COMP) == 0) {
		if (sct->n > 0) {
			if (psi_save_chunk (fp, PSI_CHUNK_DATA, sct->n, sct->data)) {
				return (1);
			}
		}
	}

	if (sct->n > 0) {
		if (psi_weak_check (sct)) {
			if (psi_save_chunk (fp, PSI_CHUNK_WEAK, sct->n, sct->weak)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int psi_save_alternates (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h, unsigned enc)
{
	if (psi_save_sector (fp, sct, c, h, 0, enc)) {
		return (1);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (psi_save_sector (fp, sct, c, h, 1, enc)) {
			return (1);
		}

		sct = sct->next;
	}

	return (0);
}

static
int psi_save_text (FILE *fp, const psi_img_t *img)
{
	int           r;
	unsigned long crc;
	unsigned char buf[16];

	if (img->comment_size == 0) {
		return (0);
	}

	psi_set_uint32_be (buf, 0, PSI_CHUNK_TEXT);
	psi_set_uint32_be (buf, 4, img->comment_size + 2);

	buf[8] = 0x0a;

	crc = 0;

	r = psi_write_crc (fp, buf, 9, &crc);
	r |= psi_write_crc (fp, img->comment, img->comment_size, &crc);
	r |= psi_write_crc (fp, buf + 8, 1, &crc);
	psi_set_uint32_be (buf, 0, crc);
	r |= psi_write_crc (fp, buf, 4, NULL);

	return (r);
}

int psi_save_psi (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h, s;
	unsigned        enc;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	if (psi_save_header (fp, img, &enc)) {
		return (1);
	}

	if (psi_save_text (fp, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (psi_save_alternates (fp, sct, c, h, enc)) {
					return (1);
				}
			}
		}
	}

	if (psi_save_chunk (fp, PSI_CHUNK_END, 0, NULL)) {
		return (1);
	}

	fflush (fp);

	return (0);
}


int psi_probe_psi_fp (FILE *fp)
{
	unsigned char buf[4];

	if (psi_read_ofs (fp, 0, buf, 4)) {
		return (0);
	}

	if (psi_get_uint32_be (buf, 0) != PSI_CHUNK_PSI) {
		return (0);
	}

	return (1);
}

int psi_probe_psi (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = psi_probe_psi_fp (fp);

	fclose (fp);

	return (r);
}
