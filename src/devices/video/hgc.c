/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/hgc.c                                      *
 * Created:     2003-08-19 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chipset/e6845.h>
#include <devices/memory.h>
#include <devices/video/video.h>
#include <devices/video/hgc.h>
#include <devices/video/mda_font.h>
#include <drivers/video/terminal.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <libini/libini.h>


#define HGC_IFREQ  1193182
#define HGC_PFREQ  16000000
#define HGC_CFREQ1 (HGC_PFREQ / 9)
#define HGC_CFREQ2 (HGC_PFREQ / 16)

#define HGC_CRTC_INDEX   4
#define HGC_CRTC_DATA    5
#define HGC_MODE         8
#define HGC_PEN_SET      9
#define HGC_STATUS       10
#define HGC_PEN_RESET    11
#define HGC_CONFIG       15

#define HGC_MODE_GRAPH   0x02
#define HGC_MODE_ENABLE  0x08
#define HGC_MODE_BLINK   0x20
#define HGC_MODE_PAGE1   0x80

#define HGC_STATUS_HSYNC 0x01
#define HGC_STATUS_PEN   0x02
#define HGC_STATUS_VIDEO 0x08
#define HGC_STATUS_VSYNC 0x80

#define HGC_CONFIG_GRAPH 0x01
#define HGC_CONFIG_PAGE1 0x02


static void hgc_del (hgc_t *hgc);


static
void hgc_line_blank (hgc_t *hgc, unsigned row)
{
	unsigned char *ptr;

	ptr = pce_video_get_row_ptr (&hgc->video, row);

	memset (ptr, 0, 3 * hgc->video.buf_w);
}

static
void hgc_line_text (hgc_t *hgc, unsigned row)
{
	unsigned            i, j, hd;
	unsigned            val, cmask;
	unsigned            addr, caddr;
	unsigned char       code, attr;
	int                 blink;
	unsigned            fgi, bgi;
	const unsigned char *mem, *col, *fg, *bg;
	unsigned char       *ptr;

	hd = hgc->crtc.reg[E6845_REG_HD];

	hgc->video.buf_next_w = 9 * hd;

	if ((9 * hd) > hgc->video.buf_w) {
		hd = hgc->video.buf_w / 9;
	}

	mem = hgc->mem + ((hgc->reg[HGC_MODE] & HGC_MODE_PAGE1) ? 0x8000 : 0);
	ptr = pce_video_get_row_ptr (&hgc->video, row);
	addr = hgc->crtc.ma;

	if (hgc->blink) {
		caddr = e6845_get_cursor_address (&hgc->crtc);
		cmask = e6845_get_cursor_mask (&hgc->crtc, 0);
	}
	else {
		caddr = -1;
		cmask = 0;
	}

	blink = (hgc->reg[HGC_MODE] & HGC_MODE_BLINK) != 0;

	for (i = 0; i < hd; i++) {
		code = mem[(2 * addr + 0) & 0x7fff];
		attr = mem[(2 * addr + 1) & 0x7fff];

		val = hgc->font[14 * code + (hgc->crtc.ra & 0x0f)] << 1;

		if ((code & 0xe0) == 0xc0) {
			val |= (val >> 1) & 1;
		}

		if (((attr & 7) == 1) && (hgc->crtc.ra == 13)) {
			val = 0x1ff;
		}

		if ((attr & 0x80) && blink && (hgc->blink == 0)) {
			val = 0;
		}

		if (addr == caddr) {
			val |= cmask;
		}

		switch (attr & 0x7f) {
		case 0x00:
		case 0x08:
			fgi = 0;
			bgi = 0;
			break;

		case 0x70:
			fgi = 0;
			bgi = ((attr & 0x80) && !blink) ? 3 : 2;
			break;

		case 0x78:
			fgi = 1;
			bgi = ((attr & 0x80) && !blink) ? 3 : 2;
			break;

		default:
			fgi = (attr & 0x08) ? 3 : 2;
			bgi = 0;
			break;
		}

		fg = hgc->rgb[fgi];
		bg = hgc->rgb[bgi];

		for (j = 0; j < 9; j++) {
			col = (val & 0x100) ? fg : bg;

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val <<= 1;
		}

		addr += 1;
	}
}

