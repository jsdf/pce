/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/space.h                                        *
 * Created:     2020-05-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PTI_SPACE_H
#define PTI_SPACE_H 1


#include <drivers/pti/pti.h>


int pti_space_add_left (pti_img_t *img, unsigned long clk);
int pti_space_add_right (pti_img_t *img, unsigned long clk);
int pti_space_add (pti_img_t *img, unsigned long clk);
int pti_space_auto (pti_img_t *img, unsigned long min);
int pti_space_max (pti_img_t *img, unsigned long max);
int pti_space_trim_left (pti_img_t *img);
int pti_space_trim_right (pti_img_t *img);
int pti_space_trim (pti_img_t *img);


#endif
