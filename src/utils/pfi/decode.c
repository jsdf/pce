/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/decode.c                                       *
 * Created:     2012-01-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pfi/pfi.h>
#include <drivers/pfi/decode-bits.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>


#define MODE_NONE       0
#define MODE_MAC_DD_490 1
#define MODE_MAC_DD_500 2


struct decode_bits_s {
	const char    *type;
	FILE          *fp;
	unsigned long rate;
};


struct decode_pri_s {
	pri_img_t     *img;
	unsigned long default_rate;
	unsigned      revolution;

	unsigned      rate_cnt;
	unsigned long *rate;

	unsigned      fold_mode;
	unsigned      fold_window;
	unsigned long max_compare;
};


static
int pfi_decode_bits_raw (FILE *fp, const unsigned char *buf, unsigned long cnt)
{
	if (fwrite (buf, (cnt + 7) / 8, 1, fp) != 1) {
		return (1);
	}

	return (0);
}

static
int pfi_decode_bits_mfm (FILE *fp, const unsigned char *buf, unsigned long cnt)
{
	unsigned msk;
	unsigned outbuf, outcnt;
	unsigned val, clk;

	msk = 0x80;

	val = 0;
	clk = 0xffff;

	outbuf = 0;
	outcnt = 0;

	while (cnt > 0) {
		val = (val << 1) | ((*buf & msk) != 0);
		clk = (clk << 1) | ((clk ^ 1) & 1);

		if ((val & 0xffff) == 0x4489) {
			if (outcnt > 0) {
				fputc (outbuf << (8 - outcnt), fp);
				outbuf = 0;
				outcnt = 0;
			}

			clk = 0xaaaa;
		}

		if ((clk & 0x8000) == 0) {
			outbuf = (outbuf << 1) | ((val & 0x8000) != 0);
			outcnt += 1;

			if (outcnt >= 8) {
				fputc (outbuf, fp);
				outbuf = 0;
				outcnt = 0;
			}
		}

		msk >>= 1;

		if (msk == 0) {
			msk = 0x80;
			buf += 1;
		}

		cnt -= 1;
	}

	return (0);
}

static
int pfi_decode_bits_gcr (FILE *fp, const unsigned char *buf, unsigned long cnt)
{
	unsigned val, msk;

	msk = 0x80;
	val = 0;

	while (cnt > 0) {
		val = (val << 1) | ((*buf & msk) != 0);

		if (val & 0x80) {
			fputc (val, fp);
			val = 0;
		}

		msk >>= 1;

		if (msk == 0) {
			msk = 0x80;
			buf += 1;
		}

		cnt -= 1;
	}

	return (0);
}

static
int pfi_decode_bits_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	int                  r;
	struct decode_bits_s *par;
	pfi_dec_t            bit;

	par = opaque;

	pfi_dec_init (&bit, 0);

	if (pfi_trk_decode_bits (trk, &bit, par->rate, par_revolution)) {
		pfi_dec_free (&bit);
		return (1);
	}

	if (strcmp (par->type, "raw") == 0) {
		r = pfi_decode_bits_raw (par->fp, bit.buf, bit.index);
	}
	else if (strcmp (par->type, "gcr-raw") == 0) {
		r = pfi_decode_bits_gcr (par->fp, bit.buf, bit.index);
	}
	else if (strcmp (par->type, "mfm-raw") == 0) {
		r = pfi_decode_bits_mfm (par->fp, bit.buf, bit.index);
	}
	else {
		r = 1;
	}

	pfi_dec_free (&bit);

	return (r);
}

int pfi_decode_bits (pfi_img_t *img, const char *type, unsigned long rate, const char *fname)
{
	int                  r;
	struct decode_bits_s par;

	par.type = type;
	par.rate = rate;
	par.fp = fopen (fname, "wb");

	if (par.fp == NULL) {
		return (1);
	}

	r = pfi_for_all_tracks (img, pfi_decode_bits_cb, &par);

	fclose (par.fp);

	return (r);
}

