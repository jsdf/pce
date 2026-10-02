/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-cas.c                                *
 * Created:     2020-04-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pti.h"
#include "pti-io.h"
#include "pti-img-cas.h"


static unsigned long cas_default_clock = 1193182;


void pti_cas_set_default_clock (unsigned long val)
{
	if (val > 0) {
		cas_default_clock = val;
	}
}

static
int cas_load_img (FILE *fp, pti_img_t *img)
{
	unsigned      i;
	int           c;
	unsigned char val;
	unsigned long frq, clk;

	while ((c = fgetc (fp)) != EOF) {
		val = c & 0xff;

		for (i = 0; i < 8; i++) {
			frq = (val & 0x80) ? 1000 : 2000;
			clk = img->clock / frq;

			if (pti_img_add_pulse (img, clk / 2, -1)) {
				return (1);
			}

			if (pti_img_add_pulse (img, clk - clk / 2, 1)) {
				return (1);
			}

			val <<= 1;
		}
	}

	return (0);
}

pti_img_t *pti_load_cas (FILE *fp, unsigned long clock)
{
	pti_img_t *img;

	if ((img = pti_img_new()) == NULL) {
		return (NULL);
	}

	if (clock == 0) {
		clock = cas_default_clock;
	}

	pti_img_set_clock (img, clock);

	if (cas_load_img (fp, img)) {
		pti_img_del (img);
		return (NULL);
	}

	pti_img_clean (img);

	return (img);
}


int pti_save_cas (FILE *fp, const pti_img_t *img)
{
	unsigned long i;
	unsigned long clk, sum, p0, p1;
	unsigned      cnt, buf;
	int           level;

	p0 = img->clock / 2000;
	p1 = img->clock / 1000;

	cnt = 0;
	buf = 0;
	sum = 0;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		clk += sum;
		sum = 0;

		if ((level == 0) || (clk > (2 * p1))) {
			while (clk >= p0) {
				buf <<= 1;
				clk -= p0;

				if (++cnt >= 8) {
					fputc (buf, fp);
					buf = 0;
					cnt = 0;
				}
			}
		}
		else if (level < 0) {
			sum = clk;
		}
		else if (level > 0) {
			if (clk < ((p0 + p1) / 2)) {
				buf = (buf << 1) | 0;
			}
			else {
				buf = (buf << 1) | 1;
			}

			if (++cnt >= 8) {
				fputc (buf, fp);
				buf = 0;
				cnt = 0;
			}
		}
	}

	if (cnt > 0) {
		fputc (buf << (8 - cnt), fp);
	}

	return (0);
}


int pti_probe_cas_fp (FILE *fp)
{
	return (0);
}

int pti_probe_cas (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pti_probe_cas_fp (fp);

	fclose (fp);

	return (r);
}
