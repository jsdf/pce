/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-ana.h                             *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFDC_IMG_ANA_H
#define PFDC_IMG_ANA_H 1


#include "pfdc.h"


pfdc_img_t *pfdc_load_anadisk (FILE *fp);

int pfdc_save_anadisk (FILE *fp, const pfdc_img_t *img);


#endif
