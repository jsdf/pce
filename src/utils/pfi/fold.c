/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/fold.c                                         *
 * Created:     2019-06-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2019-2021 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/pfi/pfi.h>
#include <drivers/pfi/track.h>


typedef struct {
	pfi_trk_t     *trk;

	unsigned long c;
	unsigned long h;

	unsigned long wdw;
	unsigned long cnt;

	char          only_right;

	unsigned      rev1;
	unsigned      rev2;

	unsigned long idx0pos;
	unsigned long idx0rem;

	unsigned long idx1pos;
	unsigned long idx1rem;
} pfi_fold_t;


static
int fold_get_index_pos (pfi_fold_t *fold, unsigned rev)
{
	unsigned long i, n;
	unsigned long idx0, idx1, clk;
	unsigned      done;
	uint32_t      *pulse;

	if ((rev < 1) || (rev >= fold->trk->index_cnt)) {
		return (1);
	}

	idx0 = fold->trk->index[rev - 1];
	idx1 = fold->trk->index[rev];

	n = fold->trk->pulse_cnt;
	pulse = fold->trk->pulse;

	clk = 0;

	done = 0;

	for (i = 0; i < n; i++) {
		if ((clk <= idx0) && ((clk + pulse[i]) > idx0)) {
			fold->idx0pos = i;
			fold->idx0rem = idx0 - clk;
			done |= 1;

			if (done == 3) {
				return (0);
			}
		}

		if ((clk <= idx1) && ((clk + pulse[i]) > idx1)) {
			fold->idx1pos = i;
			fold->idx1rem = idx1 - clk;
			done |= 2;

			if (done == 3) {
				return (0);
			}
		}

		clk += pulse[i];
	}

	return (1);
}

static
void fold_set_index (pfi_fold_t *fold, unsigned idx, unsigned long pos, unsigned long rem)
{
	unsigned long i;
	unsigned long clk, clk0, diff;
	int           sub;
	uint32_t      *pulse;

	if (idx >= fold->trk->index_cnt) {
		return;
	}

	if (pos >= fold->trk->pulse_cnt) {
		pos = fold->trk->pulse_cnt;
	}

	pulse = fold->trk->pulse;

	clk0 = (idx > 0) ? fold->trk->index[idx - 1] : 0;

	clk = rem;

	for (i = 0; i < pos; i++) {
		clk += pulse[i];
	}

	if (clk < fold->trk->index[idx]) {
		diff = fold->trk->index[idx] - clk;
		sub = 1;
	}
	else {
		diff = clk - fold->trk->index[idx];
		sub = 0;
	}

	if (par_verbose) {
		fprintf (stderr, "track %lu/%lu index %u %lu (%c%lu)\n",
			fold->c, fold->h,
			idx,
			clk - clk0,
			sub ? '-' : '+',
			diff
		);
	}

	for (i = idx; i < fold->trk->index_cnt; i++) {
		if (sub) {
			fold->trk->index[i] -= diff;
		}
		else {
			fold->trk->index[i] += diff;
		}
	}
}

static
unsigned long fold_get_diff (const uint32_t *p1, const uint32_t *p2, unsigned long cnt)
{
	unsigned long i;
	unsigned long val, tmp, dif;

	val = 0;

	for (i = 0; i < cnt; i++) {
		dif = (*p1 < *p2) ? (*p2 - *p1) : (*p1 - *p2);

		tmp = val;
		val += dif * dif;

		if (val < tmp) {
			return (-1);
		}

		p1 += 1;
		p2 += 1;
	}

	return (val);
}

static
int fold_revolution (pfi_fold_t *fold, unsigned rev)
{
	unsigned long i, n;
	unsigned long p1, p2;
	unsigned long w1, w2;
	unsigned long val, opt_val, opt_pos;
	pfi_trk_t     *trk;

	trk = fold->trk;

	if (fold_get_index_pos (fold, rev)) {
		return (1);
	}

	if (fold->idx1pos < fold->wdw / 2) {
		w1 = 0;
	}
	else {
		w1 = fold->idx1pos - fold->wdw / 2;
	}

	if ((fold->idx1pos + fold->wdw / 2) >= trk->pulse_cnt) {
		w2 = trk->pulse_cnt - 1;
	}
	else {
		w2 = fold->idx1pos + fold->wdw / 2;
	}

	opt_val = -1;
	opt_pos = 0;

	for (i = w1; i <= w2; i++) {
		p1 = fold->idx0pos;
		p2 = i;
		n = fold->cnt;

		if (fold->only_right == 0) {
			if (p1 >= (fold->cnt / 2)) {
				p2 -= fold->cnt / 2;
				p1 -= fold->cnt / 2;
			}
			else {
				p2 -= p1;
				p1 = 0;
			}
		}

		if ((p2 + n) >= fold->trk->pulse_cnt) {
			unsigned long k;

			k = p2 + n - fold->trk->pulse_cnt;

			if (p1 < k) {
				p2 -= p1;
				p1 = 0;
				fprintf (stderr, "%lu/%lu: fold overrun (rev=%u)\n",
					fold->c, fold->h, rev
				);
				return (1);
			}
			else {
				p1 -= k;
				p2 -= k;
			}

			n = fold->trk->pulse_cnt - p2;
		}

		val = fold_get_diff (trk->pulse + p1, trk->pulse + p2, n);

		if (val < opt_val) {
			opt_val = val;
			opt_pos = i;
		}
	}

	if (opt_pos != 0) {
		if (opt_val > (64 * fold->cnt)) {
			fprintf (stderr, "%lu/%lu: POS=%lu DIF=%lu AVG=%lu\n",
				fold->c, fold->h,
				opt_pos, opt_val, opt_val / fold->cnt
			);
		}

		fold_set_index (fold, rev, opt_pos, fold->idx0rem);
	}

	return (0);
}

static
int fold_track (pfi_fold_t *fold)
{
	unsigned r, r1, r2;

	if (fold->trk->index_cnt < 2) {
		return (0);
	}

	r1 = fold->rev1;
	r2 = fold->rev2;

	if (r1 < 1) {
		r1 = 1;
	}

	if (r2 >= fold->trk->index_cnt) {
		r2 = fold->trk->index_cnt - 1;
	}

	for (r = r1; r <= r2; r++) {
		fold_revolution (fold, r);
	}

	return (0);
}

static
int pfi_fold_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pfi_fold_t *fold;

	fold = opaque;

	fold->trk = trk;
	fold->c = c;
	fold->h = h;

	fold_track (fold);

	return (0);
}

int pfi_fold (pfi_img_t *img)
{
	pfi_fold_t fold;

	fold.only_right = par_pfi_fold_right;

	if (par_pfi_fold_revolution == 0) {
		fold.rev1 = 1;
		fold.rev2 = -1;
	}
	else {
		fold.rev1 = par_pfi_fold_revolution;
		fold.rev2 = par_pfi_fold_revolution;
	}

	fold.wdw = par_pfi_fold_window;
	fold.cnt = par_pfi_fold_compare;

	return (pfi_for_all_tracks (img, pfi_fold_cb, &fold));
}