static
void hgc_line_graph (hgc_t *hgc, unsigned row)
{
	unsigned            i, j;
	unsigned            hd, val, addr, ra, ma;
	const unsigned char *mem, *col, *fg, *bg;
	unsigned char       *ptr;

	hd = hgc->crtc.reg[E6845_REG_HD];

	hgc->video.buf_next_w = 16 * hd;

	if ((16 * hd) > hgc->video.buf_w) {
		hd = hgc->video.buf_w / 16;
	}

	ra = (hgc->crtc.ra & 3) << 13;
	ma = hgc->crtc.ma << 1;

	mem = hgc->mem + ((hgc->reg[HGC_MODE] & HGC_MODE_PAGE1) ? 0x8000 : 0);
	ptr = pce_video_get_row_ptr (&hgc->video, row);

	fg = hgc->rgb[4];
	bg = hgc->rgb[0];

	for (i = 0; i < hd; i++) {
		addr = (ma & 0x1fff) | ra;

		val = mem[addr];
		val = (val << 8) | mem[addr + 1];

		for (j = 0; j < 16; j++) {
			col = (val & 0x8000) ? fg : bg;

			*(ptr++) = col[0];
			*(ptr++) = col[1];
			*(ptr++) = col[2];

			val <<= 1;
		}

		ma += 2;
	}
}

static
void hgc_hsync (hgc_t *hgc)
{
	unsigned row, max, ch;

	if (hgc->mod_cnt == 0) {
		return;
	}

	ch = (hgc->crtc.reg[E6845_REG_ML] & 0x1f) + 1;
	row = ch * hgc->crtc.crow + hgc->crtc.ra;
	max = ch * hgc->crtc.reg[E6845_REG_VD];

	if (row >= max) {
		return;
	}

	if (row >= hgc->video.buf_h) {
		return;
	}

	if ((hgc->reg[HGC_MODE] & HGC_MODE_ENABLE) == 0) {
		hgc_line_blank (hgc, row);
	}
	else if (hgc->reg[HGC_MODE] & HGC_MODE_GRAPH) {
		hgc_line_graph (hgc, row);
	}
	else {
		hgc_line_text (hgc, row);
	}
}

static
void hgc_vsync (hgc_t *hgc)
{
	video_t *vid;

	vid = &hgc->video;

	if ((hgc->term != NULL) && (vid->buf_w > 0) && (vid->buf_h > 0)) {
		trm_set_size (hgc->term, vid->buf_w, vid->buf_h);

		if (hgc->mod_cnt > 0) {
			trm_set_lines (hgc->term, vid->buf, 0, vid->buf_h);
		}

		trm_update (hgc->term);
	}

	if (hgc->mod_cnt > 0) {
		hgc->mod_cnt -= 1;
	}

	vid->buf_next_h = e6845_get_vdl (&hgc->crtc);

	if (vid->buf_next_w == 0) {
		vid->buf_next_w = 720;
	}

	if (vid->buf_next_h == 0) {
		vid->buf_next_h = 350;
	}

	if ((vid->buf_w != vid->buf_next_w) || (vid->buf_h != vid->buf_next_h)) {
		pce_video_set_buf_size (vid, vid->buf_next_w, vid->buf_next_h, 3);
		hgc->mod_cnt = 1;
	}

	if (hgc->blink_cnt > 0) {
		hgc->blink_cnt -= 1;

		if (hgc->blink_cnt == 0) {
			hgc->blink = !hgc->blink;
			hgc->blink_cnt = hgc->blink_rate;
			hgc->mod_cnt = 1;
		}
	}
}

static
void hgc_clock (hgc_t *hgc, unsigned long cnt)
{
	unsigned cclk;

	if (hgc->reg[HGC_MODE] & HGC_MODE_GRAPH) {
		hgc->clock += (unsigned long) HGC_CFREQ2 * cnt;
	}
	else {
		hgc->clock += (unsigned long) HGC_CFREQ1 * cnt;
	}

	cclk = hgc->clock / HGC_IFREQ;
	hgc->clock = hgc->clock % HGC_IFREQ;

	if (cclk > 0) {
		e6845_clock (&hgc->crtc, cclk);
	}
}

static
void hgc_set_blink_rate (hgc_t *hgc, unsigned rate, int start)
{
	hgc->blink = (start != 0);
	hgc->blink_cnt = rate;
	hgc->blink_rate = rate;
	hgc->mod_cnt = 2;
}

