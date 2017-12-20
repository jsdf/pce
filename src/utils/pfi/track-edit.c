/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/track-edit.c                                   *
 * Created:     2013-12-27 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pfi/pfi.h>


static
int pfi_revolutions_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      rev1, rev2;
	unsigned long pos1, pos2, add1, add2;
	unsigned long pulse, index, clk1, clk2;
	pfi_trk_t     *dst;

	if (trk->idx_cnt < 2) {
		return (0);
	}

	rev1 = *((unsigned *) opaque + 0);
	rev2 = *((unsigned *) opaque + 1);

	if (rev1 < 1) {
		rev1 = 1;
	}

	if (rev2 > (trk->idx_cnt - 1)) {
		rev2 = trk->idx_cnt - 1;
	}

	if (rev2 < rev1) {
		return (1);
	}

	pos1 = trk->idx[rev1 - 1];
	pos2 = trk->idx[rev2];

	add1 = (par_slack1 * (trk->idx[rev1] - trk->idx[rev1 - 1])) / 100;
	add2 = (par_slack2 * (trk->idx[rev2] - trk->idx[rev2 - 1])) / 100;

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
		if ((clk1 >= pos1) && (clk1 < pos2)) {
			pfi_trk_add_pulse (dst, pulse);

			if ((pulse == 0) || (index < pulse)) {
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
	char          all, inv;
	unsigned long r1, r2;
	unsigned      revs[2];

	if (pfi_parse_range (str, &r1, &r2, &all, &inv)) {
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

	if (trk->idx_cnt < 2) {
		return (0);
	}

	clk = pfi_trk_get_clock (trk);
	len = trk->idx[trk->idx_cnt - 1] - trk->idx[0];

	rat = (60.0 * clk * (trk->idx_cnt - 1)) / (*rpm * len);
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
	unsigned long clk, len;
	double        rpm, rat;
	unsigned long mul, div;
	static double rpmtab[] = {
		393.38, 429.17, 472.14, 524.57, 590.11
	};

	if (trk->idx_cnt < 2) {
		return (0);
	}

	rpm = rpmtab[(c < 80) ? (c / 16) : 4];

	clk = pfi_trk_get_clock (trk);
	len = trk->idx[trk->idx_cnt - 1] - trk->idx[0];

	rat = (60.0 * clk * (trk->idx_cnt - 1)) / (rpm * len);
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

int pfi_set_rpm_mac (pfi_img_t *img)
{
	return (pfi_for_all_tracks (img, pfi_set_rpm_mac_cb, NULL));
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
