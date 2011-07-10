/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/mda.h                                      *
 * Created:     2003-04-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>


typedef struct {
	video_t       video;

	mem_blk_t     *memblk;
	unsigned char *mem;

	mem_blk_t     *regblk;
	unsigned char *reg;

	terminal_t    *term;

	unsigned char reg_crt[18];

	unsigned char *font;

	unsigned char rgb[16][3];

	char          blink_on;
	unsigned      blink_cnt;
	unsigned      blink_freq;

	/* these are derived from the crtc registers */
	unsigned      w;
	unsigned      h;
	unsigned      ch;

	unsigned long clk_ht;
	unsigned long clk_vt;
	unsigned long clk_hd;
	unsigned long clk_vd;

	unsigned      buf_w;
	unsigned      buf_h;
	unsigned long bufmax;
	unsigned char *buf;

	unsigned char update_state;
} mda_t;


mda_t *mda_new (unsigned long io, unsigned long mem, unsigned long size);

video_t *mda_new_ini (ini_sct_t *sct);


#endif
