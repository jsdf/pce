/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/wy700.c                                    *
 * Created:     2008-10-13 by John Elliott <jce@seasip.demon.co.uk>          *
 * Copyright:   (C) 2008-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chipset/e6845.h>
#include <devices/memory.h>
#include <devices/video/wy700.h>
#include <devices/video/wy700_font.h>
#include <drivers/video/terminal.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <libini/libini.h>


#define WY700_IFREQ  1193182
#define WY700_PFREQ  14318184
#define WY700_CFREQ1 (WY700_PFREQ / 8)
#define WY700_CFREQ2 (WY700_PFREQ / 16)
#define WY700_FFREQ  (WY700_IFREQ / 30)

#define WY700_CRTC_INDEX0  0
#define WY700_CRTC_DATA0   1
#define WY700_CRTC_INDEX   4
#define WY700_CRTC_DATA    5
#define WY700_MODE         8
#define WY700_CSEL         9
#define WY700_STATUS       10
#define WY700_PEN_RESET    11
#define WY700_PEN_SET      12
#define WY700_ADDR_LOW     13
#define WY700_ADDR_HIGH    14
#define WY700_CTRL         15

#define WY700_MODE_CS      0x01
#define WY700_MODE_G320    0x02
#define WY700_MODE_CBURST  0x04
#define WY700_MODE_ENABLE  0x08
#define WY700_MODE_G640    0x10
#define WY700_MODE_BLINK   0x20

#define WY700_CSEL_COL     0x0f
#define WY700_CSEL_INT     0x10
#define WY700_CSEL_PAL     0x20

#define WY700_STATUS_SYNC  0x01
#define WY700_STATUS_PEN   0x02
#define WY700_STATUS_VSYNC 0x08

#define WY700_CTRL_READ    0x01
#define WY700_CTRL_WRITE   0x02
#define WY700_CTRL_ENABLE  0x08


static void wy700_del (wy700_t *wy);


static
void wy700_get_color (wy700_t *wy, unsigned a, unsigned char *fg, unsigned char *bg)
{
	unsigned char afg, abg;

	afg = a & 0x0f;
	abg = (a >> 4) & 0x0f;

	if (abg == 0) {
		*bg = 0;

		if (afg == 0) {
			*fg = 0x00;
		}
		else if (afg < 8) {
			*fg = 0xc0;
		}
		else if (afg == 8) {
			*fg = 0x80;
		}
		else {
			*fg = 0xff;
		}
	}
	else if (abg < 8) {
		*bg = 0xc0;

		if (afg < 8) {
			*fg = 0x00;
		}
		else if (afg == 8) {
			*fg = 0x80;
		}
		else {
			*fg = 0xff;
		}
	}
	else if (abg == 8) {
		*bg = 0x80;

		if (afg == 0) {
			*fg = 0x00;
		}
		else if (afg < 8) {
			*fg = 0xc0;
		}
		else if (afg == 8) {
			*fg = 0x80;
		}
		else {
			*fg = 0xff;
		}
	}
	else {
		*bg = 0xff;

		if (afg == 0) {
			*fg = 0x00;
		}
		else if (afg < 8) {
			*fg = 0xc0;
		}
		else {
			*fg = 0x80;
		}
	}

	if (afg == abg) {
		*fg = *bg;
	}
}

static
void wy700_line_blank (wy700_t *wy, unsigned row)
{
	unsigned char *p;

	p = pce_video_get_row_ptr (&wy->video, row);

	memset (p, 0, 3 * wy->video.buf_w);
}