static
void hgc_set_color_index (hgc_t *hgc, unsigned i, unsigned long col)
{
	if (i < 5) {
		hgc->rgb[i][0] = (col >> 16) & 0xff;
		hgc->rgb[i][1] = (col >> 8) & 0xff;
		hgc->rgb[i][2] = col & 0xff;
		hgc->mod_cnt = 2;
	}
}

static
void hgc_set_color (hgc_t *hgc, unsigned long col)
{
	hgc_set_color_index (hgc, 0, 0);
	hgc_set_color_index (hgc, 1, pce_color_sub (col, 0x555555));
	hgc_set_color_index (hgc, 2, col);
	hgc_set_color_index (hgc, 3, pce_color_add (col, 0x555555));
}

/*
 * Set the mode register
 */
static
void hgc_set_mode (hgc_t *hgc, unsigned char val)
{
	if ((hgc->reg[HGC_CONFIG] & HGC_CONFIG_GRAPH) == 0) {
		val &= ~HGC_MODE_GRAPH;
	}

	if ((hgc->reg[HGC_CONFIG] & HGC_CONFIG_PAGE1) == 0) {
		val &= ~HGC_MODE_PAGE1;
	}

	if (hgc->reg[HGC_MODE] != val) {
		hgc->reg[HGC_MODE] = val;
		hgc->mod_cnt = 2;
	}
}

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
	unsigned char val;

	pce_video_clock1 (&hgc->video, 0);

	val = hgc->reg[HGC_STATUS];
	val &= ~(HGC_STATUS_VSYNC | HGC_STATUS_HSYNC | HGC_STATUS_VIDEO);

	if (hgc->crtc.vsync_cnt == 0) {
		val |= HGC_STATUS_VSYNC;
	}

	if (hgc->crtc.hsync_cnt > 0) {
		val |= HGC_STATUS_HSYNC;
	}

	if (e6845_get_de (&hgc->crtc)) {
		if (hgc->lfsr & 1) {
			val |= HGC_STATUS_VIDEO;
			hgc->lfsr = (hgc->lfsr >> 1) ^ 0x8016;
		}
		else {
			hgc->lfsr = hgc->lfsr >> 1;
		}
	}

	hgc->reg[HGC_STATUS] = val;

	return (val);
}

static
void hgc_set_pen_reset (hgc_t *hgc, unsigned char val)
{
	hgc->reg[HGC_STATUS] &= ~HGC_STATUS_PEN;
}

static
void hgc_set_pen_set (hgc_t *hgc, unsigned char val)
{
	pce_video_clock1 (&hgc->video, 0);
	hgc->reg[HGC_STATUS] |= HGC_STATUS_PEN;
	e6845_set_pen (&hgc->crtc);
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
 * Set the configuration register
 */
static
void hgc_set_config (hgc_t *hgc, unsigned char val)
{
	unsigned long size;

	if (hgc->reg[HGC_CONFIG] == val) {
		return;
	}

	hgc->reg[HGC_CONFIG] = val;

	if ((val & HGC_CONFIG_PAGE1) == 0) {
		size = 32768;
	}
	else {
		size = 65536;
	}

	mem_blk_set_size (hgc->memblk, size);

	hgc->mod_cnt = 2;
}

/*
 * Get an HGC register
 */
static
unsigned char hgc_reg_get_uint8 (hgc_t *hgc, unsigned long addr)
{
	switch (addr) {
	case HGC_CRTC_INDEX:
		return (e6845_get_index (&hgc->crtc));

	case HGC_CRTC_DATA:
		return (e6845_get_data (&hgc->crtc));

	case HGC_MODE:
		return (hgc_get_mode (hgc));

	case HGC_STATUS:
		return (hgc_get_status (hgc));

	case HGC_CONFIG:
		return (hgc_get_config (hgc));

	default:
		fprintf (stderr, "HGC: get reg (%04lX)\n", addr);
		return (0xff);
	}
}

static
unsigned short hgc_reg_get_uint16 (hgc_t *hgc, unsigned long addr)
{
	return (0xffff);
}

/*
 * Set an HGC register
 */
static
void hgc_reg_set_uint8 (hgc_t *hgc, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case HGC_CRTC_INDEX:
		e6845_set_index (&hgc->crtc, val);
		break;

	case HGC_CRTC_DATA:
		e6845_set_data (&hgc->crtc, val);
		hgc->mod_cnt = 2;
		break;

	case HGC_MODE:
		hgc_set_mode (hgc, val);
		break;

	case HGC_PEN_SET:
		hgc_set_pen_set (hgc, val);
		break;

	case HGC_PEN_RESET:
		hgc_set_pen_reset (hgc, val);
		break;

	case HGC_CONFIG:
		hgc_set_config (hgc, val);
		break;

	default:
		fprintf (stderr, "HGC: set reg (%04lX, %02X)\n", addr, val);
		break;
	}
}

