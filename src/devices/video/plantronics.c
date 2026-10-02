/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/plantronics.c                              *
 * Created:     2008-10-13 by John Elliott <jce@seasip.demon.co.uk>          *
 * Copyright:   (C) 2008-2020 Hampa Hug <hampa@hampa.ch>                     *
 *              (C) 2008-2016 John Elliott <jce@seasip.demon.co.uk>          *
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
#include <devices/video/cga_font.h>
#include <devices/video/plantronics.h>
#include <drivers/video/terminal.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <libini/libini.h>


#define PLA_IFREQ  1193182
#define PLA_PFREQ  14318184
#define PLA_CFREQ1 (PLA_PFREQ / 8)
#define PLA_CFREQ2 (PLA_PFREQ / 16)

#define PLA_CRTC_INDEX0   0
#define PLA_CRTC_DATA0    1
#define PLA_CRTC_INDEX    4
#define PLA_CRTC_DATA     5
#define PLA_MODE          8
#define PLA_CSEL          9
#define PLA_STATUS        10
#define PLA_PEN_RESET     11
#define PLA_PEN_SET       12
#define PLA_SPECIAL       13

#define PLA_MODE_CS       0x01
#define PLA_MODE_G320     0x02
#define PLA_MODE_CBURST   0x04
#define PLA_MODE_ENABLE   0x08
#define PLA_MODE_G640     0x10
#define PLA_MODE_BLINK    0x20

#define PLA_CSEL_COL      0x0f
#define PLA_CSEL_INT      0x10
#define PLA_CSEL_PAL      0x20

#define PLA_STATUS_SYNC   0x01
#define PLA_STATUS_PEN    0x02
#define PLA_STATUS_VSYNC  0x08

#define PLA_SPECIAL_EXT2  0x10
#define PLA_SPECIAL_EXT1  0x20
#define PLA_SPECIAL_PLANE 0x40


static void pla_del (plantronics_t *pla);