static
void wy700_line_text_1 (wy700_t *wy, unsigned row, unsigned ra)
{
	unsigned            i, j;
	int                 bl;
	unsigned            hd;
	unsigned            addr, caddr, amask;
	unsigned char       code, attr;
	unsigned            val, cmask;
	unsigned char       fg, bg, col;
	const unsigned char *fnt;
	unsigned char       *ptr;

	hd = e6845_get_hd (&wy->crtc);

	wy->video.buf_next_w = 16 * hd;

	if ((16 * hd) > wy->video.buf_w) {
		hd = wy->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&wy->video, row);
	addr = wy->crtc.ma;
	amask = 0x3fff;

	if (wy->blink) {
		caddr = e6845_get_cursor_address (&wy->crtc);
		cmask = e6845_get_cursor_mask (&wy->crtc, 0);
	}
	else {
		caddr = -1;
		cmask = 0;
	}

	bl = wy->reg[WY700_MODE] & WY700_MODE_BLINK;

	for (i = 0; i < hd; i++) {
		code = wy->mem[(2 * addr + 0) & amask];
		attr = wy->mem[(2 * addr + 1) & amask];

		fnt = wy->font + (32 * code) + (ra & 0x1e);

		val = ((unsigned) fnt[0] << 8) | fnt[1];

		if (bl && (attr & 0x80)) {
			if (wy->blink == 0) {
				attr = (attr & 0x70) | ((attr >> 4) & 0x07);
			}

			attr &= 0x7f;
		}

		if (addr == caddr) {
			val |= cmask;
		}

		wy700_get_color (wy, attr, &fg, &bg);

		for (j = 0; j < 16; j++) {
			col = (val & 0x8000) ? fg : bg;

			*(ptr++) = col;
			*(ptr++) = col;
			*(ptr++) = col;

			val <<= 1;
		}

		addr += 1;
	}
}

/*
 * Text modes
 */
static
void wy700_line_text (wy700_t *wy, unsigned row)
{
	unsigned i;

	row *= 4;

	for (i = 0; i < 4; i++) {
		if (row >= wy->video.buf_h) {
			return;
		}

		wy700_line_text_1 (wy, row, 4 * wy->crtc.ra + i);

		row += 1;
	}

	wy->video.buf_next_h = 4 * e6845_get_vdl (&wy->crtc);
}

/*
 * CGA 320 * 200 * 4 graphics mode
 */