static
unsigned long pfi_bit_diff (const unsigned char *p1, unsigned long i1, const unsigned char *p2, unsigned long i2, unsigned long cnt)
{
	unsigned long i;
	unsigned      j1, j2;
	unsigned long ret;

	j1 = ~i1 & 7;
	j2 = ~i2 & 7;

	p1 += i1 / 8;
	p2 += i2 / 8;

	ret = 0;

	for (i = 0; i < cnt; i++) {
		ret += ((*p1 >> j1) ^ (*p2 >> j2)) & 1;

		if (j1 == 0) {
			p1 += 1;
			j1 = 7;
		}
		else {
			j1 -= 1;
		}

		if (j2 == 0) {
			p2 += 1;
			j2 = 7;
		}
		else {
			j2 -= 1;
		}
	}

	return (ret);
}

static
int pfi_dec_fold_mindiff (pfi_dec_t *bit, unsigned window, unsigned long max, unsigned c, unsigned h)
{
	unsigned      i;
	unsigned long val, pos, cnt;
	unsigned long min_val, min_pos, min_cnt, dist;

	if ((bit->index < 256) || ((bit->cnt - bit->index) < 256)) {
		return (0);
	}

	min_val = 0;
	min_pos = bit->index;
	min_cnt = min_pos;

	for (i = 1; i < window; i++) {
		pos = (i & 1) ? (bit->index - i / 2) : (bit->index + i / 2);

		cnt = bit->cnt - pos;

		if (cnt > pos) {
			cnt = pos;
		}

		if (cnt > max) {
			cnt = max;
		}

		val = pfi_bit_diff (bit->buf, 0, bit->buf, pos, cnt);

		if ((val < min_val) || (i == 1)) {
			min_val = val;
			min_pos = pos;
			min_cnt = cnt;
		}

		if (val == 0) {
			break;
		}
	}

	dist = (min_pos < bit->index) ? (bit->index - min_pos) : (min_pos - bit->index);

	if (par_verbose || (min_val > (min_cnt / 16))) {
		fprintf (stderr, "track %2u/%u: %7.3f%% at %6lu %c %lu\n",
			c, h,
			100.0 * (double) min_val / min_cnt,
			min_pos, (min_pos < bit->index) ? '-' : '+', dist
		);
	}

	bit->index = min_pos;

	return (0);
}

static
unsigned long pfi_bit_run (const unsigned char *p1, unsigned long i1, const unsigned char *p2, unsigned long i2, unsigned long max)
{
	unsigned long i;
	unsigned      j1, j2;

	j1 = ~i1 & 7;
	j2 = ~i2 & 7;

	p1 += i1 / 8;
	p2 += i2 / 8;

	for (i = 0; i < max; i++) {
		if (((*p1 >> j1) ^ (*p2 >> j2)) & 1) {
			return (i);
		}

		if (j1 == 0) {
			p1 += 1;
			j1 = 7;
		}
		else {
			j1 -= 1;
		}

		if (j2 == 0) {
			p2 += 1;
			j2 = 7;
		}
		else {
			j2 -= 1;
		}
	}

	return (max);
}

static
int pfi_dec_fold_maxrun (pfi_dec_t *bit, unsigned window, unsigned long max, unsigned c, unsigned h)
{
	unsigned      i;
	unsigned long val, pos, cnt;
	unsigned long max_val, max_pos, dist;

	if ((bit->index < 256) || ((bit->cnt - bit->index) < 256)) {
		return (0);
	}

	max_val = 0;
	max_pos = bit->index;

	for (i = 1; i < window; i++) {
		pos = (i & 1) ? (bit->index - i / 2) : (bit->index + i / 2);

		cnt = bit->cnt - pos;

		if (cnt > pos) {
			cnt = pos;
		}

		if (cnt > max) {
			cnt = max;
		}

		val = pfi_bit_run (bit->buf, 0, bit->buf, pos, cnt);

		if ((val > max_val) || (i == 1)) {
			max_val = val;
			max_pos = pos;
		}
	}

	dist = (max_pos < bit->index) ? (bit->index - max_pos) : (max_pos - bit->index);

	if (par_verbose || (max_val < max)) {
		fprintf (stderr, "track %2u/%u: %6lu at %6lu (%lu %c %lu)\n",
			c, h,
			max_val, max_pos,
			bit->index, (max_pos < bit->index) ? '-' : '+', dist
		);
	}

	bit->index = max_pos;

	return (0);
}

