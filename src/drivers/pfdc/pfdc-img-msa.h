/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img-msa.h                              *
 * Created:     2013-06-03 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFDC_IMG_MSA_H
#define PFDC_IMG_MSA_H 1


#include <drivers/pfdc/pfdc.h>


pfdc_img_t *pfdc_load_msa (FILE *fp);

int pfdc_save_msa (FILE *fp, const pfdc_img_t *img);

int pfdc_probe_msa_fp (FILE *fp);
int pfdc_probe_msa (const char *fname);


#endif