static
void wy700_line_320x200x4 (wy700_t *wy, unsigned row)
{
	unsigned      i, j;
	unsigned      hd, addr, val;
	unsigned char col;
	unsigned char *ptr;

	hd = e6845_get_hd (&wy->crtc);

	wy->video.buf_next_w = 8 * hd;
	wy->video.buf_next_h = e6845_get_vdl (&wy->crtc);

	if ((8 * hd) > wy->video.buf_w) {
		hd = wy->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&wy->video, row);
	addr = (wy->crtc.ma ^ ((wy->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val = wy->mem[addr & 0x3fff];
		val = (val << 8) | wy->mem[(addr + 1) & 0x3fff];

		for (j = 0; j < 8; j++) {
			col = (val >> 8) & 0xc0;
			col |= (col >> 2) | (col >> 4) | (col >> 6);

			*(ptr++) = col;
			*(ptr++) = col;
			*(ptr++) = col;

			val <<= 2;
		}

		addr += 2;
	}
}

/*
 * CGA 640 * 200 * 2 graphics mode
 */
static
void wy700_line_640x200x2 (wy700_t *wy, unsigned row)
{
	unsigned      i, j;
	unsigned      hd, addr, val;
	unsigned char col;
	unsigned char *ptr;

	hd = e6845_get_hd (&wy->crtc);

	wy->video.buf_next_w = 16 * hd;
	wy->video.buf_next_h = e6845_get_vdl (&wy->crtc);

	if ((16 * hd) > wy->video.buf_w) {
		hd = wy->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&wy->video, row);
	addr = (wy->crtc.ma ^ ((wy->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val = wy->mem[addr & 0x3fff];
		val = (val << 8) | wy->mem[(addr + 1) & 0x3fff];

		for (j = 0; j < 16; j++) {
			col = (val & 0x8000) ? 0xff : 0x00;

			*(ptr++) = col;
			*(ptr++) = col;
			*(ptr++) = col;

			val <<= 1;
		}

		addr += 2;
	}
}

/*
 * Native 640 * 400 * 2
 */
static
void wy700_update_640x400x2 (wy700_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val;
	unsigned char       *dst;
	const unsigned char *src;

	pce_video_set_buf_size (&wy->video, 640, 400, 3);

	dst = wy->video.buf;
	ofs = 0;

	for (y = 0; y < 400; y++) {
		src = wy->mem + ofs;
		ofs += 80;

		for (x = 0; x < 80; x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
					dst[0] = 0xff;
					dst[1] = 0xff;
					dst[2] = 0xff;
				}
				else {
					dst[0] = 0x00;
					dst[1] = 0x00;
					dst[2] = 0x00;
				}

				val <<= 1;
				dst += 3;
			}
		}
	}
}

/*
 * Native 320 * 400 * 4
 */
static
void wy700_update_320x400x4 (wy700_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val, col;
	unsigned char       *dst;
	const unsigned char *src;

	pce_video_set_buf_size (&wy->video, 320, 400, 3);

	dst = wy->video.buf;
	ofs = 0;

	for (y = 0; y < 400; y++) {
		src = wy->mem + ofs;
		ofs += 80;

		for (x = 0; x < 80; x++) {
			val = src[x];

			for (i = 0; i < 4; i++) {
				col = val & 0xc0;
				col |= col >> 2;
				col |= col >> 4;

				dst[0] = col;
				dst[1] = col;
				dst[2] = col;

				val <<= 2;
				dst += 3;
			}
		}
	}
}

/*
 * Native 1280 * 400 * 2
 */
static
void wy700_update_1280x400x2 (wy700_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val;
	unsigned char       *dst;
	const unsigned char *src;

	pce_video_set_buf_size (&wy->video, 1280, 400, 3);

	dst = wy->video.buf;
	ofs = 0;

	for (y = 0; y < 400; y++) {
		src = wy->mem + ofs;
		ofs += 160;

		for (x = 0; x < 160; x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
					dst[0] = 0xff;
					dst[1] = 0xff;
					dst[2] = 0xff;
				}
				else {
					dst[0] = 0x00;
					dst[1] = 0x00;
					dst[2] = 0x00;
				}

				val <<= 1;
				dst += 3;
			}
		}
	}
}

/*
 * Native 640 * 400 * 4
 */
static
void wy700_update_640x400x4 (wy700_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val, col;
	unsigned char       *dst;
	const unsigned char *src;

	pce_video_set_buf_size (&wy->video, 640, 400, 3);

	dst = wy->video.buf;
	ofs = 0;

	for (y = 0; y < 400; y++) {
		src = wy->mem + ofs;
		ofs += 160;

		for (x = 0; x < 160; x++) {
			val = src[x];

			for (i = 0; i < 4; i++) {
				col = val & 0xc0;
				col |= col >> 2;
				col |= col >> 4;

				dst[0] = col;
				dst[1] = col;
				dst[2] = col;

				val <<= 2;
				dst += 3;
			}
		}
	}
}

/*
 * Native 1280 * 800 * 2
 */
static
void wy700_update_1280x800x2 (wy700_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val;
	unsigned char       *dst;
	const unsigned char *src;

	pce_video_set_buf_size (&wy->video, 1280, 800, 3);

	dst = wy->video.buf;
	ofs = 0;

	for (y = 0; y < 800; y++) {
		src = wy->mem + ofs;

		if (y & 1) {
			src += 0x10000;
			ofs += 160;
		}

		for (x = 0; x < 160; x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
					dst[0] = 0xff;
					dst[1] = 0xff;
					dst[2] = 0xff;
				}
				else {
					dst[0] = 0x00;
					dst[1] = 0x00;
					dst[2] = 0x00;
				}

				val <<= 1;
				dst += 3;
			}
		}
	}
}

/*
 * Native 640 * 800 * 4
 */
