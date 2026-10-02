/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/mda.h                                      *
 * Created:     2003-04-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_MDA_H
#define PCE_VIDEO_MDA_H 1


#include <chipset/e6845.h>
#include <devices/memory.h>
#include <devices/video/video.h>
#include <drivers/video/terminal.h>
#include <libini/libini.h>


typedef struct {
	video_t             video;
	e6845_t             crtc;

	mem_blk_t           *memblk;

	mem_blk_t           *regblk;
	unsigned char       *reg;

	terminal_t          *term;

	const unsigned char *font;

	unsigned long       clock;

	unsigned char       mod_cnt;

	unsigned char       rgb[4][3];

	unsigned short      lfsr;

	char                blink;
	unsigned            blink_cnt;
	unsigned            blink_rate;

	unsigned char       mem[4096];
} mda_t;


mda_t *mda_new (unsigned long io, unsigned long mem);

video_t *mda_new_ini (ini_sct_t *sct);


#endif
