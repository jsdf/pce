/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-imd.h                             *
 * Created:     2010-08-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFDC_IMG_IMD_H
#define PFDC_IMG_IMD_H 1


#include "pfdc.h"


#define PFDC_MAGIC_IMD 0x494d4420


pfdc_img_t *pfdc_load_imd (FILE *fp);

int pfdc_save_imd (FILE *fp, const pfdc_img_t *img);

int pfdc_probe_imd_fp (FILE *fp);
int pfdc_probe_imd (const char *fname);


#endif