static
void wy700_update_640x800x4 (wy700_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val, col;
	unsigned char       *dst;
	const unsigned char *src;

	pce_video_set_buf_size (&wy->video, 640, 800, 3);

	dst = wy->video.buf;
	ofs = 0;

	for (y = 0; y < 800; y++) {
		src = wy->mem + ofs;

		if (y & 1) {
			src += 0x10000;
			ofs += 160;
		}

		for (x = 0; x < 160; x++) {
			val = src[x];

			for (i = 0; i < 4; i++) {
				col = val & 0xc0;
				col |= col >> 2;
				col |= col >> 4;

				dst[0] = col;
				dst[1] = col;
				dst[2] = col;

				val <<= 2;
				dst += 3;
			}
		}
	}
}

static
void wy700_update_native (wy700_t *wy)
{
	unsigned char ctrl;

	ctrl = wy->reg[WY700_CTRL];

	if ((ctrl & WY700_CTRL_ENABLE) == 0) {
		return;
	}

	switch ((ctrl >> 4) & 7) {
	case 0:
		wy700_update_640x400x2 (wy);
		break;

	case 1:
		wy700_update_320x400x4 (wy);
		break;

	case 2:
		wy700_update_1280x400x2 (wy);
		break;

	case 3:
		wy700_update_640x400x4 (wy);
		break;

	case 4:
		wy700_update_1280x800x2 (wy);
		break;

	case 5:
		wy700_update_640x800x4 (wy);
		break;

	default:
		fprintf (stderr, "WY700: unknown mode (%02X)\n", ctrl);
		break;
	}
}

static
void wy700_hsync (wy700_t *wy)
{
	unsigned      row, max, ch;
	unsigned char mode, ctrl;

	if (wy->mod_cnt == 0) {
		return;
	}

	ch = (e6845_get_ml (&wy->crtc) & 0x1f) + 1;
	row = ch * wy->crtc.crow + wy->crtc.ra;
	max = ch * e6845_get_vd (&wy->crtc);

	if (row >= max) {
		return;
	}

	mode = wy->reg[WY700_MODE];
	ctrl = wy->reg[WY700_CTRL];

	if (row >= wy->video.buf_h) {
		return;
	}

	if ((mode & WY700_MODE_ENABLE) == 0) {
		wy700_line_blank (wy, row);
	}
	else if ((ctrl & WY700_CTRL_ENABLE) && (ctrl & 0x80)) {
		wy700_line_blank (wy, row);
	}
	else if (mode & WY700_MODE_G320) {
		if (mode & WY700_MODE_G640) {
			wy700_line_640x200x2 (wy, row);
		}
		else {
			wy700_line_320x200x4 (wy, row);
		}
	}
	else {
		wy700_line_text (wy, row);
	}
}

static
void wy700_vsync (wy700_t *wy)
{
	video_t *vid;

	vid = &wy->video;

	if ((wy->term != NULL) && (vid->buf_w > 0) && (vid->buf_h > 0)) {
		trm_set_size (wy->term, vid->buf_w, vid->buf_h);

		if (wy->mod_cnt > 0) {
			trm_set_lines (wy->term, vid->buf, 0, vid->buf_h);
		}

		trm_update (wy->term);
	}

	if (wy->mod_cnt > 0) {
		wy->mod_cnt -= 1;
	}

	if (vid->buf_next_w == 0) {
		vid->buf_next_w = 1280;
	}

	if (vid->buf_next_h == 0) {
		vid->buf_next_h = 800;
	}

	if ((vid->buf_w != vid->buf_next_w) || (vid->buf_h != vid->buf_next_h)) {
		pce_video_set_buf_size (vid, vid->buf_next_w, vid->buf_next_h, 3);
		wy->mod_cnt = 2;
	}

	if (wy->blink_cnt > 0) {
		wy->blink_cnt -= 1;

		if (wy->blink_cnt == 0) {
			wy->blink = !wy->blink;
			wy->blink_cnt = wy->blink_rate;
			wy->mod_cnt = 2;
		}
	}
}