static
void pfi_decode_weak (pri_trk_t *trk, pfi_dec_t *bit, unsigned long i1, unsigned long i2)
{
	unsigned long i;
	unsigned long val;

	if ((i1 + i2) >= bit->index) {
		return;
	}

	val = 0;

	for (i = i1; i < (bit->index - i2); i++) {
		val <<= 1;

		if (bit->weak[i >> 3] & (0x80 >> (i & 7))) {
			val |= 1;
		}

		if (val & 0x80000000) {
			pri_trk_evt_add (trk, PRI_EVENT_FUZZY, i - 31, val);
			val = 0;
		}
	}
}

static
void pfi_decode_clock (pri_trk_t *trk, pfi_dec_t *bit, unsigned long tolerance)
{
	int           have_clock;
	unsigned long i, val;
	unsigned long clk, clk_min, clk_max;
	unsigned long cnt_min, cnt_max;

	pfi_dec_clock_median (bit);
	pfi_dec_clock_average (bit);

	cnt_min = 0;
	cnt_max = 0;

	clk = trk->clock;
	clk_min = ((1000 - tolerance) * clk + 500) / 1000;
	clk_max = ((1000 + tolerance) * clk + 500) / 1000;

	have_clock = 0;

	for (i = 0; i < bit->index; i++) {
		if (bit->clk[i] < clk_min) {
			cnt_min += 1;
			cnt_max = 0;
		}
		else if (bit->clk[i] > clk_max) {
			cnt_min = 0;
			cnt_max += 1;
		}
		else {
			cnt_min = 0;
			cnt_max = 0;
		}

		if ((cnt_min > 255) || (cnt_max > 255)) {
			unsigned long j, n, v;

			n = (cnt_min > cnt_max) ? cnt_min : cnt_max;
			v = 0;

			for (j = 0; j < n; j++) {
				v += bit->clk[i - j];
			}

			clk = v / n;
			val = (65536ULL * clk) / trk->clock;

			if ((val > 64880) && (val < 66192)) {
				clk = trk->clock;
				val = 0;
			}

			pri_trk_evt_add (trk, PRI_EVENT_CLOCK, i - n + 1, val);

			clk_min = ((1000 - tolerance) * clk + 500) / 1000;
			clk_max = ((1000 + tolerance) * clk + 500) / 1000;

			cnt_min = 0;
			cnt_max = 0;

			have_clock = (val != 0);
		}
	}

	if (have_clock) {
		pri_trk_evt_add (trk, PRI_EVENT_CLOCK, 0, 0);
	}
}

static
int pfi_decode_pri_trk_cb (pfi_img_t *img, pfi_trk_t *strk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long       rate;
	pri_trk_t           *dtrk;
	struct decode_pri_s *par;
	pfi_dec_t           bit;

	par = opaque;

	if ((dtrk = pri_img_get_track (par->img, c, h, 1)) == NULL) {
		return (1);
	}

	if (pfi_dec_init (&bit, 0)) {
		return (1);
	}

	pfi_trk_rewind (strk);

	if (c < par->rate_cnt) {
		rate = par->rate[c];
	}
	else {
		rate = par->default_rate;
	}

	if (pfi_trk_decode_bits (strk, &bit, rate, par->revolution)) {
		pfi_dec_free (&bit);
		return (1);
	}

	switch (par->fold_mode) {
	case PFI_FOLD_MAXRUN:
		pfi_dec_fold_maxrun (&bit, par->fold_window, par->max_compare, c, h);
		break;

	case PFI_FOLD_MINDIFF:
		pfi_dec_fold_mindiff (&bit, par->fold_window, par->max_compare, c, h);
		break;

	default:
		bit.cnt = bit.index;
		break;
	}

	pri_trk_set_clock (dtrk, rate);
	pri_trk_set_size (dtrk, bit.index);

	memcpy (dtrk->data, bit.buf, (bit.index + 7) / 8);

	if (par_weak_bits) {
		pfi_decode_weak (dtrk, &bit, par_weak_i1, par_weak_i2);
	}

	pfi_decode_clock (dtrk, &bit, par_clock_tolerance);

	pfi_dec_free (&bit);

	pri_trk_clear_slack (dtrk);

	return (0);
}

