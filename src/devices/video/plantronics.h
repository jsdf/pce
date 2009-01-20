/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/plantronics.h                              *
 * Created:     2008-10-13 by John Elliott <jce@seasip.demon.co.uk>          *
 * Copyright:   (C) 2008-2009 Hampa Hug <hampa@hampa.ch>                     *
 *              (C) 2008 John Elliott <jce@seasip.demon.co.uk>               *
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


#ifndef PCE_VIDEO_PLANTRONICS_H
#define PCE_VIDEO_PLANTRONICS_H 1


#include <libini/libini.h>

#include <devices/video/video.h>
#include <devices/video/cga.h>


void pla_init (cga_t *pla, unsigned long io, unsigned long addr, unsigned long size);

void pla_free (cga_t *pla);

cga_t *pla_new (unsigned long io, unsigned long addr, unsigned long size);

video_t *pla_new_ini (ini_sct_t *sct);


#endif
