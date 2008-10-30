/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/terminal/term-old.c                                      *
 * Created:     2008-10-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008 Hampa Hug <hampa@hampa.ch>                          *
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

/* $Id$ */


#include <stdio.h>
#include <stdlib.h>

#include <terminal/terminal.h>
#include <terminal/term-old.h>
#include <terminal/font.h>


void trm_old_init (term_old_t *trm, terminal_t *trmnew)
{
	trm->ext = NULL;

	trm->msg_ext = NULL;
	trm->set_msg = NULL;
	trm->get_msgul = NULL;

	trm->key_ext = NULL;
	trm->set_key = NULL;

	trm->mse_ext = NULL;
	trm->set_mse = NULL;

	trm->del = NULL;
	trm->set_mode = NULL;
	trm->set_size = NULL;
	trm->set_map = NULL;
	trm->set_col = NULL;
	trm->set_crs = NULL;
	trm->set_pos = NULL;
	trm->set_chr = NULL;
	trm->set_pxl = NULL;
	trm->set_rct = NULL;

	trm->check = NULL;

	trm->mode = 0;
	trm->w = 0;
	trm->h = 0;

	trm->cursor_x = 0;
	trm->cursor_y = 0;
	trm->cursor_y1 = 0;
	trm->cursor_y2 = 0;
	trm->cursor_show = 0;

	trm->fg = 0;
	trm->bg = 0;

	trm->update_y1 = 0;
	trm->update_y2 = 0;

	trm->gbuf = NULL;
	trm->tbuf = NULL;

	trm->trm = trmnew;
}

void trm_old_free (term_old_t *trm)
{
	free (trm->gbuf);
	free (trm->tbuf);
}

void trm_old_del (term_old_t *trm)
{
	if (trm != NULL) {
		trm_old_free (trm);
	}

	free (trm);
}

void trm_old_set_msg_fct (term_old_t *trm, void *ext, void *fct)
{
	trm_set_msg_fct (trm->trm, ext, fct);
}

void trm_old_set_key_fct (term_old_t *trm, void *ext, void *fct)
{
	trm_set_key_fct (trm->trm, ext, fct);
}

int trm_old_set_msg (term_old_t *trm, const char *msg, const char *val)
{
	return (trm_set_msg_trm (trm->trm, msg, val));
}

static
void trm_old_set_update (term_old_t *trm, unsigned y1, unsigned y2)
{
	if (trm->update_y1 >= trm->update_y2) {
		trm->update_y1 = y1;
		trm->update_y2 = y2;
	}
	else {
		if (y1 < trm->update_y1) {
			trm->update_y1 = y1;
		}

		if (y2 > trm->update_y2) {
			trm->update_y2 = y2;
		}
	}
}

void trm_old_set_mode (term_old_t *trm, unsigned m, unsigned w, unsigned h)
{
	unsigned long n;

	trm->mode = m;
	trm->w = w;
	trm->h = h;

	n = (unsigned long) w * (unsigned long) h;

	if (m == TERM_MODE_TEXT) {
		trm->gbuf = realloc (trm->gbuf, n * 3 * 8 * 16);
		trm->tbuf = realloc (trm->tbuf, n * 2);
	}
	else {
		free (trm->tbuf);
		trm->tbuf = NULL;

		trm->gbuf = realloc (trm->gbuf, 3 * n);
	}

	trm->update_y1 = 0;
	trm->update_y2 = h;
}

void trm_old_set_size (term_old_t *trm, unsigned w, unsigned h)
{
}

void trm_old_set_map (term_old_t *trm, unsigned i, unsigned r, unsigned g, unsigned b)
{
	if (i < 16) {
		trm->colmap[3 * i + 0] = r >> 8;
		trm->colmap[3 * i + 1] = g >> 8;
		trm->colmap[3 * i + 2] = b >> 8;
	}

	trm_old_set_update (trm, 0, trm->h);
}

void trm_old_set_col (term_old_t *trm, unsigned fg, unsigned bg)
{
	trm->fg = fg;
	trm->bg = bg;
}

static
void trm_old_update_char (term_old_t *trm, unsigned x, unsigned y)
{
	unsigned      i, j;
	unsigned char c, v;
	unsigned      fg, bg;
	unsigned char *dst;
	unsigned char *src;

	if (trm->mode != TERM_MODE_TEXT) {
		return;
	}

	if ((x >= trm->w) || (y >= trm->h)) {
		return;
	}

	c = trm->tbuf[2 * (trm->w * y + x) + 0];
	fg = trm->tbuf[2 * (trm->w * y + x) + 1] & 0x0f;
	bg = (trm->tbuf[2 * (trm->w * y + x) + 1] >> 4) & 0x0f;

	src = fnt_8x16 + 16 * c;

	for (j = 0; j < 16; j++) {
		dst = trm->gbuf + 3 * (8 * (trm->w * (16 * y + j) + x));

		v = *(src++);

		for (i = 0; i < 8; i++) {
			if (v & 0x80) {
				dst[0] = trm->colmap[3 * fg + 0];
				dst[1] = trm->colmap[3 * fg + 1];
				dst[2] = trm->colmap[3 * fg + 2];
			}
			else {
				dst[0] = trm->colmap[3 * bg + 0];
				dst[1] = trm->colmap[3 * bg + 1];
				dst[2] = trm->colmap[3 * bg + 2];
			}

			dst += 3;
			v <<= 1;
		}
	}

	trm_old_set_update (trm, 16 * y, 16 * y + 16);
}

