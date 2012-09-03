/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pbit/gcr-mac.h                                   *
 * Created:     2012-02-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_PBIT_GCR_MAC_H
#define PCE_PBIT_GCR_MAC_H 1


#include <drivers/pbit/pbit.h>
#include <drivers/pfdc/pfdc.h>


pfdc_trk_t *pbit_decode_gcr_trk (pbit_trk_t *trk, unsigned h);
pfdc_img_t *pbit_decode_gcr (pbit_img_t *img);

int pbit_encode_gcr_trk (pbit_trk_t *dtrk, pfdc_trk_t *strk, unsigned fmt);
int pbit_encode_gcr_img (pbit_img_t *dimg, pfdc_img_t *simg);
pbit_img_t *pbit_encode_gcr (pfdc_img_t *img);


#endif
