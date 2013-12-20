/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/info.c                                         *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "comment.h"
#include "info.h"

#include <stdio.h>
#include <string.h>

#include <drivers/pri/pri.h>


static
void pri_print_range (const char *str1, unsigned long v1, unsigned long v2, const char *str2)
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
void pri_print_range_float (const char *str1, double v1, double v2, const char *str2)
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

int pri_print_info (pri_img_t *img)
{
	unsigned long c, h, cn, hn, tn;
	unsigned long h1, h2;
	unsigned long len;
	unsigned long clk, clk1, clk2;
	double        rpm, rpm1, rpm2;
	pri_cyl_t     *cyl;
	pri_trk_t     *trk;

	cn = pri_img_get_cyl_cnt (img);
	tn = 0;

	h1 = 0;
	h2 = 0;

	clk1 = 0;
	clk2 = 0;

	rpm1 = 0.0;
	rpm2 = 0.0;

	for (c = 0; c < cn; c++) {
		cyl = pri_img_get_cylinder (img, c, 0);

		if (cyl == NULL) {
			hn = 0;
		}
		else {
			hn = pri_cyl_get_trk_cnt (cyl);
		}

		h1 = ((c == 0) || (hn < h1)) ? hn : h1;
		h2 = ((c == 0) || (hn > h2)) ? hn : h2;

		if (cyl == NULL) {
			continue;
		}

		for (h = 0; h < hn; h++) {
			trk = pri_cyl_get_track (cyl, h, 0);

			if (trk == NULL) {
				clk = 0;
				len = 0;
			}
			else {
				clk = pri_trk_get_clock (trk);
				len = pri_trk_get_size (trk);
			}

			if (len > 0) {
				rpm = (60.0 * clk) / len;
			}
			else {
				rpm = 0.0;
			}

			clk1 = ((tn == 0) || (clk < clk1)) ? clk : clk1;
			clk2 = ((tn == 0) || (clk > clk2)) ? clk : clk2;

			rpm1 = ((tn == 0) || (rpm < rpm1)) ? rpm : rpm1;
			rpm2 = ((tn == 0) || (rpm > rpm2)) ? rpm : rpm2;

			tn += 1;
		}
	}

	printf ("cylinders: %lu\n", cn);
	pri_print_range ("heads:     ", h1, h2, "\n");
	printf ("tracks:    %lu\n", tn);
	pri_print_range ("clock:     ", clk1, clk2, "\n");
	pri_print_range_float ("rpm:       ", rpm1, rpm2, "\n");

	if (img->comment_size > 0) {
		fputs ("\n", stdout);
		pri_comment_show (img);
	}

	return (0);
}