static
void trm_old_update_cursor (term_old_t *trm)
{
	unsigned      i, j;
	unsigned      y1, y2;
	unsigned      col;
	unsigned char *dst;

	if (trm->mode != TERM_MODE_TEXT) {
		return;
	}

	if ((trm->cursor_x >= trm->w) || (trm->cursor_y >= trm->h)) {
		return;
	}

	y1 = (15 * trm->cursor_y1) / 255;
	y2 = (15 * trm->cursor_y2) / 255;

	col = trm->tbuf[2 * (trm->w * trm->cursor_y + trm->cursor_x) + 1] & 0x0f;

	for (j = y1; j < y2; j++) {
		dst = trm->gbuf + 3 * (8 * (trm->w * (16 * trm->cursor_y + j) + trm->cursor_x));

		for (i = 0; i < 8; i++) {
			dst[0] = trm->colmap[3 * col + 0];
			dst[1] = trm->colmap[3 * col + 1];
			dst[2] = trm->colmap[3 * col + 2];

			dst += 3;
		}
	}

	trm_old_set_update (trm, 16 * trm->cursor_y + y1, 16 * trm->cursor_y + y2);
}

void trm_old_set_crs (term_old_t *trm, unsigned y1, unsigned y2, int show)
{
	if (trm->cursor_show) {
		trm_old_update_char (trm, trm->cursor_x, trm->cursor_y);
	}

	trm->cursor_y1 = y1;
	trm->cursor_y2 = y2;
	trm->cursor_show = show;

	if (trm->cursor_show) {
		trm_old_update_cursor (trm);
	}
}

void trm_old_set_pos (term_old_t *trm, unsigned x, unsigned y)
{
	if (trm->cursor_show) {
		trm_old_update_char (trm, trm->cursor_x, trm->cursor_y);
	}

	trm->cursor_x = x;
	trm->cursor_y = y;

	if (trm->cursor_show) {
		trm_old_update_cursor (trm);
	}
}

void trm_old_set_chr (term_old_t *trm, unsigned x, unsigned y, unsigned char c)
{
	if (trm->mode != TERM_MODE_TEXT) {
		return;
	}

	if ((x >= trm->w) || (y >= trm->h)) {
		return;
	}

	trm->tbuf[2 * (trm->w * y + x) + 0] = c;
	trm->tbuf[2 * (trm->w * y + x) + 1] = (trm->bg << 4) | trm->fg;

	trm_old_update_char (trm, x, y);
}

void trm_old_set_pxl (term_old_t *trm, unsigned x, unsigned y)
{
	unsigned char *dst;

	if (trm->mode != TERM_MODE_GRAPH) {
		return;
	}

	if ((x >= trm->w) || (y >= trm->h)) {
		return;
	}

	dst = trm->gbuf + 3 * (trm->w * y + x);

	dst[0] = trm->colmap[3 * trm->fg + 0];
	dst[1] = trm->colmap[3 * trm->fg + 1];
	dst[2] = trm->colmap[3 * trm->fg + 2];

	trm_old_set_update (trm, y, y + 1);
}

void trm_old_set_rct (term_old_t *trm, unsigned x, unsigned y, unsigned w, unsigned h)
{
	unsigned i, j;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			trm_old_set_pxl (trm, x + i, y + j);
		}
	}
}

void trm_old_update (term_old_t *trm)
{
	if (trm->update_y1 < trm->update_y2) {
		if (trm->mode == TERM_MODE_TEXT) {
			trm_set_size (trm->trm, 8 * trm->w, 16 * trm->h);
			trm_set_lines (trm->trm,
				trm->gbuf + 3 * 8 * trm->w * trm->update_y1,
				trm->update_y1, trm->update_y2 - trm->update_y1
			);
		}
		else {
			trm_set_size (trm->trm, trm->w, trm->h);
			trm_set_lines (trm->trm,
				trm->gbuf + 3 * trm->w * trm->update_y1,
				trm->update_y1, trm->update_y2 - trm->update_y1
			);
		}

		trm->update_y1 = 0;
		trm->update_y2 = 0;
	}

	trm_update (trm->trm);
}
