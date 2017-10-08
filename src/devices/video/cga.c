/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/cga.c                                      *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2017 Hampa Hug <hampa@hampa.ch>                     *
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
#include <math.h>

#include <chipset/e6845.h>
#include <devices/memory.h>
#include <devices/video/cga_font.h>
#include <devices/video/cga.h>
#include <drivers/video/terminal.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <libini/libini.h>


#define CGA_IFREQ  1193182
#define CGA_PFREQ  14318184
#define CGA_CFREQ1 (CGA_PFREQ / 8)
#define CGA_CFREQ2 (CGA_PFREQ / 16)

#define CGA_CRTC_INDEX0   0
#define CGA_CRTC_DATA0    1
#define CGA_CRTC_INDEX    4
#define CGA_CRTC_DATA     5
#define CGA_MODE          8
#define CGA_CSEL          9
#define CGA_STATUS        10
#define CGA_PEN_RESET     11
#define CGA_PEN_SET       12

#define CGA_MODE_CS       0x01
#define CGA_MODE_G320     0x02
#define CGA_MODE_CBURST   0x04
#define CGA_MODE_ENABLE   0x08
#define CGA_MODE_G640     0x10
#define CGA_MODE_BLINK    0x20

#define CGA_CSEL_COL      0x0f
#define CGA_CSEL_INT      0x10
#define CGA_CSEL_PAL      0x20

#define CGA_STATUS_SYNC   0x01
#define CGA_STATUS_PEN    0x02
#define CGA_STATUS_VSYNC  0x08

#define CGA_COMPOSITE_NONE  0
#define CGA_COMPOSITE_AUTO  1
#define CGA_COMPOSITE_FORCE 2


static void cga_del (cga_t *cga);


static
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

static char cga_color_burst[8][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 1, 1, 0 },
	{ 1, 1, 0, 0, 0, 0, 1, 1 },
	{ 1, 0, 0, 0, 0, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 1, 1, 1, 0, 0, 0, 0, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 }
};


static
void cga_set_composite (cga_t *cga, unsigned mode)
{
	cga->composite = mode;
	cga->mod_cnt = 2;
}

static
void cga_set_hue (cga_t *cga, int val)
{
	unsigned i;
	double   tmp;

	cga->hue = val;

	tmp = (-(112.5 - 33) + cga->hue) * (M_PI / 180.0);

	for (i = 0; i < 8; i++) {
		cga->sin_cos_tab[i + 0] = 2.0 * sin (tmp + i * (M_PI / 4.0));
		cga->sin_cos_tab[i + 8] = 2.0 * cos (tmp + i * (M_PI / 4.0));
	}

	cga->comp_tab_ok = 0;
	cga->mod_cnt = 2;
}

static
void cga_set_saturation (cga_t *cga, int val)
{
	cga->saturation = (double) val / 100.0;
	cga->comp_tab_ok = 0;
	cga->mod_cnt = 2;
}

static
void cga_set_brightness (cga_t *cga, int val)
{
	cga->brightness = (double) val / 100.0;
	cga->comp_tab_ok = 0;
	cga->mod_cnt = 2;
}

static
unsigned char *cga_get_rgbi_buf (cga_t *cga, unsigned w)
{
	unsigned char *tmp;

	if (w < cga->rgbi_max) {
		return (cga->rgbi_buf);
	}

	if ((tmp = realloc (cga->rgbi_buf, w)) == NULL) {
		return (NULL);
	}

	cga->rgbi_max = w;
	cga->rgbi_buf = tmp;

	return (tmp);
}

