/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/track-edit.c                                   *
 * Created:     2013-12-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2021 Hampa Hug <hampa@hampa.ch>                     *
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


static
int pfi_quantize_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      idx;
	unsigned      cnt;
	unsigned long i;
	unsigned long rate, cell;
	uint32_t      val1, val2;
	uint32_t      total1, total2, index;

	rate = *(unsigned long *) opaque;

	cell = (trk->clock + rate / 2) / rate;

	idx = 0;
	total1 = 0;
	total2 = 0;

	index = (trk->index_cnt > 0) ? trk->index[0] : 0;

	for (i = 0; i < trk->pulse_cnt; i++) {
		val1 = trk->pulse[i];

		if ((idx < trk->index_cnt) && (total1 <= index) && ((total1 + val1) > index)) {
			trk->index[idx] = (trk->index[idx] - total1) + total2;
			idx += 1;
			index = (idx < trk->index_cnt) ? trk->index[idx] : 0;
		}

		cnt = (val1 + cell / 2) / cell;

		if (cnt == 0) {
			cnt = 1;
		}

		val2 = cnt * cell;

		total1 += val1;
		total2 += val2;

		trk->pulse[i] = val2;
	}

	return (0);
}

int pfi_quantize (pfi_img_t *img, unsigned long rate)
{
	return (pfi_for_all_tracks (img, pfi_quantize_cb, &rate));
}


static
int pfi_revolutions_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      rev1, rev2;
	unsigned long pos1, pos2, add1, add2;
	unsigned long clk1, clk2;
	uint32_t      pulse, index;
	pfi_trk_t     *dst;

	if (trk->index_cnt < 2) {
		return (0);
	}

	rev1 = *((unsigned *) opaque + 0);
	rev2 = *((unsigned *) opaque + 1);

	if (rev1 < 1) {
		rev1 = 1;
	}

	if (rev2 > (trk->index_cnt - 1)) {
		rev2 = trk->index_cnt - 1;
	}

	if (rev2 < rev1) {
		return (1);
	}

	pos1 = trk->index[rev1 - 1];
	pos2 = trk->index[rev2];

	add1 = (unsigned long) (par_slack1 * trk->clock + 0.5);
	add2 = (unsigned long) (par_slack2 * trk->clock + 0.5);

	pos1 = (pos1 < add1) ? 0 : (pos1 - add1);
	pos2 = pos2 + add2;

	if ((dst = pfi_trk_new()) == NULL) {
		return (1);
	}

	pfi_trk_set_clock (dst, pfi_trk_get_clock (trk));

	pfi_trk_rewind (trk);

	clk1 = 0;
	clk2 = 0;

	while ((clk1 < pos2) && (pfi_trk_get_pulse (trk, &pulse, &index) == 0)) {
		if (pulse == 0) {
			pfi_trk_add_index (dst, clk2 + index);
			break;
		}

		if (((clk1 + pulse) > pos1) && (clk1 < pos2)) {
			pfi_trk_add_pulse (dst, pulse);

			if (index < pulse) {
				pfi_trk_add_index (dst, clk2 + index);
			}

			clk2 += pulse;
		}

		clk1 += pulse;
	}

	pfi_img_set_track (img, dst, c, h);

	return (0);
}

int pfi_revolutions (pfi_img_t *img, const char *str)
{
	char          all;
	unsigned long r1, r2;
	unsigned      revs[2];

	if (pfi_parse_range (str, &r1, &r2, &all)) {
		return (1);
	}

	if (all) {
		r1 = 0;
		r2 = -1;
	}

	revs[0] = r1;
	revs[1] = r2;

	return (pfi_for_all_tracks (img, pfi_revolutions_cb, revs));
}

int pfi_slack (pfi_img_t *img, const char *str)
{
	unsigned revs[2];

	if (pfi_parse_time (str, &par_slack1, 0.001)) {
		return (1);
	}

	par_slack2 = par_slack1;

	revs[0] = 0;
	revs[1] = -1;

	return (pfi_for_all_tracks (img, pfi_revolutions_cb, revs));
}


static
int pfi_scale_track_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long mul, div;
	double        *factor;

	factor = opaque;

	mul = (unsigned long) (16.0 * 65536.0 * *factor);
	div = 16UL * 65536;

	trk = pfi_trk_scale (trk, mul, div);

	if (trk == NULL) {
		return (1);
	}

	if (pfi_img_set_track (img, trk, c, h)) {
		pfi_trk_del (trk);
		return (1);
	}

	return (0);
}

int pfi_scale_tracks (pfi_img_t *img, double factor)
{
	return (pfi_for_all_tracks (img, pfi_scale_track_cb, &factor));
}


