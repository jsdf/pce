/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/reorder.c                                      *
 * Created:     2013-06-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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
#include "reorder.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


static unsigned      par_order_cnt = 0;
static unsigned char par_order[256];


static
int psi_interleave_track_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned *il;

	par_cnt += 1;

	il = opaque;

	if (psi_trk_interleave (trk, *il)) {
		return (1);
	}

	return (0);
}

int psi_interleave_tracks (psi_img_t *img, unsigned il)
{
	int r;

	par_cnt = 0;

	r = psi_for_all_tracks (img, psi_interleave_track_cb, &il);

	if (par_verbose) {
		fprintf (stderr, "%s: interleave %lu tracks\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: interleaving failed\n", arg0);
	}

	return (r);
}


static
int psi_reorder_track_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned  i, j;
	psi_sct_t *tmp;

	par_cnt += 1;

	for (i = 0; i < par_order_cnt; i++) {
		if (i >= trk->sct_cnt) {
			return (0);
		}

		for (j = i; j < trk->sct_cnt; j++) {
			if (trk->sct[j]->s == par_order[i]) {
				break;
			}
		}

		if ((j < trk->sct_cnt) && (i != j)) {
			tmp = trk->sct[i];
			trk->sct[i] = trk->sct[j];
			trk->sct[j] = tmp;
		}
	}

	return (0);
}

int psi_reorder_tracks (psi_img_t *img, const char *order)
{
	int      r;
	unsigned i;
	unsigned val;

	i = 0;

	while (i < 256) {
		if (*order == 0) {
			break;
		}

		if (psi_parse_int_list (&order, &val)) {
			return (1);
		}

		par_order[i] = val;

		i += 1;
	}

	par_order_cnt = i;

	par_cnt = 0;

	r = psi_for_all_tracks (img, psi_reorder_track_cb, NULL);

	if (par_verbose) {
		fprintf (stderr, "%s: reorder %lu tracks\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: reordering failed\n", arg0);
	}

	return (r);
}


static
int psi_rotate_track_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned  i, i0;
	unsigned  first;
	psi_sct_t *sct;

	if (trk->sct_cnt == 0) {
		return (0);
	}

	par_cnt += 1;

	first = *(unsigned *) opaque;

	i0 = 0;

	for (i = 0; i < trk->sct_cnt; i++) {
		sct = trk->sct[i];

		if (sct->s == first) {
			i0 = i;
			break;
		}

		if ((sct->s >= first) && (sct->s < trk->sct[i0]->s)) {
			i0 = i;
		}
	}

	while (i0 > 0) {
		sct = trk->sct[0];

		for (i = 1; i < trk->sct_cnt; i++) {
			trk->sct[i - 1] = trk->sct[i];
		}

		trk->sct[trk->sct_cnt - 1] = sct;

		i0 -= 1;
	}

	return (0);
}

int psi_rotate_tracks (psi_img_t *img, unsigned first)
{
	int r;

	par_cnt = 0;

	r = psi_for_all_tracks (img, psi_rotate_track_cb, &first);

	if (par_verbose) {
		fprintf (stderr, "%s: rotate %lu tracks\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: rotating failed\n", arg0);
	}

	return (r);
}


static
int psi_sort_track_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	int       v, *rev;
	unsigned  i, j;
	psi_sct_t *tmp;

	par_cnt += 1;

	rev = opaque;

	for (i = 1; i < trk->sct_cnt; i++) {
		j = i;

		tmp = trk->sct[i];

		while (j > 0) {
			v = (tmp->s < trk->sct[j - 1]->s);

			if (*rev ? v : !v) {
				break;
			}

			trk->sct[j] = trk->sct[j - 1];

			j -= 1;
		}

		trk->sct[j] = tmp;
	}

	return (0);
}

int psi_sort_tracks (psi_img_t *img, int reverse)
{
	int r;

	par_cnt = 0;

	r = psi_for_all_tracks (img, psi_sort_track_cb, &reverse);

	if (par_verbose) {
		fprintf (stderr, "%s: sort %lu tracks\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: sort failed\n", arg0);
	}

	return (r);
}
