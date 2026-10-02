/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/video.h                                    *
 * Created:     2022-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef PCE_SPECTRUM_VIDEO_H
#define PCE_SPECTRUM_VIDEO_H 1


#include <drivers/video/terminal.h>


typedef struct {
	unsigned long       frame_length;
	unsigned long       frame_clock;
	unsigned            row_length;
	unsigned            row_clock;
	unsigned            row;
	unsigned            col;

	unsigned            frame_counter;
	unsigned            flash;
	unsigned            drop[2];

	unsigned            display_w;
	unsigned            display_h;

	const unsigned char *vram;

	unsigned char       bg;

	char                de;
	unsigned            vaddr;
	unsigned            caddr;

	unsigned char       data[2];

	unsigned char       *rgbi_buf;
	unsigned long       rgbi_cnt;

	unsigned long       rgb_buf_size;
	unsigned char       *rgb_buf;

	void                *irq_ext;
	void                (*irq_fct) (void *ext, unsigned char val);

	terminal_t          *trm;
} spec_video_t;


void spec_video_init (spec_video_t *vid);
void spec_video_free (spec_video_t *vid);

void spec_video_set_vram (spec_video_t *vid, const unsigned char *ptr);
void spec_video_set_irq_fct (spec_video_t *vid, void *ext, void *fct);
void spec_video_set_terminal (spec_video_t *vid, terminal_t *trm, int open);

void spec_video_set_size (spec_video_t *vid, unsigned w, unsigned h);

void spec_video_set_frame_drop (spec_video_t *vid, unsigned cnt);

void spec_video_set_bg (spec_video_t *vid, unsigned char val);

unsigned spec_video_get_data (spec_video_t *vid);

void spec_video_reset (spec_video_t *vid);

void spec_video_clock (spec_video_t *vid);


#endif
