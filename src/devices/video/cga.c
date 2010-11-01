/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/cga.c                                      *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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
#include <math.h>

#include <lib/log.h>
#include <lib/msg.h>

#include <devices/video/cga.h>


#define CGA_IFREQ 1193182
#define CGA_PFREQ 14318180
#define CGA_HFREQ 15750
#define CGA_VFREQ 60


#define CGA_CRTC_INDEX   0
#define CGA_CRTC_DATA    1
#define CGA_MODE         4
#define CGA_CSEL         5
#define CGA_STATUS       6
#define CGA_PEN_RESET    7
#define CGA_PEN_SET      8

#define CGA_CRTC_HT      0
#define CGA_CRTC_HD      1
#define CGA_CRTC_HS      2
#define CGA_CRTC_SYN     3
#define CGA_CRTC_VT      4
#define CGA_CRTC_VTA     5
#define CGA_CRTC_VD      6
#define CGA_CRTC_VS      7
#define CGA_CRTC_IL      8
#define CGA_CRTC_MS      9
#define CGA_CRTC_CS      10
#define CGA_CRTC_CE      11
#define CGA_CRTC_SH      12
#define CGA_CRTC_SL      13
#define CGA_CRTC_PH      14
#define CGA_CRTC_PL      15
#define CGA_CRTC_LH      16
#define CGA_CRTC_LL      17

#define CGA_MODE_CS      0x01		/* clock select */
#define CGA_MODE_G320    0x02
#define CGA_MODE_CBURST  0x04
#define CGA_MODE_ENABLE  0x08
#define CGA_MODE_G640    0x10
#define CGA_MODE_BLINK   0x20

#define CGA_CSEL_COL     0x0f
#define CGA_CSEL_INT     0x10
#define CGA_CSEL_PAL     0x20

#define CGA_STATUS_SYNC  0x01		/* -display enable */
#define CGA_STATUS_PEN   0x02
#define CGA_STATUS_VSYNC 0x08		/* vertical sync */

#define CGA_UPDATE_DIRTY   1
#define CGA_UPDATE_RETRACE 2


#include "cga_font.h"


static void cga_clock (cga_t *cga, unsigned long cnt);


unsigned char cga_rgb[16][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xaa },
	{ 0x00, 0xaa, 0x00 },
	{ 0x00, 0xaa, 0xaa },
	{ 0xaa, 0x00, 0x00 },
	{ 0xaa, 0x00, 0xaa },
	{ 0xaa, 0x55, 0x00 },
	{ 0xaa, 0xaa, 0xaa },
	{ 0x55, 0x55, 0x55 },
	{ 0x55, 0x55, 0xff },
	{ 0x55, 0xff, 0x55 },
	{ 0x55, 0xff, 0xff },
	{ 0xff, 0x55, 0x55 },
	{ 0xff, 0x55, 0xff },
	{ 0xff, 0xff, 0x55 },
	{ 0xff, 0xff, 0xff }
};


/*
 * Set the blink frequency
 */
void cga_set_blink_rate (cga_t *cga, unsigned freq)
{
	cga->blink_on = 1;
	cga->blink_cnt = freq;
	cga->blink_freq = freq;

	cga->update_state |= CGA_UPDATE_DIRTY;
}

/*
 * Get CRTC start offset
 */
unsigned cga_get_start (cga_t *cga)
{
	unsigned val;

	val = cga->reg_crt[CGA_CRTC_SH];
	val = (val << 8) | cga->reg_crt[CGA_CRTC_SL];

	return (val);
}

/*
 * Get the absolute cursor position
 */
unsigned cga_get_cursor (cga_t *cga)
{
	unsigned val;

	val = cga->reg_crt[CGA_CRTC_PH];
	val = (val << 8) | cga->reg_crt[CGA_CRTC_PL];

	return (val);
}

/*
 * Get the on screen cursor position
 */
