/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-enc-mfm.h                                *
 * Created:     2012-02-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_PRI_MFM_IBM_H
#define PCE_PRI_MFM_IBM_H 1


#include <drivers/pri/pri.h>
#include <drivers/psi/psi.h>


typedef struct {
	unsigned long clock;
	unsigned long track_size;

	char          enable_iam;
	char          auto_gap3;
	char          nopos;

	unsigned      gap4a;
	unsigned      gap1;
	unsigned      gap3;
} pri_enc_mfm_t;


typedef struct {
	unsigned      min_sct_size;
} pri_dec_mfm_t;


void pri_decode_mfm_init (pri_dec_mfm_t *par);
psi_trk_t *pri_decode_mfm_trk (pri_trk_t *trk, unsigned c, unsigned h, pri_dec_mfm_t *par);
psi_img_t *pri_decode_mfm (pri_img_t *img, pri_dec_mfm_t *par);

void pri_encode_mfm_init (pri_enc_mfm_t *par, unsigned long clock, unsigned rpm);
int pri_encode_mfm_trk (pri_trk_t *dtrk, psi_trk_t *strk, pri_enc_mfm_t *par);
int pri_encode_mfm_img (pri_img_t *dimg, psi_img_t *simg, pri_enc_mfm_t *par);
pri_img_t *pri_encode_mfm (psi_img_t *img, pri_enc_mfm_t *par);
pri_img_t *pri_encode_mfm_dd_300 (psi_img_t *img);
pri_img_t *pri_encode_mfm_hd_300 (psi_img_t *img);
pri_img_t *pri_encode_mfm_hd_360 (psi_img_t *img);


#endif
