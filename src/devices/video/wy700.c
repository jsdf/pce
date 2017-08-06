/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/wy700.c                                    *
 * Created:     2008-10-13 by John Elliott <jce@seasip.demon.co.uk>          *
 * Copyright:   (C) 2008-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/log.h>

#include <devices/video/cga.h>
#include <devices/video/wy700.h>
#include <devices/video/wy700_font.h>


#define WY700_MODE        8
#define WY700_CSEL        9
#define WY700_STATUS      10
#define WY700_BANK_OFS    13
#define WY700_BANK_BASE   14
#define WY700_CONTROL     15

#define WY700_CRTC_HT      0
#define WY700_CRTC_HD      1
#define WY700_CRTC_HS      2
#define WY700_CRTC_SYN     3
#define WY700_CRTC_VT      4
#define WY700_CRTC_VTA     5
#define WY700_CRTC_VD      6
#define WY700_CRTC_VS      7
#define WY700_CRTC_IL      8
#define WY700_CRTC_MS      9
#define WY700_CRTC_CS      10
#define WY700_CRTC_CE      11
#define WY700_CRTC_SH      12
#define WY700_CRTC_SL      13
#define WY700_CRTC_PH      14
#define WY700_CRTC_PL      15
#define WY700_CRTC_LH      16
#define WY700_CRTC_LL      17

#define WY700_MODE_CS      0x01		/* clock select */
#define WY700_MODE_G320    0x02
#define WY700_MODE_CBURST  0x04
#define WY700_MODE_ENABLE  0x08
#define WY700_MODE_G640    0x10
#define WY700_MODE_BLINK   0x20

#define WY700_CSEL_COL     0x0f
#define WY700_CSEL_INT     0x10
#define WY700_CSEL_PAL     0x20

#define WY700_STATUS_SYNC  0x01		/* -display enable */
#define WY700_STATUS_PEN   0x02
#define WY700_STATUS_VSYNC 0x08		/* vertical sync */

#define WY700_UPDATE_DIRTY   1
#define WY700_UPDATE_RETRACE 2


typedef struct {
	const unsigned char *font;
} wy700_ext_t;


static
void wy700_get_color (cga_t *wy, unsigned a, unsigned char *fg, unsigned char *bg)
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

/*
 * Get CRTC start offset
 */
