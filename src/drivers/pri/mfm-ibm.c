/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/mfm-ibm.c                                    *
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
#include "pri-img.h"
#include "mfm-ibm.h"


typedef struct {
	pri_trk_t     *trk;

	char          last;
	char          clock;

	unsigned      crc;

	unsigned long last_gap3_start;
} mfm_code_t;


static
unsigned mfm_crc (unsigned crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *src;

	src = buf;

	while (cnt > 0) {
		crc ^= (unsigned) *src << 8;

		for (i = 0; i < 8; i++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ 0x1021;
			}
			else {
				crc = crc << 1;
			}
		}

		src += 1;
		cnt -= 1;
	}

	return (crc & 0xffff);
}


static
int mfm_get_bit (mfm_code_t *mfm)
{
	unsigned long bit;

	pri_trk_get_bits (mfm->trk, &bit, 1);

	mfm->clock = !mfm->clock;

	return (bit & 1);
}

static
unsigned char mfm_decode_byte (mfm_code_t *mfm)
{
	unsigned i;
	unsigned val;

	if (mfm->clock) {
		mfm_get_bit (mfm);
	}

	val = 0;

	for (i = 0; i < 8; i++) {
		val = (val << 1) | (mfm_get_bit (mfm) != 0);

		mfm_get_bit (mfm);
	}

	return (val);
}

static
void mfm_read (mfm_code_t *mfm, unsigned char *buf, unsigned cnt)
{
	unsigned long i;

	for (i = 0; i < cnt; i++) {
		buf[i] = mfm_decode_byte (mfm);
	}
}

/*
 * Sync with MFM bit stream.
 *
 * MFM mark:
 *   A1 = 10100001 / 0A = 00001010
 *   4489 = [0] 1 [0] 0 [0] 1 [0] 0 [1] 0 [0] 0 [1] 0 [0] 1
 *
 * If mfm_sync() returns successfully the stream position is the clock
 * bit between the last mark byte and the next byte.
 */
static
int mfm_sync (mfm_code_t *mfm)
{
	unsigned long pos;
	unsigned      v1, v2, v3;

	v1 = 0;
	v2 = 0;
	v3 = 0;

	pos = mfm->trk->idx;

	while (1) {
		v1 = ((v1 << 1) | (v2 >> 15)) & 0xffff;
		v2 = ((v2 << 1) | (v3 >> 15)) & 0xffff;
		v3 = (v3 << 1) & 0xffff;

		if (mfm_get_bit (mfm)) {
			v3 |= 1;
		}

		if ((v1 == 0x4489) && (v2 == 0x4489) && (v3 == 0x4489)) {
			break;
		}

		if (mfm->trk->idx == pos) {
			return (1);
		}
	}

	mfm->clock = 1;

	return (0);
}

/*
 * Sync with the next mark and initialize the crc.
 */
static
int mfm_sync_mark (mfm_code_t *mfm, unsigned char *val)
{
	if (mfm_sync (mfm)) {
		return (1);
	}

	*val = mfm_decode_byte (mfm);

	mfm->crc = mfm_crc (0xffff, "\xa1\xa1\xa1", 3);
	mfm->crc = mfm_crc (mfm->crc, val, 1);

	return (0);
}

static
psi_sct_t *mfm_decode_idam (mfm_code_t *mfm)
{
	unsigned      c, h, s, n;
	unsigned      crc;
	unsigned char buf[8];
	psi_sct_t     *sct;

	mfm_read (mfm, buf, 6);

	crc = pri_get_uint16_be (buf, 4);

	mfm->crc = mfm_crc (mfm->crc, buf, 4);

	c = buf[0];
	h = buf[1];
	s = buf[2];
	n = buf[3];

	n = 128 << ((n < 8) ? n : 8);

	sct = psi_sct_new (c, h, s, n);

	if (sct == NULL) {
		return (NULL);
	}

	psi_sct_set_mfm_size (sct, buf[3]);

	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 1);

	if (mfm->crc != crc) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, 1);
	}

	psi_sct_fill (sct, 0);

	return (sct);
}