static
int pfi_set_clock_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long *clknew;
	unsigned long clkold;

	clknew = opaque;
	clkold = pfi_trk_get_clock (trk);

	if (clkold == *clknew) {
		return (0);
	}

	trk = pfi_trk_scale (trk, *clknew, clkold);

	if (trk == NULL) {
		return (1);
	}

	pfi_trk_set_clock (trk, *clknew);

	if (pfi_img_set_track (img, trk, c, h)) {
		pfi_trk_del (trk);
		return (1);
	}

	return (0);
}

int pfi_set_clock (pfi_img_t *img, unsigned long clock)
{
	return (pfi_for_all_tracks (img, pfi_set_clock_cb, &clock));
}


static
int pfi_set_rpm_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long clk, len;
	double        *rpm, rat;
	unsigned long mul, div;

	rpm = opaque;

	if (trk->index_cnt < 2) {
		return (0);
	}

	clk = pfi_trk_get_clock (trk);
	len = trk->index[trk->index_cnt - 1] - trk->index[0];

	rat = (60.0 * clk * (trk->index_cnt - 1)) / (*rpm * len);
	mul = (unsigned long) (16.0 * 65536.0 * rat);
	div = 16UL * 65536;

	if (mul == div) {
		return (0);
	}

	trk = pfi_trk_scale (trk, mul, div);

	if (trk == NULL) {
		return (1);
	}

	if (pfi_img_set_track (img, trk, c, h)) {
		pfi_trk_del (trk);
		return (1);
	}

	return (0);
}

int pfi_set_rpm (pfi_img_t *img, double rpm)
{
	return (pfi_for_all_tracks (img, pfi_set_rpm_cb, &rpm));
}


static
int pfi_set_rpm_mac_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      speed;
	unsigned long clk, len;
	double        rpm, rat;
	unsigned long mul, div;

	static double rpmtab_490[] = {
		393.5641, 429.3426, 472.2769, 524.7521, 590.3461
	};

	static double rpmtab_500[] = {
		401.9241, 438.4626, 482.3089, 535.8988, 602.8861
	};

	if (trk->index_cnt < 2) {
		return (0);
	}

	speed = *(unsigned *) opaque;

	if (speed == 1) {
		rpm = rpmtab_490[(c < 80) ? (c / 16) : 4];
	}
	else {
		rpm = rpmtab_500[(c < 80) ? (c / 16) : 4];
	}

	clk = pfi_trk_get_clock (trk);
	len = trk->index[trk->index_cnt - 1] - trk->index[0];

	rat = (60.0 * clk * (trk->index_cnt - 1)) / (rpm * len);
	mul = (unsigned long) (16.0 * 65536.0 * rat);
	div = 16UL * 65536;

	if (mul == div) {
		return (0);
	}

	trk = pfi_trk_scale (trk, mul, div);

	if (trk == NULL) {
		return (1);
	}

	if (pfi_img_set_track (img, trk, c, h)) {
		pfi_trk_del (trk);
		return (1);
	}

	return (0);
}

int pfi_set_rpm_mac_490 (pfi_img_t *img)
{
	unsigned speed;

	speed = 1;

	return (pfi_for_all_tracks (img, pfi_set_rpm_mac_cb, &speed));
}

int pfi_set_rpm_mac_500 (pfi_img_t *img)
{
	unsigned speed;

	speed = 2;

	return (pfi_for_all_tracks (img, pfi_set_rpm_mac_cb, &speed));
}


static
int pfi_shift_index_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pfi_trk_shift_index (trk, *(long *) opaque);

	return (0);
}

int pfi_shift_index (pfi_img_t *img, long ofs)
{
	return (pfi_for_all_tracks (img, pfi_shift_index_cb, &ofs));
}


static
int pfi_shift_index_us_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	long us, ofs;

	us = *(long *) opaque;

	if (us == 0) {
		return (0);
	}

	ofs = (us * (long long) pfi_trk_get_clock (trk)) / 1000000;

	pfi_trk_shift_index (trk, ofs);

	return (0);
}

int pfi_shift_index_us (pfi_img_t *img, long us)
{
	return (pfi_for_all_tracks (img, pfi_shift_index_us_cb, &us));
}


static
int pfi_wpcom_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long i, n;
	uint32_t      p1, p2, p3;
	uint32_t      dif;

	if (trk->pulse_cnt < 2) {
		return (0);
	}

	n = trk->pulse_cnt - 1;

	for (i = 1; i < n; i++) {
		p1 = trk->pulse[i - 1];
		p2 = trk->pulse[i];
		p3 = trk->pulse[i + 1];

		if ((2 * p2) >= (p1 + p3)) {
			continue;
		}

		dif = (p1 + p3 - 2 * p2) / 8;

		if (dif >= p2) {
			continue;
		}

		trk->pulse[i - 1] += dif / 2;
		trk->pulse[i] -= dif;
		trk->pulse[i + 1] += (dif + 1) / 2;
	}

	return (0);
}

int pfi_wpcom (pfi_img_t *img)
{
	return (pfi_for_all_tracks (img, pfi_wpcom_cb, NULL));
}