static
unsigned char pla_rgb[16][3] = {
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


static
void pla_line_blank (plantronics_t *pla, unsigned row)
{
	unsigned char *ptr;

	ptr = pce_video_get_row_ptr (&pla->video, row);

	memset (ptr, 0, 3 * pla->video.buf_w);
}

/*
 * CGA text modes
 */
static
void pla_line_mode0 (plantronics_t *pla, unsigned row)
{
	unsigned            i, j;
	unsigned            hd;
	unsigned            addr, caddr, amask;
	unsigned char       code, attr;
	unsigned char       val, mask, cmask;
	unsigned char       *ptr;
	const unsigned char *reg, *col, *fg, *bg;
	e6845_t             *crt;

	reg = pla->reg;
	crt = &pla->crtc;

	hd = e6845_get_hd (crt);

	pla->video.buf_next_w = 8 * hd;

	if ((8 * hd) > pla->video.buf_w) {
		hd = pla->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&pla->video, row);
	addr = crt->ma;
	amask = 0x7fff;

	if (pla->blink) {
		caddr = e6845_get_cursor_address (crt);
		cmask = e6845_get_cursor_mask (crt, 0);
	}
	else {
		caddr = -1;
		cmask = 0;
	}

	for (i = 0; i < hd; i++) {
		code = pla->mem[(2 * addr + 0) & amask];
		attr = pla->mem[(2 * addr + 1) & amask];

		if ((attr & 0x80) && (reg[PLA_MODE] & PLA_MODE_BLINK)) {
			if (pla->blink == 0) {
				attr = (attr & 0x70) | ((attr >> 4) & 0x07);
			}

			attr &= 0x7f;
		}

		fg = pla_rgb[attr & 15];
		bg = pla_rgb[(attr >> 4) & 15];

		val = pla->font[8 * code + crt->ra];

		if (addr == caddr) {
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
void pla_line_mode1 (plantronics_t *pla, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val;
	unsigned char       *ptr;
	const unsigned char *col;

	hd = e6845_get_hd (&pla->crtc);

	pla->video.buf_next_w = 8 * hd;

	if ((8 * hd) > pla->video.buf_w) {
		hd = pla->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&pla->video, row);
	addr = (pla->crtc.ma ^ ((pla->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val = pla->mem[addr & 0x7fff];
		val = (val << 8) | pla->mem[(addr + 1) & 0x7fff];

		for (j = 0; j < 8; j++) {
			col = pla_rgb[pla->pal[(val >> 14) & 3]];

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val <<= 2;
		}

		addr += 2;
	}
}

/*
 * CGA 640 * 200 * 2
 */
static
void pla_line_mode2 (plantronics_t *pla, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val;
	unsigned char       *ptr;
	const unsigned char *fg, *bg, *col;

	hd = e6845_get_hd (&pla->crtc);

	pla->video.buf_next_w = 16 * hd;

	if ((16 * hd) > pla->video.buf_w) {
		hd = pla->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&pla->video, row);
	addr = (pla->crtc.ma ^ ((pla->crtc.ra & 1) << 12)) << 1;

	fg = pla_rgb[pla->reg[PLA_CSEL] & 15];
	bg = pla_rgb[0];

	for (i = 0; i < hd; i++) {
		val = pla->mem[addr & 0x7fff];
		val = (val << 8) | pla->mem[(addr + 1) & 0x7fff];

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
 * Plantronics 320 * 200 * 16
 */
static
void pla_line_mode3 (plantronics_t *pla, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val0, val1, idx;
	unsigned char       *ptr;
	const unsigned char *col;

	hd = e6845_get_hd (&pla->crtc);

	pla->video.buf_next_w = 8 * hd;

	if ((8 * hd) > pla->video.buf_w) {
		hd = pla->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&pla->video, row);
	addr = (pla->crtc.ma ^ ((pla->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val0 = pla->mem[addr & 0x7fff];
		val0 = (val0 << 8) | pla->mem[(addr + 1) & 0x7fff];

		val1 = pla->mem[(addr + 16384) & 0x7fff];
		val1 = (val1 << 8) | pla->mem[(addr + 16384 + 1) & 0x7fff];

		for (j = 0; j < 8; j++) {
			idx = ((val1 >> 15) & 1) | ((val0 >> 13) & 2);
			idx |= ((val0 >> 13) & 4) | ((val1 >> 11) & 8);
			col = pla_rgb[idx];

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val0 <<= 2;
			val1 <<= 2;
		}

		addr += 2;
	}
}

/*
 * Plantronics 640 * 200 * 4
 */
static
void pla_line_mode4 (plantronics_t *pla, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val0, val1, idx;
	unsigned char       *ptr;
	const unsigned char *col;

	hd = e6845_get_hd (&pla->crtc);

	pla->video.buf_next_w = 16 * hd;

	if ((16 * hd) > pla->video.buf_w) {
		hd = pla->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&pla->video, row);
	addr = (pla->crtc.ma ^ ((pla->crtc.ra & 1) << 12)) << 1;

	for (i = 0; i < hd; i++) {
		val0 = pla->mem[addr & 0x7fff];
		val0 = (val0 << 8) | pla->mem[(addr + 1) & 0x7fff];

		val1 = pla->mem[(addr + 16384) & 0x7fff];
		val1 = (val1 << 8) | pla->mem[(addr + 16384 + 1) & 0x7fff];

		for (j = 0; j < 16; j++) {
			idx = ((val0 >> 15) & 1) | ((val1 >> 14) & 2);
			idx = pla->pal[idx];
			col = pla_rgb[idx];

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val0 <<= 1;
			val1 <<= 1;
		}

		addr += 2;
	}
}

static
void pla_hsync (plantronics_t *pla)
{
	unsigned      row, max, ch;
	unsigned char mode1, mode2;

	if (pla->mod_cnt == 0) {
		return;
	}

	mode1 = pla->reg[PLA_MODE];
	mode2 = pla->reg[PLA_SPECIAL];

	ch = (e6845_get_ml (&pla->crtc) & 0x1f) + 1;
	row = ch * pla->crtc.crow + pla->crtc.ra;
	max = ch * e6845_get_vd (&pla->crtc);

	if (row >= max) {
		return;
	}

	if (row >= pla->video.buf_h) {
		return;
	}

	if ((mode1 & PLA_MODE_ENABLE) == 0) {
		pla_line_blank (pla, row);
	}
	else if (mode1 & PLA_MODE_G320) {
		if (mode2 & (PLA_SPECIAL_EXT1 | PLA_SPECIAL_EXT2)) {
			if (mode2 & PLA_SPECIAL_EXT2) {
				pla_line_mode3 (pla, row);
			}
			else {
				pla_line_mode4 (pla, row);
			}
		}
		else if (mode1 & PLA_MODE_G640) {
			pla_line_mode2 (pla, row);
		}
		else {
			pla_line_mode1 (pla, row);
		}
	}
	else {
		pla_line_mode0 (pla, row);
	}
}

static
void pla_vsync (plantronics_t *pla)
{
	video_t *vid;

	vid = &pla->video;

	if ((pla->term != NULL) && (vid->buf_w > 0) && (vid->buf_h > 0)) {
		trm_set_size (pla->term, vid->buf_w, vid->buf_h);

		if (pla->mod_cnt > 0) {
			trm_set_lines (pla->term, vid->buf, 0, vid->buf_h);
		}

		trm_update (pla->term);
	}

	if (pla->mod_cnt > 0) {
		pla->mod_cnt -= 1;
	}

	vid->buf_next_h = e6845_get_vdl (&pla->crtc);

	if (vid->buf_next_w == 0) {
		vid->buf_next_w = 640;
	}

	if (vid->buf_next_h == 0) {
		vid->buf_next_h = 200;
	}

	if ((vid->buf_w != vid->buf_next_w) || (vid->buf_h != vid->buf_next_h)) {
		pce_video_set_buf_size (vid, vid->buf_next_w, vid->buf_next_h, 3);
		pla->mod_cnt = 1;
	}

	if (pla->blink_cnt > 0) {
		pla->blink_cnt -= 1;

		if (pla->blink_cnt == 0) {
			pla->blink = !pla->blink;
			pla->blink_cnt = pla->blink_rate;
			pla->mod_cnt = 1;
		}
	}
}

static
void pla_clock (plantronics_t *pla, unsigned long cnt)
{
	unsigned cclk;

	if (pla->reg[PLA_MODE] & PLA_MODE_CS) {
		pla->clock += (unsigned long) PLA_CFREQ1 * cnt;
	}
	else {
		pla->clock += (unsigned long) PLA_CFREQ2 * cnt;
	}

	cclk = pla->clock / PLA_IFREQ;
	pla->clock = pla->clock % PLA_IFREQ;

	if (cclk > 0) {
		e6845_clock (&pla->crtc, cclk);
	}
}

static
void pla_set_blink_rate (plantronics_t *pla, unsigned rate, int start)
{
	pla->blink = (start != 0);
	pla->blink_cnt = rate;
	pla->blink_rate = rate;
	pla->mod_cnt = 2;
}

static
void pla_set_font (plantronics_t *pla, unsigned font)
{
	if (font == 0) {
		pla->font = cga_font_thick;
	}
	else {
		pla->font = cga_font_thin;
	}

	pla->mod_cnt = 2;
}

/*
 * Set the internal palette from the mode and color select registers
 */
static
void pla_set_palette (plantronics_t *pla)
{
	unsigned char mode, csel;
	unsigned char *pal;

	pal = pla->pal;

	mode = pla->reg[PLA_MODE];
	csel = pla->reg[PLA_CSEL];

	pal[0] = csel & 0x0f;

	if (mode & PLA_MODE_CBURST) {
		pal[1] = 3;
		pal[2] = 4;
		pal[3] = 7;
	}
	else if (csel & PLA_CSEL_PAL) {
		pal[1] = 3;
		pal[2] = 5;
		pal[3] = 7;
	}
	else {
		pal[1] = 2;
		pal[2] = 4;
		pal[3] = 6;
	}

	if (csel & PLA_CSEL_INT) {
		pal[1] += 8;
		pal[2] += 8;
		pal[3] += 8;
	}
}

/*
 * Set the mode register
 */
static
void pla_set_mode (plantronics_t *pla, unsigned char val)
{
	if (pla->reg[PLA_MODE] != val) {
		pla->reg[PLA_MODE] = val;
		pla_set_palette (pla);
		pla->mod_cnt = 2;
	}
}

/*
 * Set the color select register
 */
static
void pla_set_color_select (plantronics_t *pla, unsigned char val)
{
	if (pla->reg[PLA_CSEL] != val) {
		pla->reg[PLA_CSEL] = val;
		pla_set_palette (pla);
		pla->mod_cnt = 2;
	}
}

/*
 * Get the status register
 */
static
unsigned char pla_get_status (plantronics_t *pla)
{
	unsigned char val;

	pce_video_clock1 (&pla->video, 0);

	val = pla->reg[PLA_STATUS];
	val |= (PLA_STATUS_VSYNC | PLA_STATUS_SYNC);

	if (e6845_get_de (&pla->crtc)) {
		val &= ~PLA_STATUS_SYNC;
	}

	if (pla->crtc.vsync_cnt == 0) {
		val &= ~PLA_STATUS_VSYNC;
	}

	pla->reg[PLA_STATUS] = val;

	return (val);
}

static
void pla_set_pen_reset (plantronics_t *pla, unsigned char val)
{
	pla->reg[PLA_STATUS] &= ~PLA_STATUS_PEN;
}

static
void pla_set_pen_set (plantronics_t *pla, unsigned char val)
{
	pla->reg[PLA_STATUS] |= PLA_STATUS_PEN;
	e6845_set_pen (&pla->crtc);
}

static
void pla_set_special (plantronics_t *pla, unsigned char val)
{
	if (pla->reg[PLA_SPECIAL] != val) {
		pla->reg[PLA_SPECIAL] = val;
		pla->mod_cnt = 2;
	}
}

static
unsigned char pla_get_special (plantronics_t *pla)
{
	return (pla->reg[PLA_SPECIAL]);
}

/*
 * Get a Plantronics register
 */
static
unsigned char pla_reg_get_uint8 (plantronics_t *pla, unsigned long addr)
{
	switch (addr) {
	case PLA_CRTC_INDEX:
	case PLA_CRTC_INDEX0:
		return (e6845_get_index (&pla->crtc));

	case PLA_CRTC_DATA:
	case PLA_CRTC_DATA0:
		return (e6845_get_data (&pla->crtc));

	case PLA_STATUS:
		return (pla_get_status (pla));

	case PLA_SPECIAL:
		return (pla_get_special (pla));

	default:
		break;
	}

	return (0xff);
}

static
unsigned short pla_reg_get_uint16 (plantronics_t *pla, unsigned long addr)
{
	return (0xffff);
}

/*
 * Set a Plantronics register
 */
static
void pla_reg_set_uint8 (plantronics_t *pla, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case PLA_CRTC_INDEX:
	case PLA_CRTC_INDEX0:
		e6845_set_index (&pla->crtc, val);
		break;

	case PLA_CRTC_DATA:
	case PLA_CRTC_DATA0:
		e6845_set_data (&pla->crtc, val);
		pla->mod_cnt = 2;
		break;

	case PLA_MODE:
		pla_set_mode (pla, val);
		break;

	case PLA_CSEL:
		pla_set_color_select (pla, val);
		break;

	case PLA_PEN_RESET:
		pla_set_pen_reset (pla, val);
		break;

	case PLA_PEN_SET:
		pla_set_pen_set (pla, val);
		break;

	case PLA_SPECIAL:
		pla_set_special (pla, val);
		break;

	default:
		fprintf (stderr, "PLA: set reg (%03lX, %02X)\n\n", addr, val);
		break;
	}
}

static
void pla_reg_set_uint16 (plantronics_t *pla, unsigned long addr, unsigned short val)
{
	pla_reg_set_uint8 (pla, addr, val & 0xff);

	if (addr < 15) {
		pla_reg_set_uint8 (pla, addr + 1, (val >> 8) & 0xff);
	}
}

static inline
int pla_alternate_plane (const plantronics_t *pla)
{
	if ((pla->reg[PLA_MODE] & PLA_MODE_G320) == 0) {
		return (0);
	}

	if ((pla->reg[PLA_SPECIAL] & PLA_SPECIAL_PLANE) == 0) {
		return (0);
	}

	if ((pla->reg[PLA_SPECIAL] & (PLA_SPECIAL_EXT1 | PLA_SPECIAL_EXT2)) == 0) {
		return (0);
	}

	return (1);
}

static
void pla_mem_set_uint8 (plantronics_t *pla, unsigned long addr, unsigned char val)
{
	if (pla_alternate_plane (pla)) {
		addr ^= 0x4000;
	}

	if (addr < pla->memblk->size) {
		pla->mem[addr] = val;
		pla->mod_cnt = 2;
	}
}

static
void pla_mem_set_uint16 (plantronics_t *pla, unsigned long addr, unsigned short val)
{
	if (pla_alternate_plane (pla)) {
		addr ^= 0x4000;
	}

	if (addr < pla->memblk->size) {
		pla->mem[addr] = val;

		if ((addr + 1) < pla->memblk->size) {
			pla->mem[addr + 1] = (val >> 8) & 0xff;
		}

		pla->mod_cnt = 2;
	}
}

static
unsigned char pla_mem_get_uint8 (plantronics_t *pla, unsigned long addr)
{
	if (pla_alternate_plane (pla)) {
		addr ^= 0x4000;
	}

	if (addr < pla->memblk->size) {
		return (pla->mem[addr]);
	}

	return (0);
}

static
unsigned short pla_mem_get_uint16 (plantronics_t *pla, unsigned long addr)
{
	unsigned short val;

	val = 0;

	if (pla_alternate_plane (pla)) {
		addr ^= 0x4000;
	}

	if (addr < pla->memblk->size) {
		val = pla->mem[addr];

		if ((addr + 1) < pla->memblk->size) {
			val |= (unsigned) pla->mem[addr + 1] << 8;
		}
	}

	return (val);
}

static
int pla_set_msg (plantronics_t *pla, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.font", msg)) {
		unsigned font;

		if (msg_get_uint (val, &font)) {
			return (1);
		}

		pla_set_font (pla, font);

		return (0);
	}

	return (-1);
}

static
void pla_print_info (plantronics_t *pla, FILE *fp)
{
	unsigned      col, row, w1, w2;
	unsigned      ch, vt, ht, vtl, vdl;
	unsigned      base, addr;
	unsigned long clk1, clk2;
	unsigned char status;
	unsigned char *reg;
	e6845_t       *crt;

	crt = &pla->crtc;
	reg = pla->reg;

	status = pla_get_status (pla);

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

	if ((reg[PLA_MODE] ^ PLA_MODE_CS) & (PLA_MODE_CS | PLA_MODE_G640)) {
		clk1 *= 2;
	}

	if (reg[PLA_MODE] & PLA_MODE_G640) {
		col *= 2;
		w1 *= 2;
		w2 *= 2;
	}

	clk2 = clk1 * vtl;

	fprintf (fp,
		"DEV: Plantronics Colorplus\n"
		"INDX[3D4]=%02X   COL=%3u/%3u  HFRQ=%9.3f  %u*%u\n"
		"MODE[3D8]=%02X   ROW=%3u/%3u  VFRQ=%9.3f  %u*%u\n"
		"CSEL[3D9]=%02X  CCOL=%3u/%3u   HDE=%d  HSYN=%X  BASE=%04X\n"
		"STAT[3DA]=%02X  CROW=%3u/%3u   VDE=%d  VSYN=%X  ADDR=%04X\n"
		"SPEC[3DD]=%02X    RA=%3u/%3u\n",
		e6845_get_index (crt), col, w2, (double) PLA_PFREQ / clk1, w1, vdl,
		reg[PLA_MODE], row, vtl, (double) PLA_PFREQ / clk2, w2, vtl,
		reg[PLA_CSEL], crt->ccol, ht, e6845_get_hde (crt), crt->hsync_cnt, base,
		status, crt->crow, vt, e6845_get_vde (crt), crt->vsync_cnt, addr,
		reg[PLA_SPECIAL], crt->ra, ch
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
void pla_set_terminal (plantronics_t *pla, terminal_t *trm)
{
	pla->term = trm;

	if (pla->term != NULL) {
		trm_open (pla->term, 640, 200);
	}
}

static
mem_blk_t *pla_get_mem (plantronics_t *pla)
{
	return (pla->memblk);
}

static
mem_blk_t *pla_get_reg (plantronics_t *pla)
{
	return (pla->regblk);
}

static
void pla_init (plantronics_t *pla, unsigned long io, unsigned long addr)
{
	pce_video_init (&pla->video);

	pla->video.ext = pla;
	pla->video.del = (void *) pla_del;
	pla->video.set_msg = (void *) pla_set_msg;
	pla->video.set_terminal = (void *) pla_set_terminal;
	pla->video.get_mem = (void *) pla_get_mem;
	pla->video.get_reg = (void *) pla_get_reg;
	pla->video.set_blink_rate = (void *) pla_set_blink_rate;
	pla->video.print_info = (void *) pla_print_info;
	pla->video.clock = (void *) pla_clock;

	pla->memblk = mem_blk_new (addr, 32768, 1);
	mem_blk_set_fget (pla->memblk, pla, pla_mem_get_uint8, pla_mem_get_uint16, NULL);
	mem_blk_set_fset (pla->memblk, pla, pla_mem_set_uint8, pla_mem_set_uint16, NULL);
	mem_blk_clear (pla->memblk, 0x00);
	pla->mem = pla->memblk->data;

	pla->regblk = mem_blk_new (io, 16, 1);
	mem_blk_set_fget (pla->regblk, pla, pla_reg_get_uint8, pla_reg_get_uint16, NULL);
	mem_blk_set_fset (pla->regblk, pla, pla_reg_set_uint8, pla_reg_set_uint16, NULL);
	mem_blk_clear (pla->regblk, 0x00);
	pla->reg = pla->regblk->data;

	e6845_init (&pla->crtc);
	e6845_set_hsync_fct (&pla->crtc, pla, pla_hsync);
	e6845_set_vsync_fct (&pla->crtc, pla, pla_vsync);

	pla->term = NULL;
	pla->font = cga_font_thick;
	pla->clock = 0;
	pla->mod_cnt = 0;

	pla->pal[0] = 0;
	pla->pal[1] = 11;
	pla->pal[2] = 13;
	pla->pal[3] = 15;

	pla->blink = 0;
	pla->blink_cnt = 0;
	pla->blink_rate = 16;
}

static
void pla_free (plantronics_t *pla)
{
	e6845_free (&pla->crtc);

	mem_blk_del (pla->memblk);
	mem_blk_del (pla->regblk);
}

plantronics_t *pla_new (unsigned long io, unsigned long addr)
{
	plantronics_t *pla;

	if ((pla = malloc (sizeof (plantronics_t))) == NULL) {
		return (NULL);
	}

	pla_init (pla, io, addr);

	return (pla);
}

static
void pla_del (plantronics_t *pla)
{
	if (pla != NULL) {
		pla_free (pla);
		free (pla);
	}
}

video_t *pla_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      blink, font;
	plantronics_t *pla;

	ini_get_uint32 (sct, "io", &io, 0x3d0);
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	ini_get_uint16 (sct, "blink", &blink, 16);
	ini_get_uint16 (sct, "font", &font, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "Plantronics io=0x%04lx addr=0x%05lx font=%u blink=%u\n",
		io, addr, font, blink
	);

	if ((pla = pla_new (io, addr)) == NULL) {
		return (NULL);
	}

	pla_set_blink_rate (pla, blink, 1);
	pla_set_font (pla, font);

	return (&pla->video);
}
