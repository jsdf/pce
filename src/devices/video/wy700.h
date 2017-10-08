/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/wy700.h                                    *
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


#ifndef PCE_VIDEO_WY700_H
#define PCE_VIDEO_WY700_H 1


#include <chipset/e6845.h>
#include <devices/memory.h>
#include <devices/video/video.h>
#include <drivers/video/terminal.h>
#include <libini/libini.h>


typedef struct {
	video_t             video;
	e6845_t             crtc;

	mem_blk_t           *memblk;
	unsigned char       *mem;

	mem_blk_t           *regblk;
	unsigned char       *reg;

	terminal_t          *term;

	const unsigned char *font;

	unsigned long       clock1;
	unsigned long       clock2;

	unsigned char       mod_cnt;

	char                blink;
	unsigned            blink_cnt;
	unsigned            blink_rate;
} wy700_t;


wy700_t *wy700_new (unsigned long io, unsigned long addr);

video_t *wy700_new_ini (ini_sct_t *sct);


#endif