static
int mfm_decode_dam (mfm_code_t *mfm, psi_sct_t *sct, unsigned mark)
{
	unsigned      crc;
	unsigned char buf[4];

	mfm_read (mfm, sct->data, sct->n);
	mfm_read (mfm, buf, 2);

	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 0);
	psi_sct_set_flags (sct, PSI_FLAG_DEL_DAM, mark == 0xf8);

	crc = pri_get_uint16_be (buf, 0);

	mfm->crc = mfm_crc (mfm->crc, sct->data, sct->n);

	if (mfm->crc != crc) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
	}

	return (0);
}

static
int mfm_decode_mark (mfm_code_t *mfm, psi_trk_t *trk, unsigned mark)
{
	unsigned char mark2;
	char          wrap;
	unsigned long pos;
	psi_sct_t     *sct;

	switch (mark) {
	case 0xfe: /* ID address mark */
		sct = mfm_decode_idam (mfm);

		if (sct == NULL) {
			return (1);
		}

		psi_sct_set_encoding (sct, PSI_ENC_MFM);

		pos = mfm->trk->idx;
		wrap = mfm->trk->wrap;

		if (mfm_sync_mark (mfm, &mark2) == 0) {
			if ((mark2 == 0xf8) || (mark2 == 0xfb)) {
				pos = mfm->trk->idx;
				wrap = mfm->trk->wrap;

				if (mfm_decode_dam (mfm, sct, mark2)) {
					psi_sct_del (sct);
					return (1);
				}
			}
		}

		if (sct->flags & PSI_FLAG_NO_DAM) {
			psi_sct_set_size (sct, 0, 0);
		}

		mfm->trk->idx = pos;
		mfm->trk->wrap = wrap;

		psi_trk_add_sector (trk, sct);
		break;

	case 0xfb: /* data address mark */
	case 0xf8: /* deleted data address mark */
		fprintf (stderr, "mfm: dam without idam\n");
		break;

	default:
		fprintf (stderr,
			"mfm: unknown address mark (mark=0x%02x)\n", mark
		);
	}

	return (0);
}

psi_trk_t *pri_decode_mfm_trk (pri_trk_t *trk, unsigned h)
{
	unsigned char mark;
	psi_trk_t     *dtrk;
	mfm_code_t    mfm;

	dtrk = psi_trk_new (h);

	if (dtrk == NULL) {
		return (NULL);
	}

	mfm.trk = trk;
	mfm.clock = 0;

	pri_trk_set_pos (trk, 0);

	while (trk->wrap == 0) {
		if (mfm_sync_mark (&mfm, &mark)) {
			break;
		}

		if (trk->wrap) {
			break;
		}

		if (mfm_decode_mark (&mfm, dtrk, mark)) {
			psi_trk_del (dtrk);
			return (NULL);
		}
	}

	return (dtrk);
}

psi_img_t *pri_decode_mfm (pri_img_t *img)
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
				dtrk = pri_decode_mfm_trk (trk, h);
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
void mfm_encode_byte (mfm_code_t *mfm, unsigned val, unsigned msk)
{
	unsigned      i;
	unsigned      buf;
	unsigned long max;

	if (mfm->trk->wrap) {
		return;
	}

	buf = (mfm->last != 0);

	for (i = 0; i < 8; i++) {
		buf = (buf << 2) | ((val >> 7) & 1);
		val = val << 1;

		if ((buf & 0x05) == 0) {
			buf |= 0x02;
		}
	}

	buf &= msk;

	max = mfm->trk->size - mfm->trk->idx;

	if (max > 16) {
		max = 16;
	}

	pri_trk_set_bits (mfm->trk, buf >> (16 - max), max);

	mfm->last = buf & 1;
}

