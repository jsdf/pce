/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img.h                                    *
 * Created:     2012-02-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2016 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRV_PSI_IMG_H
#define PCE_DRV_PSI_IMG_H 1


#include <drivers/psi/psi.h>


#define PSI_FORMAT_NONE    0
#define PSI_FORMAT_ANADISK 1
#define PSI_FORMAT_CP2     2
#define PSI_FORMAT_DC42    3
#define PSI_FORMAT_IMD     4
#define PSI_FORMAT_MAC     5
#define PSI_FORMAT_MSA     6
#define PSI_FORMAT_PFDC    7
#define PSI_FORMAT_PFDC0   8
#define PSI_FORMAT_PFDC1   9
#define PSI_FORMAT_PFDC2   10
#define PSI_FORMAT_PFDC4   11
#define PSI_FORMAT_PSI     12
#define PSI_FORMAT_RAW     13
#define PSI_FORMAT_RAW_CHS PSI_FORMAT_RAW
#define PSI_FORMAT_RAW_HCS 14
#define PSI_FORMAT_RAW_HTS 15
#define PSI_FORMAT_ST      16
#define PSI_FORMAT_STX     17
#define PSI_FORMAT_TC      18
#define PSI_FORMAT_TD0     19
#define PSI_FORMAT_XDF     20


unsigned psi_guess_type (const char *fname);

psi_img_t *psi_load_fp (FILE *fp, unsigned type);
psi_img_t *psi_load (const char *fname, unsigned type);

int psi_save_fp (FILE *fp, const psi_img_t *img, unsigned type);
int psi_save (const char *fname, const psi_img_t *img, unsigned type);

unsigned psi_probe_fp (FILE *fp);
unsigned psi_probe (const char *fname);


#endif
