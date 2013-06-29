/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/main.h                                         *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PRI_MAIN_H
#define PRI_MAIN_H 1


#include <drivers/pri/pri.h>
#include <drivers/psi/psi.h>


typedef int (*pri_trk_cb) (pri_img_t *img, pri_trk_t *trk,
	unsigned long c, unsigned long h, void *opaque
);


int pri_for_all_tracks (pri_img_t *img, pri_trk_cb fct, void *opaque);


#endif
