/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-enc-gcr.h                                *
 * Created:     2012-02-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_PRI_GCR_MAC_H
#define PCE_PRI_GCR_MAC_H 1


#include <drivers/pri/pri.h>
#include <drivers/psi/psi.h>


unsigned long pri_get_mac_gcr_track_length (unsigned c);
int pri_mac_gcr_checksum (unsigned char *dst, const unsigned char *src, int enc);

psi_trk_t *pri_decode_gcr_trk (pri_trk_t *trk, unsigned h);
psi_img_t *pri_decode_gcr (pri_img_t *img);

int pri_encode_gcr_trk (pri_trk_t *dtrk, psi_trk_t *strk, unsigned fmt);
int pri_encode_gcr_img (pri_img_t *dimg, psi_img_t *simg);
pri_img_t *pri_encode_gcr (psi_img_t *img);


#endif
