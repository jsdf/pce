/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/info.c                                         *
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
void pfi_print_range (const char *str1, unsigned long v1, unsigned long v2, const char *str2)
{
	fputs (str1, stdout);

	if (v1 == v2) {
		printf ("%lu", v1);
	}
	else {
		printf ("%lu - %lu", v1, v2);
	}

	fputs (str2, stdout);
}

static
void pfi_print_range_float (const char *str1, double v1, double v2, const char *str2)
{
	fputs (str1, stdout);

	if (v1 == v2) {
		printf ("%.4f", v1);
	}
	else {
		printf ("%.4f - %.4f", v1, v2);
	}

	fputs (str2, stdout);
}

int pfi_print_info (pfi_img_t *img)
{
	unsigned long c, h, cn, hn, tn;
	unsigned long h1, h2;
	unsigned long len;
	unsigned      idx, idx1, idx2;
	unsigned long clk, clk1, clk2;
	double        rpm, rpm1, rpm2;
	pfi_cyl_t     *cyl;
	pfi_trk_t     *trk;

	cn = pfi_img_get_cyl_cnt (img);
	tn = 0;

	h1 = 0;
	h2 = 0;

	idx1 = 0;
	idx2 = 0;

	clk1 = 0;
	clk2 = 0;

	rpm1 = 0.0;
	rpm2 = 0.0;

	for (c = 0; c < cn; c++) {
		cyl = pfi_img_get_cylinder (img, c, 0);

		if (cyl == NULL) {
			hn = 0;
		}
		else {
			hn = pfi_cyl_get_trk_cnt (cyl);
		}

		h1 = ((c == 0) || (hn < h1)) ? hn : h1;
		h2 = ((c == 0) || (hn > h2)) ? hn : h2;

		if (cyl == NULL) {
			continue;
		}

		for (h = 0; h < hn; h++) {
			trk = pfi_cyl_get_track (cyl, h, 0);

			if (trk == NULL) {
				idx = 0;
				clk = 0;
				len = 0;
			}
			else {
				idx = trk->idx_cnt;
				clk = pfi_trk_get_clock (trk);
				len = (trk->idx_cnt < 2) ? 0 : (trk->idx[1] - trk->idx[0]);
			}

			if (len > 0) {
				rpm = (60.0 * clk) / len;
			}
			else {
				rpm = 0.0;
			}

			idx1 = ((tn == 0) || (idx < idx1)) ? idx : idx1;
			idx2 = ((tn == 0) || (idx > idx2)) ? idx : idx2;

			clk1 = ((tn == 0) || (clk < clk1)) ? clk : clk1;
			clk2 = ((tn == 0) || (clk > clk2)) ? clk : clk2;

			rpm1 = ((tn == 0) || (rpm < rpm1)) ? rpm : rpm1;
			rpm2 = ((tn == 0) || (rpm > rpm2)) ? rpm : rpm2;

			tn += 1;
		}
	}

	printf ("cylinders: %lu\n", cn);
	pfi_print_range ("heads:     ", h1, h2, "\n");
	printf ("tracks:    %lu\n", tn);
	pfi_print_range ("clock:     ", clk1, clk2, "\n");
	pfi_print_range ("index:     ", idx1, idx2, "\n");
	pfi_print_range_float ("rpm:       ", rpm1, rpm2, "\n");

	if (img->comment_size > 0) {
		fputs ("\n", stdout);
		pfi_comment_show (img);
	}

	return (0);
}


static
int pfi_list_track_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      i;
	int           *verb;
	unsigned long len, clock;
	double        rpm;

	if (trk->size == 0) {
		return (0);
	}

	verb = opaque;

	clock = pfi_trk_get_clock (trk);

	if (trk->idx_cnt >= 2) {
		len = trk->idx[trk->idx_cnt - 1] - trk->idx[0];
		rpm = (60.0 * (trk->idx_cnt - 1) * clock) / len;
	}
	else {
		len = 0;
		rpm = 0.0;
	}

	printf ("TRACK %2lu/%lu: DATA=%6lu  CLK=%lu  LEN=%lu  IDX=%u  RPM=%.4f\n",
		c, h, trk->size, clock, len, trk->idx_cnt, rpm
	);

	if (*verb) {
		for (i = 1; i < trk->idx_cnt; i++) {
			len = trk->idx[i] - trk->idx[i - 1];
			rpm = (60.0 * clock) / len;

			printf ("\t\tR%u: IDX=[%8lu, %8lu]  LEN=%lu  RPM=%.4f\n",
				i, trk->idx[i - 1], trk->idx[i], len, rpm
			);
		}
	}

	return (0);
}

int pfi_list_tracks (pfi_img_t *img, int verb)
{
	return (pfi_for_all_tracks (img, pfi_list_track_cb, &verb));
}
