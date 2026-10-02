/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/video.h                                    *
 * Created:     2003-08-30 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_H
#define PCE_VIDEO_H 1


#include <stdio.h>

#include <devices/memory.h>


typedef struct {
	void      (*del) (void *ext);

	int       (*set_msg) (void *ext, const char *msg, const char *val);

	void      (*set_terminal) (void *ext, void *trm);

	mem_blk_t *(*get_mem) (void *ext);
	mem_blk_t *(*get_reg) (void *ext);

	void      (*set_blink_rate) (void *ext, unsigned rate, int start);

	void      (*print_info) (void *ext, FILE *fp);

	void      (*redraw) (void *ext, int now);
	void      (*clock) (void *ext, unsigned long cnt);

	void      *ext;

	unsigned      buf_w;
	unsigned      buf_h;
	unsigned      buf_bpp;
	unsigned      buf_next_w;
	unsigned      buf_next_h;
	unsigned long buf_max;
	unsigned char *buf;

	/* the dot clock (clock, remainder, last) */
	unsigned long dotclk[3];
} video_t;


/*!***************************************************************************
 * @short Increase the clock count without calling the clock() function
 *****************************************************************************/
#define pce_video_clock0(vid, cnt, div) do { \
	(vid)->dotclk[0] += ((cnt) + (vid)->dotclk[1]) / (div); \
	(vid)->dotclk[1] = ((cnt) + (vid)->dotclk[1]) % (div); \
	} while (0)


void pce_video_init (video_t *vid);

void pce_video_del (video_t *vid);

/*!***************************************************************************
 * @short Send a message to a video device
 *****************************************************************************/
int pce_video_set_msg (video_t *vid, const char *msg, const char *val);

void pce_video_set_terminal (video_t *vid, void *trm);

mem_blk_t *pce_video_get_mem (video_t *vid);
mem_blk_t *pce_video_get_reg (video_t *vid);

void pce_video_set_blink_rate (video_t *vid, unsigned rate, int start);

void pce_video_print_info (video_t *vid, FILE *fp);

void pce_video_redraw (video_t *vid, int now);

void pce_video_clock1 (video_t *vid, unsigned long cnt);

int pce_video_set_buf_size (video_t *vid, unsigned w, unsigned h, unsigned bpp);

unsigned char *pce_video_get_row_ptr (video_t *vid, unsigned row);

unsigned long pce_color_add (unsigned long col, unsigned long val);
unsigned long pce_color_sub (unsigned long col, unsigned long val);
int pce_color_get (const char *name, unsigned long *col);


#endif
