/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/video.c                                    *
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


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/msg.h>

#include "video.h"


void pce_video_init (video_t *vid)
{
	vid->ext = NULL;

	vid->buf_w = 0;
	vid->buf_h = 0;
	vid->buf_bpp = 0;
	vid->buf_next_w = 0;
	vid->buf_next_h = 0;
	vid->buf_max = 0;
	vid->buf = NULL;

	vid->dotclk[0] = 0;
	vid->dotclk[1] = 0;
	vid->dotclk[2] = 0;

	vid->del = NULL;
	vid->set_msg = NULL;
	vid->set_terminal = NULL;
	vid->get_mem = NULL;
	vid->get_reg = NULL;
	vid->set_blink_rate = NULL;
	vid->print_info = NULL;
	vid->redraw = NULL;
	vid->clock = NULL;
}

void pce_video_del (video_t *vid)
{
	pce_video_set_buf_size (vid, 0, 0, 0);

	if (vid->del != NULL) {
		vid->del (vid->ext);
	}
}

int pce_video_set_msg (video_t *vid, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		int      s;
		unsigned v;

		if (strcmp (val, "on") == 0) {
			v = 0;
			s = 1;
		}
		else if (strcmp (val, "off") == 0) {
			v = 0;
			s = 0;
		}
		else if (msg_get_uint (val, &v) == 0) {
			s = 1;
		}
		else {
			return (1);
		}

		pce_video_set_blink_rate (vid, v, s);

		return (0);
	}
	else if (msg_is_message ("emu.video.redraw", msg)) {
		int v;

		if (msg_get_bool (val, &v)) {
			return (1);
		}

		pce_video_redraw (vid, v);

		return (0);
	}

	if (vid->set_msg != NULL) {
		return (vid->set_msg (vid->ext, msg, val));
	}

	return (-1);
}

void pce_video_set_terminal (video_t *vid, void *trm)
{
	if (vid->set_terminal != NULL) {
		vid->set_terminal (vid->ext, trm);
	}
}

mem_blk_t *pce_video_get_mem (video_t *vid)
{
	if (vid->get_mem != NULL) {
		return (vid->get_mem (vid->ext));
	}

	return (NULL);
}

mem_blk_t *pce_video_get_reg (video_t *vid)
{
	if (vid->get_reg != NULL) {
		return (vid->get_reg (vid->ext));
	}

	return (NULL);
}

void pce_video_set_blink_rate (video_t *vid, unsigned rate, int start)
{
	if (vid->set_blink_rate != NULL) {
		vid->set_blink_rate (vid->ext, rate, start);
	}
}

void pce_video_print_info (video_t *vid, FILE *fp)
{
	if (vid->print_info != NULL) {
		vid->print_info (vid->ext, fp);
	}
}

void pce_video_redraw (video_t *vid, int now)
{
	if (vid->redraw != NULL) {
		vid->redraw (vid->ext, now);
	}
}

void pce_video_clock1 (video_t *vid, unsigned long cnt)
{
	vid->dotclk[0] += cnt;

	/* clocks since last call of this function */
	cnt = vid->dotclk[0] - vid->dotclk[2];
	vid->dotclk[2] = vid->dotclk[0];

	if (vid->clock != NULL) {
		vid->clock (vid->ext, cnt);
	}
}

/*
 * Set the internal screen buffer size
 */
int pce_video_set_buf_size (video_t *vid, unsigned w, unsigned h, unsigned bpp)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = (unsigned long) bpp * (unsigned long) w * (unsigned long) h;

	if (cnt == 0) {
		free (vid->buf);

		vid->buf_w = 0;
		vid->buf_h = 0;
		vid->buf_bpp = 0;
		vid->buf_max = 0;
		vid->buf = NULL;

		return (0);
	}

	if (cnt > vid->buf_max) {
		if ((tmp = realloc (vid->buf, cnt)) == NULL) {
			return (1);
		}

		vid->buf = tmp;
		vid->buf_max = cnt;
	}

	vid->buf_w = w;
	vid->buf_h = h;
	vid->buf_bpp = bpp;

	return (0);
}

/*
 * Get a pointer to a row in the internal screen buffer
 */
unsigned char *pce_video_get_row_ptr (video_t *vid, unsigned row)
{
	unsigned long ofs;

	if (row >= vid->buf_h) {
		return (NULL);
	}

	ofs = (unsigned long) vid->buf_bpp * (unsigned long) row * (unsigned long) vid->buf_w;

	return (vid->buf + ofs);
}

unsigned long pce_color_add (unsigned long col, unsigned long val)
{
	unsigned      i, v;
	unsigned long r;

	r = 0;

	for (i = 0; i < 3; i++) {
		v = (col & 0xff) + (val & 0xff);

		if (v > 255) {
			v = 255;
		}

		r = (r >> 8) | (v << 16);

		col >>= 8;
		val >>= 8;
	}

	return (r);
}

unsigned long pce_color_sub (unsigned long col, unsigned long val)
{
	unsigned      i, v, t;
	unsigned long r;

	r = 0;

	for (i = 0; i < 3; i++) {
		v = col & 0xff;
		t = val & 0xff;
		v = (t < v) ? (v - t) : 0;

		r = (r >> 8) | (v << 16);

		col >>= 8;
		val >>= 8;
	}

	return (r);
}

int pce_color_get (const char *name, unsigned long *col)
{
	unsigned i;

	static struct {
		const char *name;
		uint32_t   val;
	} colors[] = {
		{ "amber",  0xca7a2a },
		{ "green",  0x00aa2a },
		{ "gray",   0xaaaaaa },
		{ "amber1", 0xca7a00 },
		{ "amber2", 0xca7a2a },
		{ "amber3", 0xaa8a00 },
		{ "green1", 0x00aa00 },
		{ "green2", 0x00aa2a },
		{ "green3", 0x2aaa00 },
		{ NULL, 0 }
	};

	if (*name == '#') {
		*col = 0;
		name += 1;

		for (i = 0; i < 6; i++) {
			*col <<= 4;

			if ((*name >= '0') && (*name <= '9')) {
				*col += *name - '0';
			}
			else if ((*name >= 'A') && (*name <= 'F')) {
				*col += *name - 'A' + 10;
			}
			else if ((*name >= 'a') && (*name <= 'f')) {
				*col += *name - 'a' + 10;
			}
			else {
				return (1);
			}

			name += 1;
		}

		if (*name != 0) {
			return (1);
		}

		return (0);
	}

	i = 0;
	while (colors[i].name != NULL) {
		if (strcmp (colors[i].name, name) == 0) {
			*col = colors[i].val;
			return (0);
		}

		i += 1;
	}

	return (1);
}