static
unsigned wy700_get_start (cga_t *wy)
{
	unsigned val;

	val = wy->reg_crt[WY700_CRTC_SH];
	val = (val << 8) | wy->reg_crt[WY700_CRTC_SL];

	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned wy700_get_cursor (cga_t *wy)
{
	unsigned val;

	val = wy->reg_crt[WY700_CRTC_PH];
	val = (val << 8) | wy->reg_crt[WY700_CRTC_PL];

	return (val);
}

/*
 * Get the on screen cursor position
 */
int wy700_get_position (cga_t *wy, unsigned *x, unsigned *y)
{
	unsigned pos, ofs;

	if (wy->w == 0) {
		return (1);
	}

	pos = wy700_get_cursor (wy);
	ofs = wy700_get_start (wy);

	pos = (pos - ofs) & 0x3fff;

	if (pos >= (wy->w * wy->h)) {
		return (1);
	}

	*x = pos % wy->w;
	*y = pos / wy->w;

	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void wy700_update_mode0_cursor (cga_t *wy)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
	unsigned char       fg, bg;
	unsigned char       *dst;

	if (wy->blink_on == 0) {
		return;
	}

	if ((wy->reg_crt[WY700_CRTC_CS] & 0x60) == 0x20) {
		/* cursor off */
		return;
	}

	if (wy700_get_position (wy, &x, &y)) {
		return;
	}

	c1 = wy->reg_crt[WY700_CRTC_CS] & 0x1f;
	c2 = wy->reg_crt[WY700_CRTC_CE] & 0x1f;

	if (c1 >= wy->ch) {
		return;
	}

	if (c2 >= wy->ch) {
		c2 = wy->ch - 1;
	}

	wy700_get_color (wy, wy->mem[2 * (wy->w * y + x) + 1], &fg, &bg);

	dst = wy->buf + 3 * 16 * (wy->w * 4 * (wy->ch * y + c1) + x);

	for (j = (4 * c1); j <= (4 * c2) ; j++) {
		for (i = 0; i < 16; i++) {
			dst[3 * i + 0] = fg;
			dst[3 * i + 1] = fg;
			dst[3 * i + 2] = fg;
		}
		dst += 3 * 16 * wy->w;
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void wy700_update_mode0_char (cga_t *wy,
	unsigned char *dst, unsigned char c, unsigned char a)
{
	unsigned            i, j;
	int                 blink;
	unsigned short      val;
	const unsigned char *fnt;
	unsigned char       fg, bg, col;

	blink = 0;

	if (wy->reg[WY700_MODE] & WY700_MODE_BLINK) {
		if (a & 0x80) {
			blink = !wy->blink_on;

			if ((wy->reg[WY700_CSEL] & WY700_CSEL_INT) == 0) {
				a &= 0x7f;
			}
		}
	}

	wy700_get_color (wy, a, &fg, &bg);

	fnt = ((wy700_ext_t *) wy->ext)->font + 32 * c;

	for (j = 0; j < (4 * wy->ch); j++) {
		if (blink) {
			val = 0x0000;
		}
		else {

			val = (fnt[j & 30] << 8) | fnt[(j & 30) + 1];
		}

		for (i = 0; i < 16; i++) {
			col = (val & 0x8000) ? fg : bg;

			dst[3 * i + 0] = col;
			dst[3 * i + 1] = col;
			dst[3 * i + 2] = col;

			val <<= 1;
		}

		dst += (3 * 16 * wy->w);
	}
}

/*
 * Update text 80 * 25 * 4
 */
static
void wy700_update_mode0 (cga_t *wy)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;

	if (cga_set_buf_size (wy, 16 * wy->w, 4 * wy->ch * wy->h)) {
		return;
	}

	src = wy->mem;
	ofs = (2 * wy700_get_start (wy)) & 0x3ffe;

	for (y = 0; y < wy->h; y++) {
		dst = wy->buf + 3 * (16 * wy->w) * (4 * wy->ch * y);

		for (x = 0; x < wy->w; x++) {
			wy700_update_mode0_char (wy,
				dst + 3 * 16 * x, src[ofs], src[ofs + 1]
			);

			ofs = (ofs + 2) & 0x3ffe;
		}
	}

	wy700_update_mode0_cursor (wy);
}

/*
 * Update CGA 320 * 200 * 4
 */
static
void wy700_update_320x200 (cga_t *wy)
{
	unsigned            i, x, y, w, h;
	unsigned            ofs, val, col;
	unsigned char       *dst;
	const unsigned char *mem;

	if (cga_set_buf_size (wy, 8 * wy->w, 2 * wy->h)) {
		return;
	}

	dst = wy->buf;

	w = 2 * wy->w;
	h = 2 * wy->h;

	ofs = (2 * wy700_get_start (wy)) & 0x1fff;

	for (y = 0; y < h; y++) {
		mem = wy->mem + ((y & 1) << 13);

		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];

			for (i = 0; i < 4; i++) {
				col = val & 0xc0;
				col |= (col >> 2) | (col >> 4) | (col >> 6);

				*(dst++) = col;
				*(dst++) = col;
				*(dst++) = col;

				val <<= 2;
			}
		}

		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Update CGA 640 * 200 * 2
 */
static
void wy700_update_640x200 (cga_t *wy)
{
	unsigned            i, x, y, w, h;
	unsigned            ofs, val, col;
	unsigned char       *dst;
	const unsigned char *mem;

	if (cga_set_buf_size (wy, 16 * wy->w, 2 * wy->h)) {
		return;
	}

	dst = wy->buf;

	h = 2 * wy->h;
	w = 2 * wy->w;

	ofs = (2 * cga_get_start (wy)) & 0x1fff;

	for (y = 0; y < h; y++) {
		mem = wy->mem + ((y & 1) << 13);

		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];

			for (i = 0; i < 8; i++) {
				col = (val & 0x80) ? 0xff : 0x00;
				*(dst++) = col;
				*(dst++) = col;
				*(dst++) = col;
				val <<= 1;
			}
		}

		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Update native 640 * 400 * 2
 */
static
void wy700_update_640x400x2 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 640, 400)) {
		return;
	}

	dst = wy->buf;
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
 * Update native 320 * 400 * 4
 */
static
void wy700_update_320x400x4 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val, col;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 320, 400)) {
		return;
	}

	dst = wy->buf;
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
 * Update native 1280 * 400 * 2
 */
static
void wy700_update_1280x400x2 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 1280, 400)) {
		return;
	}

	dst = wy->buf;
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
 * Update native 640 * 400 * 4
 */
static
void wy700_update_640x400x4 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val, col;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 640, 400)) {
		return;
	}

	dst = wy->buf;
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
 * Update native 1280 * 800 * 2
 */
