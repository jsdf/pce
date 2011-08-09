/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-dc42.h                            *
 * Created:     2011-07-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PFDC_IMG_DC42_H
#define PFDC_IMG_DC42_H 1


#include "pfdc.h"


pfdc_img_t *pfdc_load_dc42 (FILE *fp);

int pfdc_save_dc42 (FILE *fp, const pfdc_img_t *img);

int pfdc_probe_dc42_fp (FILE *fp);
int pfdc_probe_dc42 (const char *fname);


#endif