static
pri_img_t *pfi_decode_pri (pfi_img_t *img, unsigned mode, unsigned long rate)
{
	unsigned            i;
	struct decode_pri_s par;
	unsigned long       rate_mac[80];

	static unsigned long rate_mac_490[] = {
		381132, 349371, 317610, 285849, 254088
	};

	static unsigned long rate_mac_500[] = {
		373205, 342104, 311004, 279904, 248803

	};

	if ((par.img = pri_img_new()) == NULL) {
		return (NULL);
	}

	par.default_rate = rate;
	par.revolution = par_revolution;

	if (mode == MODE_MAC_DD_490) {
		for (i = 0; i < 80; i++) {
			rate_mac[i] = rate_mac_490[i / 16];
		}

		par.rate_cnt = 80;
		par.rate = rate_mac;
	}
	else if (mode == MODE_MAC_DD_500) {
		for (i = 0; i < 80; i++) {
			rate_mac[i] = rate_mac_500[i / 16];
		}

		par.rate_cnt = 80;
		par.rate = rate_mac;
	}
	else {
		par.rate_cnt = 0;
		par.rate = NULL;
	}

	par.fold_mode = par_fold_mode;
	par.fold_window = par_fold_window;
	par.max_compare = par_fold_max;

	if (pfi_for_all_tracks (img, pfi_decode_pri_trk_cb, &par)) {
		pri_img_del (par.img);
		return (NULL);
	}

	if (img->comment_size > 0) {
		pri_img_set_comment (par.img, img->comment, img->comment_size);
	}

	return (par.img);
}

int pfi_decode_bits_pri (pfi_img_t *img, unsigned mode, unsigned long rate, const char *fname)
{
	int       r;
	FILE      *fp;
	pri_img_t *dimg;

	if ((dimg = pfi_decode_pri (img, mode, rate)) == NULL) {
		return (1);
	}

	if ((fp = fopen (fname, "wb")) == NULL) {
		pri_img_del (dimg);
		return (1);
	}

	r = pri_img_save_fp (fp, dimg, PRI_FORMAT_PRI);

	fclose (fp);

	pri_img_del (dimg);

	return (r);
}


int pfi_decode (pfi_img_t *img, const char *type, unsigned long rate, const char *fname)
{
	if (rate == 0) {
		rate = 500000;
	}
	else if (rate <= 1000) {
		rate *= 1000;
	}

	if (strcmp (type, "pri") == 0) {
		return (pfi_decode_bits_pri (img, MODE_NONE, rate, fname));
	}
	else if (strcmp (type, "pri-mac") == 0) {
		return (pfi_decode_bits_pri (img, MODE_MAC_DD_500, rate, fname));
	}
	else if (strcmp (type, "pri-mac-490") == 0) {
		return (pfi_decode_bits_pri (img, MODE_MAC_DD_490, rate, fname));
	}
	else if (strcmp (type, "pri-mac-500") == 0) {
		return (pfi_decode_bits_pri (img, MODE_MAC_DD_500, rate, fname));
	}
	else if (strcmp (type, "mfm-raw") == 0) {
		return (pfi_decode_bits (img, type, rate, fname));
	}
	else if (strcmp (type, "gcr-raw") == 0) {
		return (pfi_decode_bits (img, type, rate, fname));
	}
	else if (strcmp (type, "raw") == 0) {
		return (pfi_decode_bits (img, type, rate, fname));
	}
	else if (strcmp (type, "text") == 0) {
		return (pfi_decode_text (img, fname));
	}

	return (1);
}
