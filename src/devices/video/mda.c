/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/mda.c                                      *
 * Created:     2003-04-13 by Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <lib/log.h>
#include <lib/msg.h>

#include "mda.h"


#define MDA_IFREQ 1193182
#define MDA_PFREQ 16257000
#define MDA_HFREQ 18430
#define MDA_VFREQ 50

/* character width */
#define MDA_CW    9

/* mda registers */
#define MDA_CRTC_INDEX   0
#define MDA_CRTC_DATA    1
#define MDA_MODE         4
#define MDA_STATUS       6

#define MDA_CRTC_HT      0
#define MDA_CRTC_HD      1
#define MDA_CRTC_HS      2
#define MDA_CRTC_SYN     3
#define MDA_CRTC_VT      4
#define MDA_CRTC_VTA     5
#define MDA_CRTC_VD      6
#define MDA_CRTC_VS      7
#define MDA_CRTC_IL      8
#define MDA_CRTC_MS      9
#define MDA_CRTC_CS      10
#define MDA_CRTC_CE      11
#define MDA_CRTC_SH      12
#define MDA_CRTC_SL      13
#define MDA_CRTC_PH      14
#define MDA_CRTC_PL      15
#define MDA_CRTC_LH      16
#define MDA_CRTC_LL      17

/* mode control register */
#define MDA_MODE_ENABLE  0x08
#define MDA_MODE_BLINK   0x20

/* CRTC status register */
#define MDA_STATUS_HSYNC 0x01		/* horizontal sync */
#define MDA_STATUS_VIDEO 0x08		/* video signal */

#define MDA_UPDATE_DIRTY   1
#define MDA_UPDATE_RETRACE 2


#include "mda_font.h"


static void mda_clock (mda_t *mda, unsigned long cnt);


/*
 * Set the blink frequency
 */
static
void mda_set_blink_rate (mda_t *mda, unsigned freq)
{
	mda->blink_on = 1;
	mda->blink_cnt = freq;
	mda->blink_freq = freq;

	mda->update_state |= MDA_UPDATE_DIRTY;
}