static
void wy700_update_1280x800x2 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 1280, 800)) {
		return;
	}

	dst = wy->buf;
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
 * Update native 640 * 800 * 4
 */
static
void wy700_update_640x800x4 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            ofs;
	unsigned char       val, col;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 640, 800)) {
		return;
	}

	dst = wy->buf;
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
void wy700_update_native (cga_t *wy)
{
	unsigned char ctl;

	ctl = wy->reg[WY700_CONTROL];

	if ((ctl & 0x08) == 0) {
		cga_update (wy);

		return;
	}

	switch ((ctl >> 4) & 7) {
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
		fprintf (stderr, "WY700: unknown mode (%02X)\n", ctl);
		break;
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void wy700_update_blank (cga_t *wy)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	cga_set_buf_size (wy, 1280, 800);

	dst = wy->buf;

	for (y = 0; y < wy->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < wy->buf_w; x++) {
			fx = (x % 16) < 8;

			dst[0] = (fx != fy) ? 0x20 : 0x00;
			dst[1] = dst[0];
			dst[2] = dst[0];

			dst += 3;
		}
	}
}

/*
 * Update the internal screen buffer
 */
void wy700_update (cga_t *wy)
{
	unsigned char mode, ctl;

	wy->buf_w = 0;
	wy->buf_h = 0;

	mode = wy->reg[WY700_MODE];
	ctl = wy->reg[WY700_CONTROL];

	if (ctl & 0x80) {
		wy700_update_native (wy);
	}
	else if ((mode & WY700_MODE_ENABLE) == 0) {
		wy700_update_blank (wy);
	}
	else if (mode & WY700_MODE_G320) {
		if (mode & WY700_MODE_G640) {
			wy700_update_640x200 (wy);
		}
		else {
			wy700_update_320x200 (wy);
		}
	}
	else {
		wy700_update_mode0 (wy);
	}
}


static
unsigned char wy700_get_control (const cga_t *wy)
{
	return (wy->reg[WY700_CONTROL]);
}

static
void wy700_set_control (cga_t *wy, unsigned char val)
{
	if (wy->reg[WY700_CONTROL] == val) {
		return;
	}

	wy->reg[WY700_CONTROL] = val;

	wy->update_state |= WY700_UPDATE_DIRTY;
}

/*
 * Get a Wyse 700 register
 */
static
unsigned char wy700_reg_get_uint8 (cga_t *wy, unsigned long addr)
{
	switch (addr) {
	case WY700_CONTROL:
		return (wy700_get_control (wy));

	default:
		return (cga_reg_get_uint8 (wy, addr));
	}
}

static
unsigned short wy700_reg_get_uint16 (cga_t *wy, unsigned long addr)
{
	return (wy700_reg_get_uint8 (wy, addr));
}

/*
 * Set a Wyse 700 register
 */
static
void wy700_reg_set_uint8 (cga_t *wy, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case WY700_BANK_OFS:
		wy->reg[WY700_BANK_OFS] = val;
		break;

	case WY700_BANK_BASE:
		wy->reg[WY700_BANK_BASE] = val;
		break;

	case WY700_CONTROL:
		wy700_set_control (wy, val);
		break;

	case WY700_MODE:
		if (val == 0) {
			wy700_set_control (wy, 0);
		}
		cga_reg_set_uint8 (wy, addr, val);
		break;

	default:
		cga_reg_set_uint8 (wy, addr, val);
		break;
	}
}

void wy700_reg_set_uint16 (cga_t *wy, unsigned long addr, unsigned short val)
{
	wy700_reg_set_uint8 (wy, addr, val & 0xff);
}

static
unsigned char wy700_mem_get_uint8 (cga_t *wy, unsigned long addr)
{
	unsigned char ctl;

	ctl = wy->reg[WY700_CONTROL];

	if (addr <= 0xffff) {
		if (ctl & 1) {
			addr += 0x10000;
		}
	}
	else {
		addr &= 0x7fff;
	}

	return (wy->mem[addr]);
}

static
unsigned short wy700_mem_get_uint16 (cga_t *wy, unsigned long addr)
{
	unsigned short val;

	val = wy700_mem_get_uint8 (wy, addr);

	if ((addr + 1) < wy->memblk->size) {
		val |= (unsigned) wy700_mem_get_uint8 (wy, addr + 1) << 8;
	}

	return (val);
}

static
void wy700_mem_set_uint8 (cga_t *wy, unsigned long addr, unsigned char val)
{
	unsigned char ctl;

	ctl = wy->reg[WY700_CONTROL];

	if (addr <= 0xffff) {
		if (ctl & 2) {
			addr += 0x10000;
		}
	}
	else {
		addr &= 0x7fff;
	}

	if (wy->mem[addr] == val) {
		return;
	}

	wy->mem[addr] = val;

	wy->update_state |= WY700_UPDATE_DIRTY;
}

