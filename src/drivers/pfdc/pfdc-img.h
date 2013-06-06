/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img.h                                  *
 * Created:     2012-02-14 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRV_PFDC_IMG_H
#define PCE_DRV_PFDC_IMG_H 1


#include <drivers/pfdc/pfdc.h>


#define PFDC_FORMAT_NONE    0
#define PFDC_FORMAT_ANADISK 1
#define PFDC_FORMAT_CP2     2
#define PFDC_FORMAT_DC42    3
#define PFDC_FORMAT_IMD     4
#define PFDC_FORMAT_PFDC    5
#define PFDC_FORMAT_PFDC0   6
#define PFDC_FORMAT_PFDC1   7
#define PFDC_FORMAT_PFDC2   8
#define PFDC_FORMAT_PFDC4   9
#define PFDC_FORMAT_RAW     10
#define PFDC_FORMAT_ST      11
#define PFDC_FORMAT_TC      12
#define PFDC_FORMAT_TD0     13
#define PFDC_FORMAT_XDF     14


unsigned pfdc_guess_type (const char *fname);

pfdc_img_t *pfdc_load_fp (FILE *fp, unsigned type);
pfdc_img_t *pfdc_load (const char *fname, unsigned type);

int pfdc_save_fp (FILE *fp, const pfdc_img_t *img, unsigned type);
int pfdc_save (const char *fname, const pfdc_img_t *img, unsigned type);

unsigned pfdc_probe_fp (FILE *fp);
unsigned pfdc_probe (const char *fname);


#endif