static
void wy700_clock (wy700_t *wy, unsigned long cnt)
{
	unsigned      cclk;
	unsigned char ctrl;

	ctrl = wy->reg[WY700_CTRL];

	if (ctrl & 0x80) {
		wy->clock2 += cnt;

		if (wy->clock2 >= WY700_FFREQ) {
			wy700_update_native (wy);
			wy->clock2 -= WY700_FFREQ;

			trm_set_size (wy->term, wy->video.buf_w, wy->video.buf_h);
			trm_set_lines (wy->term, wy->video.buf, 0, wy->video.buf_h);
			trm_update (wy->term);
		}
	}
	else {
		if (wy->reg[WY700_MODE] & WY700_MODE_CS) {
			wy->clock1 += (unsigned long) WY700_CFREQ1 * cnt;
		}
		else {
			wy->clock1 += (unsigned long) WY700_CFREQ2 * cnt;
		}

		cclk = wy->clock1 / WY700_IFREQ;
		wy->clock1 = wy->clock1 % WY700_IFREQ;

		if (cclk > 0) {
			e6845_clock (&wy->crtc, cclk);
		}
	}
}

static
void wy700_set_blink_rate (wy700_t *wy, unsigned rate, int start)
{
	wy->blink = (start != 0);
	wy->blink_cnt = rate;
	wy->blink_rate = rate;
	wy->mod_cnt = 2;
}

static
void wy700_set_font (wy700_t *wy, unsigned font)
{
	if (font == 0) {
		wy->font = wy700_font_thick;
	}
	else {
		wy->font = wy700_font_thin;
	}

	wy->mod_cnt = 2;
}

static
void wy700_reset_native (wy700_t *wy)
{
	wy->reg[WY700_CTRL] &= ~0x80;
	wy->mod_cnt = 2;
}

/*
 * Set the mode register
 */
static
void wy700_set_mode (wy700_t *wy, unsigned char val)
{
	wy->reg[WY700_MODE] = val;
	wy700_reset_native (wy);
	wy->mod_cnt = 2;
}

/*
 * Set the color select register
 */
static
void wy700_set_color_select (wy700_t *wy, unsigned char val)
{
	if (wy->reg[WY700_CSEL] != val) {
		wy->reg[WY700_CSEL] = val;
		wy->mod_cnt = 2;
	}
}

/*
 * Get the status register
 */
static
unsigned char wy700_get_status (wy700_t *wy)
{
	unsigned char val;

	pce_video_clock1 (&wy->video, 0);

	val = wy->reg[WY700_STATUS];

	if ((wy->reg[WY700_CTRL] & 0x80)) {
		val &= ~(WY700_STATUS_VSYNC | WY700_STATUS_SYNC);

		if (wy->clock2 < (WY700_FFREQ / 16)) {
			val |= WY700_STATUS_VSYNC | WY700_STATUS_SYNC;
		}
		else if ((wy->clock2 % (WY700_FFREQ / 800)) < (WY700_FFREQ / (800 * 16))) {
			val |= WY700_STATUS_SYNC;
		}
	}
	else {
		val |= (WY700_STATUS_VSYNC | WY700_STATUS_SYNC);

		if (e6845_get_vde (&wy->crtc)) {
			val &= ~WY700_STATUS_VSYNC;

			if (e6845_get_hde (&wy->crtc)) {
				val &= ~WY700_STATUS_SYNC;
			}
		}
	}

	return (val);
}

static
void wy700_set_pen_reset (wy700_t *wy, unsigned char val)
{
	wy->reg[WY700_STATUS] &= ~WY700_STATUS_PEN;
}

static
void wy700_set_pen_set (wy700_t *wy, unsigned char val)
{
	wy->reg[WY700_STATUS] |= WY700_STATUS_PEN;
	e6845_set_pen (&wy->crtc);
}

/*
 * Set the native control port
 */