int cga_get_position (cga_t *cga, unsigned *x, unsigned *y)
{
	unsigned pos, ofs;

	if (cga->w == 0) {
		return (1);
	}

	pos = cga_get_cursor (cga) & 0x1fff;
	ofs = cga_get_start (cga) & 0x1fff;

	if ((pos < ofs) || (pos >= (ofs + cga->w * cga->h))) {
		return (1);
	}

	*x = (pos - ofs) % cga->w;
	*y = (pos - ofs) / cga->w;

	return (0);
}

/*
 * Set the internal palette from the mode and color select registers
 */
static
void cga_set_palette (cga_t *cga)
{
	unsigned char mode, csel;

	mode = cga->reg[CGA_MODE];
	csel = cga->reg[CGA_CSEL];

	cga->pal[0] = csel & 0x0f;

	if (mode & CGA_MODE_CBURST) {
		cga->pal[1] = 3;
		cga->pal[2] = 4;
		cga->pal[3] = 7;
	}
	else if (csel & CGA_CSEL_PAL) {
		cga->pal[1] = 3;
		cga->pal[2] = 5;
		cga->pal[3] = 7;
	}
	else {
		cga->pal[1] = 2;
		cga->pal[2] = 4;
		cga->pal[3] = 6;
	}

	if (csel & CGA_CSEL_INT) {
		cga->pal[1] += 8;
		cga->pal[2] += 8;
		cga->pal[3] += 8;
	}
}

/*
 * Set the timing values from the CRTC registers
 */
static
void cga_set_timing (cga_t *cga)
{
	cga->ch = (cga->reg_crt[CGA_CRTC_MS] & 0x1f) + 1;
	cga->w = cga->reg_crt[CGA_CRTC_HD];
	cga->h = cga->reg_crt[CGA_CRTC_VD];

	cga->clk_ht = 8 * (cga->reg_crt[CGA_CRTC_HT] + 1);
	cga->clk_hd = 8 * cga->reg_crt[CGA_CRTC_HD];

	cga->clk_vt = cga->ch * (cga->reg_crt[CGA_CRTC_VT] + 1) * cga->clk_ht;
	cga->clk_vd = cga->ch * cga->reg_crt[CGA_CRTC_VD] * cga->clk_ht;
}

/*
 * Get the dot clock
 */
static
unsigned long cga_get_dotclock (cga_t *cga)
{
	unsigned long long clk;

	clk = cga->video.dotclk[0];
	clk = (CGA_PFREQ * clk) / CGA_IFREQ;

	if ((cga->reg[CGA_MODE] & CGA_MODE_CS) == 0) {
		clk >>= 1;
	}

	return (clk);
}

/*
 * Set the internal screen buffer size
 */
