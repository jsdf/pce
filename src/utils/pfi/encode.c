/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/encode.c                                       *
 * Created:     2014-01-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2022 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pfi/pfi.h>
#include <drivers/pfi/decode-bits.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>


static
int pfi_encode_track (pfi_trk_t *dtrk, pri_trk_t *strk, unsigned c, unsigned h)
{
	unsigned long       i, j, n;
	unsigned long       dclk, sclk;
	unsigned long       dval, sval, total;
	unsigned long       s1, s2;
	const unsigned char *s;

	pfi_trk_reset (dtrk);
	pfi_trk_set_clock (dtrk, par_pfi_clock);

	sclk = strk->clock;
	dclk = dtrk->clock;

	sval = 0;
	dval = 0;

	total = 0;

	s1 = (unsigned long) (par_slack1 * strk->clock + 0.5);
	s2 = (unsigned long) (par_slack2 * strk->clock + 0.5);

	j = (strk->size - s1 % strk->size) % strk->size;
	n = par_revolution * strk->size;
	n += s1 + s2;
	s = strk->data;

	for (i = 0; i < n; i++) {
		sval += dclk;
		dval += sval / sclk;
		sval %= sclk;

		if (s[j >> 3] & (0x80 >> (j & 7))) {
			if (pfi_trk_add_pulse (dtrk, dval)) {
				return (1);
			}

			total += dval;
			dval = 0;
		}

		j += 1;

		if (j >= strk->size) {
			pfi_trk_add_index (dtrk, total + dval);
			j = 0;
		}
	}

	return (0);
}

static
int pfi_encode_pri (pfi_img_t *dimg, const char *fname)
{
	unsigned  c, h;
	pri_img_t *simg;
	pri_cyl_t *cyl;
	pri_trk_t *strk;
	pfi_trk_t *dtrk;

	if ((simg = pri_img_load (fname, PRI_FORMAT_NONE)) == NULL) {
		return (1);
	}

	for (c = 0; c < simg->cyl_cnt; c++) {
		if ((cyl = simg->cyl[c]) == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			if ((strk = cyl->trk[h]) == NULL) {
				continue;
			}

			dtrk = pfi_img_get_track (dimg, c, h, 1);

			if (dtrk == NULL) {
				return (1);
			}

			if (pfi_encode_track (dtrk, strk, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

int pfi_encode (pfi_img_t *img, const char *type, const char *fname)
{
	if (strcmp (type, "pri") == 0) {
		return (pfi_encode_pri (img, fname));
	}
	else if (strcmp (type, "text") == 0) {
		return (pfi_encode_text (img, fname));
	}

	return (1);
}
