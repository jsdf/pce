/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-enc-fm.c                                 *
 * Created:     2014-12-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014 Hampa Hug <hampa@hampa.ch>                          *
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
#include "pri-enc-fm.h"


typedef struct {
	pri_trk_t     *trk;

	char          clock;

	unsigned      crc;

	unsigned long last_gap3_start;
} fm_code_t;


/*
 * Calculate the CRC for FM
 */
static
unsigned fm_crc (unsigned crc, const void *buf, unsigned cnt)
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


/*
 * Get the next FM bit
 */
static
int fm_get_bit (fm_code_t *fm)
{
	unsigned long bit;

	pri_trk_get_bits (fm->trk, &bit, 1);

	fm->clock = !fm->clock;

	return (bit & 1);
}

/*
 * Read an FM byte
 */
static
unsigned char fm_decode_byte (fm_code_t *fm)
{
	unsigned i;
	unsigned val;

	if (fm->clock) {
		fm_get_bit (fm);
	}

	val = 0;

	for (i = 0; i < 8; i++) {
		val = (val << 1) | (fm_get_bit (fm) != 0);

		fm_get_bit (fm);
	}

	return (val);
}

/*
 * Read FM data
 */
static
void fm_read (fm_code_t *fm, unsigned char *buf, unsigned cnt)
{
	unsigned long i;

	for (i = 0; i < cnt; i++) {
		buf[i] = fm_decode_byte (fm);
	}
}

/*
 * Sync with an FM mark and initialize the CRC.
 *
 * FE = 11111110 / C7 = 11000111
 * F57E = 1 1 1 1  0 1 0 1  0 1 1 1  1 1 1 0
 *
 * FB = 11111011 / C7 = 11000111
 * F56F = 1 1 1 1  0 1 0 1  0 1 1 0  1 1 1 1
 *
 * F8 = 11111000 / C7 = 11000111
 * F56A = 1 1 1 1  0 1 0 1  0 1 1 0  1 0 1 0
 *
 * FC = 11111100 / D7 = 11010111
 * F77A = 1 1 1 1  0 1 1 1  0 1 1 1  1 0 1 0
 */
static
int fm_sync_mark (fm_code_t *fm, unsigned char *val)
{
	unsigned long pos;
	unsigned      v;

	v = 0;

	pos = fm->trk->idx;

	while (1) {
		v = ((v << 1) | (fm_get_bit (fm) != 0)) & 0xffff;

		if (v == 0xf57e) {
			*val = 0xfe;
			break;
		}
		else if (v == 0xf56f) {
			*val = 0xfb;
			break;
		}
		else if (v == 0xf56a) {
			*val = 0xf8;
			break;
		}

		if (fm->trk->idx == pos) {
			return (1);
		}
	}

	fm->crc = fm_crc (0xffff, val, 1);

	fm->clock = 1;

	return (0);
}

static
psi_sct_t *fm_decode_idam (fm_code_t *fm)
{
	unsigned      c, h, s, n;
	unsigned      crc;
	unsigned long pos;
	unsigned char buf[8];
	psi_sct_t     *sct;

	pos = fm->trk->idx;

	fm_read (fm, buf, 6);

	crc = pri_get_uint16_be (buf, 4);

	fm->crc = fm_crc (fm->crc, buf, 4);

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
	psi_sct_set_position (sct, pos / 2);
	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 1);
	psi_sct_set_encoding (sct, PSI_ENC_FM);

	if (fm->crc != crc) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, 1);
	}

	psi_sct_fill (sct, 0);

	return (sct);
}

static
int fm_decode_dam (fm_code_t *fm, psi_sct_t *sct, unsigned mark)
{
	unsigned      crc;
	unsigned char buf[4];

	fm_read (fm, sct->data, sct->n);
	fm_read (fm, buf, 2);

	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 0);
	psi_sct_set_flags (sct, PSI_FLAG_DEL_DAM, mark == 0xf8);

	crc = pri_get_uint16_be (buf, 0);

	fm->crc = fm_crc (fm->crc, sct->data, sct->n);

	if (fm->crc != crc) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
	}

	return (0);
}