static
void hgc_reg_set_uint16 (hgc_t *hgc, unsigned long addr, unsigned short val)
{
	hgc_reg_set_uint8 (hgc, addr, val & 0xff);

	if (addr < 15) {
		hgc_reg_set_uint8 (hgc, addr + 1, (val >> 8) & 0xff);
	}
}

static
unsigned char hgc_mem_get_uint8 (hgc_t *hgc, unsigned long addr)
{
	return (hgc->mem[addr & 0xffff]);
}

static
unsigned short hgc_mem_get_uint16 (hgc_t *hgc, unsigned long addr)
{
	unsigned short val;

	val = hgc->mem[(addr + 1) & 0xffff];
	val = (val << 8) | hgc->mem[addr & 0xffff];

	return (val);
}

static
void hgc_mem_set_uint8 (hgc_t *hgc, unsigned long addr, unsigned char val)
{
	hgc->mem[addr & 0xffff] = val;
	hgc->mod_cnt = 2;
}

static
void hgc_mem_set_uint16 (hgc_t *hgc, unsigned long addr, unsigned short val)
{
	hgc->mem[(addr + 0) & 0xffff] = val & 0xff;
	hgc->mem[(addr + 1) & 0xffff] = (val >> 8) & 0xff;
	hgc->mod_cnt = 2;
}

static
int hgc_set_msg (hgc_t *hgc, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.color", msg)) {
		unsigned long v;

		if (pce_color_get (val, &v)) {
			return (1);
		}

		hgc_set_color (hgc, v);

		return (0);
	}

	return (-1);
}

