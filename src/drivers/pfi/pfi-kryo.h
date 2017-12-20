/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-kryo.h                                   *
 * Created:     2012-01-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2017 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_IMG_KRYO_H
#define PFI_IMG_KRYO_H 1


#include <drivers/pfi/pfi.h>


pfi_img_t *pfi_load_kryo (FILE *fp);
pfi_img_t *pfi_load_kryo_set (const char *fname);

int pfi_save_kryo_set (const char *fname, pfi_img_t *img);

int pfi_probe_kryo_fp (FILE *fp);
int pfi_probe_kryo (const char *fname);


#endif