static
void cga_line_composite (cga_t *cga, unsigned char *dst, const unsigned char *src, unsigned w)
{
	unsigned i, j, x, rgbi;
	unsigned r, g, b;
	double   Ys[8], Is[8], Qs[8];
	double   R, G, B, Y, I, Q;
	double   luma, chroma, composite;

	rgbi = 0;
	luma = 0.0;

	R = 0.0;
	G = 0.0;
	B = 0.0;

	for (j = 0; j < 8; j++) {
		Ys[j] = 0.0;
		Is[j] = 0.0;
		Qs[j] = 0.0;
	}

	for (x = 0; x < (2 * w); x++) {
		if ((x & 1) == 0) {
			rgbi = *(src++);
			luma = (rgbi & 8) ? 0.33 : 0;
		}

		i = x & 7;

		chroma = cga_color_burst[rgbi & 7][i] ? 0.66 : 0.0;
		composite = luma + chroma;

		Ys[i] = composite;
		Is[i] = composite * cga->sin_cos_tab[i + 8];
		Qs[i] = composite * cga->sin_cos_tab[i];

		Y = Ys[i];
		I = Is[i];
		Q = Qs[i];

		for (j = 1; j < 8; j++) {
			Y += Ys[(i + j) & 7];
			I += Is[(i + j) & 7];
			Q += Qs[(i + j) & 7];
		}

		if (Y > 8*1.0000) Y = 8*1.0000; else if (Y < 0.0) Y = 0.0;
		if (I > 8*0.5957) I = 8*0.5957; else if (I < -8*0.5957) I = -8*0.5957;
		if (Q > 8*0.5226) Q = 8*0.5226; else if (Q < -8*0.5226) Q = -8*0.5226;

		Y *= cga->brightness;
		I *= cga->saturation;
		Q *= cga->saturation;

		R += Y + 0.9562948323208939905 * I + 0.6210251254447287141 * Q;
		G += Y - 0.2721214740839773195 * I - 0.6473809535176157223 * Q;
		B += Y - 1.1069899085671282160 * I + 1.7046149754988293290 * Q;

		if (x & 1) {
			if (R <= 0.0) r = 0; else if (R >= 16.0) r = 255; else r = 16.0 * R;
			if (G <= 0.0) g = 0; else if (G >= 16.0) g = 255; else g = 16.0 * G;
			if (B <= 0.0) b = 0; else if (B >= 16.0) b = 255; else b = 16.0 * B;

			dst[0] = r;
			dst[1] = g;
			dst[2] = b;

			dst += 3;

			R = 0.0;
			G = 0.0;
			B = 0.0;
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

	hue = cga->hue;
	sat = cga->saturation;
	lum = cga->brightness;

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

static
void cga_line_blank (cga_t *cga, unsigned row)
{
	unsigned char *ptr;

	ptr = pce_video_get_row_ptr (&cga->video, row);

	memset (ptr, 0, 3 * cga->video.buf_w);
}

/*
 * Text modes
 */
static
void cga_line_mode0 (cga_t *cga, unsigned row)
{
	unsigned            i, j;
	unsigned            hd;
	unsigned            addr, caddr, amask;
	unsigned char       code, attr;
	unsigned char       val, mask, cmask;
	unsigned char       *ptr;
	const unsigned char *reg, *col, *fg, *bg;
	e6845_t             *crt;

	reg = cga->reg;
	crt = &cga->crtc;

	hd = e6845_get_hd (crt);

	cga->video.buf_next_w = 8 * hd;

	if ((8 * hd) > cga->video.buf_w) {
		hd = cga->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&cga->video, row);
	addr = crt->ma;
	amask = 0x3fff;

	if (cga->blink) {
		caddr = e6845_get_cursor_address (crt) & amask;
		cmask = e6845_get_cursor_mask (crt, 0);
	}
	else {
		caddr = -1;
		cmask = 0;
	}

	for (i = 0; i < hd; i++) {
		code = cga->mem[(2 * addr + 0) & amask];
		attr = cga->mem[(2 * addr + 1) & amask];

		if ((attr & 0x80) && (reg[CGA_MODE] & CGA_MODE_BLINK)) {
			if (cga->blink == 0) {
				attr = (attr & 0x70) | ((attr >> 4) & 0x07);
			}

			attr &= 0x7f;
		}

		fg = cga_rgb[attr & 15];
		bg = cga_rgb[(attr >> 4) & 15];

		val = cga->font[8 * code + crt->ra];

		if ((addr & amask) == caddr) {
			val |= cmask;
		}

		mask = 0x80;

		for (j = 0; j < 8; j++) {
			col = (val & mask) ? fg : bg;

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			mask >>= 1;
		}

		addr += 1;
	}
}

/*
 * CGA 320 * 200 * 4
 */
static
void cga_line_mode1 (cga_t *cga, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val;
	unsigned char       *ptr;
	const unsigned char *col;

	hd = e6845_get_hd (&cga->crtc);

	cga->video.buf_next_w = 8 * hd;

	if ((8 * hd) > cga->video.buf_w) {
		hd = cga->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&cga->video, row);
	addr = (cga->crtc.ma ^ ((cga->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val = cga->mem[addr & 0x3fff];
		val = (val << 8) | cga->mem[(addr + 1) & 0x3fff];

		for (j = 0; j < 8; j++) {
			col = cga_rgb[cga->pal[(val >> 14) & 3]];

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val <<= 2;
		}

		addr += 2;
	}
}

/*
 * CGA 320 * 200 * 4 composite
 */
static
void cga_line_mode1c (cga_t *cga, unsigned row)
{
	unsigned      i, j;
	unsigned      hd, addr, val;
	unsigned char col;
	unsigned char *rgb, *dst, *ptr;

	hd = e6845_get_hd (&cga->crtc);

	cga->video.buf_next_w = 16 * hd;

	if ((16 * hd) > cga->video.buf_w) {
		hd = cga->video.buf_w / 16;
	}

	rgb = cga_get_rgbi_buf (cga, 16 * hd);
	dst = pce_video_get_row_ptr (&cga->video, row);

	ptr = rgb;
	addr = (cga->crtc.ma ^ ((cga->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val = cga->mem[addr & 0x3fff];
		val = (val << 8) | cga->mem[(addr + 1) & 0x3fff];

		for (j = 0; j < 8; j++) {
			col = cga->pal[(val >> 14) & 3];

			*(ptr++) = col;
			*(ptr++) = col;

			val <<= 2;
		}

		addr += 2;
	}

	cga_line_composite (cga, dst, rgb, 16 * hd);
}

/*
 * CGA 640 * 200 * 2
 */
static
void cga_line_mode2 (cga_t *cga, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val;
	unsigned char       *ptr;
	const unsigned char *fg, *bg, *col;

	hd = e6845_get_hd (&cga->crtc);

	cga->video.buf_next_w = 16 * hd;

	if ((16 * hd) > cga->video.buf_w) {
		hd = cga->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&cga->video, row);
	addr = (cga->crtc.ma ^ ((cga->crtc.ra & 1) << 12)) << 1;

	fg = cga_rgb[cga->reg[CGA_CSEL] & 15];
	bg = cga_rgb[0];

	for (i = 0; i < hd; i++) {
		val = cga->mem[addr & 0x3fff];
		val = (val << 8) | cga->mem[(addr + 1) & 0x3fff];

		for (j = 0; j < 16; j++) {
			col = (val & 0x8000) ? fg : bg;

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val <<= 1;
		}

		addr += 2;
	}
}

/*
 * CGA 640 * 200 * 2 composite
 */
static
void cga_line_mode2c (cga_t *cga, unsigned row)
{
	unsigned      i, j;
	unsigned      hd, addr, val;
	unsigned char *rgb, *dst, *ptr;
	unsigned char fg, bg;

	hd = e6845_get_hd (&cga->crtc);

	cga->video.buf_next_w = 16 * hd;

	if ((16 * hd) > cga->video.buf_w) {
		hd = cga->video.buf_w / 16;
	}

	rgb = cga_get_rgbi_buf (cga, 16 * hd);
	dst = pce_video_get_row_ptr (&cga->video, row);

	ptr = rgb;
	addr = (cga->crtc.ma ^ ((cga->crtc.ra & 1) << 12)) << 1;

	fg = cga->reg[CGA_CSEL] & 15;
	bg = 0;

	for (i = 0; i < hd; i++) {
		val = cga->mem[addr & 0x3fff];
		val = (val << 8) | cga->mem[(addr + 1) & 0x3fff];

		for (j = 0; j < 16; j++) {
			*(ptr++) = (val & 0x8000) ? fg : bg;
			val <<= 1;
		}

		addr += 2;
	}

	cga_line_composite (cga, dst, rgb, 16 * hd);
}

/*
 * CGA 640 * 200 * 2 composite (auto)
 */
static
void cga_line_mode2c_auto (cga_t *cga, unsigned row)
{
	unsigned            i;
	unsigned            hd, addr, val;
	unsigned            avgmsk, avgval, pat;
	unsigned char       *ptr;
	const unsigned char *col;

	if (cga->comp_tab_ok == 0) {
		cga_make_comp_tab (cga);
	}

	hd = e6845_get_hd (&cga->crtc);

	cga->video.buf_next_w = 16 * hd;

	if ((16 * hd) > cga->video.buf_w) {
		hd = cga->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&cga->video, row);
	addr = (cga->crtc.ma ^ ((cga->crtc.ra & 1) << 12)) << 1;

	pat = 0;
	val = 0;
	avgmsk = (cga->mem[addr & 0x3fff] >> 7) & 1;
	avgval = avgmsk;

	for (i = 0; i < (16 * hd); i++) {
		if ((i & 7) == 0) {
			val = cga->mem[addr & 0x3fff];
			pat = (val << 4) | ((val >> 4) & 0x0f);

			addr += 1;

			val = val << 8;

			if ((i + 8) < (16 * hd)) {
				val |= cga->mem[addr & 0x3fff];
			}
		}
		else if ((i & 3) == 0) {
			pat = pat >> 4;
		}

		avgval = avgval - ((avgmsk >> 3) & 1);
		avgmsk = avgmsk << 1;

		if (val & 0x4000) {
			avgmsk |= 1;
			avgval += 1;
		}

		col = cga->comp_tab + 3 * (16 * avgval + (pat & 0x0f));

		*(ptr++) = col[0];
		*(ptr++) = col[1];
		*(ptr++) = col[2];

		val <<= 1;
	}
}

static
void cga_hsync (cga_t *cga)
{
	unsigned      row, max, ch;
	unsigned char mode;

	if (cga->mod_cnt == 0) {
		return;
	}

	mode = cga->reg[CGA_MODE];

	ch = (e6845_get_ml (&cga->crtc) & 0x1f) + 1;
	row = ch * cga->crtc.crow + cga->crtc.ra;
	max = ch * e6845_get_vd (&cga->crtc);

	if (row >= max) {
		return;
	}

	if (row >= cga->video.buf_h) {
		return;
	}

	if ((mode & CGA_MODE_ENABLE) == 0) {
		cga_line_blank (cga, row);
	}
	else if (mode & CGA_MODE_G320) {
		if (mode & CGA_MODE_G640) {
			if (cga->composite == CGA_COMPOSITE_FORCE) {
				cga_line_mode2c (cga, row);
			}
			else if ((cga->composite == CGA_COMPOSITE_AUTO) && ((mode & CGA_MODE_CBURST) == 0)) {
				cga_line_mode2c_auto (cga, row);
			}
			else {
				cga_line_mode2 (cga, row);
			}
		}
		else {
			if (cga->composite == CGA_COMPOSITE_FORCE) {
				cga_line_mode1c (cga, row);
			}
			else {
				cga_line_mode1 (cga, row);
			}
		}
	}
	else {
		cga_line_mode0 (cga, row);
	}
}

static
void cga_vsync (cga_t *cga)
{
	video_t *vid;

	vid = &cga->video;

	if ((cga->term != NULL) && (vid->buf_w > 0) && (vid->buf_h > 0)) {
		trm_set_size (cga->term, vid->buf_w, vid->buf_h);

		if (cga->mod_cnt > 0) {
			trm_set_lines (cga->term, vid->buf, 0, vid->buf_h);
		}

		trm_update (cga->term);
	}

	if (cga->mod_cnt > 0) {
		cga->mod_cnt -= 1;
	}

	vid->buf_next_h = e6845_get_vdl (&cga->crtc);

	if (vid->buf_next_w == 0) {
		vid->buf_next_w = 640;
	}

	if (vid->buf_next_h == 0) {
		vid->buf_next_h = 200;
	}

	if ((vid->buf_w != vid->buf_next_w) || (vid->buf_h != vid->buf_next_h)) {
		pce_video_set_buf_size (vid, vid->buf_next_w, vid->buf_next_h, 3);
		cga->mod_cnt = 1;
	}

	if (cga->blink_cnt > 0) {
		cga->blink_cnt -= 1;

		if (cga->blink_cnt == 0) {
			cga->blink = !cga->blink;
			cga->blink_cnt = cga->blink_rate;
			cga->mod_cnt = 1;
		}
	}
}

static
void cga_clock (cga_t *cga, unsigned long cnt)
{
	unsigned cclk;

	if (cga->reg[CGA_MODE] & CGA_MODE_CS) {
		cga->clock += (unsigned long) CGA_CFREQ1 * cnt;
	}
	else {
		cga->clock += (unsigned long) CGA_CFREQ2 * cnt;
	}

	cclk = cga->clock / CGA_IFREQ;
	cga->clock = cga->clock % CGA_IFREQ;

	if (cclk > 0) {
		e6845_clock (&cga->crtc, cclk);
	}
}

static
void cga_set_blink_rate (cga_t *cga, unsigned rate)
{
	cga->blink = 1;
	cga->blink_cnt = rate;
	cga->blink_rate = rate;
	cga->mod_cnt = 2;
}

static
void cga_set_font (cga_t *cga, unsigned font)
{
	if (font == 0) {
		cga->font = cga_font_thick;
	}
	else {
		cga->font = cga_font_thin;
	}

	cga->mod_cnt = 2;
}

/*
 * Set the internal palette from the mode and color select registers
 */
static
void cga_set_palette (cga_t *cga)
{
	unsigned char mode, csel;
	unsigned char *pal;

	pal = cga->pal;

	mode = cga->reg[CGA_MODE];
	csel = cga->reg[CGA_CSEL];

	pal[0] = csel & 0x0f;

	if (mode & CGA_MODE_CBURST) {
		pal[1] = 3;
		pal[2] = 4;
		pal[3] = 7;
	}
	else if (csel & CGA_CSEL_PAL) {
		pal[1] = 3;
		pal[2] = 5;
		pal[3] = 7;
	}
	else {
		pal[1] = 2;
		pal[2] = 4;
		pal[3] = 6;
	}

	if (csel & CGA_CSEL_INT) {
		pal[1] += 8;
		pal[2] += 8;
		pal[3] += 8;
	}
}

/*
 * Set the mode register
 */
static
void cga_set_mode (cga_t *cga, unsigned char val)
{
	if (cga->reg[CGA_MODE] != val) {
		cga->reg[CGA_MODE] = val;
		cga_set_palette (cga);
		cga->mod_cnt = 2;
	}
}

/*
 * Set the color select register
 */
static
void cga_set_color_select (cga_t *cga, unsigned char val)
{
	if (cga->reg[CGA_CSEL] != val) {
		cga->reg[CGA_CSEL] = val;
		cga->comp_tab_ok = 0;
		cga_set_palette (cga);
		cga->mod_cnt = 2;
	}
}

/*
 * Get the status register
 */
static
unsigned char cga_get_status (cga_t *cga)
{
	unsigned char val;

	pce_video_clock1 (&cga->video, 0);

	val = cga->reg[CGA_STATUS];
	val |= (CGA_STATUS_VSYNC | CGA_STATUS_SYNC);

	if (e6845_get_de (&cga->crtc)) {
		val &= ~CGA_STATUS_SYNC;
	}

	if (cga->crtc.vsync_cnt == 0) {
		val &= ~CGA_STATUS_VSYNC;
	}

	cga->reg[CGA_STATUS] = val;

	return (val);
}

static
void cga_set_pen_reset (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_STATUS] &= ~CGA_STATUS_PEN;
}

static
void cga_set_pen_set (cga_t *cga, unsigned char val)
{
	pce_video_clock1 (&cga->video, 0);
	cga->reg[CGA_STATUS] |= CGA_STATUS_PEN;
	e6845_set_pen (&cga->crtc);
}

/*
 * Get a CGA register
 */
static
unsigned char cga_reg_get_uint8 (cga_t *cga, unsigned long addr)
{
	switch (addr) {
	case CGA_CRTC_INDEX:
	case CGA_CRTC_INDEX0:
		return (e6845_get_index (&cga->crtc));

	case CGA_CRTC_DATA:
	case CGA_CRTC_DATA0:
		return (e6845_get_data (&cga->crtc));

	case CGA_STATUS:
		return (cga_get_status (cga));

	default:
		break;
	}

	return (0xff);
}

static
unsigned short cga_reg_get_uint16 (cga_t *cga, unsigned long addr)
{
	return (0xffff);
}

/*
 * Set a CGA register
 */
static
void cga_reg_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case CGA_CRTC_INDEX:
	case CGA_CRTC_INDEX0:
		e6845_set_index (&cga->crtc, val);
		break;

	case CGA_CRTC_DATA:
	case CGA_CRTC_DATA0:
		e6845_set_data (&cga->crtc, val);
		cga->mod_cnt = 2;
		break;

	case CGA_MODE:
		cga_set_mode (cga, val);
		break;

	case CGA_CSEL:
		cga_set_color_select (cga, val);
		break;

	case CGA_PEN_RESET:
		cga_set_pen_reset (cga, val);
		break;

	case CGA_PEN_SET:
		cga_set_pen_set (cga, val);
		break;

	default:
		fprintf (stderr, "CGA: set reg (%03lX, %02X)\n\n", addr, val);
		break;
	}
}

static
void cga_reg_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
	cga_reg_set_uint8 (cga, addr, val & 0xff);

	if (addr < 15) {
		cga_reg_set_uint8 (cga, addr + 1, (val >> 8) & 0xff);
	}
}

static
unsigned char cga_mem_get_uint8 (cga_t *cga, unsigned long addr)
{
	return (cga->mem[addr & 0x3fff]);
}

static
unsigned short cga_mem_get_uint16 (cga_t *cga, unsigned long addr)
{
	unsigned short val;

	val = cga->mem[(addr + 1) & 0x3fff];
	val = (val << 8) | cga->mem[addr & 0x3fff];

	return (val);
}

static
void cga_mem_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
	cga->mem[addr & 0x3fff] = val;
	cga->mod_cnt = 2;
}

static
void cga_mem_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
	cga->mem[(addr + 0) & 0x3fff] = val & 0xff;
	cga->mem[(addr + 1) & 0x3fff] = (val >> 8) & 0xff;
	cga->mod_cnt = 2;
}

static
int cga_set_msg (cga_t *cga, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned v;

		if (msg_get_uint (val, &v)) {
			return (1);
		}

		cga_set_blink_rate (cga, v);

		return (0);
	}
	else if (msg_is_message ("emu.video.brightness", msg)) {
		int v;

		if (msg_get_sint (val, &v)) {
			return (1);
		}

		cga_set_brightness (cga, v);

		return (0);
	}
	else if (msg_is_message ("emu.video.composite", msg)) {
		int      v;
		unsigned mode;

		if (strcmp (val, "auto") == 0) {
			mode = CGA_COMPOSITE_AUTO;
		}
		else if (msg_get_bool (val, &v) == 0) {
			if (v) {
				mode = CGA_COMPOSITE_FORCE;
			}
			else {
				mode = CGA_COMPOSITE_NONE;
			}
		}
		else {
			return (1);
		}

		cga_set_composite (cga, mode);

		return (0);
	}
	else if (msg_is_message ("emu.video.composite.cycle", msg)) {
		unsigned mode;

		if (cga->composite == CGA_COMPOSITE_NONE) {
			mode = CGA_COMPOSITE_AUTO;
			fprintf (stderr, "cga: composite auto\n");
		}
		else if (cga->composite == CGA_COMPOSITE_AUTO) {
			mode = CGA_COMPOSITE_FORCE;
			fprintf (stderr, "cga: composite on\n");
		}
		else {
			mode = CGA_COMPOSITE_NONE;
			fprintf (stderr, "cga: composite off\n");
		}

		cga_set_composite (cga, mode);

		return (0);
	}
	else if (msg_is_message ("emu.video.font", msg)) {
		unsigned font;

		if (msg_get_uint (val, &font)) {
			return (1);
		}

		cga_set_font (cga, font);

		return (0);
	}
	else if (msg_is_message ("emu.video.hue", msg)) {
		int hue;

		if (msg_get_sint (val, &hue)) {
			return (1);
		}

		cga_set_hue (cga, hue);

		return (0);
	}
	else if (msg_is_message ("emu.video.saturation", msg)) {
		int sat;

		if (msg_get_sint (val, &sat)) {
			return (1);
		}

		cga_set_saturation (cga, sat);

		return (0);
	}

	return (-1);
}