static
void wy700_set_control (wy700_t *wy, unsigned char val)
{
	wy->mod_cnt = 2;

	if ((val & 0x80) == 0) {
		switch (val) {
		case 0x00:
			break;

		case 0x01:
			wy700_set_font (wy, 0);
			break;

		case 0x02:
			wy700_set_font (wy, 0);
			break;

		case 0x03:
			wy700_set_font (wy, 1);
			break;

		default:
			fprintf (stderr, "WY700: unknown command (%02X)\n", val);
			break;
		}

		return;
	}

	wy->reg[WY700_CTRL] = val;
}

static
unsigned char wy700_reg_get_uint8 (wy700_t *wy, unsigned long addr)
{
	switch (addr) {
	case WY700_CRTC_INDEX:
	case WY700_CRTC_INDEX0:
		return (e6845_get_index (&wy->crtc));

	case WY700_CRTC_DATA:
	case WY700_CRTC_DATA0:
		return (e6845_get_data (&wy->crtc));

	case WY700_STATUS:
		return (wy700_get_status (wy));

	default:
		return (0xff);
	}
}

static
unsigned short wy700_reg_get_uint16 (wy700_t *wy, unsigned long addr)
{
	return (0xffff);
}

static
void wy700_reg_set_uint8 (wy700_t *wy, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case WY700_CRTC_INDEX:
	case WY700_CRTC_INDEX0:
		e6845_set_index (&wy->crtc, val);
		break;

	case WY700_CRTC_DATA:
	case WY700_CRTC_DATA0:
		e6845_set_data (&wy->crtc, val);
		wy700_reset_native (wy);
		wy->mod_cnt = 2;
		break;

	case WY700_MODE:
		wy700_set_mode (wy, val);
		break;

	case WY700_CSEL:
		wy700_set_color_select (wy, val);
		break;

	case WY700_PEN_RESET:
		wy700_set_pen_reset (wy, val);
		break;

	case WY700_PEN_SET:
		wy700_set_pen_set (wy, val);
		break;

	case WY700_ADDR_LOW:
	case WY700_ADDR_HIGH:
		wy->reg[addr] = val;
		wy->mod_cnt = 2;
		break;

	case WY700_CTRL:
		wy700_set_control (wy, val);
		break;

	default:
		fprintf (stderr, "WY-700: set reg (%03lX, %02X)\n", addr, val);
		break;
	}
}

static
void wy700_reg_set_uint16 (wy700_t *wy, unsigned long addr, unsigned short val)
{
	wy700_reg_set_uint8 (wy, addr, val & 0xff);

	if (addr < 15) {
		wy700_reg_set_uint8 (wy, addr + 1, (val >> 8) & 0xff);
	}
}

static
unsigned char wy700_mem_get_uint8 (wy700_t *wy, unsigned long addr)
{
	addr &= 0x1ffff;

	if (addr <= 0xffff) {
		if (wy->reg[WY700_CTRL] & WY700_CTRL_READ) {
			addr += 0x10000;
		}
	}
	else {
		addr &= 0x7fff;
	}

	return (wy->mem[addr]);
}

static
unsigned short wy700_mem_get_uint16 (wy700_t *wy, unsigned long addr)
{
	unsigned short val;

	val = wy700_mem_get_uint8 (wy, addr + 1);
	val = (val << 8) | wy700_mem_get_uint8 (wy, addr);

	return (val);
}

static
void wy700_mem_set_uint8 (wy700_t *wy, unsigned long addr, unsigned char val)
{
	addr &= 0x1ffff;

	if (addr <= 0xffff) {
		if (wy->reg[WY700_CTRL] & WY700_CTRL_WRITE) {
			addr += 0x10000;
		}
	}
	else {
		addr &= 0x7fff;
	}

	wy->mem[addr] = val;
	wy->mod_cnt = 2;
}

