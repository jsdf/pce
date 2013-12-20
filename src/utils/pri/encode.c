/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/encode.c                                       *
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
#include "encode.h"

#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi-img.h>
#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/gcr-mac.h>
#include <drivers/pri/mfm-ibm.h>


extern pri_enc_mfm_t par_enc_mfm;


int pri_encode (pri_img_t **img, const char *type, const char *fname)
{
	psi_img_t *simg;
	pri_img_t *dimg;

	if ((simg = psi_load (fname, PSI_FORMAT_NONE)) == NULL) {
		return (1);
	}

	if (strcmp (type, "gcr") == 0) {
		dimg = pri_encode_gcr (simg);
	}
	else if (strcmp (type, "mfm") == 0) {
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else if (strcmp (type, "mfm-dd-300") == 0) {
		par_enc_mfm.clock = 500000;
		par_enc_mfm.track_size = 500000 / 5;
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else if (strcmp (type, "mfm-hd-300") == 0) {
		par_enc_mfm.clock = 1000000;
		par_enc_mfm.track_size = 1000000 / 5;
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else if (strcmp (type, "mfm-hd-360") == 0) {
		par_enc_mfm.clock = 1000000;
		par_enc_mfm.track_size = 1000000 / 6;
		dimg = pri_encode_mfm (simg, &par_enc_mfm);
	}
	else {
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
