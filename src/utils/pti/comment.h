/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/comment.h                                      *
 * Created:     2020-04-30 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PTI_COMMENT_H
#define PTI_COMMENT_H 1


#include <drivers/pti/pti.h>


int pti_comment_add (pti_img_t *img, const char *str);
int pti_comment_load (pti_img_t *img, const char *fname);
int pti_comment_save (pti_img_t *img, const char *fname);
int pti_comment_set (pti_img_t *img, const char *str);
int pti_comment_show (pti_img_t *img);


#endif
