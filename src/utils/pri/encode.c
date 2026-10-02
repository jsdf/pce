/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/encode.c                                       *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2018 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi-img.h>
#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/pri-enc-fm.h>
#include <drivers/pri/pri-enc-gcr.h>
#include <drivers/pri/pri-enc-mfm.h>


extern pri_enc_fm_t  par_enc_fm;
extern pri_enc_mfm_t par_enc_mfm;


static
unsigned get_encoding (const psi_trk_t *trk)
{
	unsigned  i;
	psi_sct_t *sct;

	for (i = 0; i < trk->sct_cnt; i++) {
		sct = trk->sct[i];

		switch (sct->encoding & PSI_ENC_MASK) {
		case PSI_ENC_FM:
			return (PSI_ENC_FM);

		case PSI_ENC_MFM:
			return (PSI_ENC_MFM);
		}
	}

	return (PSI_ENC_UNKNOWN);
}

static
int pri_encode_auto_img (pri_img_t *dimg, psi_img_t *simg)
{
	unsigned      enc;
	unsigned long c, h;
	psi_cyl_t     *cyl;
	psi_trk_t     *trk;
	pri_trk_t     *dtrk;

	for (c = 0; c < simg->cyl_cnt; c++) {
		cyl = simg->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			dtrk = pri_img_get_track (dimg, c, h, 1);

			if (dtrk == NULL) {
				return (1);
			}

			enc = get_encoding (trk);

			if (enc == PSI_ENC_FM) {
				if (pri_trk_set_size (dtrk, par_enc_fm.track_size)) {
					return (1);
				}

				pri_trk_set_clock (dtrk, par_enc_fm.clock);
				pri_trk_clear_16 (dtrk, 0xffff);

				if (pri_encode_fm_trk (dtrk, trk, &par_enc_fm)) {
					return (1);
				}
			}
			else {
				if (pri_trk_set_size (dtrk, par_enc_mfm.track_size)) {
					return (1);
				}

				pri_trk_set_clock (dtrk, par_enc_mfm.clock);
				pri_trk_clear_16 (dtrk, 0x9254);

				if (pri_encode_mfm_trk (dtrk, trk, &par_enc_mfm)) {
					return (1);
				}
			}
		}
	}

	return (0);
}

static
pri_img_t *pri_encode_auto (psi_img_t *img)
{
	pri_img_t *dimg;

	if ((dimg = pri_img_new()) == NULL) {
		return (NULL);
	}

	if (pri_encode_auto_img (dimg, img)) {
		pri_img_del (dimg);
		return (NULL);
	}

	return (dimg);
}

int pri_encode (pri_img_t **img, const char *type, const char *fname)
{
	psi_img_t *simg;
	pri_img_t *dimg;

	if (strcmp (type, "text") == 0) {
		return (pri_encode_text (*img, fname));
	}

	if ((simg = psi_load (fname, PSI_FORMAT_NONE)) == NULL) {
		fprintf (stderr, "%s: loading failed (%s)\n", arg0, fname);
		return (1);
	}

	if (strcmp (type, "auto") == 0) {
		dimg = pri_encode_auto (simg);
	}
	else if (strcmp2 (type, "ibm-fm", "fm") == 0) {
		dimg = pri_encode_fm (simg, &par_enc_fm);
	}
	else if (strcmp2 (type, "ibm-fm-sd-300", "fm-sd-300") == 0) {
		par_enc_fm.clock = 250000;
		par_enc_fm.track_size = 250000 / 5;
		dimg = pri_encode_fm (simg, &par_enc_fm);
	}
	else if (strcmp2 (type, "mac-gcr", "gcr") == 0) {
		dimg = pri_encode_gcr (simg);
	}
	else if (strcmp2 (type, "ibm-mfm", "mfm") == 0) {
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else if (strcmp2 (type, "ibm-mfm-dd-300", "mfm-dd-300") == 0) {
		par_enc_mfm.clock = 500000;
		par_enc_mfm.track_size = 500000 / 5;
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else if (strcmp2 (type, "ibm-mfm-hd-300", "mfm-hd-300") == 0) {
		par_enc_mfm.clock = 1000000;
		par_enc_mfm.track_size = 1000000 / 5;
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else if (strcmp2 (type, "ibm-mfm-hd-360", "mfm-hd-360") == 0) {
		par_enc_mfm.clock = 1000000;
		par_enc_mfm.track_size = 1000000 / 6;
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else {
		fprintf (stderr, "%s: unknown encode type (%s)\n", arg0, type);
		dimg = NULL;
	}

	if ((dimg != NULL) && (simg->comment_size > 0)) {
		pri_img_set_comment (dimg, simg->comment, simg->comment_size);
	}

	psi_img_del (simg);

	if (dimg == NULL) {
		return (1);
	}

	pri_img_del (*img);

	*img = dimg;

	return (0);
}