static
void mda_set_color (mda_t *mda, unsigned i1, unsigned i2, unsigned r, unsigned g, unsigned b)
{
	unsigned i;

	if ((i1 > 16) || (i2 > 16)) {
		return;
	}

	r &= 0xff;
	g &= 0xff;
	b &= 0xff;

	for (i = i1; i <= i2; i++) {
		mda->rgb[i][0] = r;
		mda->rgb[i][1] = g;
		mda->rgb[i][2] = b;
	}
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
 * Get CRTC start offset
 */
static
unsigned mda_get_start (mda_t *mda)
{
	unsigned val;

	val = mda->reg_crt[0x0c];
	val = (val << 8) | mda->reg_crt[0x0d];

	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned mda_get_cursor (mda_t *mda)
{
	unsigned val;

	val = mda->reg_crt[0x0e];
	val = (val << 8) | mda->reg_crt[0x0f];

	return (val);
}

/*
 * Get the on screen cursor position
 */
static
int mda_get_position (mda_t *mda, unsigned *x, unsigned *y)
{
	unsigned pos, ofs;

	if ((mda->w == 0) || (mda->h == 0)) {
		return (1);
	}

	pos = mda_get_cursor (mda) & 0x0fff;
	ofs = mda_get_start (mda) & 0x0fff;

	if ((pos < ofs) || (pos >= (ofs + mda->w * mda->h))) {
		return (1);
	}

	*x = (pos - ofs) % mda->w;
	*y = (pos - ofs) / mda->w;

	return (0);
}

/*
 * Set the timing values from the CRTC registers
 */
static
void mda_set_timing (mda_t *mda)
{
	mda->ch = (mda->reg_crt[MDA_CRTC_MS] & 0x1f) + 1;
	mda->w = mda->reg_crt[MDA_CRTC_HD];
	mda->h = mda->reg_crt[MDA_CRTC_VD];

	mda->clk_ht = MDA_CW * (mda->reg_crt[MDA_CRTC_HT] + 1);
	mda->clk_hd = MDA_CW * mda->reg_crt[MDA_CRTC_HD];

	mda->clk_vt = mda->ch * (mda->reg_crt[MDA_CRTC_VT] + 1) * mda->clk_ht;
	mda->clk_vd = mda->ch * mda->reg_crt[MDA_CRTC_VD] * mda->clk_ht;
}

/*
 * Get the dot clock
 */
static
unsigned long mda_get_dotclock (mda_t *mda)
{
	unsigned long long clk;

	clk = mda->video.dotclk[0];
	clk = (MDA_PFREQ * clk) / MDA_IFREQ;

	return (clk);
}

/*
 * Set the internal screen buffer size
 */
static
int mda_set_buf_size (mda_t *mda, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > mda->bufmax) {
		tmp = realloc (mda->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		mda->buf = tmp;
		mda->bufmax = cnt;
	}

	mda->buf_w = w;
	mda->buf_h = h;

	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void mda_draw_cursor (mda_t *mda)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
	const unsigned char *src;
	const unsigned char *col;
	unsigned char       *dst;

	if (mda->blink_on == 0) {
		return;
	}

	if ((mda->reg_crt[MDA_CRTC_CS] & 0x60) == 0x20) {
		/* cursor off */
		return;
	}

	src = mda->mem;

	if (mda_get_position (mda, &x, &y)) {
		return;
	}

	c1 = mda->reg_crt[MDA_CRTC_CS] & 0x1f;
	c2 = mda->reg_crt[MDA_CRTC_CE] & 0x1f;

	if (c1 >= mda->ch) {
		return;
	}

	if (c2 >= mda->ch) {
		c2 = mda->ch - 1;
	}

	col = mda->rgb[src[2 * (mda->w * y + x) + 1] & 0x0f];
	dst = mda->buf + 3 * MDA_CW * (mda->w * (mda->ch * y + c1) + x);

	for (j = c1; j <= c2; j++) {
		for (i = 0; i < MDA_CW; i++) {
			dst[3 * i + 0] = col[0];
			dst[3 * i + 1] = col[1];
			dst[3 * i + 2] = col[2];
		}
		dst += 3 * MDA_CW * mda->w;
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void mda_draw_char (mda_t *mda, unsigned char *buf, unsigned char c, unsigned char a)
{
	unsigned            i, j;
	int                 elg, blk;
	unsigned            ull;
	unsigned            val;
	unsigned char       *dst;
	const unsigned char *fnt;
	const unsigned char *fg, *bg;

	blk = 0;

	if (mda->reg[MDA_MODE] & MDA_MODE_BLINK) {
		if (a & 0x80) {
			blk = !mda->blink_on;
		}

		a &= 0x7f;
	}

	ull = ((a & 0x07) == 1) ? 13 : 0xffff;
	elg = ((c >= 0xc0) && (c <= 0xdf));

	fg = mda->rgb[a & 0x0f];
	bg = mda->rgb[(a >> 4) & 0x0f];

	fnt = mda->font + 14 * c;

	dst = buf;

	for (j = 0; j < mda->ch; j++) {
		if (blk) {
			val = 0x000;
		}
		else if (j == ull) {
			val = 0x1ff;
		}
		else {
			val = fnt[j % 14] << 1;

			if (elg) {
				val |= (val >> 1) & 1;
			}
		}

		for (i = 0; i < MDA_CW; i++) {
			if (val & 0x100) {
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

		dst += (3 * MDA_CW * mda->w);
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void mda_update_blank (mda_t *mda)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	mda_set_buf_size (mda, 720, 350);

	dst = mda->buf;

	for (y = 0; y < mda->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < mda->buf_w; x++) {
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
void mda_update (mda_t *mda)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;

	if ((mda->reg[MDA_MODE] & MDA_MODE_ENABLE) == 0) {
		mda_update_blank (mda);
		return;
	}

	if ((mda->w == 0) || (mda->h == 0)) {
		mda_update_blank (mda);
		return;
	}

	if (mda_set_buf_size (mda, MDA_CW * mda->w, mda->ch * mda->h)) {
		return;
	}

	src = mda->mem;
	ofs = (2 * mda_get_start (mda)) & 0x0ffe;

	dst = mda->buf;

	for (y = 0; y < mda->h; y++) {
		for (x = 0; x < mda->w; x++) {
			mda_draw_char (mda, dst + 3 * MDA_CW * x, src[ofs], src[ofs + 1]);

			ofs = (ofs + 2) & 0x0ffe;
		}

		dst += 3 * (MDA_CW * mda->w) * mda->ch;
	}

	mda_draw_cursor (mda);
}


/*
 * Get a CRTC register
 */
static
unsigned char mda_crtc_get_reg (mda_t *mda, unsigned reg)
{
	if (reg > 15) {
		return (0xff);
	}

	return (mda->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void mda_crtc_set_reg (mda_t *mda, unsigned reg, unsigned char val)
{
	if (reg > 15) {
		return;
	}

	if (mda->reg_crt[reg] == val) {
		return;
	}

	mda->reg_crt[reg] = val;

	mda_set_timing (mda);

	mda->update_state |= MDA_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char mda_get_crtc_index (mda_t *mda)
{
	return (mda->reg[MDA_CRTC_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char mda_get_crtc_data (mda_t *mda)
{
	return (mda_crtc_get_reg (mda, mda->reg[MDA_CRTC_INDEX]));
}

/*
 * Get the mode control register
 */
static
unsigned char mda_get_mode (mda_t *mda)
{
	return (mda->reg[MDA_MODE]);
}

/*
 * Get the status register
 */
static
unsigned char mda_get_status (mda_t *mda)
{
	unsigned char val, vid;
	unsigned long clk;

	mda_clock (mda, 0);

	clk = mda_get_dotclock (mda);

	/* simulate the video signal */
	mda->reg[MDA_STATUS] ^= MDA_STATUS_VIDEO;

	val = mda->reg[MDA_STATUS] & ~MDA_STATUS_VIDEO;
	vid = mda->reg[MDA_STATUS] & MDA_STATUS_VIDEO;

	val |= MDA_STATUS_HSYNC;

	if (mda->clk_ht > 0) {
		if ((clk % mda->clk_ht) < mda->clk_hd) {
			val &= ~MDA_STATUS_HSYNC;

			if (clk < mda->clk_vd) {
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
void mda_set_crtc_index (mda_t *mda, unsigned char val)
{
	mda->reg[MDA_CRTC_INDEX] = val;
}

/*
 * Set the CRTC data register
 */
static
void mda_set_crtc_data (mda_t *mda, unsigned char val)
{
	mda->reg[MDA_CRTC_DATA] = val;

	mda_crtc_set_reg (mda, mda->reg[MDA_CRTC_INDEX], val);
}

/*
 * Set the mode register
 */
static
void mda_set_mode (mda_t *mda, unsigned char val)
{
	mda->reg[MDA_MODE] = val;

	mda->update_state |= MDA_UPDATE_DIRTY;
}

/*
 * Get an MDA register
 */
static
unsigned char mda_reg_get_uint8 (mda_t *mda, unsigned long addr)
{
	switch (addr) {
	case MDA_CRTC_INDEX:
		return (mda_get_crtc_index (mda));

	case MDA_CRTC_DATA:
		return (mda_get_crtc_data (mda));

	case MDA_STATUS:
		return (mda_get_status (mda));

	default:
		return (0xff);
	}
}

static
unsigned short mda_reg_get_uint16 (mda_t *mda, unsigned long addr)
{
	unsigned short ret;

	ret = mda_reg_get_uint8 (mda, addr);
	ret |= mda_reg_get_uint8 (mda, addr + 1) << 8;

	return (ret);
}

/*
 * Set an MDA register
 */
static
void mda_reg_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case MDA_CRTC_INDEX:
		mda_set_crtc_index (mda, val);
		break;

	case MDA_CRTC_DATA:
		mda_set_crtc_data (mda, val);
		break;

	case MDA_MODE:
		mda_set_mode (mda, val);
		break;
	}
}

static
void mda_reg_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
	mda_reg_set_uint8 (mda, addr + 0, val & 0xff);
	mda_reg_set_uint8 (mda, addr + 1, (val >> 8) & 0xff);
}


static
void mda_mem_set_uint8 (mda_t *mda, unsigned long addr, unsigned char val)
{
	if (mda->mem[addr] == val) {
		return;
	}

	mda->mem[addr] = val;

	mda->update_state |= MDA_UPDATE_DIRTY;
}

static
void mda_mem_set_uint16 (mda_t *mda, unsigned long addr, unsigned short val)
{
	mda_mem_set_uint8 (mda, addr + 0, val & 0xff);

	if ((addr + 1) < mda->memblk->size) {
		mda_mem_set_uint8 (mda, addr + 1, (val >> 8) & 0xff);
	}
}


static
void mda_del (mda_t *mda)
{
	if (mda != NULL) {
		mem_blk_del (mda->memblk);
		mem_blk_del (mda->regblk);
		free (mda);
	}
}

static
int mda_set_msg (mda_t *mda, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}

		mda_set_blink_rate (mda, freq);

		return (0);
	}

	return (-1);
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

static
void mda_print_info (mda_t *mda, FILE *fp)
{
	unsigned i;
	unsigned pos, ofs;

	fprintf (fp, "DEV: MDA\n");

	pos = mda_get_cursor (mda);
	ofs = mda_get_start (mda);

	fprintf (fp, "MDA: OFS=%04X  POS=%04X\n",
		 ofs, pos
	);

	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		mda_get_dotclock (mda),
		mda->clk_ht, mda->clk_hd,
		mda->clk_vt, mda->clk_vd
	);

	fprintf (fp, "REG: CRTC=%02X  MODE=%02X  STATUS=%02X\n",
		mda->reg[MDA_CRTC_INDEX], mda_get_mode (mda),
		mda_get_status (mda)
	);

	fprintf (fp, "CRTC=[%02X", mda->reg_crt[0]);

	for (i = 1; i < 18; i++) {
		if ((i & 7) == 0) {
			fputs ("-", fp);
		}
		else {
			fputs (" ", fp);
		}
		fprintf (fp, "%02X", mda->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}

/*
 * Force a screen update
 */
static
void mda_redraw (mda_t *mda, int now)
{
	if (now) {
		if (mda->term != NULL) {
			mda_update (mda);
			trm_set_size (mda->term, mda->buf_w, mda->buf_h);
			trm_set_lines (mda->term, mda->buf, 0, mda->buf_h);
			trm_update (mda->term);
		}
	}

	mda->update_state |= MDA_UPDATE_DIRTY;
}

static
void mda_clock (mda_t *mda, unsigned long cnt)
{
	unsigned long clk;

	if (mda->clk_vt < 50000) {
		return;
	}

	clk = mda_get_dotclock (mda);

	if (clk < mda->clk_vd) {
		mda->update_state &= ~MDA_UPDATE_RETRACE;
		return;
	}

	if (clk >= mda->clk_vt) {
		mda->video.dotclk[0] = 0;
		mda->video.dotclk[1] = 0;
		mda->video.dotclk[2] = 0;
	}

	if (mda->update_state & MDA_UPDATE_RETRACE) {
		return;
	}

	if (mda->blink_cnt > 0) {
		mda->blink_cnt -= 1;

		if (mda->blink_cnt == 0) {
			mda->blink_cnt = mda->blink_freq;
			mda->blink_on = !mda->blink_on;
			mda->update_state |= MDA_UPDATE_DIRTY;
		}
	}

	if (mda->term != NULL) {
		if (mda->update_state & MDA_UPDATE_DIRTY) {
			mda_update (mda);
			trm_set_size (mda->term, mda->buf_w, mda->buf_h);
			trm_set_lines (mda->term, mda->buf, 0, mda->buf_h);
		}

		trm_update (mda->term);
	}

	mda->update_state = MDA_UPDATE_RETRACE;
}

mda_t *mda_new (unsigned long io, unsigned long mem, unsigned long size)
{
	unsigned i;
	mda_t    *mda;

	mda = malloc (sizeof (mda_t));
	if (mda == NULL) {
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
	mda->video.redraw = (void *) mda_redraw;
	mda->video.clock = (void *) mda_clock;

	if (size < 4096) {
		size = 4096;
	}

	mda->memblk = mem_blk_new (mem, size, 1);
	mda->memblk->ext = mda;
	mda->memblk->set_uint8 = (void *) mda_mem_set_uint8;
	mda->memblk->set_uint16 = (void *) mda_mem_set_uint16;
	mda->mem = mda->memblk->data;
	mem_blk_clear (mda->memblk, 0x00);

	mda->regblk = mem_blk_new (io, 16, 1);
	mda->regblk->ext = mda;
	mda->regblk->set_uint8 = (void *) mda_reg_set_uint8;
	mda->regblk->set_uint16 = (void *) mda_reg_set_uint16;
	mda->regblk->get_uint8 = (void *) mda_reg_get_uint8;
	mda->regblk->get_uint16 = (void *) mda_reg_get_uint16;
	mda->reg = mda->regblk->data;
	mem_blk_clear (mda->regblk, 0x00);

	mda->term = NULL;

	for (i = 0; i < 18; i++) {
		mda->reg_crt[i] = 0;
	}

	mda->font = mda_font_8x14;

	mda_set_color (mda, 0, 0, 0x00, 0x00, 0x00);
	mda_set_color (mda, 1, 7, 0x00, 0xaa, 0x00);
	mda_set_color (mda, 8, 15, 0xaa, 0xff, 0xaa);

	mda->w = 0;
	mda->h = 0;
	mda->ch = 0;

	mda->clk_ht = 0;
	mda->clk_vt = 0;
	mda->clk_hd = 0;
	mda->clk_vd = 0;

	mda->bufmax = 0;
	mda->buf = NULL;

	mda->update_state = 0;

	return (mda);
}

video_t *mda_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned long col0, col1, col2;
	const char    *col;
	unsigned      blink;
	mda_t         *mda;

	ini_get_uint32 (sct, "io", &io, 0x3b4);
	ini_get_uint32 (sct, "address", &addr, 0xb0000);
	ini_get_uint32 (sct, "size", &size, 4096);

	ini_get_uint16 (sct, "blink", &blink, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "MDA io=0x%04lx addr=0x%05lx size=0x%05lx\n",
		io, addr, size
	);

	ini_get_string (sct, "color", &col, "green");

	mda_get_color (col, &col0, &col1, &col2);

	ini_get_uint32 (sct, "color_background", &col0, col0);
	ini_get_uint32 (sct, "color_normal", &col1, col1);
	ini_get_uint32 (sct, "color_bright", &col2, col2);

	mda = mda_new (io, addr, size);
	if (mda == NULL) {
		return (NULL);
	}

	mda_set_color (mda, 0, 0, col0 >> 16, col0 >> 8, col0);
	mda_set_color (mda, 1, 7, col1 >> 16, col1 >> 8, col1);
	mda_set_color (mda, 8, 15, col2 >> 16, col2 >> 8, col2);

	mda_set_blink_rate (mda, blink);

	return (&mda->video);
}
