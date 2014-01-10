/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/decode.c                                       *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2014 Hampa Hug <hampa@hampa.ch>                     *
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
#include <drivers/pri/gcr-mac.h>
#include <drivers/pri/mfm-ibm.h>


extern pri_dec_mfm_t par_dec_mfm;


static
int pri_decode_fm_raw_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	FILE          *fp;
	unsigned      outbuf, outcnt;
	unsigned      val, clk;
	unsigned long bit;

	fp = opaque;

	pri_trk_set_pos (trk, 0);

	val = 0;
	clk = 0xffff;

	outbuf = 0;
	outcnt = 0;

	while (trk->wrap == 0) {
		pri_trk_get_bits (trk, &bit, 1);

		val = ((val << 1) | (bit & 1)) & 0xffff;
		clk = (clk << 1) | (~clk & val & 1);

		if ((val == 0xf57e) || (val == 0xf56f) || (val == 0xf56a)) {
			clk = 0xaaaa;

			if (outcnt > 0) {
				outbuf = outbuf << (8 - outcnt);
				outcnt = 8;
			}
		}
		else if ((clk & 0x8000) == 0) {
			outbuf = (outbuf << 1) | ((val >> 15) & 1);
			outcnt += 1;
		}

		if (outcnt >= 8) {
			fputc (outbuf & 0xff, fp);
			outbuf = 0;
			outcnt = 0;
		}
	}

	return (0);
}

static
int pri_decode_fm_raw (pri_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = pri_for_all_tracks (img, pri_decode_fm_raw_cb, fp);

	fclose (fp);

	return (r);
}


static
int pri_decode_gcr_raw_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	FILE          *fp;
	unsigned      val;
	unsigned long bit;

	fp = opaque;

	pri_trk_set_pos (trk, 0);

	val = 0;

	while (trk->wrap == 0) {
		pri_trk_get_bits (trk, &bit, 1);

		val = (val << 1) | (bit & 1);

		if (val & 0x80) {
			fputc (val, fp);
			val = 0;
		}
	}

	return (0);
}

static
int pri_decode_gcr_raw (pri_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = pri_for_all_tracks (img, pri_decode_gcr_raw_cb, fp);

	fclose (fp);

	return (r);
}


static
int pri_decode_mfm_raw_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	FILE          *fp;
	unsigned      outbuf, outcnt;
	unsigned      val, clk;
	unsigned long bit;

	fp = opaque;

	pri_trk_set_pos (trk, 0);

	val = 0;
	clk = 0;

	outbuf = 0;
	outcnt = 0;

	while (trk->wrap == 0) {
		pri_trk_get_bits (trk, &bit, 1);

		val = (val << 1) | (bit & 1);
		clk = (clk << 1) | (~clk & 1);

		if ((clk & 1) == 0) {
			outbuf = (outbuf << 1) | (val & 1);
			outcnt += 1;
		}

		if ((val & 0xffff) == 0x4489) {
			outbuf = 0xa1;
			outcnt = 8;
			clk = 0;
		}

		if (outcnt >= 8) {
			fputc (outbuf & 0xff, fp);
			outbuf = 0;
			outcnt = 0;
		}
	}

	return (0);
}

static
int pri_decode_mfm_raw (pri_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = pri_for_all_tracks (img, pri_decode_mfm_raw_cb, fp);

	fclose (fp);

	return (r);
}


static
int pri_decode_raw_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	FILE *fp;

	fp = opaque;

	if (fwrite (trk->data, (trk->size + 7) / 8, 1, fp) != 1) {
		return (1);
	}

	return (0);
}

int pri_decode_raw (pri_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = pri_for_all_tracks (img, pri_decode_raw_cb, fp);

	fclose (fp);

	return (r);
}


int pri_decode (pri_img_t *img, const char *type, const char *fname)
{
	int       r;
	psi_img_t *dimg;

	if (strcmp (type, "fm-raw") == 0) {
		return (pri_decode_fm_raw (img, fname));
	}
	else if (strcmp (type, "gcr-raw") == 0) {
		return (pri_decode_gcr_raw (img, fname));
	}
	else if (strcmp (type, "mfm-raw") == 0) {
		return (pri_decode_mfm_raw (img, fname));
	}
	else if (strcmp (type, "raw") == 0) {
		return (pri_decode_raw (img, fname));
	}

	if (strcmp (type, "fm") == 0) {
		par_dec_mfm.decode_mfm = 0;
		par_dec_mfm.decode_fm = 1;
		dimg = pri_decode_mfm (img, &par_dec_mfm);
	}
	else if (strcmp (type, "gcr") == 0) {
		dimg = pri_decode_gcr (img);
	}
	else if (strcmp (type, "mfm") == 0) {
		par_dec_mfm.decode_mfm = 1;
		par_dec_mfm.decode_fm = 0;
		dimg = pri_decode_mfm (img, &par_dec_mfm);
	}
	else if (strcmp (type, "mfm-fm") == 0) {
		par_dec_mfm.decode_mfm = 1;
		par_dec_mfm.decode_fm = 1;
		dimg = pri_decode_mfm (img, &par_dec_mfm);
	}
	else {
		dimg = NULL;
	}

	if (dimg == NULL) {
		return (1);
	}

	if (img->comment_size > 0) {
		psi_img_set_comment (dimg, img->comment, img->comment_size);
	}

	r = psi_save (fname, dimg, PSI_FORMAT_NONE);

	psi_img_del (dimg);

	return (r);
}