static
void mfm_encode_bytes (mfm_code_t *mfm, const unsigned char *buf, unsigned cnt)
{
	while (cnt > 0) {
		mfm_encode_byte (mfm, *buf, 0xffff);
		buf += 1;
		cnt -= 1;
	}
}

static
void mfm_encode_mark (mfm_code_t *mfm, unsigned char mark)
{
	mfm_encode_byte (mfm, 0xa1, 0xffdf);
	mfm_encode_byte (mfm, 0xa1, 0xffdf);
	mfm_encode_byte (mfm, 0xa1, 0xffdf);
	mfm_encode_byte (mfm, mark, 0xffff);

	mfm->crc = mfm_crc (0xffff, "\xa1\xa1\xa1", 3);
	mfm->crc = mfm_crc (mfm->crc, &mark, 1);
}

static
void mfm_encode_sector (mfm_code_t *mfm, psi_sct_t *sct, unsigned gap3)
{
	unsigned      i;
	unsigned      flags;
	unsigned char buf[8];

	if (sct->position != 0xffffffff) {
		if ((2 * sct->position) >= mfm->last_gap3_start) {
			pri_trk_set_pos (mfm->trk, 2 * sct->position);
		}
		else {
			pri_trk_set_pos (mfm->trk, mfm->last_gap3_start);
		}
	}

	flags = sct->flags;

	for (i = 0; i < 12; i++) {
		mfm_encode_byte (mfm, 0, 0xffff);
	}

	mfm_encode_mark (mfm, 0xfe);

	buf[0] = sct->c;
	buf[1] = sct->h;
	buf[2] = sct->s;
	buf[3] = psi_sct_get_mfm_size (sct);

	mfm->crc = mfm_crc (mfm->crc, buf, 4);

	if (flags & PSI_FLAG_CRC_ID) {
		mfm->crc = ~mfm->crc;
	}

	buf[4] = (mfm->crc >> 8) & 0xff;
	buf[5] = mfm->crc & 0xff;

	mfm_encode_bytes (mfm, buf, 6);

	for (i = 0; i < 22; i++) {
		mfm_encode_byte (mfm, 0x4e, 0xffff);
	}

	if (flags & PSI_FLAG_NO_DAM) {
		mfm->last_gap3_start = mfm->trk->idx;
		return;
	}

	for (i = 0; i < 12; i++) {
		mfm_encode_byte (mfm, 0x00, 0xffff);
	}

	mfm_encode_mark (mfm, (flags & PSI_FLAG_DEL_DAM) ? 0xf8 : 0xfb);

	mfm_encode_bytes (mfm, sct->data, sct->n);

	mfm->crc = mfm_crc (mfm->crc, sct->data, sct->n);

	if (flags & PSI_FLAG_CRC_DATA) {
		mfm->crc = ~mfm->crc;
	}

	buf[0] = (mfm->crc >> 8) & 0xff;
	buf[1] = mfm->crc & 0xff;

	mfm_encode_bytes (mfm, buf, 2);

	mfm->last_gap3_start = mfm->trk->idx;

	for (i = 0; i < gap3; i++) {
		mfm_encode_byte (mfm, 0x4e, 0xffff);
	}
}

