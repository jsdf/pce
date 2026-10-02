/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/olivetti.c                                 *
 * Created:     2011-09-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2020 Hampa Hug <hampa@hampa.ch>                     *
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
#include <devices/video/olivetti.h>
#include <devices/video/olivetti_font.h>
#include <drivers/video/terminal.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <libini/libini.h>


#define M24_IFREQ  1193182
#define M24_PFREQ  24000000
#define M24_CFREQ1 (M24_PFREQ / 8)
#define M24_CFREQ2 (M24_PFREQ / 16)

#define M24_CRTC_INDEX0     0
#define M24_CRTC_DATA0      1
#define M24_CRTC_INDEX      4
#define M24_CRTC_DATA       5
#define M24_MODE            8
#define M24_CSEL            9
#define M24_STATUS          10
#define M24_PEN_RESET       11
#define M24_PEN_SET         12
#define M24_MODE2           14

#define M24_MODE_CS         0x01
#define M24_MODE_G320       0x02
#define M24_MODE_CBURST     0x04
#define M24_MODE_ENABLE     0x08
#define M24_MODE_G640       0x10
#define M24_MODE_BLINK      0x20

#define M24_CSEL_COL        0x0f
#define M24_CSEL_INT        0x10
#define M24_CSEL_PAL        0x20

#define M24_STATUS_SYNC     0x01
#define M24_STATUS_PEN      0x02
#define M24_STATUS_VSYNC    0x08

#define M24_MODE2_M20       0x01
#define M24_MODE2_DEGAUSS   0x02
#define M24_MODE2_SETSEL    0x04
#define M24_MODE2_PAGESEL   0x08
#define M24_MODE2_NIMODE0   0x10
#define M24_MODE2_IMODE1    0x20
#define M24_MODE2_UNDERLINE 0x40
#define M24_MODE2_Z8000CLKE 0x80


static void m24_del (m24_t *m24);


unsigned char m24_rgbi[16 * 4] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xaa, 0x00,
	0x00, 0xaa, 0x00, 0x00,
	0x00, 0xaa, 0xaa, 0x00,
	0xaa, 0x00, 0x00, 0x00,
	0xaa, 0x00, 0xaa, 0x00,
	0xaa, 0x55, 0x00, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0x55, 0x55, 0xff, 0x00,
	0x55, 0xff, 0x55, 0x00,
	0x55, 0xff, 0xff, 0x00,
	0xff, 0x55, 0x55, 0x00,
	0xff, 0x55, 0xff, 0x00,
	0xff, 0xff, 0x55, 0x00,
	0xff, 0xff, 0xff, 0x00
};

unsigned char m24_gray[16 * 4] = {
	0x00, 0x00, 0x00, 0x00,
	0x18, 0x18, 0x18, 0x00,
	0x30, 0x30, 0x30, 0x00,
	0x48, 0x48, 0x48, 0x00,
	0x60, 0x60, 0x60, 0x00,
	0x78, 0x78, 0x78, 0x00,
	0x90, 0x90, 0x90, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0x6d, 0x6d, 0x6d, 0x00,
	0x85, 0x85, 0x85, 0x00,
	0x9d, 0x9d, 0x9d, 0x00,
	0xb6, 0xb6, 0xb6, 0x00,
	0xce, 0xce, 0xce, 0x00,
	0xe6, 0xe6, 0xe6, 0x00,
	0xff, 0xff, 0xff, 0x00
};


static
void m24_line_blank (m24_t *m24, unsigned row)
{
	unsigned char *p;

	p = pce_video_get_row_ptr (&m24->video, row);

	memset (p, 0, 3 * m24->video.buf_w);
}

/*
 * M24 text modes
 */