static
void wy700_mem_set_uint16 (cga_t *wy, unsigned long addr, unsigned short val)
{
	wy700_mem_set_uint8 (wy, addr, val & 0xff);

	if ((addr + 1) < wy->memblk->size) {
		wy700_mem_set_uint8 (wy, addr + 1, (val >> 8) & 0xff);
	}
}


static
void wy700_set_terminal (cga_t *wy, terminal_t *trm)
{
	wy->term = trm;

	if (wy->term != NULL) {
		trm_open (wy->term, 1280, 800);
	}
}


static
void wy700_print_info (cga_t *wy, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: Wyse 700\n");

	fprintf (fp, "CGA: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		cga_get_start (wy),
		cga_get_cursor (wy),
		wy->reg[WY700_CSEL] & 0x0f,
		(wy->reg[WY700_CSEL] >> 5) & 1
	);

	fprintf (fp,
		"REG: MODE=%02X  CSEL=%02X  STATUS=%02X"
		"  PAL=[%02X %02X %02X %02X]\n",
		wy->reg[WY700_MODE],
		wy->reg[WY700_CSEL],
		wy->reg[WY700_STATUS],
		wy->pal[0], wy->pal[1], wy->pal[2], wy->pal[3]
	);

	fprintf (fp,
		"REG: CTL=%02X  BANK OFS=%02X  BANK BAS=%02X\n",
		wy->reg[WY700_CONTROL],
		wy->reg[WY700_BANK_OFS],
		wy->reg[WY700_BANK_BASE]
	);

	fprintf (fp, "CRTC=[%02X", wy->reg_crt[0]);
	for (i = 1; i < 18; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", wy->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}


static
void wy700_set_font (cga_t *wy, unsigned font)
{
	wy700_ext_t *ext;

	ext = wy->ext;

	if (font == 0) {
		ext->font = wy700_font_thick;
	}
	else {
		ext->font = wy700_font_thin;
	}
}

void wy700_init (cga_t *wy, unsigned long io, unsigned long addr, unsigned long size)
{
	if (size < 131072) {
		size = 131072;
	}

	cga_init (wy, io, addr, size);

	wy->ext = malloc (sizeof (wy700_ext_t));

	wy->video.set_terminal = (void *) wy700_set_terminal;
	wy->video.print_info = (void *) wy700_print_info;

	wy->update = wy700_update;

	wy->regblk->set_uint8 = (void *) wy700_reg_set_uint8;
	wy->regblk->set_uint16 = (void *) wy700_reg_set_uint16;
	wy->regblk->get_uint8 = (void *) wy700_reg_get_uint8;
	wy->regblk->get_uint16 = (void *) wy700_reg_get_uint16;

	wy->memblk->set_uint8 = (void *) wy700_mem_set_uint8;
	wy->memblk->set_uint16 = (void *) wy700_mem_set_uint16;
	wy->memblk->get_uint8 = (void *) wy700_mem_get_uint8;
	wy->memblk->get_uint16 = (void *) wy700_mem_get_uint16;

	wy700_set_font (wy, 0);
}

void wy700_free (cga_t *wy)
{
	free (wy->ext);

	cga_free (wy);
}

cga_t *wy700_new (unsigned long io, unsigned long addr, unsigned long size)
{
	cga_t *wy;

	if ((wy = malloc (sizeof (cga_t))) == NULL) {
		return (NULL);
	}

	wy700_init (wy, io, addr, size);

	return (wy);
}

video_t *wy700_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned      blink, font;
	cga_t         *wy;

	ini_get_uint32 (sct, "io", &io, 0x3d0);
	ini_get_uint32 (sct, "address", &addr, 0xa0000);
	ini_get_uint32 (sct, "size", &size, 131072);
	ini_get_uint16 (sct, "font", &font, 0);
	ini_get_uint16 (sct, "blink", &blink, 0);

	if (size < 131072) {
		size = 131072;
	}

	pce_log_tag (MSG_INF,
		"VIDEO:", "WY-700 io=0x%04lx addr=0x%05lx size=0x%05lx font=%u\n",
		io, addr, size, font
	);

	if ((wy = wy700_new (io, addr, size)) == NULL) {
		return (NULL);
	}

	wy700_set_font (wy, font);

	cga_set_blink_rate (wy, blink);

	return (&wy->video);
}