int pri_encode_mfm_trk (pri_trk_t *dtrk, psi_trk_t *strk, pri_mfm_t *par)
{
	unsigned      i;
	unsigned long bits;
	unsigned      gap3, scnt;
	psi_sct_t     *sct;
	mfm_code_t    mfm;

	mfm.trk = dtrk;
	mfm.last = 0;
	mfm.last_gap3_start = 0;

	pri_trk_set_pos (dtrk, 0);

	gap3 = par->gap3;

	if (par->auto_gap3) {
		bits = par->gap4a + par->gap1;

		if (par->enable_iam) {
			bits += 16;
		}

		scnt = 0;

		for (i = 0; i < strk->sct_cnt; i++) {
			sct = strk->sct[i];

			if ((sct->flags & PSI_FLAG_NO_DAM) == 0) {
				scnt += 1;
				bits += strk->sct[i]->n + 12 + 10 + 22 + 12 + 4 + 2;
			}
			else {
				bits += strk->sct[i]->n + 12 + 10 + 22;
			}
		}

		bits *= 16;

		if (scnt > 0) {
			if (bits < dtrk->size) {
				gap3 = (dtrk->size - bits) / (16 * scnt);

				if (gap3 > par->gap3) {
					gap3 = par->gap3;
				}
			}
			else {
				pri_trk_set_size (dtrk, bits);
			}
		}
	}

	for (i = 0; i < par->gap4a; i++) {
		mfm_encode_byte (&mfm, 0x4e, 0xffff);
	}

	if (par->enable_iam) {
		for (i = 0; i < 12; i++) {
			mfm_encode_byte (&mfm, 0x00, 0xffff);
		}

		mfm_encode_byte (&mfm, 0xc2, 0xff7f);
		mfm_encode_byte (&mfm, 0xc2, 0xff7f);
		mfm_encode_byte (&mfm, 0xc2, 0xff7f);
		mfm_encode_byte (&mfm, 0xfc, 0xffff);
	}

	for (i = 0; i < par->gap1; i++) {
		mfm_encode_byte (&mfm, 0x4e, 0xffff);
	}

	for (i = 0; i < strk->sct_cnt; i++) {
		sct = strk->sct[i];

		mfm_encode_sector (&mfm, sct, gap3);
	}

	while (dtrk->wrap == 0) {
		mfm_encode_byte (&mfm, 0x4e, 0xffff);
	}

	return (0);
}

int pri_encode_mfm_img (pri_img_t *dimg, psi_img_t *simg, pri_mfm_t *par)
{
	unsigned long c, h;
	psi_cyl_t     *cyl;
	psi_trk_t     *trk;
	pri_trk_t     *dtrk;

	for (c = 0; c < simg->cyl_cnt; c++) {
		cyl = simg->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			dtrk = pri_img_get_track (dimg, c, h, 1);

			if (dtrk == NULL) {
				return (1);
			}

			if (pri_trk_set_size (dtrk, par->track_size)) {
				return (1);
			}

			pri_trk_set_clock (dtrk, par->clock);
			pri_trk_clear_16 (dtrk, 0x9254);

			if (pri_encode_mfm_trk (dtrk, trk, par)) {
				return (1);
			}
		}
	}

	return (0);
}

pri_img_t *pri_encode_mfm (psi_img_t *img, pri_mfm_t *par)
{
	pri_img_t *dimg;

	dimg = pri_img_new();

	if (dimg == NULL) {
		return (NULL);
	}

	if (pri_encode_mfm_img (dimg, img, par)) {
		pri_img_del (dimg);
		return (NULL);
	}

	return (dimg);
}

void pri_mfm_init (pri_mfm_t *par, unsigned long clock, unsigned rpm)
{
	par->clock = clock;
	par->track_size = 60 * clock / rpm;

	par->enable_iam = 0;
	par->auto_gap3 = 1;

	par->gap4a = 96;
	par->gap1 = 0;
	par->gap3 = 80;
}

pri_img_t *pri_encode_mfm_dd_300 (psi_img_t *img)
{
	pri_mfm_t par;

	pri_mfm_init (&par, 500000, 300);

	return (pri_encode_mfm (img, &par));
}

pri_img_t *pri_encode_mfm_hd_300 (psi_img_t *img)
{
	pri_mfm_t par;

	pri_mfm_init (&par, 1000000, 300);

	return (pri_encode_mfm (img, &par));
}

pri_img_t *pri_encode_mfm_hd_360 (psi_img_t *img)
{
	pri_mfm_t par;

	pri_mfm_init (&par, 1000000, 360);

	return (pri_encode_mfm (img, &par));
}
