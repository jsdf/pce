/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/info.c                                         *
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
				idx = trk->index_cnt;
				clk = pfi_trk_get_clock (trk);
				len = (trk->index_cnt < 2) ? 0 : (trk->index[1] - trk->index[0]);
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
	unsigned long len, clk, clock;
	double        rpm;

	if (trk->pulse_cnt == 0) {
		return (0);
	}

	verb = opaque;

	clock = pfi_trk_get_clock (trk);

	if (trk->index_cnt >= 2) {
		len = trk->index[trk->index_cnt - 1] - trk->index[0];
		rpm = (60.0 * (trk->index_cnt - 1) * clock) / len;
	}
	else {
		len = 0;
		rpm = 0.0;
	}

	printf ("TRACK %2lu/%lu: PULSE=%6lu  CLK=%lu  LEN=%lu  IDX=%u  RPM=%.4f\n",
		c, h, trk->pulse_cnt, clock, len, trk->index_cnt, rpm
	);

	if (*verb == 0) {
		return (0);
	}

	if (trk->index_cnt > 0) {
		printf ("\t    [%8lu, %8lu]  T=%7.3f  LEN=%lu\n",
			0UL,
			(unsigned long) trk->index[0],
			(1000.0 * trk->index[0]) / clock,
			(unsigned long) trk->index[0]
		);
	}

	for (i = 1; i < trk->index_cnt; i++) {
		len = trk->index[i] - trk->index[i - 1];
		rpm = (60.0 * clock) / len;

		printf ("\tR%u: [%8lu, %8lu]  T=%7.3f  LEN=%lu  RPM=%.4f\n",
			i,
			(unsigned long) trk->index[i - 1],
			(unsigned long) trk->index[i],
			(1000.0 * len) / clock,
			len, rpm
		);
	}

	if (trk->index_cnt > 0) {
		clk = pfi_trk_get_clk (trk, trk->pulse_cnt);

		if (clk < trk->index[trk->index_cnt - 1]) {
			len = 0;
		}
		else {
			len = clk - trk->index[trk->index_cnt - 1];
		}

		printf ("\t    [%8lu, %8lu]  T=%7.3f  LEN=%lu\n",
			(unsigned long) trk->index[trk->index_cnt - 1],
			clk,
			(1000.0 * len) / clock,
			len
		);
	}

	return (0);
}

int pfi_list_tracks (pfi_img_t *img, int verb)
{
	return (pfi_for_all_tracks (img, pfi_list_track_cb, &verb));
}