static
void cga_print_info (cga_t *cga, FILE *fp)
{
	unsigned      col, row, w1, w2;
	unsigned      ch, vt, ht, vtl, vdl;
	unsigned      base, addr;
	unsigned long clk1, clk2;
	unsigned char status;
	unsigned char *reg;
	e6845_t       *crt;

	crt = &cga->crtc;
	reg = cga->reg;

	status = cga_get_status (cga);

	base = e6845_get_start_address (crt);
	addr = crt->ma + crt->ccol;

	ch = (e6845_get_ml (crt) & 0x1f) + 1;
	row = ch * crt->crow + crt->ra;
	col = 8 * crt->ccol;
	vtl = e6845_get_vtl (crt);
	vdl = e6845_get_vdl (crt);
	ht = e6845_get_ht (crt) + 1;
	vt = e6845_get_vt (crt) + 1;

	clk1 = 8 * (e6845_get_ht (crt) + 1);
	w1 = 8 * e6845_get_hd (crt);
	w2 = clk1;

	if ((reg[CGA_MODE] ^ CGA_MODE_CS) & (CGA_MODE_CS | CGA_MODE_G640)) {
		clk1 *= 2;
	}

	if (reg[CGA_MODE] & CGA_MODE_G640) {
		col *= 2;
		w1 *= 2;
		w2 *= 2;
	}

	clk2 = clk1 * vtl;

	fprintf (fp,
		"DEV: CGA\n"
		"INDX[3D4]=%02X   COL=%3u/%3u  HFRQ=%9.3f  %u*%u\n"
		"MODE[3D8]=%02X   ROW=%3u/%3u  VFRQ=%9.3f  %u*%u\n"
		"CSEL[3D9]=%02X  CCOL=%3u/%3u   HDE=%d  HSYN=%X  BASE=%04X\n"
		"STAT[3DA]=%02X  CROW=%3u/%3u   VDE=%d  VSYN=%X  ADDR=%04X\n"
		"                RA=%3u/%3u\n",
		e6845_get_index (crt), col, w2, (double) CGA_PFREQ / clk1, w1, vdl,
		reg[CGA_MODE], row, vtl, (double) CGA_PFREQ / clk2, w2, vtl,
		reg[CGA_CSEL], crt->ccol, ht, e6845_get_hde (crt), crt->hsync_cnt, base,
		status, crt->crow, vt, e6845_get_vde (crt), crt->vsync_cnt, addr,
		crt->ra, ch
	);

	fprintf (fp,
		"HT[00]=%02X  VT[04]=%02X  IL[08]=%02X  AH[12]=%02X  LH[16]=%02X\n"
		"HD[01]=%02X  VA[05]=%02X  ML[09]=%02X  AL[13]=%02X  LL[17]=%02X\n"
		"HS[02]=%02X  VD[06]=%02X  CS[10]=%02X  CH[14]=%02X  ROWDSP=%u\n"
		"SW[03]=%02X  VS[07]=%02X  CE[11]=%02X  CL[15]=%02X  ROWTOT=%u\n",
		crt->reg[0], crt->reg[4], crt->reg[8], crt->reg[12], crt->reg[16],
		crt->reg[1], crt->reg[5], crt->reg[9], crt->reg[13], crt->reg[17],
		crt->reg[2], crt->reg[6], crt->reg[10], crt->reg[14], vdl,
		crt->reg[3], crt->reg[7], crt->reg[11], crt->reg[15], vtl
	);

	fflush (fp);
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
void cga_init (cga_t *cga, unsigned long io, unsigned long addr)
{
	pce_video_init (&cga->video);

	cga->video.ext = cga;
	cga->video.del = (void *) cga_del;
	cga->video.set_msg = (void *) cga_set_msg;
	cga->video.set_terminal = (void *) cga_set_terminal;
	cga->video.get_mem = (void *) cga_get_mem;
	cga->video.get_reg = (void *) cga_get_reg;
	cga->video.print_info = (void *) cga_print_info;
	cga->video.clock = (void *) cga_clock;

	cga->memblk = mem_blk_new (addr, 16384, 1);
	mem_blk_set_fget (cga->memblk, cga, cga_mem_get_uint8, cga_mem_get_uint16, NULL);
	mem_blk_set_fset (cga->memblk, cga, cga_mem_set_uint8, cga_mem_set_uint16, NULL);
	mem_blk_clear (cga->memblk, 0x00);
	cga->mem = mem_blk_get_data (cga->memblk);
	mem_blk_set_size (cga->memblk, 32768);

	cga->regblk = mem_blk_new (io, 16, 1);
	mem_blk_set_fget (cga->regblk, cga, cga_reg_get_uint8, cga_reg_get_uint16, NULL);
	mem_blk_set_fset (cga->regblk, cga, cga_reg_set_uint8, cga_reg_set_uint16, NULL);
	mem_blk_clear (cga->regblk, 0x00);
	cga->reg = mem_blk_get_data (cga->regblk);

	e6845_init (&cga->crtc);
	e6845_set_hsync_fct (&cga->crtc, cga, cga_hsync);
	e6845_set_vsync_fct (&cga->crtc, cga, cga_vsync);

	cga->term = NULL;
	cga->font = cga_font_thick;
	cga->clock = 0;
	cga->mod_cnt = 0;

	cga->rgbi_max = 0;
	cga->rgbi_buf = NULL;

	cga->pal[0] = 0;
	cga->pal[1] = 11;
	cga->pal[2] = 13;
	cga->pal[3] = 15;

	cga->blink = 0;
	cga->blink_cnt = 0;
	cga->blink_rate = 16;

	cga->composite = CGA_COMPOSITE_AUTO;
	cga->comp_tab_ok = 0;
	cga->comp_tab = NULL;
	cga->hue = 0.0;
	cga->saturation = 2.0 / 3.0;
	cga->brightness = 1.0;

	cga_set_hue (cga, 0.0);
}

static
void cga_free (cga_t *cga)
{
	e6845_free (&cga->crtc);

	free (cga->rgbi_buf);

	mem_blk_del (cga->memblk);
	mem_blk_del (cga->regblk);
}

cga_t *cga_new (unsigned long io, unsigned long addr)
{
	cga_t *cga;

	if ((cga = malloc (sizeof (cga_t))) == NULL) {
		return (NULL);
	}

	cga_init (cga, io, addr);

	return (cga);
}

static
void cga_del (cga_t *cga)
{
	if (cga != NULL) {
		cga_free (cga);
		free (cga);
	}
}

video_t *cga_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      blink, font;
	cga_t         *cga;

	ini_get_uint32 (sct, "io", &io, 0x3d0);
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	ini_get_uint16 (sct, "blink", &blink, 16);
	ini_get_uint16 (sct, "font", &font, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "CGA io=0x%04lx addr=0x%05lx font=%u blink=%u\n",
		io, addr, font, blink
	);

	if ((cga = cga_new (io, addr)) == NULL) {
		return (NULL);
	}

	cga_set_blink_rate (cga, blink);
	cga_set_font (cga, font);

	return (&cga->video);
}
