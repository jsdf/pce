/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/mda.c                                      *
 * Created:     2003-04-13 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <chipset/e6845.h>
#include <devices/memory.h>
#include <devices/video/mda.h>
#include <devices/video/mda_font.h>
#include <drivers/video/terminal.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <libini/libini.h>


#define MDA_IFREQ 1193182
#define MDA_PFREQ 16257000
#define MDA_CFREQ (MDA_PFREQ / 9)

#define MDA_CRTC_INDEX   4
#define MDA_CRTC_DATA    5
#define MDA_MODE         8
#define MDA_STATUS       10

#define MDA_MODE_ENABLE  0x08
#define MDA_MODE_BLINK   0x20

#define MDA_STATUS_HSYNC 0x01
#define MDA_STATUS_VIDEO 0x08


static void mda_del (mda_t *mda);


static
void mda_line_blank (mda_t *mda, unsigned row)
{
	unsigned char *ptr;

	ptr = pce_video_get_row_ptr (&mda->video, row);

	memset (ptr, 0, 3 * mda->video.buf_w);
}

static
void mda_line_text (mda_t *mda, unsigned row)
{
	unsigned            i, j, hd;
	unsigned            val, cmask;
	unsigned            addr, caddr;
	unsigned char       code, attr;
	const unsigned char *fg, *bg, *col;
	unsigned char       *ptr;

	hd = e6845_get_hd (&mda->crtc);

	mda->video.buf_next_w = 9 * hd;

	if ((9 * hd) > mda->video.buf_w) {
		hd = mda->video.buf_w / 9;
	}

	ptr = pce_video_get_row_ptr (&mda->video, row);
	addr = mda->crtc.ma;

	if (mda->blink) {
		caddr = e6845_get_cursor_address (&mda->crtc);
		cmask = e6845_get_cursor_mask (&mda->crtc, 0);
	}
	else {
		caddr = -1;
		cmask = 0;
	}

	for (i = 0; i < hd; i++) {
		code = mda->mem[(2 * addr + 0) & 0x0fff];
		attr = mda->mem[(2 * addr + 1) & 0x0fff];

		val = mda->font[14 * code + (mda->crtc.ra & 0x0f)] << 1;

		if ((code & 0xe0) == 0xc0) {
			val |= (val >> 1) & 1;
		}

		if (((attr & 7) == 1) && (mda->crtc.ra == 13)) {
			val = 0x1ff;
		}

		if ((attr & 0x80) && (mda->reg[MDA_MODE] & MDA_MODE_BLINK)) {
			if (mda->blink == 0) {
				val = 0;
			}
		}

		if (addr == caddr) {
			val |= cmask;
		}

		if ((attr & 0x77) == 0x70) {
			fg = mda->rgb[0];
			bg = mda->rgb[7];

			if ((attr & 0x80) && (~mda->reg[MDA_MODE] & MDA_MODE_BLINK)) {
				bg = mda->rgb[15];
			}
		}
		else {
			fg = mda->rgb[attr & 0x0f];
			bg = mda->rgb[0];
		}

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
void mda_hsync (mda_t *mda)
{
	unsigned row, max, ch;

	if (mda->mod_cnt == 0) {
		return;
	}

	ch = (e6845_get_ml (&mda->crtc) & 0x1f) + 1;
	row = ch * mda->crtc.crow + mda->crtc.ra;
	max = ch * e6845_get_vd (&mda->crtc);

	if (row >= max) {
		return;
	}

	if (row >= mda->video.buf_h) {
		return;
	}

	if ((mda->reg[MDA_MODE] & MDA_MODE_ENABLE) == 0) {
		mda_line_blank (mda, row);
	}
	else {
		mda_line_text (mda, row);
	}
}

static
void mda_vsync (mda_t *mda)
{
	video_t *vid;

	vid = &mda->video;

	if ((mda->term != NULL) && (vid->buf_w > 0) && (vid->buf_h > 0)) {
		trm_set_size (mda->term, vid->buf_w, vid->buf_h);

		if (mda->mod_cnt > 0) {
			trm_set_lines (mda->term, vid->buf, 0, vid->buf_h);
		}

		trm_update (mda->term);
	}

	if (mda->mod_cnt > 0) {
		mda->mod_cnt -= 1;
	}

	vid->buf_next_h = e6845_get_vdl (&mda->crtc);

	if (vid->buf_next_w == 0) {
		vid->buf_next_w = 720;
	}

	if (vid->buf_next_h == 0) {
		vid->buf_next_h = 350;
	}

	if ((vid->buf_w != vid->buf_next_w) || (vid->buf_h != vid->buf_next_h)) {
		pce_video_set_buf_size (vid, vid->buf_next_w, vid->buf_next_h, 3);
		mda->mod_cnt = 1;
	}

	if (mda->blink_cnt > 0) {
		mda->blink_cnt -= 1;

		if (mda->blink_cnt == 0) {
			mda->blink = !mda->blink;
			mda->blink_cnt = mda->blink_rate;
			mda->mod_cnt = 1;
		}
	}
}

static
void mda_clock (mda_t *mda, unsigned long cnt)
{
	unsigned cclk;

	mda->clock += (unsigned long) MDA_CFREQ * cnt;
	cclk = mda->clock / MDA_IFREQ;
	mda->clock = mda->clock % MDA_IFREQ;

	if (cclk > 0) {
		e6845_clock (&mda->crtc, cclk);
	}
}

static
void mda_set_blink_rate (mda_t *mda, unsigned rate)
{
	mda->blink = 1;
	mda->blink_cnt = rate;
	mda->blink_rate = rate;
	mda->mod_cnt = 2;
}

static
void mda_set_color (mda_t *mda, unsigned i1, unsigned i2, unsigned r, unsigned g, unsigned b)
{
	if ((i1 > 15) || (i2 > 15)) {
		return;
	}

	r &= 0xff;
	g &= 0xff;
	b &= 0xff;

	while (i1 <= i2) {
		mda->rgb[i1][0] = r;
		mda->rgb[i1][1] = g;
		mda->rgb[i1][2] = b;
		i1 += 1;
	}

	mda->mod_cnt = 2;
}

/*
 * Map a color name to background/normal/bright RGB values
 */
static
void mda_get_color (const char *name,
	unsigned long *back, unsigned long *normal, unsigned long *bright)
{
	*back = 0x000000;

	if (strcmp (name, "amber") == 0) {
		*normal = 0xe89050;
		*bright = 0xfff0c8;
	}
	else if (strcmp (name, "green") == 0) {
		*normal = 0x55aa55;
		*bright = 0xaaffaa;
	}
	else if (strcmp (name, "gray") == 0) {
		*normal = 0xaaaaaa;
		*bright = 0xffffff;
	}
	else {
		*normal = 0xe89050;
		*bright = 0xfff0c8;
	}
}

/*
 * Set the mode register
 */
static
void mda_set_mode (mda_t *mda, unsigned char val)
{
	if (mda->reg[MDA_MODE] != val) {
		mda->reg[MDA_MODE] = val;
		mda->mod_cnt = 2;
	}
}

/*
 * Get the status register
 */
static
unsigned char mda_get_status (mda_t *mda)
{
	unsigned char val;

	pce_video_clock1 (&mda->video, 0);

	val = mda->reg[MDA_STATUS] | 0xf0;
	val &= ~(MDA_STATUS_HSYNC | MDA_STATUS_VIDEO);

	if (mda->crtc.hsync_cnt > 0) {
		val |= MDA_STATUS_HSYNC;
	}

	if (e6845_get_de (&mda->crtc)) {
		if (mda->lfsr & 1) {
			val |= MDA_STATUS_VIDEO;
			mda->lfsr = (mda->lfsr >> 1) ^ 0x8016;
		}
		else {
			mda->lfsr = mda->lfsr >> 1;
		}
	}

	mda->reg[MDA_STATUS] = val;

	return (val);
}

/*
 * Get an MDA register
 */
static
unsigned char mda_reg_get_uint8 (mda_t *mda, unsigned long addr)
{
	switch (addr) {
	case MDA_CRTC_DATA:
		return (e6845_get_data (&mda->crtc));

	case MDA_STATUS:
		return (mda_get_status (mda));

	default:
		return (0xff);
	}
}

static
unsigned short mda_reg_get_uint16 (mda_t *mda, unsigned long addr)
{
	return (0xffff);
}

/*
 * Set an MDA register
 */
static
void mda_reg_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case MDA_CRTC_INDEX:
		e6845_set_index (&mda->crtc, val);
		break;

	case MDA_CRTC_DATA:
		e6845_set_data (&mda->crtc, val);
		mda->mod_cnt = 2;
		break;

	case MDA_MODE:
		mda_set_mode (mda, val);
		break;
	}
}

