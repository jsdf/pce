/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-dc42.h                               *
 * Created:     2011-07-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PSI_IMG_DC42_H
#define PSI_IMG_DC42_H 1


#include <drivers/psi/psi.h>


psi_img_t *psi_load_dc42 (FILE *fp);

int psi_save_dc42 (FILE *fp, const psi_img_t *img);

int psi_probe_dc42_fp (FILE *fp);
int psi_probe_dc42 (const char *fname);


#endif
