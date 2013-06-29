/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/gcr-mac.c                                    *
 * Created:     2012-02-01 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/psi/psi.h>

#include "pri.h"
#include "gcr-mac.h"


static const unsigned char gcr_enc_tab[64] = {
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static const unsigned char gcr_dec_tab[256] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
	0xff, 0xff, 0x02, 0x03, 0xff, 0x04, 0x05, 0x06,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x08,
	0xff, 0xff, 0xff, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
	0xff, 0xff, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
	0xff, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x1b, 0xff, 0x1c, 0x1d, 0x1e,
	0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0x20, 0x21,
	0xff, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x29, 0x2a, 0x2b,
	0xff, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
	0xff, 0xff, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
	0xff, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};


static
unsigned long gcr_get_track_length (unsigned c)
{
	static unsigned long tab[5] = {
		76262, 69902, 63540, 57190, 50838
	};

	if (c < 80) {
		return (tab[c / 16]);
	}

	return (0);
}

unsigned gcr_get_format (psi_img_t *img)
{
	unsigned c;

	for (c = 0; c < img->cyl_cnt; c++) {
		if (img->cyl[c]->trk_cnt > 1) {
			return (0x22);
		}
	}

	return (0x02);
}

static
void gcr_fill_track (pri_trk_t *trk)
{
	unsigned long cnt, n;

	cnt = trk->size - trk->idx;

	while (cnt > 0) {
		n = (cnt < 10) ? cnt : 10;

		pri_trk_set_bits (trk, 0xff >> (10 - n), n);

		cnt -= n;
	}

}

static
int gcr_checksum (unsigned char *dst, const unsigned char *src, int enc)
{
	unsigned i;
	unsigned chk[3], tmp, val;

	chk[0] = 0;
	chk[1] = 0;
	chk[2] = 0;

	for (i = 0; i < 524; i++) {
		if ((i % 3) == 0) {
			chk[0] = ((chk[0] << 1) & 0x1fe) | ((chk[0] >> 7) & 0x01);
		}

		val = src[i];

		if (!enc) {
			val ^= chk[0];
		}

		chk[2] += (val & 0xff) + ((chk[0] >> 8) & 1);
		chk[0] &= 0xff;

		if (enc) {
			val ^= chk[0];
		}

		dst[i] = val & 0xff;

		tmp = chk[2];
		chk[2] = chk[1];
		chk[1] = chk[0];
		chk[0] = tmp;
	}

	chk[0] &= 0xff;

	if (enc) {
		dst[524] = chk[1];
		dst[525] = chk[0];
		dst[526] = chk[2];
	}
	else {
		if ((src[524] != chk[1]) || (src[525] != chk[0]) || (src[526] != chk[2])) {
			return (1);
		}
	}

	return (0);
}

static
unsigned gcr_decode_byte (pri_trk_t *trk, int xlat)
{
	unsigned      val, cnt;
	unsigned long bit;

	pri_trk_get_bits (trk, &bit, 8);

	val = bit;
	cnt = 8;

	while (((val & 0x80) == 0) && (cnt < 64)) {
		pri_trk_get_bits (trk, &bit, 1);
		val = (val << 1) | (bit & 1);
		cnt += 1;
	}

	if (cnt >= 64) {
		val = 0;
	}

	if (xlat) {
		val = gcr_dec_tab[val & 0xff];
	}

	return (val);
}

static
int gcr_decode_data (pri_trk_t *trk, unsigned char *dst)
{
	unsigned      i;
	unsigned      val;
	unsigned char buf[527];
	unsigned char high;

	high = 0;

	for (i = 0; i < 524; i++) {
		if ((i % 3) == 0) {
			high = gcr_decode_byte (trk, 1) << 2;
		}

		val = gcr_decode_byte (trk, 1);
		val = (val & 0x3f) | (high & 0xc0);
		high <<= 2;

		buf[i] = val;
	}

	high = gcr_decode_byte (trk, 1) << 2;
	buf[524] = (gcr_decode_byte (trk, 1) & 0x3f) | (high & 0xc0);
	buf[525] = (gcr_decode_byte (trk, 1) & 0x3f) | ((high << 2) & 0xc0);
	buf[526] = (gcr_decode_byte (trk, 1) & 0x3f) | ((high << 4) & 0xc0);

	if (gcr_checksum (dst, buf, 0)) {
		return (1);
	}

	return (0);
}

