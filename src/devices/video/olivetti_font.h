/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/olivetti_font.h                            *
 * Created:     2017-08-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_VIDEO_OLIVETTI_FONT_H
#define PCE_VIDEO_OLIVETTI_FONT_H 1


extern unsigned char m24_font[256 * 16];

extern unsigned char m24_scrambler_p2[4096];
extern unsigned char m24_scrambler_p3[4096];


#endif