static
void mda_reg_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
	mda_reg_set_uint8 (mda, addr, val & 0xff);

	if (addr < 12) {
		mda_reg_set_uint8 (mda, addr + 1, (val >> 8) & 0xff);
	}
}

static
unsigned char mda_mem_get_uint8 (mda_t *mda, unsigned long addr)
{
	return (mda->mem[addr & 0x0fff]);
}

static
unsigned short mda_mem_get_uint16 (mda_t *mda, unsigned long addr)
{
	unsigned short val;

	val = mda->mem[(addr + 1) & 0x0fff];
	val = (val << 8) | mda->mem[addr & 0x0fff];

	return (val);
}

static
void mda_mem_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
	mda->mem[addr & 0x0fff] = val;
	mda->mod_cnt = 2;
}

static
void mda_mem_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
	mda->mem[(addr + 0) & 0x0fff] = val & 0xff;
	mda->mem[(addr + 1) & 0x0fff] = (val >> 8) & 0xff;
	mda->mod_cnt = 2;
}

static
int mda_set_msg (mda_t *mda, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned v;

		if (msg_get_uint (val, &v)) {
			return (1);
		}

		mda_set_blink_rate (mda, v);

		return (0);
	}

	return (-1);
}

static
void mda_print_info (mda_t *mda, FILE *fp)
{
	unsigned      col, row, w1, w2;
	unsigned      ch, vt, ht, vtl, vdl;
	unsigned      base, addr;
	unsigned long clk1, clk2;
	unsigned char status;
	unsigned char *reg;
	e6845_t       *crt;

	crt = &mda->crtc;
	reg = mda->reg;

	status = mda_get_status (mda);

	base = e6845_get_start_address (crt);
	addr = crt->ma + crt->ccol;

	ch = (e6845_get_ml (crt) & 0x1f) + 1;
	row = ch * crt->crow + crt->ra;
	col = 9 * crt->ccol;
	vtl = e6845_get_vtl (crt);
	vdl = e6845_get_vdl (crt);
	ht = e6845_get_ht (crt) + 1;
	vt = e6845_get_vt (crt) + 1;

	clk1 = 9 * ht;
	clk2 = clk1 * vtl;
	w1 = 9 * e6845_get_hd (crt);
	w2 = 9 * ht;

	fprintf (fp,
		"DEV: MDA\n"
		"INDX[3B4]=%02X   COL=%3u/%3u  HFRQ=%9.3f  %u*%u\n"
		"MODE[3B8]=%02X   ROW=%3u/%3u  VFRQ=%9.3f  %u*%u\n"
		"    [3B9]=%02X  CCOL=%3u/%3u   HDE=%d  HSYN=%X  BASE=%04X\n"
		"STAT[3BA]=%02X  CROW=%3u/%3u   VDE=%d  VSYN=%X  ADDR=%04X\n"
		"                RA=%3u/%3u\n",
		e6845_get_index (crt), col, w2, (double) MDA_PFREQ / clk1, w1, vdl,
		reg[MDA_MODE], row, vtl, (double) MDA_PFREQ / clk2, w2, vtl,
		reg[9], crt->ccol, ht, e6845_get_hde (crt), crt->hsync_cnt, base,
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
void mda_set_terminal (mda_t *mda, terminal_t *trm)
{
	mda->term = trm;

	if (mda->term != NULL) {
		trm_open (mda->term, 720, 350);
	}
}

static
mem_blk_t *mda_get_mem (mda_t *mda)
{
	return (mda->memblk);
}

static
mem_blk_t *mda_get_reg (mda_t *mda)
{
	return (mda->regblk);
}

mda_t *mda_new (unsigned long io, unsigned long mem)
{
	mda_t *mda;

	if ((mda = malloc (sizeof (mda_t))) == NULL) {
		return (NULL);
	}

	pce_video_init (&mda->video);

	mda->video.ext = mda;
	mda->video.del = (void *) mda_del;
	mda->video.set_msg = (void *) mda_set_msg;
	mda->video.set_terminal = (void *) mda_set_terminal;
	mda->video.get_mem = (void *) mda_get_mem;
	mda->video.get_reg = (void *) mda_get_reg;
	mda->video.print_info = (void *) mda_print_info;
	mda->video.clock = (void *) mda_clock;

	mda->memblk = mem_blk_new (mem, 32768, 0);
	mem_blk_set_data (mda->memblk, mda->mem, 0);
	mem_blk_set_fget (mda->memblk, mda, mda_mem_get_uint8, mda_mem_get_uint16, NULL);
	mem_blk_set_fset (mda->memblk, mda, mda_mem_set_uint8, mda_mem_set_uint16, NULL);
	memset (mda->mem, 0, sizeof (mda->mem));

	mda->regblk = mem_blk_new (io, 12, 1);
	mem_blk_set_fget (mda->regblk, mda, mda_reg_get_uint8, mda_reg_get_uint16, NULL);
	mem_blk_set_fset (mda->regblk, mda, mda_reg_set_uint8, mda_reg_set_uint16, NULL);
	mda->reg = mda->regblk->data;
	mem_blk_clear (mda->regblk, 0x00);

	e6845_init (&mda->crtc);
	e6845_set_hsync_fct (&mda->crtc, mda, mda_hsync);
	e6845_set_vsync_fct (&mda->crtc, mda, mda_vsync);

	mda->term = NULL;
	mda->font = mda_font_8x14;
	mda->clock = 0;
	mda->mod_cnt = 0;
	mda->lfsr = 1;

	mda->blink = 0;
	mda->blink_cnt = 0;
	mda->blink_rate = 16;

	mda_set_color (mda, 0, 0, 0x00, 0x00, 0x00);
	mda_set_color (mda, 1, 7, 0x00, 0xaa, 0x00);
	mda_set_color (mda, 8, 15, 0xaa, 0xff, 0xaa);

	return (mda);
}

static
void mda_del (mda_t *mda)
{
	if (mda != NULL) {
		e6845_free (&mda->crtc);

		mem_blk_del (mda->memblk);
		mem_blk_del (mda->regblk);

		free (mda);
	}
}

video_t *mda_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned long col0, col1, col2;
	unsigned      blink;
	const char    *col;
	mda_t         *mda;

	ini_get_uint32 (sct, "io", &io, 0x3b0);
	ini_get_uint32 (sct, "address", &addr, 0xb0000);
	ini_get_uint16 (sct, "blink", &blink, 16);
	ini_get_string (sct, "color", &col, "green");

	pce_log_tag (MSG_INF,
		"VIDEO:", "MDA io=0x%04lx addr=0x%05lx blink=%u\n",
		io, addr, blink
	);

	mda_get_color (col, &col0, &col1, &col2);

	ini_get_uint32 (sct, "color_background", &col0, col0);
	ini_get_uint32 (sct, "color_normal", &col1, col1);
	ini_get_uint32 (sct, "color_bright", &col2, col2);

	if ((mda = mda_new (io, addr)) == NULL) {
		return (NULL);
	}

	mda_set_color (mda, 0, 0, col0 >> 16, col0 >> 8, col0);
	mda_set_color (mda, 1, 7, col1 >> 16, col1 >> 8, col1);
	mda_set_color (mda, 8, 15, col2 >> 16, col2 >> 8, col2);

	mda_set_blink_rate (mda, blink);

	return (&mda->video);
}