int cga_set_buf_size (cga_t *cga, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > cga->bufmax) {
		tmp = realloc (cga->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		cga->buf = tmp;
		cga->bufmax = cnt;
	}

	cga->buf_w = w;
	cga->buf_h = h;

	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void cga_mode0_update_cursor (cga_t *cga)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
	const unsigned char *col;
	unsigned char       *dst;

	if (cga->blink_on == 0) {
		return;
	}

	if ((cga->reg_crt[CGA_CRTC_CS] & 0x60) == 0x20) {
		/* cursor off */
		return;
	}

	if (cga_get_position (cga, &x, &y)) {
		return;
	}

	c1 = cga->reg_crt[CGA_CRTC_CS] & 0x1f;
	c2 = cga->reg_crt[CGA_CRTC_CE] & 0x1f;

	if (c1 >= cga->ch) {
		return;
	}

	if (c2 >= cga->ch) {
		c2 = cga->ch - 1;
	}

	col = cga_rgb[cga->mem[2 * (cga->w * y + x) + 1] & 0x0f];
	dst = cga->buf + 3 * 8 * (cga->w * (cga->ch * y + c1) + x);

	for (j = c1; j <= c2; j++) {
		for (i = 0; i < 8; i++) {
			dst[3 * i + 0] = col[0];
			dst[3 * i + 1] = col[1];
			dst[3 * i + 2] = col[2];
		}
		dst += 3 * 8 * cga->w;
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void cga_mode0_update_char (cga_t *cga,
	unsigned char *dst, unsigned char c, unsigned char a)
{
	unsigned            i, j;
	int                 blk;
	unsigned char       val;
	const unsigned char *fnt;
	const unsigned char *fg, *bg;

	blk = 0;

	if (cga->reg[CGA_MODE] & CGA_MODE_BLINK) {
		if (a & 0x80) {
			blk = !cga->blink_on;

			if ((cga->reg[CGA_CSEL] & CGA_CSEL_INT) == 0) {
				a &= 0x7f;
			}
		}
	}

	fg = cga_rgb[a & 0x0f];
	bg = cga_rgb[(a >> 4) & 0x0f];

	fnt = cga->font + 8 * c;

	for (j = 0; j < cga->ch; j++) {
		if (blk) {
			val = 0x00;
		}
		else {
			val = fnt[j & 7];
		}

		for (i = 0; i < 8; i++) {
			if (val & 0x80) {
				dst[3 * i + 0] = fg[0];
				dst[3 * i + 1] = fg[1];
				dst[3 * i + 2] = fg[2];
			}
			else {
				dst[3 * i + 0] = bg[0];
				dst[3 * i + 1] = bg[1];
				dst[3 * i + 2] = bg[2];
			}

			val <<= 1;
		}

		dst += (3 * 8 * cga->w);
	}
}

/*
 * Update mode 0 (text 80 * 25 * 16)
 */
static
void cga_mode0_update (cga_t *cga)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;

	if (cga_set_buf_size (cga, 8 * cga->w, cga->ch * cga->h)) {
		return;
	}

	src = cga->mem;
	ofs = (2 * cga_get_start (cga)) & 0x3ffe;

	for (y = 0; y < cga->h; y++) {
		dst = cga->buf + 3 * (8 * cga->w) * (cga->ch * y);

		for (x = 0; x < cga->w; x++) {
			cga_mode0_update_char (cga,
				dst + 3 * 8 * x, src[ofs], src[ofs + 1]
			);

			ofs = (ofs + 2) & 0x3ffe;
		}
	}

	cga_mode0_update_cursor (cga);
}


/*
 * Update mode 1 (graphics 320 * 200 * 4)
 */
static
void cga_mode1_update (cga_t *cga)
{
	unsigned            i, x, y, w, h;
	unsigned            ofs, val, idx;
	unsigned char       *dst;
	const unsigned char *col, *mem;

	if (cga_set_buf_size (cga, 8 * cga->w, 2 * cga->h)) {
		return;
	}

	dst = cga->buf;

	w = 2 * cga->w;
	h = 2 * cga->h;

	ofs = (2 * cga_get_start (cga)) & 0x1fff;

	for (y = 0; y < h; y++) {
		mem = cga->mem + ((y & 1) << 13);

		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];

			for (i = 0; i < 4; i++) {
				idx = (val >> 6) & 3;
				idx = cga->pal[idx];
				col = cga_rgb[idx];
				val = (val << 2) & 0xff;

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				dst += 3;
			}
		}

		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Update mode 2 (graphics 640 * 200 * 2)
 */
static
void cga_mode2_update (cga_t *cga)
{
	unsigned            i, x, y, w, h;
	unsigned            ofs, val, idx;
	unsigned char       *dst;
	const unsigned char *col, *mem;

	if (cga_set_buf_size (cga, 16 * cga->w, 2 * cga->h)) {
		return;
	}

	dst = cga->buf;

	h = 2 * cga->h;
	w = 2 * cga->w;

	ofs = (2 * cga_get_start (cga)) & 0x1fff;

	for (y = 0; y < h; y++) {
		mem = cga->mem + ((y & 1) << 13);

		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];

			for (i = 0; i < 8; i++) {
				idx = (val & 0x80) ? cga->pal[0] : 0;
				col = cga_rgb[idx];
				val = (val << 1) & 0xff;

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				dst += 3;
			}
		}

		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Create the composite mode color lookup table
 *
 * This is adapted from DOSBox
 */
static
void cga_make_comp_tab (cga_t *cga)
{
	unsigned            i, j;
	double              Y, I, Q, t;
	double              r, g, b;
	double              hue, sat, lum;
	double              hv, sh, ch;
	const unsigned char *src;
	unsigned char       *dst;

	if (cga->comp_tab == NULL) {
		cga->comp_tab = malloc (5 * 16 * 3);
	}

	hue = 0.0;
	sat = 2.0 / 3.0;
	lum = 1.0;

	if (cga->reg[CGA_CSEL] & CGA_CSEL_PAL) {
		hv = 35.0;
	}
	else {
		hv = 50.0;
	}

	hv = (hv + hue) * 0.017453292519943295474;
	sh = sin (hv);
	ch = cos (hv);

	src = cga_rgb[cga->reg[CGA_CSEL] & 0x0f];

	for (i = 0; i < 16; i++) {
		I = ((i & 0x08) ? 1.0 : 0.0) - ((i & 0x02) ? 1.0 : 0.0);
		Q = ((i & 0x04) ? 1.0 : 0.0) - ((i & 0x01) ? 1.0 : 0.0);

		I = sat * (0.5 * I);
		Q = sat * (0.5 * Q);

		t = ch * I + sh * Q;
		Q = ch * Q - sh * I;
		I = t;

		for (j = 0; j < 5; j++) {
			Y = lum * (double) j / 4.0;

			r = Y + 0.956 * I + 0.621 * Q;
			g = Y - 0.272 * I - 0.647 * Q;
			b = Y - 1.105 * I + 1.702 * Q;

			r = (r < 0.0) ? 0.0 : ((r > 1.0) ? 1.0 : r);
			g = (g < 0.0) ? 0.0 : ((g > 1.0) ? 1.0 : g);
			b = (b < 0.0) ? 0.0 : ((b > 1.0) ? 1.0 : b);

			dst = cga->comp_tab + 3 * (16 * j + i);

			dst[0] = (unsigned char) (src[0] * r);
			dst[1] = (unsigned char) (src[1] * g);
			dst[2] = (unsigned char) (src[2] * b);
		}
	}

	cga->comp_tab_ok = 1;
}

/*
 * Update mode 2 (graphics 640 * 200 * 2) for composite
 */
static
void cga_mode2c_update (cga_t *cga)
{
	unsigned            x, y, w, h, n;
	unsigned            avgmsk, avgval, pat;
	const unsigned char *mem[2];
	const unsigned char *src, *col;
	unsigned char       *dst;

	if (cga->comp_tab_ok == 0) {
		cga_make_comp_tab (cga);
	}

	if (cga_set_buf_size (cga, 16 * cga->w, 2 * cga->h)) {
		return;
	}

	mem[0] = cga->mem;
	mem[1] = cga->mem + 8192;

	dst = cga->buf;

	h = 2 * cga->h;
	w = 2 * cga->w;

	pat = 0;

	for (y = 0; y < h; y++) {
		src = mem[y & 1];

		avgmsk = (src[0] >> 7) & 1;
		avgval = avgmsk;

		for (x = 0; x < (8 * w); x++) {
			if ((x & 3) == 0) {
				if ((x & 7) == 0) {
					pat = src[x >> 3];
					pat = (pat << 4) | ((pat >> 4) & 0x0f);
				}
				else {
					pat = pat >> 4;
				}
			}

			avgval = avgval - ((avgmsk >> 3) & 1);
			avgmsk = avgmsk << 1;

			n = x + 1;

			if (n < (8 * w)) {
				if (src[n >> 3] & (0x80 >> (n & 7))) {
					avgmsk |= 1;
					avgval += 1;
				}
			}

			col = cga->comp_tab + 3 * (16 * avgval + (pat & 0x0f));

			dst[0] = col[0];
			dst[1] = col[1];
			dst[2] = col[2];

			dst += 3;
		}

		mem[y & 1] += w;
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void cga_update_blank (cga_t *cga)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	cga_set_buf_size (cga, 640, 200);

	dst = cga->buf;

	for (y = 0; y < cga->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < cga->buf_w; x++) {
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
void cga_update (cga_t *cga)
{
	unsigned char mode;

	cga->buf_w = 0;
	cga->buf_h = 0;

	mode = cga->reg[CGA_MODE];

	if ((mode & CGA_MODE_ENABLE) == 0) {
		cga_update_blank (cga);
	}
	else if (mode & CGA_MODE_G320) {
		if (mode & CGA_MODE_G640) {
			if ((mode & CGA_MODE_CBURST) == 0) {
				cga_mode2c_update (cga);
			}
			else {
				cga_mode2_update (cga);
			}
		}
		else {
			cga_mode1_update (cga);
		}
	}
	else {
		cga_mode0_update (cga);
	}
}


/*
 * Get a CRTC register
 */
static
unsigned char cga_crtc_get_reg (cga_t *cga, unsigned reg)
{
	if ((reg < 12) || (reg > 17)) {
		return (0xff);
	}

	return (cga->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void cga_crtc_set_reg (cga_t *cga, unsigned reg, unsigned char val)
{
	if (reg > 15) {
		return;
	}

	if (cga->reg_crt[reg] == val) {
		return;
	}

	cga->reg_crt[reg] = val;

	cga_set_timing (cga);

	cga->update_state |= CGA_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char cga_get_crtc_index (cga_t *cga)
{
	return (cga->reg[CGA_CRTC_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char cga_get_crtc_data (cga_t *cga)
{
	return (cga_crtc_get_reg (cga, cga->reg[CGA_CRTC_INDEX]));
}

/*
 * Get the status register
 */
static
unsigned char cga_get_status (cga_t *cga)
{
	unsigned char val;
	unsigned long clk;

	cga_clock (cga, 0);

	clk = cga_get_dotclock (cga);

	val = cga->reg[CGA_STATUS];

	val |= (CGA_STATUS_VSYNC | CGA_STATUS_SYNC);

	if (clk < cga->clk_vd) {
		val &= ~CGA_STATUS_VSYNC;

		if (cga->clk_ht > 0) {
			if ((clk % cga->clk_ht) < cga->clk_hd) {
				val &= ~CGA_STATUS_SYNC;
			}
		}
	}

	return (val);
}

/*
 * Set the CRTC index register
 */
static
void cga_set_crtc_index (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_CRTC_INDEX] = val & 0x1f;
}

/*
 * Set the CRTC data register
 */
static
void cga_set_crtc_data (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_CRTC_DATA] = val;

	cga_crtc_set_reg (cga, cga->reg[CGA_CRTC_INDEX], val);
}

/*
 * Set the mode register
 */
static
void cga_set_mode (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_MODE] = val;

	cga_set_palette (cga);
	cga_set_timing (cga);

	cga->update_state |= CGA_UPDATE_DIRTY;
}

/*
 * Set the color select register
 */
static
void cga_set_color_select (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_CSEL] = val;

	cga_set_palette (cga);

	cga->update_state |= CGA_UPDATE_DIRTY;
}

/*
 * Get a CGA register
 */
unsigned char cga_reg_get_uint8 (cga_t *cga, unsigned long addr)
{
	switch (addr) {
	case CGA_CRTC_INDEX:
		return (cga_get_crtc_index (cga));

	case CGA_CRTC_DATA:
		return (cga_get_crtc_data (cga));

	case CGA_STATUS:
		return (cga_get_status (cga));
	}

	return (0xff);
}

unsigned short cga_reg_get_uint16 (cga_t *cga, unsigned long addr)
{
	unsigned short ret;

	ret = cga_reg_get_uint8 (cga, addr);

	if ((addr + 1) < cga->regblk->size) {
		ret |= cga_reg_get_uint8 (cga, addr + 1) << 8;
	}

	return (ret);
}

/*
 * Set a CGA register
 */
void cga_reg_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case CGA_CRTC_INDEX:
		cga_set_crtc_index (cga, val);
		break;

	case CGA_CRTC_DATA:
		cga_set_crtc_data (cga, val);
		break;

	case CGA_MODE:
		cga_set_mode (cga, val);
		break;

	case CGA_CSEL:
		cga_set_color_select (cga, val);
		break;

	case CGA_PEN_RESET:
		cga->reg[CGA_STATUS] &= ~CGA_STATUS_PEN;
		break;

	case CGA_PEN_SET:
		cga->reg[CGA_STATUS] |= CGA_STATUS_PEN;
		break;

	default:
		cga->reg[addr] = val;
		break;
	}
}

void cga_reg_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
	cga_reg_set_uint8 (cga, addr, val & 0xff);

	if ((addr + 1) < cga->regblk->size) {
		cga_reg_set_uint8 (cga, addr + 1, val >> 8);
	}
}


static
void cga_mem_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
	if (cga->mem[addr] == val) {
		return;
	}

	cga->mem[addr] = val;

	cga->update_state |= CGA_UPDATE_DIRTY;
}

static
void cga_mem_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
	cga_mem_set_uint8 (cga, addr, val & 0xff);

	if ((addr + 1) < cga->memblk->size) {
		cga_mem_set_uint8 (cga, addr + 1, (val >> 8) & 0xff);
	}
}


static
int cga_set_msg (cga_t *cga, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}

		cga_set_blink_rate (cga, freq);

		return (0);
	}

	return (-1);
}