static
int fm_decode_mark (fm_code_t *fm, psi_trk_t *trk, unsigned mark)
{
	unsigned char mark2;
	char          wrap;
	unsigned long pos;
	psi_sct_t     *sct;

	switch (mark) {
	case 0xfe: /* ID address mark */
		sct = fm_decode_idam (fm);

		if (sct == NULL) {
			return (1);
		}

		pos = fm->trk->idx;
		wrap = fm->trk->wrap;

		if (fm_sync_mark (fm, &mark2) == 0) {
			if ((mark2 == 0xf8) || (mark2 == 0xfb)) {
				pos = fm->trk->idx;
				wrap = fm->trk->wrap;

				if (fm_decode_dam (fm, sct, mark2)) {
					psi_sct_del (sct);
					return (1);
				}
			}
		}

		if (sct->flags & PSI_FLAG_NO_DAM) {
			psi_sct_set_size (sct, 0, 0);
		}

		fm->trk->idx = pos;
		fm->trk->wrap = wrap;

		psi_trk_add_sector (trk, sct);
		break;

	case 0xfb: /* data address mark */
	case 0xf8: /* deleted data address mark */
		fprintf (stderr, "fm: dam without idam\n");
		break;

	default:
		fprintf (stderr,
			"fm: unknown mark (0x%02x)\n", mark
		);
	}

	return (0);
}

psi_trk_t *pri_decode_fm_trk (pri_trk_t *trk, unsigned h)
{
	unsigned char mark;
	psi_trk_t     *dtrk;
	fm_code_t     fm;

	if ((dtrk = psi_trk_new (h)) == NULL) {
		return (NULL);
	}

	fm.trk = trk;
	fm.clock = 0;

	pri_trk_set_pos (trk, 0);

	while (trk->wrap == 0) {
		if (fm_sync_mark (&fm, &mark)) {
			break;
		}

		if ((trk->wrap) && (trk->idx >= 16)) {
			break;
		}

		if (fm_decode_mark (&fm, dtrk, mark)) {
			psi_trk_del (dtrk);
			return (NULL);
		}
	}

	return (dtrk);
}

