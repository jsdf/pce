/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/video.c                                    *
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


#include "main.h"
#include "spectrum.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>

#include <drivers/video/terminal.h>


#define VIDEO_W 448
#define VIDEO_H 320


static unsigned char rgbi_vals[16][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xaa },
	{ 0xaa, 0x00, 0x00 },
	{ 0xaa, 0x00, 0xaa },
	{ 0x00, 0xaa, 0x00 },
	{ 0x00, 0xaa, 0xaa },
	{ 0xaa, 0xaa, 0x00 },
	{ 0xaa, 0xaa, 0xaa },

	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xff },
	{ 0xff, 0x00, 0x00 },
	{ 0xff, 0x00, 0xff },
	{ 0x00, 0xff, 0x00 },
	{ 0x00, 0xff, 0xff },
	{ 0xff, 0xff, 0x00 },
	{ 0xff, 0xff, 0xff }
};


void spec_video_init (spec_video_t *vid)
{
	vid->frame_length = 69888;
	vid->row_length = 224;

	vid->frame_clock = 0;
	vid->row_clock = 0;
	vid->row = 0;

	vid->frame_counter = 0;

	vid->drop[0] = 0;
	vid->drop[1] = 0;

	vid->display_w = 256;
	vid->display_h = 192;

	vid->vram = NULL;

	vid->rgbi_buf = NULL;

	vid->rgb_buf_size = 0;
	vid->rgb_buf = NULL;

	vid->rgbi_buf = malloc (VIDEO_W * VIDEO_H);

	vid->irq_ext = NULL;
	vid->irq_fct = NULL;

	vid->trm = NULL;
}

void spec_video_free (spec_video_t *vid)
{
	free (vid->rgbi_buf);
	free (vid->rgb_buf);
}

void spec_video_set_vram (spec_video_t *vid, const unsigned char *ptr)
{
	vid->vram = ptr;
}

void spec_video_set_irq_fct (spec_video_t *vid, void *ext, void *fct)
{
	vid->irq_ext = ext;
	vid->irq_fct = fct;
}

void spec_video_set_terminal (spec_video_t *vid, terminal_t *trm, int open)
{
	vid->trm = trm;

	if (open) {
		if (vid->trm != NULL) {
			trm_open (vid->trm, vid->display_w, vid->display_h);
		}
	}
}

void spec_video_set_size (spec_video_t *vid, unsigned w, unsigned h)
{
	if (w < 256) {
		w = 256;
	}
	else if (w > VIDEO_W) {
		w = VIDEO_W;
	}

	if (h < 192) {
		h = 192;
	}
	else if (h > VIDEO_H) {
		h = VIDEO_H;
	}

	vid->display_w = w;
	vid->display_h = h;
}

void spec_video_set_frame_drop (spec_video_t *vid, unsigned cnt)
{
	vid->drop[1] = cnt;
}

void spec_video_set_bg (spec_video_t *vid, unsigned char val)
{
	vid->bg = val & 0x0f;
}

unsigned spec_video_get_data (spec_video_t *vid)
{
	if (vid->de) {
		if (vid->frame_clock & 2) {
			return (vid->data[0]);
		}
		else {
			return (vid->data[1]);
		}
	}

	return (0xff);
}

void spec_video_reset (spec_video_t *vid)
{
	vid->frame_clock = 0;
	vid->row_clock = 0;
	vid->row = 0;
	vid->col = 0;

	vid->flash = 0;

	vid->bg = 15;

	vid->de = 0;
	vid->vaddr = 0x0000;
	vid->caddr = 0x1800;
	vid->rgbi_cnt = 0;
}

static
void video_set_irq (spec_video_t *vid, int val)
{
	if (vid->irq_fct != NULL) {
		vid->irq_fct (vid->irq_ext, val);
	}
}

static
unsigned char *video_alloc_rgb (spec_video_t *vid, unsigned w, unsigned h)
{
	unsigned long size;
	unsigned char *ptr;

	size = 3UL * (unsigned long) w * (unsigned long) h;

	if (vid->rgb_buf_size == size) {
		return (vid->rgb_buf);
	}

	if ((ptr = realloc (vid->rgb_buf, size)) == NULL) {
		return (NULL);
	}

	vid->rgb_buf_size = size;
	vid->rgb_buf = ptr;

	return (vid->rgb_buf);
}

static
void video_update (spec_video_t *vid)
{
	unsigned            i, j;
	unsigned            w, h;
	const unsigned char *s, *p;
	unsigned char       *d;
	unsigned char       *rgb;

	if (vid->drop[0] > 0) {
		vid->drop[0] -= 1;
		return;
	}

	vid->drop[0] = vid->drop[1];

	w = vid->display_w;
	h = vid->display_h;

	if ((rgb = video_alloc_rgb (vid, w, h)) == NULL) {
		return;
	}

	s = vid->rgbi_buf + (64 - (h - 192) / 2) * VIDEO_W - (w - 256) / 2;
	d = rgb;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			p = rgbi_vals[s[i] & 15];
			*(d++) = *(p++);
			*(d++) = *(p++);
			*(d++) = *(p++);
		}

		s += VIDEO_W;
	}

	trm_set_size (vid->trm, w, h);
	trm_set_lines (vid->trm, rgb, 0, h);
	trm_update (vid->trm);
}

void spec_video_clock (spec_video_t *vid)
{
	unsigned      i;
	unsigned      trow;
	unsigned char msk, col, val;
	unsigned char fg, bg;

	if ((vid->frame_clock & 3) == 0) {
		if (vid->de) {
			msk = vid->vram[vid->vaddr + vid->col];
			col = vid->vram[vid->caddr + vid->col];

			vid->data[0] = msk;
			vid->data[1] = col;

			fg = col & 7;
			bg = (col >> 3) & 7;

			if (col & 0x40) {
				fg |= 8;
				bg |= 8;
			}

			if (col & 0x80) {
				if (vid->flash & 0x10) {
					val = fg;
					fg = bg;
					bg = val;
				}
			}

			vid->col += 1;

			for (i = 0; i < 8; i++) {
				val = (msk & 0x80) ? fg : bg;
				vid->rgbi_buf[vid->rgbi_cnt++] = val;
				msk <<= 1;
			}
		}
		else {
			for (i = 0; i < 8; i++) {
				vid->rgbi_buf[vid->rgbi_cnt++] = vid->bg;
			}
		}
	}

	vid->frame_clock += 1;
	vid->row_clock += 1;

	if (vid->row_clock < vid->row_length) {
		if (vid->row_clock >= 128) {
			vid->de = 0;
		}
		return;
	}

	vid->col = 0;
	vid->row += 1;
	vid->row_clock = 0;

	if ((vid->row >= 64) && (vid->row < 256)) {
		vid->de = 1;

		trow = vid->row - 64;
		vid->vaddr = ((trow & 7) << 8) | ((trow & 0x38) << 2) | ((trow & 0x1c0) << 5);
		vid->caddr = 0x1800 + 32 * (trow / 8);
	}
	else {
		vid->de = 0;
	}

	if (vid->frame_clock < vid->frame_length) {
		return;
	}

	video_update (vid);

	vid->row = 0;
	vid->frame_clock = 0;
	vid->vaddr = 0x0000;
	vid->caddr = 0x1800;
	vid->rgbi_cnt = 0;
	vid->flash += 1;
	vid->frame_counter += 1;

	video_set_irq (vid, 1);
	video_set_irq (vid, 0);
}
