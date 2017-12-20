/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/encode.c                                       *
 * Created:     2014-01-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2017 Hampa Hug <hampa@hampa.ch>                     *
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


static
int pfi_encode_track (pfi_trk_t *dtrk, pri_trk_t *strk, unsigned c, unsigned h)
{
	unsigned long       i, j, n;
	unsigned long       dclk, sclk;
	unsigned long       val, rem, cnt, tmp;
	const unsigned char *s;

	pfi_trk_reset (dtrk);
	pfi_trk_set_clock (dtrk, par_pfi_clock);

	sclk = strk->clock;
	dclk = dtrk->clock;

	j = ((100 - (par_slack1 % 100)) * strk->size) / 100;
	n = par_revolution * strk->size;
	n += ((par_slack1 + par_slack2) * strk->size + 99) / 100;
	s = strk->data;

	cnt = 0;
	val = 0;
	rem = 0;

	for (i = 0; i < n; i++) {
		val += 1;

		if (s[j >> 3] & (0x80 >> (j & 7))) {
			val = dclk * val + rem;
			rem = val % sclk;
			val = val / sclk;

			if (pfi_trk_add_pulse (dtrk, val)) {
				return (1);
			}

			cnt += val;
			val = 0;
		}

		j += 1;

		if (j >= strk->size) {
			tmp = (dclk * val + rem) / sclk;
			pfi_trk_add_index (dtrk, cnt + tmp);
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

	return (1);
}
