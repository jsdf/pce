/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/hgc.c                                      *
 * Created:     2003-08-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2008 Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/log.h>

#include <devices/video/hgc.h>


#define HGC_IFREQ 1193182
#define HGC_PFREQ 16257000
#define HGC_HFREQ 18430
#define HGC_VFREQ 50

/* character width */
#define HGC_CW    9

/* hgc registers */
#define HGC_CRTC_INDEX   0
#define HGC_CRTC_DATA    1
#define HGC_MODE         4
#define HGC_STATUS       6
#define HGC_CONFIG       11

#define HGC_CRTC_HT      0
#define HGC_CRTC_HD      1
#define HGC_CRTC_HS      2
#define HGC_CRTC_SYN     3
#define HGC_CRTC_VT      4
#define HGC_CRTC_VTA     5
#define HGC_CRTC_VD      6
#define HGC_CRTC_VS      7
#define HGC_CRTC_IL      8
#define HGC_CRTC_MS      9
#define HGC_CRTC_CS      10
#define HGC_CRTC_CE      11
#define HGC_CRTC_SH      12
#define HGC_CRTC_SL      13
#define HGC_CRTC_PH      14
#define HGC_CRTC_PL      15
#define HGC_CRTC_LH      16
#define HGC_CRTC_LL      17

/* mode control register */
#define HGC_MODE_GRAPH   0x02
#define HGC_MODE_ENABLE  0x08
#define HGC_MODE_BLINK   0x20
#define HGC_MODE_PAGE1   0x80

/* CRTC status register */
#define HGC_STATUS_HSYNC 0x01		/* horizontal sync */
#define HGC_STATUS_LIGHT 0x02
#define HGC_STATUS_VIDEO 0x08
#define HGC_STATUS_VSYNC 0x80		/* -vertical sync */

/* configuration switch register */
#define HGC_CONFIG_GRAPH 0x01
#define HGC_CONFIG_PAGE1 0x02


#include "hgc_font.h"


static
unsigned char hgc_rgb[16][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xe8, 0x90, 0x50 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 },
	{ 0xff, 0xf0, 0xc8 }
};


/*
 * Get CRTC start offset
 */