static
void hgc_print_info (hgc_t *hgc, FILE *fp)
{
	unsigned      mul;
	unsigned      col, row, w1, w2;
	unsigned      ch, vt, ht, vtl, vdl;
	unsigned      base, addr;
	unsigned long clk1, clk2;
	unsigned char status;
	unsigned char *reg;
	e6845_t       *crt;

	crt = &hgc->crtc;
	reg = hgc->reg;

	status = hgc_get_status (hgc);

	base = e6845_get_start_address (crt);
	addr = crt->ma + crt->ccol;

	mul = (hgc->reg[HGC_MODE] & HGC_MODE_GRAPH) ? 16 : 9;

	ch = (e6845_get_ml (crt) & 0x1f) + 1;
	row = ch * crt->crow + crt->ra;
	col = mul * crt->ccol;
	vtl = e6845_get_vtl (crt);
	vdl = e6845_get_vdl (crt);
	ht = e6845_get_ht (crt) + 1;
	vt = e6845_get_vt (crt) + 1;

	clk1 = mul * ht;
	clk2 = clk1 * vtl;
	w1 = mul * e6845_get_hd (crt);
	w2 = mul * ht;

	fprintf (fp,
		"DEV: HGC\n"
		"INDX[3B4]=%02X   COL=%3u/%3u  HFRQ=%9.3f  %u*%u\n"
		"MODE[3B8]=%02X   ROW=%3u/%3u  VFRQ=%9.3f  %u*%u\n"
		"    [3B9]=%02X  CCOL=%3u/%3u   HDE=%d  HSYN=%X  BASE=%04X\n"
		"STAT[3BA]=%02X  CROW=%3u/%3u   VDE=%d  VSYN=%X  ADDR=%04X\n"
		"CONF[3BF]=%02X    RA=%3u/%3u\n",
		e6845_get_index (crt), col, w2, (double) HGC_PFREQ / clk1, w1, vdl,
		reg[HGC_MODE], row, vtl, (double) HGC_PFREQ / clk2, w2, vtl,
		reg[9], crt->ccol, ht, e6845_get_hde (crt), crt->hsync_cnt, base,
		status, crt->crow, vt, e6845_get_vde (crt), crt->vsync_cnt, addr,
		reg[HGC_CONFIG], crt->ra, ch
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

hgc_t *hgc_new (unsigned long io, unsigned long mem)
{
	hgc_t *hgc;

	if ((hgc = malloc (sizeof (hgc_t))) == NULL) {
		return (NULL);
	}

	pce_video_init (&hgc->video);

	hgc->video.ext = hgc;
	hgc->video.del = (void *) hgc_del;
	hgc->video.set_msg = (void *) hgc_set_msg;
	hgc->video.set_terminal = (void *) hgc_set_terminal;
	hgc->video.get_mem = (void *) hgc_get_mem;
	hgc->video.get_reg = (void *) hgc_get_reg;
	hgc->video.set_blink_rate = (void *) hgc_set_blink_rate;
	hgc->video.print_info = (void *) hgc_print_info;
	hgc->video.clock = (void *) hgc_clock;

	hgc->memblk = mem_blk_new (mem, 65536, 1);
	mem_blk_set_fget (hgc->memblk, hgc, hgc_mem_get_uint8, hgc_mem_get_uint16, NULL);
	mem_blk_set_fset (hgc->memblk, hgc, hgc_mem_set_uint8, hgc_mem_set_uint16, NULL);
	hgc->mem = hgc->memblk->data;
	mem_blk_clear (hgc->memblk, 0x00);
	mem_blk_set_size (hgc->memblk, 32768);

	hgc->regblk = mem_blk_new (io, 16, 1);
	mem_blk_set_fget (hgc->regblk, hgc, hgc_reg_get_uint8, hgc_reg_get_uint16, NULL);
	mem_blk_set_fset (hgc->regblk, hgc, hgc_reg_set_uint8, hgc_reg_set_uint16, NULL);
	hgc->reg = hgc->regblk->data;
	mem_blk_clear (hgc->regblk, 0x00);

	e6845_init (&hgc->crtc);
	e6845_set_hsync_fct (&hgc->crtc, hgc, hgc_hsync);
	e6845_set_vsync_fct (&hgc->crtc, hgc, hgc_vsync);

	hgc->term = NULL;
	hgc->font = mda_font_8x14;
	hgc->clock = 0;
	hgc->mod_cnt = 0;
	hgc->lfsr = 1;

	hgc->blink = 0;
	hgc->blink_cnt = 0;
	hgc->blink_rate = 16;

	hgc_set_color (hgc, 0xaaaaaa);

	return (hgc);
}

static
void hgc_del (hgc_t *hgc)
{
	if (hgc != NULL) {
		e6845_free (&hgc->crtc);

		mem_blk_del (hgc->memblk);
		mem_blk_del (hgc->regblk);

		free (hgc);
	}
}

video_t *hgc_new_ini (ini_sct_t *sct)
{
	unsigned      i;
	unsigned long io, addr;
	unsigned long col[5];
	unsigned      blink;
	const char    *colname;
	hgc_t         *hgc;

	ini_get_uint32 (sct, "io", &io, 0x3b0);
	ini_get_uint32 (sct, "address", &addr, 0xb0000);
	ini_get_uint16 (sct, "blink", &blink, 16);
	ini_get_string (sct, "color", &colname, "amber");

	pce_log_tag (MSG_INF,
		"VIDEO:", "HGC io=0x%04lx addr=0x%05lx blink=%u\n",
		io, addr, blink
	);

	if (pce_color_get (colname, col + 2)) {
		pce_log (MSG_ERR, "*** unknown color (%s)\n", colname);
		col[2] = 0xaaaaaa;
	}

	col[0] = 0;
	col[1] = pce_color_sub (col[2], 0x555555);
	col[3] = pce_color_add (col[2], 0x555555);
	col[4] = col[2];

	ini_get_uint32 (sct, "color_background", &col[0], col[0]);
	ini_get_uint32 (sct, "color_dim", &col[1], col[1]);
	ini_get_uint32 (sct, "color_normal", &col[2], col[2]);
	ini_get_uint32 (sct, "color_bright", &col[3], col[3]);
	ini_get_uint32 (sct, "color_graphics", &col[4], col[4]);

	if ((hgc = hgc_new (io, addr)) == NULL) {
		return (NULL);
	}

	for (i = 0; i < 5; i++) {
		hgc_set_color_index (hgc, i, col[i]);
	}

	hgc_set_blink_rate (hgc, blink, 1);

	return (&hgc->video);
}
