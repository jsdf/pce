/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-enc-fm.h                                 *
 * Created:     2014-12-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_PRI_FM_IBM_H
#define PCE_PRI_FM_IBM_H 1


#include <drivers/pri/pri.h>
#include <drivers/psi/psi.h>


typedef struct {
	unsigned long clock;
	unsigned long track_size;

	char          enable_iam;
	char          auto_gap3;

	unsigned      gap4a;
	unsigned      gap1;
	unsigned      gap3;
} pri_enc_fm_t;


psi_trk_t *pri_decode_fm_trk (pri_trk_t *trk, unsigned h);
psi_img_t *pri_decode_fm (pri_img_t *img);

void pri_encode_fm_init (pri_enc_fm_t *par, unsigned long clock, unsigned rpm);
int pri_encode_fm_trk (pri_trk_t *dtrk, psi_trk_t *strk, pri_enc_fm_t *par);
int pri_encode_fm_img (pri_img_t *dimg, psi_img_t *simg, pri_enc_fm_t *par);
pri_img_t *pri_encode_fm (psi_img_t *img, pri_enc_fm_t *par);
pri_img_t *pri_encode_fm_sd_300 (psi_img_t *img);


#endif
