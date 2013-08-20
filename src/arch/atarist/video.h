/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/video.h                                     *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_VIDEO_H
#define PCE_ATARIST_VIDEO_H 1


#include <devices/memory.h>
#include <drivers/video/terminal.h>


typedef struct {
	memory_t            *mem;
	unsigned long       base;
	unsigned long       addr;

	mem_blk_t           reg;

	unsigned char       sync_mode;
	unsigned char       shift_mode;
	unsigned char       mode;
	char                mono;

	unsigned short      palette[16];
	unsigned char       pal_col[16][3];
	unsigned char       pal_mono[2][3];

	unsigned            w;
	unsigned            h;

	const unsigned char *src;
	unsigned char       *rgb;
	unsigned char       *dst;

	unsigned            hb1;
	unsigned            hb2;
	unsigned            vb1;
	unsigned            vb2;
	unsigned            clk;
	unsigned            line;
	unsigned            frame;

	unsigned            frame_skip;
	unsigned            frame_skip_max;

	unsigned long       frame_rate[2];

	terminal_t          *trm;

	unsigned char       hb_val;
	void                *hb_ext;
	void                (*set_hb) (void *ext, unsigned char val);

	unsigned char       vb_val;
	void                *vb_ext;
	void                (*set_vb) (void *ext, unsigned char val);
} st_video_t;


int st_video_init (st_video_t *vid, unsigned long addr, int mono);
void st_video_free (st_video_t *vid);

st_video_t *st_video_new (unsigned long addr, int mono);
void st_video_del (st_video_t *vid);

void st_video_set_memory (st_video_t *vid, memory_t *mem);

void st_video_set_hb_fct (st_video_t *vid, void *ext, void *fct);
void st_video_set_vb_fct (st_video_t *vid, void *ext, void *fct);

void st_video_set_terminal (st_video_t *vid, terminal_t *trm);

void st_video_set_frame_skip (st_video_t *vid, unsigned skip);

void st_video_redraw (st_video_t *vid);

void st_video_reset (st_video_t *vid);

void st_video_clock (st_video_t *vid, unsigned cnt);


#endif
