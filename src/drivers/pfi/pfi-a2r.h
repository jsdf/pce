/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-a2r.h                                    *
 * Created:     2019-06-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2019 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PFI_IMG_A2R_H
#define PFI_IMG_A2R_H 1


#include <drivers/pfi/pfi.h>


pfi_img_t *pfi_load_a2r (FILE *fp);

int pfi_save_a2r (FILE *fp, pfi_img_t *img);

int pfi_probe_a2r_fp (FILE *fp);
int pfi_probe_a2r (const char *fname);


#endif
