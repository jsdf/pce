/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfdc/pfdc-img-io.h                                 *
 * Created:     2010-08-21 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFDC_IMG_IO_H
#define PFDC_IMG_IO_H 1


#include <drivers/block/pfdc.h>


#define PFDC_FORMAT_NONE  0
#define PFDC_FORMAT_PFDC  1
#define PFDC_FORMAT_PFDC0 2
#define PFDC_FORMAT_PFDC1 3
#define PFDC_FORMAT_ANA   4
#define PFDC_FORMAT_DC42  5
#define PFDC_FORMAT_IMD   6
#define PFDC_FORMAT_RAW   7
#define PFDC_FORMAT_TD0   8


pfdc_img_t *pfdc_img_load_fp (FILE *fp, unsigned type);

pfdc_img_t *pfdc_img_load (const char *fname, unsigned type);

int pfdc_img_save_fp (FILE *fp, const pfdc_img_t *img, unsigned type);

int pfdc_img_save (const char *fname, const pfdc_img_t *img, unsigned type);


#endif
