/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-ana.c                                *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psi.h"
#include "psi-img-ana.h"


static
int ana_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned      c, h;
	unsigned      cnt;
	size_t        r;
	psi_sct_t     *sct;
	unsigned char buf[8];

	while (1) {
		r = fread (buf, 1, 8, fp);

		if (r == 0) {
			return (0);
		}

		if (r != 8) {
			return (1);
		}

		c = buf[0];
		h = buf[1];

		cnt = (buf[7] << 8) | buf[6];

		sct = psi_sct_new (buf[2], buf[3], buf[4], cnt);

		if (sct == NULL) {
			return (1);
		}

		psi_sct_set_mfm_size (sct, buf[5]);

		if (psi_img_add_sector (img, sct, c, h)) {
			psi_sct_del (sct);
			return (1);
		}

		if (fread (sct->data, 1, cnt, fp) != cnt) {
			return (1);
		}
	}
}

psi_img_t *psi_load_anadisk (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (ana_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}

static
int ana_save_sector (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h)
{
	unsigned      n;
	unsigned char buf[8];

	n = psi_sct_get_mfm_size (sct);

	buf[0] = c;
	buf[1] = h;
	buf[2] = sct->c;
	buf[3] = sct->h;
	buf[4] = sct->s;
	buf[5] = n;
	buf[6] = sct->n & 0xff;
	buf[7] = (sct->n >> 8) & 0xff;

	if (fwrite (buf, 1, 8, fp) != 8) {
		return (1);
	}

	if (fwrite (sct->data, 1, sct->n, fp) != sct->n) {
		return (1);
	}

	return (0);
}

int psi_save_anadisk (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h, s;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (ana_save_sector (fp, sct, c, h)) {
					return (1);
				}
			}
		}
	}

	fflush (fp);

	return (0);
}