static
unsigned hgc_get_start (hgc_t *hgc)
{
	unsigned val;

	val = hgc->reg_crt[0x0c];
	val = (val << 8) | hgc->reg_crt[0x0d];

	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned hgc_get_cursor (hgc_t *hgc)
{
	unsigned val;

	val = hgc->reg_crt[0x0e];
	val = (val << 8) | hgc->reg_crt[0x0f];

	return (val);
}

/*
 * Get the on screen cursor position
 */
static
int hgc_get_position (hgc_t *hgc, unsigned *x, unsigned *y)
{
	unsigned pos, ofs;

	if ((hgc->w == 0) || (hgc->h == 0)) {
		return (1);
	}

	pos = hgc_get_cursor (hgc) & 0x3fff;
	ofs = hgc_get_start (hgc) & 0x3fff;

	if ((pos < ofs) || (pos >= (ofs + hgc->w * hgc->h))) {
		return (1);
	}

	*x = (pos - ofs) % hgc->w;
	*y = (pos - ofs) / hgc->w;

	return (0);
}

/*
 * Get a pointer to the active page
 */
static
unsigned char *hgc_get_page (hgc_t *hgc)
{
	unsigned char *val;

	val = hgc->mem;

	if (hgc->reg[HGC_MODE] & HGC_MODE_PAGE1) {
		val += 32768;
	}

	return (val);
}

/*
 * Set the timing values from the CRTC registers
 */
static
void hgc_set_timing (hgc_t *hgc)
{
	hgc->ch = (hgc->reg_crt[HGC_CRTC_MS] & 0x1f) + 1;
	hgc->w = hgc->reg_crt[HGC_CRTC_HD];
	hgc->h = hgc->reg_crt[HGC_CRTC_VD];

	if (hgc->reg[HGC_MODE] & HGC_MODE_GRAPH) {
		hgc->clk_ht = 16 * (hgc->reg_crt[HGC_CRTC_HT] + 1);
		hgc->clk_hs = 16 * hgc->reg_crt[HGC_CRTC_HS];
	}
	else {
		hgc->clk_ht = HGC_CW * (hgc->reg_crt[HGC_CRTC_HT] + 1);
		hgc->clk_hs = HGC_CW * hgc->reg_crt[HGC_CRTC_HS];
	}

	hgc->clk_vt = hgc->ch * (hgc->reg_crt[HGC_CRTC_VT] + 1) * hgc->clk_ht;
	hgc->clk_vs = hgc->ch * hgc->reg_crt[HGC_CRTC_VS] * hgc->clk_ht;
}

/*
 * Get the dot clock
 */
static
unsigned long hgc_get_dotclock (hgc_t *hgc)
{
	unsigned long long clk;

	clk = hgc->video.dotclk[0];
	clk = (HGC_PFREQ * clk) / HGC_IFREQ;

	return (clk);
}

/*
 * Set the internal screen buffer size
 */
static
int hgc_set_buf_size (hgc_t *hgc, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > hgc->bufmax) {
		tmp = realloc (hgc->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		hgc->buf = tmp;
		hgc->bufmax = cnt;
	}

	hgc->buf_w = w;
	hgc->buf_h = h;

	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void hgc_text_draw_cursor (hgc_t *hgc)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
	const unsigned char *src;
	const unsigned char *col;
	unsigned char       *dst;

	if ((hgc->reg_crt[HGC_CRTC_CS] & 0x60) == 0x20) {
		/* cursor off */
		return;
	}

	src = hgc_get_page (hgc);

	if (hgc_get_position (hgc, &x, &y)) {
		return;
	}

	c1 = hgc->reg_crt[HGC_CRTC_CS] & 0x1f;
	c2 = hgc->reg_crt[HGC_CRTC_CE] & 0x1f;

	if (c1 >= hgc->ch) {
		return;
	}

	if (c2 >= hgc->ch) {
		c2 = hgc->ch - 1;
	}

	col = hgc_rgb[src[2 * (hgc->w * y + x) + 1] & 0x0f];
	dst = hgc->buf + 3 * HGC_CW * (hgc->w * (hgc->ch * y + c1) + x);

	for (j = c1; j <= c2; j++) {
		for (i = 0; i < HGC_CW; i++) {
			dst[3 * i + 0] = col[0];
			dst[3 * i + 1] = col[1];
			dst[3 * i + 2] = col[2];
		}
		dst += 3 * HGC_CW * hgc->w;
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void hgc_text_draw_char (hgc_t *hgc,
	unsigned char *buf, unsigned char c, unsigned char a)
{
	unsigned            i, j;
	unsigned            map;
	unsigned char       *dst;
	const unsigned char *fg, *bg;

	if (hgc->reg[HGC_MODE] & HGC_MODE_BLINK) {
		/* blinking is not supported */
		a &= 0x7f;
	}

	fg = hgc_rgb[a & 0x0f];
	bg = hgc_rgb[(a >> 4) & 0x0f];

	dst = buf;

	for (j = 0; j < hgc->ch; j++) {
		map = hgc->font[14 * c + (j % 14)];

		if ((c >= 0xc0) && (c <= 0xdf)) {
			map = (map << 1) | (map & 1);
		}
		else {
			map = map << 1;
		}

		for (i = 0; i < HGC_CW; i++) {
			if (map & (0x100 >> i)) {
				dst[3 * i + 0] = fg[0];
				dst[3 * i + 1] = fg[1];
				dst[3 * i + 2] = fg[2];
			}
			else {
				dst[3 * i + 0] = bg[0];
				dst[3 * i + 1] = bg[1];
				dst[3 * i + 2] = bg[2];
			}
		}

		dst += (3 * HGC_CW * hgc->w);
	}

	/* underline */
	if (((a & 0x07) == 1) && (hgc->ch >= 13)) {
		dst = buf + 3 * 13 * (HGC_CW * hgc->w);
		for (i = 0; i < HGC_CW; i++) {
			dst[3 * i + 0] = fg[0];
			dst[3 * i + 1] = fg[1];
			dst[3 * i + 2] = fg[2];
		}
	}
}

/*
 * Update text mode
 */
static
void hgc_update_text (hgc_t *hgc)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;

	if (hgc_set_buf_size (hgc, HGC_CW * hgc->w, hgc->ch * hgc->h)) {
		return;
	}

	src = hgc_get_page (hgc);
	ofs = (hgc_get_start (hgc) << 1) & 0x7ffe;

	dst = hgc->buf;

	for (y = 0; y < hgc->h; y++) {
		for (x = 0; x < hgc->w; x++) {
			hgc_text_draw_char (hgc, dst + 3 * HGC_CW * x,
				src[ofs], src[ofs + 1]
			);

			ofs = (ofs + 2) & 0x7ffe;
		}

		dst += 3 * (HGC_CW * hgc->w) * hgc->ch;
	}

	hgc_text_draw_cursor (hgc);
}

/*
 * Update graphic mode
 */
static
void hgc_update_graph (hgc_t *hgc)
{
	unsigned      x, y, cx, cy;
	unsigned      w, h;
	unsigned      addr;
	unsigned char val;
	unsigned char *mem, *src;
	unsigned char *dst;

	w = 2 * hgc->w;
	h = hgc->ch * hgc->h;

	if (hgc_set_buf_size (hgc, 8 * w, h)) {
		return;
	}

	mem = hgc_get_page (hgc);
	dst = hgc->buf;

	cy = 0;
	addr = 0;

	for (y = 0; y < h; y++) {
		src = mem + (cy & 3) * 0x2000;

		for (x = 0; x < w; x++) {
			val = src[(addr + x) & 0x1fff];

			for (cx = 0; cx < 8; cx++) {
				if (val & 0x80) {
					dst[0] = hgc_rgb[15][0];
					dst[1] = hgc_rgb[15][1];
					dst[2] = hgc_rgb[15][2];
				}
				else {
					dst[0] = hgc_rgb[0][0];
					dst[1] = hgc_rgb[0][1];
					dst[2] = hgc_rgb[0][2];
				}

				dst += 3;
				val <<= 1;
			}
		}

		cy += 1;

		if (cy >= hgc->ch) {
			cy = 0;
			addr = (addr + 2 * hgc->w) & 0x1fff;
		}
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void hgc_update_blank (hgc_t *hgc)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	hgc_set_buf_size (hgc, 720, 350);

	dst = hgc->buf;

	for (y = 0; y < hgc->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < hgc->buf_w; x++) {
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
static
void hgc_update (hgc_t *hgc)
{
	if ((hgc->reg[HGC_MODE] & HGC_MODE_ENABLE) == 0) {
		hgc_update_blank (hgc);
		return;
	}

	if ((hgc->w == 0) || (hgc->h == 0)) {
		hgc_update_blank (hgc);
		return;
	}

	if (hgc->reg[HGC_MODE] & HGC_MODE_GRAPH) {
		hgc_update_graph (hgc);
	}
	else {
		hgc_update_text (hgc);
	}
}


/*
 * Get a CRTC register
 */
static
unsigned char hgc_crtc_get_reg (hgc_t *hgc, unsigned reg)
{
	if (reg > 15) {
		return (0xff);
	}

	return (hgc->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void hgc_crtc_set_reg (hgc_t *hgc, unsigned reg, unsigned char val)
{
	if (reg > 15) {
		return;
	}

	if (hgc->reg_crt[reg] == val) {
		return;
	}

	hgc->reg_crt[reg] = val;

	hgc_set_timing (hgc);

	hgc->update_state |= 1;
}


/*
 * Get the CRTC index register
 */
static
unsigned char hgc_get_crtc_index (hgc_t *hgc)
{
	return (hgc->reg[HGC_CRTC_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char hgc_get_crtc_data (hgc_t *hgc)
{
	return (hgc_crtc_get_reg (hgc, hgc->reg[HGC_CRTC_INDEX]));
}

/*
 * Get the configuration register
 */
static
unsigned char hgc_get_config (hgc_t *hgc)
{
	return (hgc->reg[HGC_CONFIG]);
}

/*
 * Get the mode control register
 */
static
unsigned char hgc_get_mode (hgc_t *hgc)
{
	return (hgc->reg[HGC_MODE]);
}

/*
 * Get the status register
 */
static
unsigned char hgc_get_status (hgc_t *hgc)
{
	unsigned char val, vid;
	unsigned long clk;

	clk = hgc_get_dotclock (hgc);

	/* simulate the video signal */
	hgc->reg[HGC_STATUS] ^= HGC_STATUS_VIDEO;

	val = hgc->reg[HGC_STATUS] & ~HGC_STATUS_VIDEO;
	vid = hgc->reg[HGC_STATUS] & HGC_STATUS_VIDEO;

	val |= HGC_STATUS_HSYNC;
	val &= ~HGC_STATUS_VSYNC;

	if (clk < hgc->clk_vs) {
		val |= HGC_STATUS_VSYNC;
	}

	if (hgc->clk_ht > 0) {
		if ((clk % hgc->clk_ht) < hgc->clk_hs) {
			val &= ~HGC_STATUS_HSYNC;

			if (clk < hgc->clk_vs) {
				val |= vid;
			}
		}
	}

	return (val);
}

/*
 * Set the CRTC index register
 */
static
void hgc_set_crtc_index (hgc_t *hgc, unsigned char val)
{
	hgc->reg[HGC_CRTC_INDEX] = val;
}

/*
 * Set the CRTC data register
 */
static
void hgc_set_crtc_data (hgc_t *hgc, unsigned char val)
{
	hgc->reg[HGC_CRTC_DATA] = val;

	hgc_crtc_set_reg (hgc, hgc->reg[HGC_CRTC_INDEX], val);
}

/*
 * Set the configuration register
 */
static
void hgc_set_config (hgc_t *hgc, unsigned char val)
{
	hgc->reg[HGC_CONFIG] = val;
}

/*
 * Set the mode control register
 */
static
void hgc_set_mode (hgc_t *hgc, unsigned char val)
{
	unsigned char cfg;

	cfg = hgc_get_config (hgc);

	if ((cfg & HGC_CONFIG_GRAPH) == 0) {
		val &= ~HGC_MODE_GRAPH;
	}

	if ((cfg & HGC_CONFIG_PAGE1) == 0) {
		val &= ~HGC_MODE_PAGE1;
	}

	if (mem_blk_get_size (hgc->memblk) < 65536) {
		val &= ~HGC_MODE_PAGE1;
	}

	if (hgc->reg[HGC_MODE] == val) {
		return;
	}

	hgc->reg[HGC_MODE] = val;

	hgc_set_timing (hgc);

	hgc->update_state |= 1;
}

/*
 * Get an HGC register
 */
static
unsigned char hgc_reg_get_uint8 (hgc_t *hgc, unsigned long addr)
{
	switch (addr) {
	case HGC_CRTC_INDEX:
		return (hgc_get_crtc_index (hgc));

	case HGC_CRTC_DATA:
		return (hgc_get_crtc_data (hgc));

	case HGC_MODE:
		return (hgc_get_mode (hgc));

	case HGC_STATUS:
		return (hgc_get_status (hgc));

	case HGC_CONFIG:
		return (hgc_get_config (hgc));

	default:
		return (0xff);
	}
}

static
unsigned short hgc_reg_get_uint16 (hgc_t *hgc, unsigned long addr)
{
	unsigned short ret;

	ret = hgc_reg_get_uint8 (hgc, addr);
	ret |= hgc_reg_get_uint8 (hgc, addr + 1) << 8;

	return (ret);
}

/*
 * Set an HGC register
 */
static
void hgc_reg_set_uint8 (hgc_t *hgc, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case HGC_CRTC_INDEX:
		hgc_set_crtc_index (hgc, val);
		break;

	case HGC_CRTC_DATA:
		hgc_set_crtc_data (hgc, val);
		break;

	case HGC_MODE:
		hgc_set_mode (hgc, val);
		break;

	case HGC_CONFIG:
		hgc_set_config (hgc, val);
		break;
	}
}

static
void hgc_reg_set_uint16 (hgc_t *hgc, unsigned long addr, unsigned short val)
{
	hgc_reg_set_uint8 (hgc, addr + 0, val & 0xff);
	hgc_reg_set_uint8 (hgc, addr + 1, (val >> 8) & 0xff);
}


static
void hgc_mem_set_uint8 (hgc_t *hgc, unsigned long addr, unsigned char val)
{
	if (hgc->mem[addr] == val) {
		return;
	}

	hgc->mem[addr] = val;

	hgc->update_state |= 1;
}

static
void hgc_mem_set_uint16 (hgc_t *hgc, unsigned long addr, unsigned short val)
{
	hgc_mem_set_uint8 (hgc, addr + 0, val & 0xff);

	if ((addr + 1) < hgc->memblk->size) {
		hgc_mem_set_uint8 (hgc, addr + 1, (val >> 8) & 0xff);
	}
}


static
void hgc_del (hgc_t *hgc)
{
	if (hgc != NULL) {
		mem_blk_del (hgc->memblk);
		mem_blk_del (hgc->regblk);
		free (hgc);
	}
}

static
int hgc_set_msg (hgc_t *hgc, const char *msg, const char *val)
{
	return (1);
}

static
void hgc_set_terminal (hgc_t *hgc, terminal_t *trm)
{
	hgc->term = trm;

	if (hgc->term != NULL) {
		trm_open (hgc->term, 720, 350);
	}
}

static
mem_blk_t *hgc_get_mem (hgc_t *hgc)
{
	return (hgc->memblk);
}

static
mem_blk_t *hgc_get_reg (hgc_t *hgc)
{
	return (hgc->regblk);
}

static
void hgc_print_info (hgc_t *hgc, FILE *fp)
{
	unsigned i;
	unsigned pos, ofs;

	fprintf (fp, "DEV: Hercules Graphics Card\n");

	pos = hgc_get_cursor (hgc);
	ofs = hgc_get_start (hgc);

	fprintf (fp, "HGC: PAGE=%d  OFS=%04X  POS=%04X\n",
		(hgc->reg[HGC_MODE] & HGC_MODE_PAGE1) != 0,
		ofs, pos
	);

	fprintf (fp, "REG: CRTC=%02X  MODE=%02X  STATUS=%02X  CONF=%02X\n",
		hgc->reg[HGC_CRTC_INDEX], hgc_get_mode (hgc),
		hgc_get_status (hgc), hgc_get_config (hgc)
	);

	fprintf (fp, "CRTC=[%02X", hgc->reg_crt[0]);

	for (i = 1; i < 18; i++) {
		if ((i & 7) == 0) {
			fputs ("-", fp);
		}
		else {
			fputs (" ", fp);
		}
		fprintf (fp, "%02X", hgc->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}

static
int hgc_screenshot (hgc_t *hgc, FILE *fp)
{
	hgc_update (hgc);

	fprintf (fp, "P6\n%u %u\n255 ", hgc->buf_w, hgc->buf_h);
	fwrite (hgc->buf, 1, 3 * hgc->buf_w * hgc->buf_h, fp);

	return (0);
}

/*
 * Force a screen update
 */
static
void hgc_redraw (hgc_t *hgc)
{
	hgc->update_state |= 1;
}

static
void hgc_clock (hgc_t *hgc, unsigned cnt)
{
	unsigned long clk;

	clk = hgc_get_dotclock (hgc);

	if (clk < hgc->clk_vt) {
		return;
	}

	hgc->video.dotclk[0] = 0;
	hgc->video.dotclk[1] = 0;
	hgc->video.dotclk[2] = 0;

	if ((hgc->update_state & 1) == 0) {
		if (hgc->term != NULL) {
			trm_update (hgc->term);
		}
		return;
	}

	hgc->update_state &= ~1;

	if (hgc->term != NULL) {
		hgc_update (hgc);

		trm_set_size (hgc->term, hgc->buf_w, hgc->buf_h);
		trm_set_lines (hgc->term, hgc->buf, 0, hgc->buf_h);
		trm_update (hgc->term);
	}
}

hgc_t *hgc_new (unsigned long io, unsigned long mem, unsigned long size)
{
	unsigned i;
	hgc_t    *hgc;

	hgc = malloc (sizeof (hgc_t));
	if (hgc == NULL) {
		return (NULL);
	}

	pce_video_init (&hgc->video);

	hgc->video.ext = hgc;
	hgc->video.del = (void *) hgc_del;
	hgc->video.set_msg = (void *) hgc_set_msg;
	hgc->video.set_terminal = (void *) hgc_set_terminal;
	hgc->video.get_mem = (void *) hgc_get_mem;
	hgc->video.get_reg = (void *) hgc_get_reg;
	hgc->video.print_info = (void *) hgc_print_info;
	hgc->video.screenshot = (void *) hgc_screenshot;
	hgc->video.redraw = (void *) hgc_redraw;
	hgc->video.clock = (void *) hgc_clock;

	size = (size <= 32768) ? 32768UL : 65536UL;

	hgc->memblk = mem_blk_new (mem, size, 1);
	hgc->memblk->ext = hgc;
	hgc->memblk->set_uint8 = (void *) hgc_mem_set_uint8;
	hgc->memblk->set_uint16 = (void *) hgc_mem_set_uint16;
	hgc->mem = hgc->memblk->data;
	mem_blk_clear (hgc->memblk, 0x00);

	hgc->regblk = mem_blk_new (io, 16, 1);
	hgc->regblk->ext = hgc;
	hgc->regblk->set_uint8 = (void *) hgc_reg_set_uint8;
	hgc->regblk->set_uint16 = (void *) hgc_reg_set_uint16;
	hgc->regblk->get_uint8 = (void *) hgc_reg_get_uint8;
	hgc->regblk->get_uint16 = (void *) hgc_reg_get_uint16;
	hgc->reg = hgc->regblk->data;
	mem_blk_clear (hgc->regblk, 0x00);

	hgc->term = NULL;

	for (i = 0; i < 18; i++) {
		hgc->reg_crt[i] = 0;
	}

	hgc->font = hgc_font_8x14;

	hgc->w = 0;
	hgc->h = 0;
	hgc->ch = 0;

	hgc->clk_ht = 0;
	hgc->clk_vt = 0;
	hgc->clk_hs = 0;
	hgc->clk_vs = 0;

	hgc->bufmax = 0;
	hgc->buf = NULL;

	hgc->update_state = 0;

	return (hgc);
}

video_t *hgc_new_ini (ini_sct_t *sct)
{
	unsigned long io, mem, size;
	hgc_t         *hgc;

	ini_get_uint32 (sct, "io", &io, 0x3b4);
	ini_get_uint32 (sct, "address", &mem, 0xb0000);
	ini_get_uint32 (sct, "size", &size, 65536);

	hgc = hgc_new (io, mem, size);
	if (hgc == NULL) {
		return (NULL);
	}

	return (&hgc->video);
}
