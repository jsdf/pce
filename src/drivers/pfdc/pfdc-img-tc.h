/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img-tc.h                               *
 * Created:     2011-08-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFDC_IMG_TC_H
#define PFDC_IMG_TC_H 1


#include <drivers/pfdc/pfdc.h>


pfdc_img_t *pfdc_load_tc (FILE *fp);


#endif
