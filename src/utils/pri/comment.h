/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/comment.h                                      *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PRI_COMMENT_H
#define PRI_COMMENT_H 1


#include <drivers/pri/pri.h>


int pri_comment_add (pri_img_t *img, const char *str);
int pri_comment_load (pri_img_t *img, const char *fname);
int pri_comment_save (pri_img_t *img, const char *fname);
int pri_comment_set (pri_img_t *img, const char *str);
int pri_comment_show (pri_img_t *img);


#endif
