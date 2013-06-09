/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-pfdc.h                               *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PSI_IMG_PFDC_H
#define PSI_IMG_PFDC_H 1


#include <drivers/psi/psi.h>


#define PFDC_MAGIC 0x50464443


psi_img_t *psi_load_pfdc (FILE *fp);

int psi_save_pfdc (FILE *fp, const psi_img_t *img, unsigned vers);

int psi_probe_pfdc_fp (FILE *fp);
int psi_probe_pfdc (const char *fname);


#endif