static
void wy700_mem_set_uint16 (wy700_t *wy, unsigned long addr, unsigned short val)
{
	wy700_mem_set_uint8 (wy, addr + 0, val & 0xff);
	wy700_mem_set_uint8 (wy, addr + 1, (val >> 8) & 0xff);
	wy->mod_cnt = 2;
}

static
int wy700_set_msg (wy700_t *wy, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.font", msg)) {
		unsigned font;

		if (msg_get_uint (val, &font)) {
			return (1);
		}

		wy700_set_font (wy, font);

		return (0);
	}

	return (-1);
}

static
void wy700_print_info (wy700_t *wy, FILE *fp)
{
	unsigned      col, row, w1, w2;
	unsigned      ch, vt, ht, vtl, vdl;
	unsigned      base, addr;
	unsigned long clk1, clk2;
	unsigned char status;
	e6845_t       *crt;

	crt = &wy->crtc;

	status = wy700_get_status (wy);

	base = e6845_get_start_address (crt);
	addr = crt->ma + crt->ccol;

	ch = (e6845_get_ml (crt) & 0x1f) + 1;
	row = ch * wy->crtc.crow + wy->crtc.ra;
	col = 8 * crt->ccol;
	vtl = e6845_get_vtl (crt);
	vdl = e6845_get_vdl (crt);
	ht = e6845_get_ht (crt) + 1;
	vt = e6845_get_vt (crt) + 1;

	clk1 = 8 * (e6845_get_ht (crt) + 1);
	w1 = 8 * e6845_get_hd (crt);
	w2 = clk1;

	if ((wy->reg[WY700_MODE] ^ WY700_MODE_CS) & (WY700_MODE_CS | WY700_MODE_G640)) {
		clk1 *= 2;
	}

	clk2 = clk1 * vtl;

	fprintf (fp, "DEV: WY-700\n");

	fprintf (fp,
		"INDX[3D4]=%02X   COL=%3u/%3u  HFRQ=%9.3f  %u*%u\n"
		"MODE[3D8]=%02X   ROW=%3u/%3u  VFRQ=%9.3f  %u*%u\n"
		"CSEL[3D9]=%02X  CCOL=%3u/%3u   HDE=%d  HSYN=%X  BASE=%04X\n"
		"STAT[3DA]=%02X  CROW=%3u/%3u   VDE=%d  VSYN=%X  ADDR=%04X\n"
		"ADRL[3DD]=%02X    RA=%3u/%3u\n"
		"ADRH[3DE]=%02X\n"
		"CTRL[3DF]=%02X\n",
		e6845_get_index (crt), col, w2, (double) WY700_PFREQ / clk1, w1, vdl,
		wy->reg[WY700_MODE], row, vtl, (double) WY700_PFREQ / clk2, w2, vtl,
		wy->reg[WY700_CSEL], crt->ccol, ht, e6845_get_hde (crt), crt->hsync_cnt, base,
		status, crt->crow, vt, e6845_get_vde (crt), crt->vsync_cnt, addr,
		wy->reg[WY700_ADDR_LOW], crt->ra, ch,
		wy->reg[WY700_ADDR_HIGH],
		wy->reg[WY700_CTRL]
	);

	fprintf (fp,
		"HT[00]=%02X  VT[04]=%02X  IL[08]=%02X  AH[12]=%02X  LH[16]=%02X\n"
		"HD[01]=%02X  VA[05]=%02X  ML[09]=%02X  AL[13]=%02X  LL[17]=%02X\n"
		"HS[02]=%02X  VD[06]=%02X  CS[10]=%02X  CH[14]=%02X  ROWDSP=%u\n"
		"SW[03]=%02X  VS[07]=%02X  CE[11]=%02X  CL[15]=%02X  ROWTOT=%u\n",
		crt->reg[0], crt->reg[4], crt->reg[8], crt->reg[12], crt->reg[16],
		crt->reg[1], crt->reg[5], crt->reg[9], crt->reg[13], crt->reg[17],
		crt->reg[2], crt->reg[6], crt->reg[10], crt->reg[14], e6845_get_vdl (crt),
		crt->reg[3], crt->reg[7], crt->reg[11], crt->reg[15], e6845_get_vtl (crt)
	);

	fflush (fp);
}