static
void m24_line_mode0 (m24_t *m24, unsigned row)
{
	unsigned            i, j;
	int                 ul, bl;
	unsigned            hd;
	unsigned            addr, caddr, amask;
	unsigned char       code, attr;
	unsigned char       val, mask, cmask;
	const unsigned char *font, *fg, *bg, *col;
	unsigned char       *ptr;

	hd = e6845_get_hd (&m24->crtc);

	m24->video.buf_next_w = 8 * hd;

	if ((8 * hd) > m24->video.buf_w) {
		hd = m24->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&m24->video, row);
	addr = m24->crtc.ma;
	amask = 0x7fff;

	if (m24->blink) {
		caddr = e6845_get_cursor_address (&m24->crtc);
		cmask = e6845_get_cursor_mask (&m24->crtc, 0);
	}
	else {
		caddr = -1;
		cmask = 0;
	}

	font = (m24->reg[M24_MODE2] & M24_MODE2_SETSEL) ? m24->font : m24->font;

	ul = (m24->mono && (m24->reg[M24_MODE] & M24_MODE_CBURST)) || (m24->reg[M24_MODE2] & M24_MODE2_UNDERLINE);
	bl = m24->reg[M24_MODE] & M24_MODE_BLINK;

	for (i = 0; i < hd; i++) {
		code = m24->mem[(2 * addr + 0) & amask];
		attr = m24->mem[(2 * addr + 1) & amask];

		val = font[16 * code + (m24->crtc.ra & 0x0f)];

		if (bl && (attr & 0x80)) {
			if (m24->blink == 0) {
				attr = (attr & 0x70) | ((attr >> 4) & 0x07);
			}

			attr &= 0x7f;
		}

		if (ul && ((attr & 7) == 1)) {
			if (m24->mono) {
				attr |= 7;
			}

			if (m24->crtc.ra == 14) {
				val = 0xff;
			}
		}

		if (addr == caddr) {
			val |= cmask;
		}

		fg = m24->rgbi + 4 * (attr & 15);
		bg = m24->rgbi + 4 * ((attr >> 4) & 15);

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
 * M24 320 * 400 * 4
 */
static
void m24_line_mode1 (m24_t *m24, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val;
	unsigned char       *ptr;
	const unsigned char *col;

	hd = e6845_get_hd (&m24->crtc);

	m24->video.buf_next_w = 8 * hd;

	if ((8 * hd) > m24->video.buf_w) {
		hd = m24->video.buf_w / 8;
	}

	ptr = pce_video_get_row_ptr (&m24->video, row);
	addr = (m24->crtc.ma ^ ((m24->crtc.ra & 2) << 11)) << 1;

	for (i = 0; i < hd; i++) {
		val = m24->mem[addr & 0x7fff];
		val = (val << 8) | m24->mem[(addr + 1) & 0x7fff];

		for (j = 0; j < 8; j++) {
			col = m24->rgbi + 4 * m24->pal[(val >> 14) & 3];

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val <<= 2;
		}

		addr += 2;
	}
}

/*
 * M24 640 * 400 * 2
 */
static
void m24_line_mode2 (m24_t *m24, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, addr, val;
	unsigned char       *ptr;
	const unsigned char *fg, *bg, *col;

	hd = e6845_get_hd (&m24->crtc);

	m24->video.buf_next_w = 16 * hd;

	if ((16 * hd) > m24->video.buf_w) {
		hd = m24->video.buf_w / 16;
	}

	ptr = pce_video_get_row_ptr (&m24->video, row);

	addr = 2 * (m24->crtc.ma & 0x1fff);

	if (m24->reg[M24_MODE2] & M24_MODE2_M20) {
		addr |= (m24->crtc.ra & 3) << 13;
	}
	else {
		addr |= (m24->crtc.ra & 2) << 12;
	}

	fg = m24->rgbi + 4 * (m24->reg[M24_CSEL] & 15);
	bg = m24->rgbi;

	for (i = 0; i < hd; i++) {
		val = m24->mem[addr & 0x7fff];
		val = (val << 8) | m24->mem[(addr + 1) & 0x7fff];

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

static
void m24_hsync (m24_t *m24)
{
	unsigned      row, max, ch;
	unsigned char mode1;

	if (m24->mod_cnt == 0) {
		return;
	}

	mode1 = m24->reg[M24_MODE];

	ch = (e6845_get_ml (&m24->crtc) & 0x1f) + 1;
	row = ch * m24->crtc.crow + m24->crtc.ra;
	max = ch * e6845_get_vd (&m24->crtc);

	if (row >= max) {
		return;
	}

	if (row >= m24->video.buf_h) {
		return;
	}

	if ((mode1 & M24_MODE_ENABLE) == 0) {
		m24_line_blank (m24, row);
	}
	else if (mode1 & M24_MODE_G320) {
		if (mode1 & M24_MODE_G640) {
			m24_line_mode2 (m24, row);
		}
		else {
			m24_line_mode1 (m24, row);
		}
	}
	else {
		m24_line_mode0 (m24, row);
	}
}

static
void m24_vsync (m24_t *m24)
{
	video_t *vid;

	vid = &m24->video;

	if ((m24->term != NULL) && (vid->buf_w > 0) && (vid->buf_h > 0)) {
		trm_set_size (m24->term, vid->buf_w, vid->buf_h);

		if (m24->mod_cnt > 0) {
			trm_set_lines (m24->term, vid->buf, 0, vid->buf_h);
		}

		trm_update (m24->term);
	}

	if (m24->mod_cnt > 0) {
		m24->mod_cnt -= 1;
	}

	vid->buf_next_h = e6845_get_vdl (&m24->crtc);

	if (vid->buf_next_w == 0) {
		vid->buf_next_w = 640;
	}

	if (vid->buf_next_h == 0) {
		vid->buf_next_h = 400;
	}

	if ((vid->buf_w != vid->buf_next_w) || (vid->buf_h != vid->buf_next_h)) {
		pce_video_set_buf_size (vid, vid->buf_next_w, vid->buf_next_h, 3);
		m24->mod_cnt = 1;
	}

	if (m24->blink_cnt > 0) {
		m24->blink_cnt -= 1;

		if (m24->blink_cnt == 0) {
			m24->blink = !m24->blink;
			m24->blink_cnt = m24->blink_rate;
			m24->mod_cnt = 1;
		}
	}
}

static
void m24_clock (m24_t *m24, unsigned long cnt)
{
	unsigned cclk;

	if (m24->reg[M24_MODE] & M24_MODE_CS) {
		m24->clock += (unsigned long) M24_CFREQ1 * cnt;
	}
	else {
		m24->clock += (unsigned long) M24_CFREQ2 * cnt;
	}

	cclk = m24->clock / M24_IFREQ;
	m24->clock = m24->clock % M24_IFREQ;

	if (cclk > 0) {
		e6845_clock (&m24->crtc, cclk);
	}
}

static
void m24_set_blink_rate (m24_t *m24, unsigned rate, int start)
{
	m24->blink = (start != 0);
	m24->blink_cnt = rate;
	m24->blink_rate = rate;
	m24->mod_cnt = 2;
}

/*
 * Set the internal palette from the mode and color select registers
 */
static
void m24_set_palette (m24_t *m24)
{
	unsigned char mode, csel;
	unsigned char *pal;

	pal = m24->pal;

	mode = m24->reg[M24_MODE];
	csel = m24->reg[M24_CSEL];

	pal[0] = csel & 0x0f;

	if (mode & M24_MODE_CBURST) {
		pal[1] = 3;
		pal[2] = 4;
		pal[3] = 7;
	}
	else if (csel & M24_CSEL_PAL) {
		pal[1] = 3;
		pal[2] = 5;
		pal[3] = 7;
	}
	else {
		pal[1] = 2;
		pal[2] = 4;
		pal[3] = 6;
	}

	if (csel & M24_CSEL_INT) {
		pal[1] += 8;
		pal[2] += 8;
		pal[3] += 8;
	}
}

static
unsigned char m24_get_scrambler (m24_t *m24, unsigned char val)
{
	unsigned idx;

	idx = ((m24->crtc.index & 0x0f) << 8) | (val & 0xff);

	return (m24->scrambler[idx]);
}

/*
 * Set the mode register
 */
static
void m24_set_mode (m24_t *m24, unsigned char val)
{
	if (m24->reg[M24_MODE] != val) {
		m24->reg[M24_MODE] = val;
		m24_set_palette (m24);
		m24->mod_cnt = 2;
	}
}

/*
 * Set the color select register
 */
static
void m24_set_color_select (m24_t *m24, unsigned char val)
{
	if (m24->reg[M24_CSEL] != val) {
		m24->reg[M24_CSEL] = val;
		m24_set_palette (m24);
		m24->mod_cnt = 2;
	}
}

/*
 * Get the status register
 */
static
unsigned char m24_get_status (m24_t *m24)
{
	unsigned char val;

	pce_video_clock1 (&m24->video, 0);

	val = m24->reg[M24_STATUS];
	val |= (M24_STATUS_VSYNC | M24_STATUS_SYNC);

	if (e6845_get_de (&m24->crtc)) {
		val &= ~M24_STATUS_SYNC;
	}

	if (m24->crtc.vsync_cnt == 0) {
		val &= ~M24_STATUS_VSYNC;
	}

	m24->reg[M24_STATUS] = val;

	return (val);
}

static
void m24_set_pen_reset (m24_t *m24, unsigned char val)
{
	m24->reg[M24_STATUS] &= ~M24_STATUS_PEN;
}

static
void m24_set_pen_set (m24_t *m24, unsigned char val)
{
	m24->reg[M24_STATUS] |= M24_STATUS_PEN;
	e6845_set_pen (&m24->crtc);
}

static
void m24_set_mode2 (m24_t *m24, unsigned char val)
{
	if (m24->reg[M24_MODE2] != val) {
		m24->reg[M24_MODE2] = val;
		m24->mod_cnt = 2;
	}
}

static
unsigned char m24_reg_get_uint8 (m24_t *m24, unsigned long addr)
{
	switch (addr) {
	case M24_CRTC_INDEX:
	case M24_CRTC_INDEX0:
		return (e6845_get_index (&m24->crtc));

	case M24_CRTC_DATA:
	case M24_CRTC_DATA0:
		return (e6845_get_data (&m24->crtc));

	case M24_STATUS:
		return (m24_get_status (m24));

	default:
		return (0xff);
	}
}

static
unsigned short m24_reg_get_uint16 (m24_t *m24, unsigned long addr)
{
	return (0xffff);
}

static
void m24_reg_set_uint8 (m24_t *m24, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case M24_CRTC_INDEX:
	case M24_CRTC_INDEX0:
		e6845_set_index (&m24->crtc, val);
		break;

	case M24_CRTC_DATA:
	case M24_CRTC_DATA0:
		e6845_set_data (&m24->crtc, m24_get_scrambler (m24, val));
		m24->mod_cnt = 2;
		break;

	case M24_MODE:
		m24_set_mode (m24, val);
		break;

	case M24_CSEL:
		m24_set_color_select (m24, val);
		break;

	case M24_PEN_RESET:
		m24_set_pen_reset (m24, val);
		break;

	case M24_PEN_SET:
		m24_set_pen_set (m24, val);
		break;

	case M24_MODE2:
		m24_set_mode2 (m24, val);
		break;

	default:
		/* fprintf (stderr, "M24: set reg (%03lX, %02X)\n", addr, val); */
		break;
	}
}

static
void m24_reg_set_uint16 (m24_t *m24, unsigned long addr, unsigned short val)
{
	m24_reg_set_uint8 (m24, addr, val & 0xff);

	if (addr < 15) {
		m24_reg_set_uint8 (m24, addr + 1, (val >> 8) & 0xff);
	}
}

static
void m24_set_mono (m24_t *m24, int mono)
{
	m24->mono = (mono != 0);

	m24->reg[M24_STATUS] &= 0xcf;

	if (mono) {
		m24->reg[M24_STATUS] |= 0x30;
		m24->rgbi = m24_gray;
	}
	else {
		m24->reg[M24_STATUS] |= 0x20;
		m24->rgbi = m24_rgbi;
	}

	m24->mod_cnt = 2;
}

static
unsigned char m24_mem_get_uint8 (m24_t *m24, unsigned long addr)
{
	return (m24->mem[addr & 0x7fff]);
}

static
unsigned short m24_mem_get_uint16 (m24_t *m24, unsigned long addr)
{
	unsigned short val;

	val = m24->mem[(addr + 1) & 0x7fff];
	val = (val << 8) | m24->mem[addr & 0x7fff];

	return (val);
}

static
void m24_mem_set_uint8 (m24_t *m24, unsigned long addr, unsigned char val)
{
	m24->mem[addr & 0x7fff] = val;
	m24->mod_cnt = 2;
}

static
void m24_mem_set_uint16 (m24_t *m24, unsigned long addr, unsigned short val)
{
	m24->mem[(addr + 0) & 0x7fff] = val & 0xff;
	m24->mem[(addr + 1) & 0x7fff] = (val >> 8) & 0xff;
	m24->mod_cnt = 2;
}

static
int m24_set_msg (m24_t *m24, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.mono", msg)) {
		unsigned mono;

		if (msg_get_uint (val, &mono)) {
			return (1);
		}

		m24_set_mono (m24, mono != 0);

		return (0);
	}

	return (-1);
}

static
void m24_print_info (m24_t *m24, FILE *fp)
{
	unsigned      col, row, w1, w2;
	unsigned      ch, vt, ht, vtl, vdl;
	unsigned      base, addr;
	unsigned long clk1, clk2;
	unsigned char status;
	unsigned char *reg;
	e6845_t       *crt;

	crt = &m24->crtc;
	reg = m24->reg;

	status = m24_get_status (m24);

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

	if ((reg[M24_MODE] ^ M24_MODE_CS) & (M24_MODE_CS | M24_MODE_G640)) {
		clk1 *= 2;
	}

	if (reg[M24_MODE] & M24_MODE_G640) {
		col *= 2;
		w1 *= 2;
		w2 *= 2;
	}

	clk2 = clk1 * vtl;

	fprintf (fp,
		"DEV: Olivetti M24\n"
		"INDX[3D4]=%02X   COL=%3u/%3u  HFRQ=%9.3f  %u*%u\n"
		"MODE[3D8]=%02X   ROW=%3u/%3u  VFRQ=%9.3f  %u*%u\n"
		"CSEL[3D9]=%02X  CCOL=%3u/%3u   HDE=%d  HSYN=%X  BASE=%04X\n"
		"STAT[3DA]=%02X  CROW=%3u/%3u   VDE=%d  VSYN=%X  ADDR=%04X\n"
		"MOD2[3DE]=%02X    RA=%3u/%3u\n",
		e6845_get_index (crt), col, w2, (double) M24_PFREQ / clk1, w1, vdl,
		reg[M24_MODE], row, vtl, (double) M24_PFREQ / clk2, w2, vtl,
		reg[M24_CSEL], crt->ccol, ht, e6845_get_hde (crt), crt->hsync_cnt, base,
		status, crt->crow, vt, e6845_get_vde (crt), crt->vsync_cnt, addr,
		reg[M24_MODE2], crt->ra, ch
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
void m24_set_terminal (m24_t *m24, terminal_t *trm)
{
	m24->term = trm;

	if (m24->term != NULL) {
		trm_open (m24->term, 640, 400);
	}
}

static
mem_blk_t *m24_get_mem (m24_t *m24)
{
	return (m24->memblk);
}

static
mem_blk_t *m24_get_reg (m24_t *m24)
{
	return (m24->regblk);
}

static
void m24_init (m24_t *m24, unsigned long io, unsigned long addr)
{
	pce_video_init (&m24->video);

	m24->video.ext = m24;
	m24->video.del = (void *) m24_del;
	m24->video.set_msg = (void *) m24_set_msg;
	m24->video.set_terminal = (void *) m24_set_terminal;
	m24->video.get_mem = (void *) m24_get_mem;
	m24->video.get_reg = (void *) m24_get_reg;
	m24->video.set_blink_rate = (void *) m24_set_blink_rate;
	m24->video.print_info = (void *) m24_print_info;
	m24->video.clock = (void *) m24_clock;

	m24->memblk = mem_blk_new (addr, 32768, 1);
	mem_blk_set_fget (m24->memblk, m24, m24_mem_get_uint8, m24_mem_get_uint16, NULL);
	mem_blk_set_fset (m24->memblk, m24, m24_mem_set_uint8, m24_mem_set_uint16, NULL);
	mem_blk_clear (m24->memblk, 0x00);
	m24->mem = m24->memblk->data;

	m24->regblk = mem_blk_new (io, 16, 1);
	mem_blk_set_fget (m24->regblk, m24, m24_reg_get_uint8, m24_reg_get_uint16, NULL);
	mem_blk_set_fset (m24->regblk, m24, m24_reg_set_uint8, m24_reg_set_uint16, NULL);
	mem_blk_clear (m24->regblk, 0x00);
	m24->reg = m24->regblk->data;

	e6845_init (&m24->crtc);
	e6845_set_hsync_fct (&m24->crtc, m24, m24_hsync);
	e6845_set_vsync_fct (&m24->crtc, m24, m24_vsync);

	m24->term = NULL;
	m24->font = m24_font;
	m24->rgbi = m24_rgbi;
	m24->scrambler = m24_scrambler_p3;

	m24->clock = 0;
	m24->mod_cnt = 0;

	m24->pal[0] = 0;
	m24->pal[1] = 11;
	m24->pal[2] = 13;
	m24->pal[3] = 15;

	m24->mono = 0;

	m24->blink = 0;
	m24->blink_cnt = 0;
	m24->blink_rate = 16;

	m24->reg[M24_STATUS] = 0xe0;
}

static
void m24_free (m24_t *m24)
{
	e6845_free (&m24->crtc);

	mem_blk_del (m24->memblk);
	mem_blk_del (m24->regblk);
}

m24_t *m24_new (unsigned long io, unsigned long addr)
{
	m24_t *m24;

	if ((m24 = malloc (sizeof (m24_t))) == NULL) {
		return (NULL);
	}

	m24_init (m24, io, addr);

	return (m24);
}

static
void m24_del (m24_t *m24)
{
	if (m24 != NULL) {
		m24_free (m24);
		free (m24);
	}
}

video_t *m24_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      blink;
	int           mono;
	m24_t         *m24;

	ini_get_uint32 (sct, "io", &io, 0x3d0);
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	ini_get_uint16 (sct, "blink", &blink, 16);
	ini_get_bool (sct, "mono", &mono, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "Olivetti io=0x%04lx addr=0x%05lx mono=%d\n",
		io, addr, mono
	);

	if ((m24 = m24_new (io, addr)) == NULL) {
		return (NULL);
	}

	m24_set_mono (m24, mono);
	m24_set_blink_rate (m24, blink, 1);

	return (&m24->video);
}
