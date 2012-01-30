/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/olivetti.h                                 *
 * Created:     2011-09-26 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_OLIVETTI_H
#define PCE_VIDEO_OLIVETTI_H 1


#include <libini/libini.h>

#include <devices/video/video.h>
#include <devices/video/cga.h>


void m24_init (cga_t *m24, unsigned long io, unsigned long addr, unsigned long size);

void m24_free (cga_t *m24);

cga_t *m24_new (unsigned long io, unsigned long addr, unsigned long size);

video_t *m24_new_ini (ini_sct_t *sct);


#endif