static
psi_sct_t *pri_decode_gcr_sct (pri_trk_t *trk)
{
	unsigned      i;
	unsigned      lc, lh, ls, lm, ck;
	unsigned char buf[524];
	psi_sct_t     *sct;

	lc = gcr_decode_byte (trk, 1);
	ls = gcr_decode_byte (trk, 1);
	lh = gcr_decode_byte (trk, 1);
	lm = gcr_decode_byte (trk, 1);
	ck = gcr_decode_byte (trk, 1);

	ck ^= lc ^ ls ^ lh ^ lm;
	lc = (lc & 0x3f) | ((lh & 0x1f) << 6);
	lh = (lh >> 5) & 3;

	sct = psi_sct_new (lc, lh, ls, 512);

	if (sct == NULL) {
		return (NULL);
	}

	psi_sct_set_encoding (sct, PSI_ENC_GCR);
	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 1);
	psi_sct_set_gcr_format (sct, lm);

	if (ck != 0) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, 1);
	}

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;

	for (i = 0; i < 64; i++) {
		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = gcr_decode_byte (trk, 0);

		if ((buf[0] == 0xd5) && (buf[1] == 0xaa)) {
			if (buf[2] != 0xad) {
				return (sct);
			}

			buf[2] = gcr_decode_byte (trk, 1);

			if (buf[2] != ls) {
				return (sct);
			}

			break;
		}
	}

	if (i >= 64) {
		return (sct);
	}

	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 0);

	if (gcr_decode_data (trk, buf)) {
		fprintf (stderr, "gcr: data crc error (%u/%u/%u)\n", lc, lh, ls);
		psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
	}

	memcpy (sct->data, buf + 12, 512);

	psi_sct_set_tags (sct, buf, 12);

	return (sct);
}

psi_trk_t *pri_decode_gcr_trk (pri_trk_t *trk, unsigned h)
{
	unsigned long pos;
	char          wrap;
	unsigned char buf[3];
	psi_sct_t     *sct;
	psi_trk_t     *dtrk;

	dtrk = psi_trk_new (h);

	if (dtrk == NULL) {
		return (NULL);
	}

	pri_trk_set_pos (trk, 0);

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;

	while (trk->wrap == 0) {
		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = gcr_decode_byte (trk, 0);

		pos = trk->idx;
		wrap = trk->wrap;

		if ((buf[0] == 0xd5) && (buf[1] == 0xaa) && (buf[2] == 0x96)) {
			sct = pri_decode_gcr_sct (trk);

			if (sct != NULL) {
				psi_trk_add_sector (dtrk, sct);
			}

			trk->idx = pos;
			trk->wrap = wrap;
		}
	}

	return (dtrk);
}

psi_img_t *pri_decode_gcr (pri_img_t *img)
{
	unsigned long c, h;
	pri_cyl_t     *cyl;
	pri_trk_t     *trk;
	psi_img_t     *dimg;
	psi_trk_t     *dtrk;

	dimg = psi_img_new();

	if (dimg == NULL) {
		return (NULL);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (cyl == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk == NULL) {
				dtrk = psi_trk_new (h);
			}
			else {
				dtrk = pri_decode_gcr_trk (trk, h);
			}

			if ((dtrk->sct_cnt == 0) && ((h + 1) == cyl->trk_cnt)) {
				psi_trk_del (dtrk);
				continue;
			}

			if (dtrk == NULL) {
				psi_img_del (dimg);
				return (NULL);
			}

			psi_img_add_track (dimg, dtrk, c);
		}
	}

	return (dimg);
}