static
void wy700_set_terminal (wy700_t *wy, terminal_t *trm)
{
	wy->term = trm;

	if (wy->term != NULL) {
		trm_open (wy->term, 1280, 800);
	}
}

static
mem_blk_t *wy700_get_mem (wy700_t *wy)
{
	return (wy->memblk);
}

static
mem_blk_t *wy700_get_reg (wy700_t *wy)
{
	return (wy->regblk);
}

static
void wy700_init (wy700_t *wy, unsigned long io, unsigned long addr)
{
	pce_video_init (&wy->video);

	wy->video.ext = wy;
	wy->video.del = (void *) wy700_del;
	wy->video.set_msg = (void *) wy700_set_msg;
	wy->video.set_terminal = (void *) wy700_set_terminal;
	wy->video.get_mem = (void *) wy700_get_mem;
	wy->video.get_reg = (void *) wy700_get_reg;
	wy->video.set_blink_rate = (void *) wy700_set_blink_rate;
	wy->video.print_info = (void *) wy700_print_info;
	wy->video.clock = (void *) wy700_clock;

	wy->memblk = mem_blk_new (addr, 131072, 1);
	mem_blk_set_fget (wy->memblk, wy, wy700_mem_get_uint8, wy700_mem_get_uint16, NULL);
	mem_blk_set_fset (wy->memblk, wy, wy700_mem_set_uint8, wy700_mem_set_uint16, NULL);
	mem_blk_clear (wy->memblk, 0x00);
	wy->mem = wy->memblk->data;

	wy->regblk = mem_blk_new (io, 16, 1);
	mem_blk_set_fget (wy->regblk, wy, wy700_reg_get_uint8, wy700_reg_get_uint16, NULL);
	mem_blk_set_fset (wy->regblk, wy, wy700_reg_set_uint8, wy700_reg_set_uint16, NULL);
	mem_blk_clear (wy->regblk, 0x00);
	wy->reg = wy->regblk->data;

	e6845_init (&wy->crtc);
	e6845_set_hsync_fct (&wy->crtc, wy, wy700_hsync);
	e6845_set_vsync_fct (&wy->crtc, wy, wy700_vsync);

	wy->term = NULL;
	wy->font = wy700_font_thick;
	wy->clock1 = 0;
	wy->clock2 = 0;

	wy->mod_cnt = 0;

	wy->blink = 0;
	wy->blink_cnt = 0;
	wy->blink_rate = 16;
}

static
void wy700_free (wy700_t *wy)
{
	e6845_free (&wy->crtc);

	mem_blk_del (wy->memblk);
	mem_blk_del (wy->regblk);
}

wy700_t *wy700_new (unsigned long io, unsigned long addr)
{
	wy700_t *wy;

	if ((wy = malloc (sizeof (wy700_t))) == NULL) {
		return (NULL);
	}

	wy700_init (wy, io, addr);

	return (wy);
}

static
void wy700_del (wy700_t *wy)
{
	if (wy != NULL) {
		wy700_free (wy);
		free (wy);
	}
}

video_t *wy700_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      blink, font;
	wy700_t       *wy;

	ini_get_uint32 (sct, "io", &io, 0x3d0);
	ini_get_uint32 (sct, "address", &addr, 0xa0000);
	ini_get_uint16 (sct, "blink", &blink, 16);
	ini_get_uint16 (sct, "font", &font, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "WY-700 io=0x%04lx addr=0x%05lx font=%u blink=%u\n",
		io, addr, font, blink
	);

	if ((wy = wy700_new (io, addr)) == NULL) {
		return (NULL);
	}

	wy700_set_font (wy, font);
	wy700_set_blink_rate (wy, blink, 1);

	return (&wy->video);
}
