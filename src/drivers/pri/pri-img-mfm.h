/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img-mfm.h                                *
 * Created:     2023-12-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2023-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_PRI_IMG_MFM_H
#define PCE_PRI_IMG_MFM_H 1


#include <drivers/pri/pri.h>


pri_img_t *pri_load_mfm (FILE *fp);

int pri_save_mfm (FILE *fp, const pri_img_t *img);

int pri_probe_mfm_fp (FILE *fp);
int pri_probe_mfm (const char *fname);


#endif