static
void pri_encode_gcr_sct (pri_trk_t *trk, unsigned char *src, unsigned c, unsigned h, unsigned s, unsigned fmt)
{
	unsigned      i, v;
	unsigned char buf[527];
	unsigned char *p;

	h = (h << 5) | ((c >> 6) & 0x1f);

	for (i = 0; i < 38; i++) {
		pri_trk_set_bits (trk, 0xff, 10);
	}

	pri_trk_set_bits (trk, 0xd5aa96, 24);
	pri_trk_set_bits (trk, gcr_enc_tab[c & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[s & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[h & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[fmt & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[(c ^ h ^ s ^ fmt) & 0x3f], 8);
	pri_trk_set_bits (trk, 0xdeaa, 16);

	for (i = 0; i < 8; i++) {
		pri_trk_set_bits (trk, 0xff, 10);
	}

	pri_trk_set_bits (trk, 0xd5aaad, 24);
	pri_trk_set_bits (trk, gcr_enc_tab[s & 0x3f], 8);

	gcr_checksum (buf, src, 1);

	p = buf;

	for (i = 0; i < 522; i += 3) {
		v = ((p[0] & 0xc0) >> 2) | ((p[1] & 0xc0) >> 4) | ((p[2] & 0xc0) >> 6);

		pri_trk_set_bits (trk, gcr_enc_tab[v & 0x3f], 8);
		pri_trk_set_bits (trk, gcr_enc_tab[p[0] & 0x3f], 8);
		pri_trk_set_bits (trk, gcr_enc_tab[p[1] & 0x3f], 8);
		pri_trk_set_bits (trk, gcr_enc_tab[p[2] & 0x3f], 8);

		p += 3;
	}

	v = ((p[0] & 0xc0) >> 2) | ((p[1] & 0xc0) >> 4);
	pri_trk_set_bits (trk, gcr_enc_tab[v & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[p[0] & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[p[1] & 0x3f], 8);

	v = ((p[2] & 0xc0) >> 2) | ((p[3] & 0xc0) >> 4) | ((p[4] & 0xc0) >> 6);
	pri_trk_set_bits (trk, gcr_enc_tab[v & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[p[2] & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[p[3] & 0x3f], 8);
	pri_trk_set_bits (trk, gcr_enc_tab[p[4] & 0x3f], 8);

	pri_trk_set_bits (trk, 0xdeaa, 16);
}

int pri_encode_gcr_trk (pri_trk_t *dtrk, psi_trk_t *strk, unsigned fmt)
{
	unsigned      i, f;
	unsigned char buf[524];
	psi_sct_t     *sct;

	pri_trk_set_pos (dtrk, 0);

	for (i = 0; i < 32; i++) {
		pri_trk_set_bits (dtrk, 0xff, 10);
	}

	for (i = 0; i < strk->sct_cnt; i++) {
		sct = strk->sct[i];

		if (sct->n < 512) {
			memcpy (buf + 12, sct->data, sct->n);
		}
		else {
			memcpy (buf + 12, sct->data, 512);
		}

		f = psi_sct_get_gcr_format (sct);

		if (f == 0) {
			f = fmt;
		}

		psi_sct_get_tags (sct, buf, 12);

		pri_encode_gcr_sct (dtrk, buf, sct->c, sct->h, sct->s, f);
	}

	if (dtrk->wrap) {
		return (1);
	}

	gcr_fill_track (dtrk);

	return (0);
}

int pri_encode_gcr_img (pri_img_t *dimg, psi_img_t *simg)
{
	unsigned long c, h;
	unsigned long size;
	unsigned      fmt;
	psi_cyl_t     *cyl;
	psi_trk_t     *trk;
	pri_trk_t     *dtrk;

	fmt = gcr_get_format (simg);

	for (c = 0; c < simg->cyl_cnt; c++) {
		cyl = simg->cyl[c];

		size = gcr_get_track_length (c);

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			dtrk = pri_img_get_track (dimg, c, h, 1);

			if (dtrk == NULL) {
				return (1);
			}

			if (pri_trk_set_size (dtrk, size)) {
				return (1);
			}

			pri_trk_set_clock (dtrk, 500000);

			if (pri_encode_gcr_trk (dtrk, trk, fmt)) {
				return (1);
			}
		}
	}

	return (0);
}

pri_img_t *pri_encode_gcr (psi_img_t *img)
{
	pri_img_t *dimg;

	dimg = pri_img_new();

	if (dimg == NULL) {
		return (NULL);
	}

	if (pri_encode_gcr_img (dimg, img)) {
		pri_img_del (dimg);
		return (NULL);
	}

	return (dimg);
}