psi_img_t *pri_decode_fm (pri_img_t *img)
{
	unsigned long c, h;
	pri_cyl_t     *cyl;
	pri_trk_t     *trk;
	psi_img_t     *dimg;
	psi_trk_t     *dtrk;

	if ((dimg = psi_img_new()) == NULL) {
		return (NULL);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		if ((cyl = img->cyl[c]) == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk == NULL) {
				dtrk = psi_trk_new (h);
			}
			else {
				dtrk = pri_decode_fm_trk (trk, h);
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
void fm_encode_byte (fm_code_t *fm, unsigned val, unsigned clk)
{
	unsigned      i;
	unsigned      buf;
	unsigned long max;

	if (fm->trk->wrap) {
		return;
	}

	buf = 0;

	for (i = 0; i < 8; i++) {
		buf = (buf << 2) | ((clk >> 6) & 2) | ((val >> 7) & 1);
		val = val << 1;
		clk = clk << 1;
	}

	max = fm->trk->size - fm->trk->idx;

	if (max > 16) {
		max = 16;
	}

	pri_trk_set_bits (fm->trk, buf >> (16 - max), max);
}

static
void fm_encode_bytes (fm_code_t *fm, const unsigned char *buf, unsigned cnt)
{
	while (cnt > 0) {
		fm_encode_byte (fm, *buf, 0xff);
		buf += 1;
		cnt -= 1;
	}
}

static
void fm_encode_sector (fm_code_t *fm, psi_sct_t *sct, unsigned gap3)
{
	unsigned      i;
	unsigned      flags;
	unsigned long pos;
	unsigned char buf[8];

	if ((sct->position != 0xffffffff) && (sct->position >= (16 * 8))) {
		pos = 2 * (sct->position - 16 * 8);

		if (pos >= fm->last_gap3_start) {
			pri_trk_set_pos (fm->trk, pos);
		}
		else {
			pri_trk_set_pos (fm->trk, fm->last_gap3_start);
		}
	}

	flags = sct->flags;

	for (i = 0; i < 6; i++) {
		fm_encode_byte (fm, 0, 0xff);
	}

	fm_encode_byte (fm, 0xfe, 0xc7);

	fm->crc = fm_crc (0xffff, "\xfe", 1);

	buf[0] = sct->c;
	buf[1] = sct->h;
	buf[2] = sct->s;
	buf[3] = psi_sct_get_mfm_size (sct);

	fm->crc = fm_crc (fm->crc, buf, 4);

	if (flags & PSI_FLAG_CRC_ID) {
		fm->crc = ~fm->crc;
	}

	buf[4] = (fm->crc >> 8) & 0xff;
	buf[5] = fm->crc & 0xff;

	fm_encode_bytes (fm, buf, 6);

	for (i = 0; i < 11; i++) {
		fm_encode_byte (fm, 0xff, 0xff);
	}

	if (flags & PSI_FLAG_NO_DAM) {
		fm->last_gap3_start = fm->trk->idx;
		return;
	}

	for (i = 0; i < 6; i++) {
		fm_encode_byte (fm, 0x00, 0xff);
	}

	if (flags & PSI_FLAG_DEL_DAM) {
		fm_encode_byte (fm, 0xf8, 0xc7);
		fm->crc = fm_crc (0xffff, "\xf8", 1);
	}
	else {
		fm_encode_byte (fm, 0xfb, 0xc7);
		fm->crc = fm_crc (0xffff, "\xfb", 1);
	}

	fm_encode_bytes (fm, sct->data, sct->n);

	fm->crc = fm_crc (fm->crc, sct->data, sct->n);

	if (flags & PSI_FLAG_CRC_DATA) {
		fm->crc = ~fm->crc;
	}

	buf[0] = (fm->crc >> 8) & 0xff;
	buf[1] = fm->crc & 0xff;

	fm_encode_bytes (fm, buf, 2);

	fm->last_gap3_start = fm->trk->idx;

	for (i = 0; i < gap3; i++) {
		fm_encode_byte (fm, 0xff, 0xff);
	}
}

int pri_encode_fm_trk (pri_trk_t *dtrk, psi_trk_t *strk, pri_enc_fm_t *par)
{
	unsigned      i;
	unsigned long bits;
	unsigned      gap3, scnt;
	psi_sct_t     *sct;
	fm_code_t     fm;

	fm.trk = dtrk;
	fm.last_gap3_start = 0;

	pri_trk_set_pos (dtrk, 0);

	gap3 = par->gap3;

	if (par->auto_gap3) {
		bits = par->gap4a + par->gap1;

		if (par->enable_iam) {
			bits += 7;
		}

		scnt = 0;

		for (i = 0; i < strk->sct_cnt; i++) {
			sct = strk->sct[i];

			if ((sct->encoding & PSI_ENC_MASK) != PSI_ENC_FM) {
				continue;
			}

			if ((sct->flags & PSI_FLAG_NO_DAM) == 0) {
				scnt += 1;
				bits += strk->sct[i]->n + 6 + 7 + 11 + 6 + 1 + 2;
			}
			else {
				bits += strk->sct[i]->n + 6 + 7 + 11;
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
				gap3 = 1;

				pri_trk_set_size (dtrk, bits + 16 * (scnt * gap3 + 4));
			}
		}
	}

	for (i = 0; i < par->gap4a; i++) {
		fm_encode_byte (&fm, 0xff, 0xff);
	}

	if (par->enable_iam) {
		for (i = 0; i < 6; i++) {
			fm_encode_byte (&fm, 0x00, 0xff);
		}

		fm_encode_byte (&fm, 0xfc, 0xd7);
	}

	for (i = 0; i < par->gap1; i++) {
		fm_encode_byte (&fm, 0xff, 0xff);
	}

	for (i = 0; i < strk->sct_cnt; i++) {
		sct = strk->sct[i];

		if ((sct->encoding & PSI_ENC_MASK) == PSI_ENC_FM) {
			fm_encode_sector (&fm, sct, gap3);
		}
	}

	while (dtrk->wrap == 0) {
		fm_encode_byte (&fm, 0xff, 0xff);
	}

	return (0);
}

int pri_encode_fm_img (pri_img_t *dimg, psi_img_t *simg, pri_enc_fm_t *par)
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
			pri_trk_clear_16 (dtrk, 0xffff);

			if (pri_encode_fm_trk (dtrk, trk, par)) {
				return (1);
			}
		}
	}

	return (0);
}

pri_img_t *pri_encode_fm (psi_img_t *img, pri_enc_fm_t *par)
{
	pri_img_t *dimg;

	if ((dimg = pri_img_new()) == NULL) {
		return (NULL);
	}

	if (pri_encode_fm_img (dimg, img, par)) {
		pri_img_del (dimg);
		return (NULL);
	}

	return (dimg);
}

void pri_encode_fm_init (pri_enc_fm_t *par, unsigned long clock, unsigned rpm)
{
	par->clock = clock;
	par->track_size = 60 * clock / rpm;

	par->enable_iam = 0;
	par->auto_gap3 = 1;

	par->gap4a = 40;
	par->gap1 = 26;
	par->gap3 = 27;
}

pri_img_t *pri_encode_fm_sd_300 (psi_img_t *img)
{
	pri_enc_fm_t par;

	pri_encode_fm_init (&par, 250000, 300);

	return (pri_encode_fm (img, &par));
}