static
void cga_set_terminal (cga_t *cga, terminal_t *trm)
{
	cga->term = trm;

	if (cga->term != NULL) {
		trm_open (cga->term, 640, 200);
	}
}

static
mem_blk_t *cga_get_mem (cga_t *cga)
{
	return (cga->memblk);
}

static
mem_blk_t *cga_get_reg (cga_t *cga)
{
	return (cga->regblk);
}

static
void cga_print_info (cga_t *cga, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: CGA\n");

	fprintf (fp, "CGA: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		cga_get_start (cga),
		cga_get_cursor (cga),
		cga->reg[CGA_CSEL] & 0x0f,
		(cga->reg[CGA_CSEL] >> 5) & 1
	);

	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		cga_get_dotclock (cga),
		cga->clk_ht, cga->clk_hd,
		cga->clk_vt, cga->clk_vd
	);

	fprintf (fp,
		"REG: MODE=%02X  CSEL=%02X  STATUS=%02X"
		"  PAL=[%02X %02X %02X %02X]\n",
		cga->reg[CGA_MODE],
		cga->reg[CGA_CSEL],
		cga->reg[CGA_STATUS],
		cga->pal[0], cga->pal[1], cga->pal[2], cga->pal[3]
	);

	fprintf (fp, "CRTC=[%02X", cga->reg_crt[0]);
	for (i = 1; i < 18; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", cga->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}

/*
 * Force a screen update
 */
static
void cga_redraw (cga_t *cga, int now)
{
	if (now) {
		if (cga->term != NULL) {
			cga->update (cga);

			trm_set_size (cga->term, cga->buf_w, cga->buf_h);
			trm_set_lines (cga->term, cga->buf, 0, cga->buf_h);
			trm_update (cga->term);
		}
	}

	cga->update_state |= CGA_UPDATE_DIRTY;
}

static
void cga_clock (cga_t *cga, unsigned long cnt)
{
	unsigned long clk;

	if (cga->clk_vt < 50000) {
		return;
	}

	clk = cga_get_dotclock (cga);

	if (clk < cga->clk_vd) {
		cga->update_state &= ~CGA_UPDATE_RETRACE;
		return;
	}

	if (clk >= cga->clk_vt) {
		cga->video.dotclk[0] = 0;
		cga->video.dotclk[1] = 0;
		cga->video.dotclk[2] = 0;
	}

	if (cga->update_state & CGA_UPDATE_RETRACE) {
		return;
	}

	if (cga->blink_cnt > 0) {
		cga->blink_cnt -= 1;

		if (cga->blink_cnt == 0) {
			cga->blink_cnt = cga->blink_freq;
			cga->blink_on = !cga->blink_on;

			if ((cga->reg[CGA_MODE] & CGA_MODE_G320) == 0) {
				cga->update_state |= CGA_UPDATE_DIRTY;
			}
		}
	}

	if (cga->term != NULL) {
		if (cga->update_state & CGA_UPDATE_DIRTY) {
			cga->update (cga);
			trm_set_size (cga->term, cga->buf_w, cga->buf_h);
			trm_set_lines (cga->term, cga->buf, 0, cga->buf_h);
		}

		trm_update (cga->term);
	}

	cga->update_state = CGA_UPDATE_RETRACE;
}

void cga_free (cga_t *cga)
{
	mem_blk_del (cga->memblk);
	mem_blk_del (cga->regblk);
	free (cga->comp_tab);
}

void cga_del (cga_t *cga)
{
	if (cga != NULL) {
		cga_free (cga);
		free (cga);
	}
}

void cga_init (cga_t *cga, unsigned long io, unsigned long addr, unsigned long size)
{
	unsigned i;

	pce_video_init (&cga->video);

	cga->video.ext = cga;
	cga->video.del = (void *) cga_del;
	cga->video.set_msg = (void *) cga_set_msg;
	cga->video.set_terminal = (void *) cga_set_terminal;
	cga->video.get_mem = (void *) cga_get_mem;
	cga->video.get_reg = (void *) cga_get_reg;
	cga->video.print_info = (void *) cga_print_info;
	cga->video.redraw = (void *) cga_redraw;
	cga->video.clock = (void *) cga_clock;

	cga->ext = NULL;
	cga->update = cga_update;

	size = (size < 16384) ? 16384UL : size;

	cga->memblk = mem_blk_new (addr, size, 1);
	cga->memblk->ext = cga;
	cga->memblk->set_uint8 = (void *) cga_mem_set_uint8;
	cga->memblk->set_uint16 = (void *) &cga_mem_set_uint16;
	cga->mem = cga->memblk->data;
	mem_blk_clear (cga->memblk, 0x00);

	cga->regblk = mem_blk_new (io, 16, 1);
	cga->regblk->ext = cga;
	cga->regblk->set_uint8 = (void *) cga_reg_set_uint8;
	cga->regblk->set_uint16 = (void *) cga_reg_set_uint16;
	cga->regblk->get_uint8 = (void *) cga_reg_get_uint8;
	cga->regblk->get_uint16 = (void *) cga_reg_get_uint16;
	cga->reg = cga->regblk->data;
	mem_blk_clear (cga->regblk, 0x00);

	cga->term = NULL;

	for (i = 0; i < 18; i++) {
		cga->reg_crt[i] = 0;
	}

	cga->pal[0] = 0;
	cga->pal[1] = 11;
	cga->pal[2] = 13;
	cga->pal[3] = 15;

	cga->font = cga_font_thick;

	cga->comp_tab_ok = 0;
	cga->comp_tab = NULL;

	cga->blink_on = 1;
	cga->blink_cnt = 0;
	cga->blink_freq = 16;

	cga->w = 0;
	cga->h = 0;
	cga->ch = 0;

	cga->clk_ht = 0;
	cga->clk_vt = 0;
	cga->clk_hd = 0;
	cga->clk_vd = 0;

	cga->bufmax = 0;
	cga->buf = NULL;

	cga->update_state = 0;
}

cga_t *cga_new (unsigned long io, unsigned long addr, unsigned long size)
{
	cga_t *cga;

	cga = malloc (sizeof (cga_t));
	if (cga == NULL) {
		return (NULL);
	}

	cga_init (cga, io, addr, size);

	return (cga);
}

video_t *cga_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned      font;
	unsigned      blink;
	cga_t         *cga;

	ini_get_uint32 (sct, "io", &io, 0x3d4);
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	ini_get_uint32 (sct, "size", &size, 16384);
	ini_get_uint16 (sct, "font", &font, 0);
	ini_get_uint16 (sct, "blink", &blink, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "CGA io=0x%04lx addr=0x%05lx size=0x%05lx font=%u\n",
		io, addr, size, font
	);

	cga = cga_new (io, addr, size);
	if (cga == NULL) {
		return (NULL);
	}

	if (font == 0) {
		cga->font = cga_font_thick;
	}
	else {
		cga->font = cga_font_thin;
	}

	cga_set_blink_rate (cga, blink);

	return (&cga->video);
}
