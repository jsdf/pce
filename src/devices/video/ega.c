/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/ega.c                                      *
 * Created:     2003-09-06 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/log.h>
#include <lib/msg.h>

#include <devices/video/ega.h>


#define EGA_IFREQ  1193182

#define EGA_PFREQ0 14318180
#define EGA_HFREQ0 15750
#define EGA_VFREQ0 60

#define EGA_PFREQ1 16257000
#define EGA_HFREQ1 21850
#define EGA_VFREQ1 60

#define EGA_PFREQ2 16257000
#define EGA_HFREQ2 18430
#define EGA_VFREQ2 50


#define EGA_MONITOR_ECD 0
#define EGA_MONITOR_CGA 1
#define EGA_MONITOR_MDA 2


#define EGA_ATC_INDEX     0x10		/* attribute controller index/data register */
#define EGA_STATUS0       0x12		/* input status register 0 */
#define EGA_MOUT_W        0x12		/* miscellaneous output register */
#define EGA_SEQ_INDEX     0x14		/* sequencer index register */
#define EGA_SEQ_DATA      0x15		/* sequencer data register */
#define EGA_GENOA         0x18		/* GENOA EGA extension register */
#define EGA_MOUT          0x1c		/* miscellaneous output register */
#define EGA_GRC_INDEX     0x1e		/* graphics controller index register */
#define EGA_GRC_DATA      0x1f		/* graphics controller data register */
#define EGA_CRT_INDEX     0x24		/* crtc index register */
#define EGA_CRT_DATA      0x25		/* crtc data register */
#define EGA_STATUS1       0x2a		/* input status register 1 */


#define EGA_ATC_MODE      16		/* attribute mode control register */
#define EGA_ATC_CPE       18		/* color plane enable register */
#define EGA_ATC_HPP       19            /* horizontal pel panning register */

#define EGA_SEQ_RESET     0		/* reset register */
#define EGA_SEQ_CLOCK     1		/* clocking mode register */
#define EGA_SEQ_MAPMASK   2		/* map mask register */
#define EGA_SEQ_CMAPSEL   3		/* character map select register */
#define EGA_SEQ_MODE      4		/* memory mode register */

#define EGA_GRC_SETRESET  0		/* set/reset register */
#define EGA_GRC_ENABLESR  1		/* enable set/reset register */
#define EGA_GRC_COLCMP    2		/* color compare register */
#define EGA_GRC_ROTATE    3		/* data rotate register */
#define EGA_GRC_READMAP   4		/* read map select register */
#define EGA_GRC_MODE      5		/* graphics mode register */
#define EGA_GRC_MISC      6		/* miscellaneous register */
#define EGA_GRC_CDC       7		/* color don't care register */
#define EGA_GRC_BITMASK   8		/* bitmask register */

#define EGA_CRT_HT        0		/* horizontal total */
#define EGA_CRT_HD        1		/* horizontal display enable end */
#define EGA_CRT_HBS       2		/* start horizontal blanking */
#define EGA_CRT_HBE       3		/* end horizontal blanking */
#define EGA_CRT_HRS       4		/* start horizontal retrace pulse */
#define EGA_CRT_HRE       5		/* end horizontal retrace pulse */
#define EGA_CRT_VT        6		/* vertical total */
#define EGA_CRT_OF        7		/* overflow */
#define EGA_CRT_MS        9		/* maximum scan line */
#define EGA_CRT_CS        10		/* cursor start */
#define EGA_CRT_CE        11            /* cursor end */
#define EGA_CRT_SAH       12		/* start address high */
#define EGA_CRT_SAL       13		/* start address low */
#define EGA_CRT_CLH       14		/* cursor location high */
#define EGA_CRT_CLL       15		/* cursor location low */
#define EGA_CRT_VRS       16		/* vertical retrace start */
#define EGA_CRT_VRE       17		/* vertical retrace end */
#define EGA_CRT_VD        18		/* vertical display enable end */
#define EGA_CRT_OFS       19		/* offset */
#define EGA_CRT_ULL       20		/* underline location */
#define EGA_CRT_MODE      23		/* crt mode control */
#define EGA_CRT_LC        24            /* line compare */

#define EGA_STATUS0_SS    0x10		/* switch sense */
#define EGA_STATUS0_CI    0x80		/* crt interrupt */

#define EGA_MOUT_IOS      0x01		/* I/O address select */
#define EGA_MOUT_ERAM     0x02		/* enable ram */
#define EGA_MOUT_CS       0x04		/* clock select */
#define EGA_MOUT_HSP      0x40		/* horizontal sync polarity */
#define EGA_MOUT_VSP      0x80		/* vertical sync polarity */

#define EGA_STATUS1_DE    0x01		/* display enable */
#define EGA_STATUS1_VR    0x08

#define EGA_ATC_MODE_G    0x01		/* graphics / alpha-numeric mode */
#define EGA_ATC_MODE_ME   0x02		/* mono emulation */
#define EGA_ATC_MODE_ELG  0x04		/* enable line graphics */
#define EGA_ATC_MODE_EB   0x08		/* enable blinking */

#define EGA_SEQ_RESET_ASR 0x01		/* asynchronous reset */
#define EGA_SEQ_RESET_SR  0x02		/* synchronous reset */

#define EGA_SEQ_CLOCK_D89 0x01		/* 8/9 dot clocks */
#define EGA_SEQ_CLOCK_SL  0x04		/* shift load */
#define EGA_SEQ_CLOCK_DC  0x08		/* dot clock */
#define EGA_SEQ_CLOCK_SH4 0x10		/* shift 4 */
#define EGA_SEQ_CLOCK_SO  0x20		/* screen off */

#define EGA_SEQ_MODE_EM   0x02		/* extended memory */
#define EGA_SEQ_MODE_OE   0x04		/* odd/even */
#define EGA_SEQ_MODE_CH4  0x08		/* chain 4 */

#define EGA_GRC_MODE_WM   0x03		/* write mode */
#define EGA_GRC_MODE_RM   0x08		/* read mode */
#define EGA_GRC_MODE_OE   0x10		/* odd/even mode */
#define EGA_GRC_MODE_SR   0x20

#define EGA_GRC_MISC_GM   0x01		/* graphics mode */
#define EGA_GRC_MISC_OE   0x02		/* odd/even mode */
#define EGA_GRC_MISC_MM   0x0c		/* memory map */

#define EGA_CRT_VRE_CVI   0x10		/* clear vertical interrupt */
#define EGA_CRT_VRE_EVI   0x20		/* enable vertical interrupt */

#define EGA_CRT_MODE_CMS0 0x01
#define EGA_CRT_MODE_WB   0x40		/* byte/word mode */

#define EGA_UPDATE_DIRTY   1
#define EGA_UPDATE_RETRACE 2


static void ega_clock (ega_t *ega, unsigned long cnt);


static
void ega_set_irq (ega_t *ega, unsigned char val)
{
	val = (val != 0);

	if (ega->set_irq_val == val) {
		return;
	}

	if (val) {
		ega->reg[EGA_STATUS0] |= EGA_STATUS0_CI;
	}
	else {
		ega->reg[EGA_STATUS0] &= ~EGA_STATUS0_CI;
	}

	ega->set_irq_val = val;

	if (ega->set_irq != NULL) {
		ega->set_irq (ega->set_irq_ext, val);
	}
}

/*
 * Set the configuration switches
 */
static
void ega_set_switches (ega_t *ega, unsigned val)
{
	static unsigned char mon[16] = {
		EGA_MONITOR_CGA, EGA_MONITOR_CGA,
		EGA_MONITOR_CGA, EGA_MONITOR_ECD,
		EGA_MONITOR_MDA, EGA_MONITOR_MDA,
		EGA_MONITOR_CGA, EGA_MONITOR_CGA,
		EGA_MONITOR_CGA, EGA_MONITOR_ECD,
		EGA_MONITOR_MDA, EGA_MONITOR_MDA,
		EGA_MONITOR_ECD, EGA_MONITOR_ECD,
		EGA_MONITOR_ECD, EGA_MONITOR_ECD
	};

	ega->switches = val;

	ega->monitor = mon[val & 0x0f];
}

/*
 * Set the blink frequency
 */
static
void ega_set_blink_rate (ega_t *ega, unsigned freq)
{
	ega->blink_on = 1;
	ega->blink_cnt = freq;
	ega->blink_freq = freq;

	ega->update_state |= EGA_UPDATE_DIRTY;
}

/*
 * Get the displayed width in pixels
 */
static
unsigned ega_get_w (ega_t *ega)
{
	unsigned val;

	val = ega->reg_crt[EGA_CRT_HD] + 1;

	val *= (ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_D89) ? 8 : 9;

	return (val);
}

/*
 * Get the displayed height in pixels
 */
static
unsigned ega_get_h (ega_t *ega)
{
	unsigned h;

	h = ega->reg_crt[EGA_CRT_VD];
	h |= (ega->reg_crt[EGA_CRT_OF] << 7) & 0x100;
	h += 1;

	return (h);
}

/*
 * Get the character width in pixels
 */
static
unsigned ega_get_cw (ega_t *ega)
{
	if ((ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_D89) == 0) {
		return (9);
	}

	return (8);
}

/*
 * Get the character height in pixels
 */
static
unsigned ega_get_ch (ega_t *ega)
{
	return ((ega->reg_crt[EGA_CRT_MS] & 0x1f) + 1);
}

/*
 * Get CRT start offset
 */
static
unsigned ega_get_start (ega_t *ega)
{
	unsigned val;

	val = ega->reg_crt[EGA_CRT_SAH];
	val = (val << 8) | ega->reg_crt[EGA_CRT_SAL];

	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned ega_get_cursor (ega_t *ega)
{
	unsigned val;

	val = ega->reg_crt[EGA_CRT_CLH];
	val = (val << 8) | ega->reg_crt[EGA_CRT_CLL];

	return (val);
}

/*
 * Get the line compare register
 */
static
unsigned ega_get_line_compare (const ega_t *ega)
{
	unsigned val;

	val = ega->reg_crt[EGA_CRT_LC];

	if (ega->reg_crt[EGA_CRT_OF] & 0x10) {
		val += 256;
	}

	return (val);
}

/*
 * Get a palette entry
 */
static
void ega_get_palette (ega_t *ega, unsigned idx,
	unsigned char *r, unsigned char *g, unsigned char *b)
{
	unsigned v;
	unsigned mon;

	if ((ega->reg[EGA_MOUT] & EGA_MOUT_VSP) == 0) {
		mon = EGA_MONITOR_CGA;
	}
	else {
		mon = ega->monitor;
	}

	idx &= ega->reg_atc[EGA_ATC_CPE];

	v = ega->reg_atc[idx & 0x0f];

	if (mon == EGA_MONITOR_MDA) {
		*r = 0;
		*g = 0;
		*b = 0;

		if (v & 0x08) {
			*r += 0xe8;
			*g += 0x90;
			*b += 0x50;
		}

		if (v & 0x10) {
			*r += 0x17;
			*g += 0x60;
			*b += 0x78;
		}
	}
	else if (mon == EGA_MONITOR_CGA) {
		*r = (v & 0x04) ? 0xaa : 0x00;
		*r += (v & 0x10) ? 0x55 : 0x00;

		*g = (v & 0x02) ? 0xaa : 0x00;
		*g += (v & 0x10) ? 0x55 : 0x00;

		*b = (v & 0x01) ? 0xaa : 0x00;
		*b += (v & 0x10) ? 0x55 : 0x00;

		if (v == 0x06) {
			*g = 0x55;
		}
	}
	else {
		*b = ((v << 1) & 0x02) | ((v >> 3) & 0x01);
		*b |= *b << 2;
		*b |= *b << 4;

		*g = ((v << 0) & 0x02) | ((v >> 4) & 0x01);
		*g |= *g << 2;
		*g |= *g << 4;

		*r = ((v >> 1) & 0x02) | ((v >> 5) & 0x01);
		*r |= *r << 2;
		*r |= *r << 4;
	}
}

/*
 * Get a transformed CRTC address
 */
static
unsigned ega_get_crtc_addr (ega_t *ega, unsigned addr, unsigned row)
{
	if ((ega->reg_crt[EGA_CRT_MODE] & EGA_CRT_MODE_WB) == 0) {
		/* word mode */
		addr = ((addr << 1) | (addr >> 15)) & 0xffff;
	}

	if ((ega->reg_crt[EGA_CRT_MODE] & EGA_CRT_MODE_CMS0) == 0) {
		/* CGA 8K bank simulation */
		addr = (addr & ~0x2000) | ((row & 1) << 13);
	}

	return (addr);
}

/*
 * Set the timing values from the CRT registers
 */
static
void ega_set_timing (ega_t *ega)
{
	int           d;
	unsigned      cw;
	unsigned long v;

	cw = ega_get_cw (ega);

	v = cw * (ega->reg_crt[EGA_CRT_HT] + 2);
	d = (ega->clk_ht != v);
	ega->clk_ht = v;

	v = cw * (ega->reg_crt[EGA_CRT_HD] + 1);
	d |= (ega->clk_hd != v);
	ega->clk_hd = v;

	v = ega->reg_crt[EGA_CRT_VT];
	v |= (ega->reg_crt[EGA_CRT_OF] << 8) & 0x100;
	v = (v + 1) * ega->clk_ht;
	d |= (ega->clk_vt != v);
	ega->clk_vt = v;

	v = ega->reg_crt[EGA_CRT_VD];
	v |= (ega->reg_crt[EGA_CRT_OF] << 7) & 0x100;
	v = (v + 1) * ega->clk_ht;
	d |= (ega->clk_vd != v);
	ega->clk_vd = v;

	if (d) {
		ega->update_state |= EGA_UPDATE_DIRTY;
	}
}

/*
 * Get the dot clock
 */
static
unsigned long ega_get_dotclock (ega_t *ega)
{
	unsigned long long clk;

	clk = ega->video.dotclk[0];

	if (ega->reg[EGA_MOUT] & EGA_MOUT_CS) {
		clk *= EGA_PFREQ1;
	}
	else {
		clk *= EGA_PFREQ0;
	}

	if (ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_DC) {
		clk = clk / (2 * EGA_IFREQ);
	}
	else {
		clk = clk / EGA_IFREQ;
	}

	return (clk);
}

/*
 * Get a pointer to the bitmap for a character
 */
static
const unsigned char *ega_get_font (ega_t *ega, unsigned chr, unsigned atr)
{
	const unsigned char *fnt;
	unsigned            ofs;

	ofs = ega->reg_seq[EGA_SEQ_CMAPSEL];

	if (atr & 0x08) {
		ofs = (ofs >> 2) & 0x03;
	}
	else {
		ofs = ofs & 0x03;
	}

	fnt = ega->mem + 0x20000 + (16384 * ofs) + (32 * (chr & 0xff));

	return (fnt);
}

/*
 * Set the internal screen buffer size
 */
static
int ega_set_buf_size (ega_t *ega, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > ega->bufmax) {
		tmp = realloc (ega->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		ega->buf = tmp;
		ega->bufmax = cnt;
	}

	ega->buf_w = w;
	ega->buf_h = h;

	return (0);
}

/*
 * Draw a character in the internal buffer
 */
static
void ega_mode0_update_char (ega_t *ega, unsigned char *dst, unsigned w,
	unsigned cw, unsigned ch, unsigned c, unsigned a, int crs)
{
	unsigned            x, y;
	unsigned            val;
	unsigned            c1, c2;
	unsigned            ull;
	int                 incrs, elg, blk;
	unsigned char       fg[3], bg[3];
	const unsigned char *fnt;

	blk = 0;

	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_EB) {
		if (a & 0x80) {
			blk = !ega->blink_on;
		}

		a &= 0x7f;
	}

	ega_get_palette (ega, a & 0x0f, fg, fg + 1, fg + 2);
	ega_get_palette (ega, (a >> 4) & 0x0f, bg, bg + 1, bg + 2);

	c1 = ega->reg_crt[EGA_CRT_CS] & 0x1f;
	c2 = ega->reg_crt[EGA_CRT_CE] & 0x1f;
	crs = (crs && ega->blink_on);
	incrs = (c2 < c1);

	elg = 0;

	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_ELG) {
		if ((c >= 0xc0) && (c <= 0xdf)) {
			elg = 1;
		}
	}

	ull = 0xffff;

	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_ME) {
		if ((a & 0x07) == 1) {
			ull = ega->reg_crt[EGA_CRT_ULL] & 0x1f;
		}
	}

	fnt = ega_get_font (ega, c, a);

	for (y = 0; y < ch; y++) {
		if (blk) {
			val = 0;
		}
		else if (y == ull) {
			val = 0xffff;
		}
		else {
			val = fnt[y] << 1;

			if (elg) {
				val |= (val >> 1) & 1;
			}
		}

		if (crs) {
			if (y == c1) {
				incrs = !incrs;
			}

			if (y == c2) {
				incrs = !incrs;
			}

			if (incrs) {
				val = 0xffff;
			}
		}

		for (x = 0; x < cw; x++) {
			if (val & 0x100) {
				dst[3 * x + 0] = fg[0];
				dst[3 * x + 1] = fg[1];
				dst[3 * x + 2] = fg[2];
			}
			else {
				dst[3 * x + 0] = bg[0];
				dst[3 * x + 1] = bg[1];
				dst[3 * x + 2] = bg[2];
			}

			val <<= 1;
		}

		dst += 3 * w;
	}
}

/*
 * Update text mode
 */
static
void ega_update_text (ega_t *ega)
{
	unsigned            x, y, w, h, cw, ch;
	unsigned            w2, h2;
	unsigned            addr, rptr, rofs, p;
	unsigned            cpos;
	const unsigned char *src;
	unsigned char       *dst;

	w = ega_get_w (ega);
	h = ega_get_h (ega);
	cw = ega_get_cw (ega);
	ch = ega_get_ch (ega);

	if (ega_set_buf_size (ega, w, h)) {
		return;
	}

	src = ega->mem;
	dst = ega->buf;

	addr = ega->latch_addr;
	rofs = 2 * ega->reg_crt[EGA_CRT_OFS];
	cpos = ega_get_cursor (ega);

	y = 0;

	while (y < h) {
		h2 = h - y;

		if (h2 > ch) {
			h2 = ch;
		}

		dst = ega->buf + 3UL * y * w;

		rptr = addr;

		x = 0;
		while (x < w) {
			w2 = w - x;

			if (w2 > cw) {
				w2 = cw;
			}

			p = ega_get_crtc_addr (ega, rptr, 0);

			ega_mode0_update_char (ega, dst + 3 * x, w, w2, h2,
				src[p], src[p + 0x10000], rptr == cpos
			);

			rptr = (rptr + 1) & 0xffff;

			x += w2;
		}

		addr = (addr + rofs) & 0xffff;

		y += h2;
	}
}

/*
 * Update graphics mode
 */
static
void ega_update_graphics (ega_t *ega)
{
	unsigned            x, y, w, h;
	unsigned            row, col, cw, ch;
	unsigned            lcmp, hpp;
	unsigned            addr, rptr, rofs, ptr;
	unsigned char       blink1, blink2;
	unsigned            msk, bit;
	unsigned            idx;
	unsigned char       buf[4];
	const unsigned char *src;
	unsigned char       *dst;

	w = ega_get_w (ega);
	h = ega_get_h (ega);
	cw = ega_get_cw (ega);
	ch = ega_get_ch (ega);

	lcmp = ega_get_line_compare (ega);

	if (ega_set_buf_size (ega, w, h)) {
		return;
	}

	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_EB) {
		blink1 = 0xff;
		blink2 = ega->blink_on ? 0xff : 0x00;
	}
	else {
		blink1 = 0x00;
		blink2 = 0x00;
	}

	hpp = ega->latch_hpp;

	src = ega->mem;
	dst = ega->buf;

	addr = ega->latch_addr;
	rofs = 2 * ega->reg_crt[EGA_CRT_OFS];

	row = 0;
	y = 0;

	while (y < h) {
		if (y == lcmp) {
			addr = 0;
		}

		dst = ega->buf + 3UL * y * w;

		rptr = addr;

		ptr = ega_get_crtc_addr (ega, rptr, row);

		buf[0] = src[ptr + 0x00000];
		buf[1] = src[ptr + 0x10000];
		buf[2] = src[ptr + 0x20000];
		buf[3] = (src[ptr + 0x30000] ^ blink1) | blink2;

		msk = 0x80 >> (hpp & 7);
		bit = (2 * hpp) & 6;
		col = hpp;

		rptr = (rptr + 1) & 0xffff;

		x = 0;

		while (x < w) {
			if (col >= cw) {
				ptr = ega_get_crtc_addr (ega, rptr, row);

				buf[0] = src[ptr + 0x00000];
				buf[1] = src[ptr + 0x10000];
				buf[2] = src[ptr + 0x20000];
				buf[3] = (src[ptr + 0x30000] ^ blink1) | blink2;

				msk = 0x80;
				bit = 0;
				col = 0;

				rptr = (rptr + 1) & 0xffff;
			}

			if (ega->reg_grc[EGA_GRC_MODE] & EGA_GRC_MODE_SR) {
				/* CGA 4 color mode */

				idx = (buf[0] >> (6 - bit)) & 0x03;
				bit += 2;

				if (bit > 6) {
					buf[0] = buf[1];
					buf[1] = 0;
					bit = 0;
				}
			}
			else {
				idx = (buf[0] & msk) ? 0x01 : 0x00;
				idx |= (buf[1] & msk) ? 0x02 : 0x00;
				idx |= (buf[2] & msk) ? 0x04 : 0x00;
				idx |= (buf[3] & msk) ? 0x08 : 0x00;
				msk >>= 1;
			}

			ega_get_palette (ega, idx, dst, dst + 1, dst + 2);

			dst += 3;
			col += 1;
			x += 1;
		}

		row += 1;

		if (row >= ch) {
			row = 0;
			addr = (addr + rofs) & 0xffff;
		}

		y += 1;
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void ega_update_blank (ega_t *ega)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	ega_set_buf_size (ega, 320, 200);

	dst = ega->buf;

	for (y = 0; y < 200; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < 320; x++) {
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
void ega_update (ega_t *ega)
{
	int      show;
	unsigned w, h;

	ega->buf_w = 0;
	ega->buf_h = 0;

	w = ega_get_w (ega);
	h = ega_get_h (ega);

	show = 1;

	if (ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_SO) {
		show = 0;
	}
	else if ((ega->reg_seq[EGA_SEQ_RESET] & EGA_SEQ_RESET_SR) == 0) {
		show = 0;
	}
	else if ((ega->reg_seq[EGA_SEQ_RESET] & EGA_SEQ_RESET_ASR) == 0) {
		show = 0;
	}
	else if ((w < 16) || (h < 16)) {
		show = 0;
	}

	if (show == 0) {
		ega_update_blank (ega);
		return;
	}

	if (ega->reg_grc[EGA_GRC_MISC] & EGA_GRC_MISC_GM) {
		ega_update_graphics (ega);
	}
	else {
		ega_update_text (ega);
	}
}


static
unsigned char ega_mem_get_uint8 (ega_t *ega, unsigned long addr)
{
	unsigned rdmode;
	unsigned a0;

	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) {
		return (0xff);
	}

	switch ((ega->reg_grc[EGA_GRC_MISC] >> 2) & 3) {
	case 0: /* 128K at A000 */
		break;

	case 1: /* 64K at A000 */
		if (addr > 0xffff) {
			return (0xff);
		}
		break;

	case 2: /* 32K at B000 */
		if ((addr < 0x10000) || (addr > 0x17fff)) {
			return (0xff);
		}
		addr -= 0x10000;
		break;

	case 3: /* 32K at B800 */
		if ((addr < 0x18000) || (addr > 0x1ffff)) {
			return (0xff);
		}
		addr -= 0x18000;
		break;
	}

	addr &= 0xffff;

	a0 = addr & 1;

	if ((ega->reg_seq[EGA_SEQ_MODE] & EGA_SEQ_MODE_OE) == 0) {
		addr &= 0xfffe;
	}

	rdmode = (ega->reg_grc[EGA_GRC_MODE] >> 3) & 1;

	ega->latch[0] = ega->mem[addr + 0x00000];
	ega->latch[1] = ega->mem[addr + 0x10000];
	ega->latch[2] = ega->mem[addr + 0x20000];
	ega->latch[3] = ega->mem[addr + 0x30000];

	if (rdmode == 0) {
		unsigned map;

		map = ega->reg_grc[EGA_GRC_READMAP] & 3;

		if ((ega->reg_seq[EGA_SEQ_MODE] & EGA_SEQ_MODE_OE) == 0) {
			map = (map & 0x02) + a0;
		}

		return (ega->latch[map]);
	}
	else {
		unsigned char msk, val, c1, c2;

		val = 0x00;
		msk = 0x80;

		while (msk != 0) {
			c1 = (ega->latch[0] & msk) ? 0x01 : 0x00;
			c1 |= (ega->latch[1] & msk) ? 0x02 : 0x00;
			c1 |= (ega->latch[2] & msk) ? 0x04 : 0x00;
			c1 |= (ega->latch[3] & msk) ? 0x08 : 0x00;
			c1 &= ega->reg_grc[EGA_GRC_CDC];

			c2 = ega->reg_grc[EGA_GRC_COLCMP];
			c2 &= ega->reg_grc[EGA_GRC_CDC];

			if (c1 == c2) {
				val |= msk;
			}

			msk >>= 1;
		}

		return (val);
	}

	return (0);
}

static
unsigned short ega_mem_get_uint16 (ega_t *ega, unsigned long addr)
{
	unsigned short val;

	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) {
		return (0);
	}

	val = ega_mem_get_uint8 (ega, addr);
	val |= ega_mem_get_uint8 (ega, addr + 1) << 8;

	return (val);
}

static
void ega_mem_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
	unsigned      wrmode;
	unsigned      rot;
	unsigned char mapmsk, bitmsk;
	unsigned char esr, set;
	unsigned char col[4];

	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) {
		return;
	}

	switch ((ega->reg_grc[EGA_GRC_MISC] >> 2) & 3) {
	case 0: /* 128K at A000 */
		break;

	case 1: /* 64K at A000 */
		if (addr > 0xffff) {
			return;
		}
		break;

	case 2: /* 32K at B000 */
		if ((addr < 0x10000) || (addr > 0x17fff)) {
			return;
		}
		addr -= 0x10000;
		break;

	case 3: /* 32K at B800 */
		if ((addr < 0x18000) || (addr > 0x1ffff)) {
			return;
		}
		addr -= 0x18000;
		break;
	}

	addr &= 0xffff;

	wrmode = ega->reg_grc[EGA_GRC_MODE] & 3;
	mapmsk = ega->reg_seq[EGA_SEQ_MAPMASK];

	if ((ega->reg_seq[EGA_SEQ_MODE] & EGA_SEQ_MODE_OE) == 0) {
		mapmsk &= (addr & 1) ? 0x0a : 0x05;
		addr &= 0xfffe;
	}

	switch (wrmode) {
	case 0: /* write mode 0 */
		rot = ega->reg_grc[EGA_GRC_ROTATE] & 7;
		val = ((val >> rot) | (val << (8 - rot))) & 0xff;

		esr = ega->reg_grc[EGA_GRC_ENABLESR];
		set = ega->reg_grc[EGA_GRC_SETRESET];

		col[0] = (esr & 0x01) ? ((set & 0x01) ? 0xff : 0x00) : val;
		col[1] = (esr & 0x02) ? ((set & 0x02) ? 0xff : 0x00) : val;
		col[2] = (esr & 0x04) ? ((set & 0x04) ? 0xff : 0x00) : val;
		col[3] = (esr & 0x08) ? ((set & 0x08) ? 0xff : 0x00) : val;
		break;

	case 1: /* write mode 1 */
		col[0] = ega->latch[0];
		col[1] = ega->latch[1];
		col[2] = ega->latch[2];
		col[3] = ega->latch[3];
		break;

	case 2: /* write mode 2 */
		col[0] = (val & 0x01) ? 0xff : 0x00;
		col[1] = (val & 0x02) ? 0xff : 0x00;
		col[2] = (val & 0x04) ? 0xff : 0x00;
		col[3] = (val & 0x08) ? 0xff : 0x00;
		break;

	default:
		return;
	}

	if (wrmode != 1) {
		switch ((ega->reg_grc[EGA_GRC_ROTATE] >> 3) & 3) {
		case 0: /* copy */
			break;

		case 1: /* and */
			col[0] &= ega->latch[0];
			col[1] &= ega->latch[1];
			col[2] &= ega->latch[2];
			col[3] &= ega->latch[3];
			break;

		case 2: /* or */
			col[0] |= ega->latch[0];
			col[1] |= ega->latch[1];
			col[2] |= ega->latch[2];
			col[3] |= ega->latch[3];
			break;

		case 3: /* xor */
			col[0] ^= ega->latch[0];
			col[1] ^= ega->latch[1];
			col[2] ^= ega->latch[2];
			col[3] ^= ega->latch[3];
			break;
		}
	}

	bitmsk = ega->reg_grc[EGA_GRC_BITMASK];

	col[0] = (col[0] & bitmsk) | (ega->latch[0] & ~bitmsk);
	col[1] = (col[1] & bitmsk) | (ega->latch[1] & ~bitmsk);
	col[2] = (col[2] & bitmsk) | (ega->latch[2] & ~bitmsk);
	col[3] = (col[3] & bitmsk) | (ega->latch[3] & ~bitmsk);

	if (mapmsk & 0x01) {
		ega->mem[addr + 0x00000] = col[0];
	}

	if (mapmsk & 0x02) {
		ega->mem[addr + 0x10000] = col[1];
	}

	if (mapmsk & 0x04) {
		ega->mem[addr + 0x20000] = col[2];
	}

	if (mapmsk & 0x08) {
		ega->mem[addr + 0x30000] = col[3];
	}

	ega->update_state |= EGA_UPDATE_DIRTY;
}

static
void ega_mem_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) {
		return;
	}

	ega_mem_set_uint8 (ega, addr, val & 0xff);
	ega_mem_set_uint8 (ega, addr + 1, (val >> 8) & 0xff);
}


/*
 * Set an attribute controller register
 */
static
void ega_atc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
	if (reg > 21) {
		return;
	}

	if (ega->reg_atc[reg] == val) {
		return;
	}

	ega->reg_atc[reg] = val;

	ega->update_state |= EGA_UPDATE_DIRTY;
}


/*
 * Set a sequencer register
 */
static
void ega_seq_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
	if (reg > 4) {
		return;
	}

	if (ega->reg_seq[reg] == val) {
		return;
	}

	switch (reg) {
	case EGA_SEQ_RESET: /* 0 */
		ega->reg_seq[EGA_SEQ_RESET] = val;
		break;

	case EGA_SEQ_CLOCK: /* 1 */
		ega->reg_seq[EGA_SEQ_CLOCK] = val;
		ega->update_state |= EGA_UPDATE_DIRTY;
		break;

	case EGA_SEQ_MAPMASK: /* 2 */
		ega->reg_seq[EGA_SEQ_MAPMASK] = val;
		break;

	case EGA_SEQ_CMAPSEL: /* 3 */
		ega->reg_seq[EGA_SEQ_CMAPSEL] = val;
		ega->update_state |= EGA_UPDATE_DIRTY;
		break;

	case EGA_SEQ_MODE: /* 4 */
		ega->reg_seq[EGA_SEQ_MODE] = val;
		break;
	}
}


/*
 * Set a graphics controller register
 */
static
void ega_grc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
	if (reg > 8) {
		return;
	}

	if (ega->reg_grc[reg] == val) {
		return;
	}

	ega->reg_grc[reg] = val;

	if (reg == EGA_GRC_MODE) {
		ega->update_state |= EGA_UPDATE_DIRTY;
	}
}


/*
 * Get a CRTC register
 */
static
unsigned char ega_crtc_get_reg (ega_t *ega, unsigned reg)
{
	if ((reg < 12) || (reg > 17)) {
		return (0xff);
	}

	return (ega->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void ega_crtc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
	if (reg > 24) {
		return;
	}

	if (ega->reg_crt[reg] == val) {
		return;
	}

	if (reg == EGA_CRT_VRE) {
		if ((val & EGA_CRT_VRE_CVI) == 0) {
			ega_set_irq (ega, 0);
		}
	}

	ega->reg_crt[reg] = val;

	ega_set_timing (ega);

	ega->update_state |= EGA_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char ega_get_crtc_index (ega_t *ega)
{
	return (ega->reg[EGA_CRT_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char ega_get_crtc_data (ega_t *ega)
{
	return (ega_crtc_get_reg (ega, ega->reg[EGA_CRT_INDEX] & 0x1f));
}

/*
 * Get the input status register 0
 */
static
unsigned char ega_get_input_status_0 (ega_t *ega)
{
	unsigned bit;

	bit = (ega->reg[EGA_MOUT] >> 2) & 3;

	if (ega->switches & (0x08 >> bit)) {
		ega->reg[EGA_STATUS0] |= EGA_STATUS0_SS;
	}
	else {
		ega->reg[EGA_STATUS0] &= ~EGA_STATUS0_SS;
	}

	return (ega->reg[EGA_STATUS0]);
}

/*
 * Get the miscellaneous output register
 */
static
unsigned char ega_get_misc_out (ega_t *ega)
{
	return (ega->reg[EGA_MOUT]);
}

/*
 * Get the input status register 1
 */
static
unsigned char ega_get_input_status_1 (ega_t *ega)
{
	unsigned char val;
	unsigned long clk;

	ega_clock (ega, 0);

	ega->atc_flipflop = 0;

	clk = ega_get_dotclock (ega);

	val = ega->reg[EGA_STATUS1];
	val &= ~(EGA_STATUS1_DE | EGA_STATUS1_VR);

	if (clk >= ega->clk_vd) {
		val |= (EGA_STATUS1_DE | EGA_STATUS1_VR);
	}
	else if ((clk % ega->clk_ht) >= ega->clk_hd) {
		val |= EGA_STATUS1_DE;
	}

	/* don't know what this is, but the bios needs it */
	val = (val & ~0x30) | ((val + 0x10) & 0x30);

	ega->reg[EGA_STATUS1] = val;

	return (val);
}

/*
 * Get an EGA register
 */
static
unsigned char ega_reg_get_uint8 (ega_t *ega, unsigned long addr)
{
	if (addr < 0x10) {
		if (ega->reg[EGA_MOUT] & EGA_MOUT_IOS) {
			return (0xff);
		}

		addr += 0x20;
	}
	else if (addr > 0x20) {
		if ((ega->reg[EGA_MOUT] & EGA_MOUT_IOS) == 0) {
			return (0xff);
		}
	}

	switch (addr) {
	case EGA_STATUS0: /* 3C2 */
		return (ega_get_input_status_0 (ega));

	case EGA_GENOA: /* 3C8 */
		return (0x02);

	case EGA_MOUT: /* 3CC */
		return (ega_get_misc_out (ega));

	case EGA_CRT_INDEX: /* 3D4 */
		return (ega_get_crtc_index (ega));

	case EGA_CRT_DATA: /* 3D5 */
		return (ega_get_crtc_data (ega));

	case EGA_STATUS1: /* 3DA */
		return (ega_get_input_status_1 (ega));
	}

	return (0xff);
}

static
unsigned short ega_reg_get_uint16 (ega_t *ega, unsigned long addr)
{
	unsigned short ret;

	ret = ega_reg_get_uint8 (ega, addr);
	ret |= ega_reg_get_uint8 (ega, addr + 1) << 8;

	return (ret);
}


/*
 * Set the attribute controller index/data register
 */
static
void ega_set_atc_index (ega_t *ega, unsigned char val)
{
	if (ega->atc_flipflop == 0) {
		ega->reg[EGA_ATC_INDEX] = val;
		ega->atc_flipflop = 1;
	}
	else {
		ega_atc_set_reg (ega, ega->reg[EGA_ATC_INDEX] & 0x1f, val);
		ega->atc_flipflop = 0;
	}
}

/*
 * Set the sequencer index register
 */
static
void ega_set_seq_index (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_SEQ_INDEX] = val;
}

/*
 * Set the sequencer data register
 */
static
void ega_set_seq_data (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_SEQ_DATA] = val;

	ega_seq_set_reg (ega, ega->reg[EGA_SEQ_INDEX] & 7, val);
}

/*
 * Set the graphics controller index register
 */
static
void ega_set_grc_index (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_GRC_INDEX] = val;
}

/*
 * Set the graphics controller data register
 */
static
void ega_set_grc_data (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_GRC_DATA] = val;

	ega_grc_set_reg (ega, ega->reg[EGA_GRC_INDEX] & 0x0f, val);
}

/*
 * Set the CRTC index register
 */
static
void ega_set_crtc_index (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_CRT_INDEX] = val;
}

/*
 * Set the CRTC data register
 */
static
void ega_set_crtc_data (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_CRT_DATA] = val;

	ega_crtc_set_reg (ega, ega->reg[EGA_CRT_INDEX] & 0x1f, val);
}

/*
 * Set the miscellaneous output register
 */
static
void ega_set_misc_out (ega_t *ega, unsigned char val)
{
	ega->reg[EGA_MOUT] = val;

	ega->update_state |= EGA_UPDATE_DIRTY;
}

/*
 * Set an EGA register
 */
static
void ega_reg_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
	if (addr < 0x10) {
		if (ega->reg[EGA_MOUT] & EGA_MOUT_IOS) {
			return;
		}

		addr += 0x20;
	}
	else if (addr > 0x20) {
		if ((ega->reg[EGA_MOUT] & EGA_MOUT_IOS) == 0) {
			return;
		}
	}

	switch (addr) {
	case EGA_ATC_INDEX: /* 3C0 */
		ega_set_atc_index (ega, val);
		break;

	case EGA_ATC_INDEX + 1: /* 3C1 */
		ega_set_atc_index (ega, val);
		break;

	case EGA_MOUT_W: /* 3C2 */
		ega_set_misc_out (ega, val);
		break;

	case EGA_SEQ_INDEX: /* 3C4 */
		ega_set_seq_index (ega, val);
		break;

	case EGA_SEQ_DATA: /* 3C5 */
		ega_set_seq_data (ega, val);
		break;

	case EGA_MOUT: /* 3CC */
		break;

	case EGA_GRC_INDEX: /* 3CE */
		ega_set_grc_index (ega, val);

	case EGA_GRC_DATA: /* 3CF */
		ega_set_grc_data (ega, val);
		break;

	case EGA_CRT_INDEX: /* 3D4 */
		ega_set_crtc_index (ega, val);
		break;

	case EGA_CRT_DATA: /* 3D5 */
		ega_set_crtc_data (ega, val);
		break;

	case EGA_STATUS1: /* 3DA */
		break;

	default:
		if (addr < 0x30) {
			ega->reg[addr] = val;
		}
		break;
	}
}

static
void ega_reg_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
	ega_reg_set_uint8 (ega, addr, val & 0xff);
	ega_reg_set_uint8 (ega, addr + 1, val >> 8);
}


static
int ega_set_msg (ega_t *ega, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}

		ega_set_blink_rate (ega, freq);

		return (0);
	}

	return (-1);
}

static
void ega_set_terminal (ega_t *ega, terminal_t *trm)
{
	ega->term = trm;

	if (ega->term != NULL) {
		trm_open (ega->term, 640, 400);
	}
}

static
mem_blk_t *ega_get_mem (ega_t *ega)
{
	return (ega->memblk);
}

static
mem_blk_t *ega_get_reg (ega_t *ega)
{
	return (ega->regblk);
}

static
void ega_print_regs (ega_t *ega, FILE *fp, const char *name, const unsigned char *reg, unsigned cnt)
{
	unsigned i;

	fprintf (fp, "%s=[%02X", name, reg[0]);

	for (i = 1; i < cnt; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", reg[i]);
	}

	fputs ("]\n", fp);
}

static
void ega_print_info (ega_t *ega, FILE *fp)
{
	fprintf (fp, "DEV: EGA\n");

	fprintf (fp, "EGA: ADDR=%04X  ROFS=%04X  CURS=%04X  LCMP=%04X\n",
		ega_get_start (ega),
		2 * ega->reg_crt[EGA_CRT_OFS],
		ega_get_cursor (ega),
		ega_get_line_compare (ega)
	);

	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		ega_get_dotclock (ega),
		ega->clk_ht, ega->clk_hd,
		ega->clk_vt, ega->clk_vd
	);

	fprintf (fp, "MOUT=%02X  ST0=%02X  ST1=%02X\n",
		ega->reg[EGA_MOUT],
		ega->reg[EGA_STATUS0],
		ega->reg[EGA_STATUS1]
	);

	ega_print_regs (ega, fp, "CRT", ega->reg_crt, 25);
	ega_print_regs (ega, fp, "ATC", ega->reg_atc, 22);
	ega_print_regs (ega, fp, "SEQ", ega->reg_seq, 5);
	ega_print_regs (ega, fp, "GRC", ega->reg_grc, 9);

	fflush (fp);
}

/*
 * Force a screen update
 */
static
void ega_redraw (ega_t *ega, int now)
{
	if (now) {
		if (ega->term != NULL) {
			ega_update (ega);
			trm_set_size (ega->term, ega->buf_w, ega->buf_h);
			trm_set_lines (ega->term, ega->buf, 0, ega->buf_h);
			trm_update (ega->term);
		}
	}

	ega->update_state |= EGA_UPDATE_DIRTY;
}

static
void ega_clock (ega_t *ega, unsigned long cnt)
{
	unsigned      addr;
	unsigned long clk;

	if (ega->clk_vt < 50000) {
		return;
	}

	clk = ega_get_dotclock (ega);

	if (clk < ega->clk_vd) {
		ega->update_state &= ~EGA_UPDATE_RETRACE;
		return;
	}

	if (clk >= ega->clk_vt) {
		ega->video.dotclk[0] = 0;
		ega->video.dotclk[1] = 0;
		ega->video.dotclk[2] = 0;

		ega->latch_hpp = ega->reg_atc[EGA_ATC_HPP] & 0x0f;

		addr = ega_get_start (ega);

		if (ega->latch_addr != addr) {
			ega->latch_addr = addr;
			ega->update_state |= EGA_UPDATE_DIRTY;
		}
	}

	if (ega->update_state & EGA_UPDATE_RETRACE) {
		return;
	}

	if (ega->blink_cnt > 0) {
		ega->blink_cnt -= 1;

		if (ega->blink_cnt == 0) {
			ega->blink_cnt = ega->blink_freq;
			ega->blink_on = !ega->blink_on;

			if ((ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_G) == 0) {
				ega->update_state |= EGA_UPDATE_DIRTY;
			}
			else if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_EB) {
				ega->update_state |= EGA_UPDATE_DIRTY;
			}
		}
	}

	if (ega->term != NULL) {
		if (ega->update_state & EGA_UPDATE_DIRTY) {
			ega_update (ega);
			trm_set_size (ega->term, ega->buf_w, ega->buf_h);
			trm_set_lines (ega->term, ega->buf, 0, ega->buf_h);
		}

		trm_update (ega->term);
	}

	ega->update_state = EGA_UPDATE_RETRACE;

	if ((ega->reg_crt[EGA_CRT_VRE] & EGA_CRT_VRE_EVI) == 0) {
		/* vertical retrace interrupt enabled */
		ega_set_irq (ega, 1);
	}
}

void ega_set_irq_fct (ega_t *ega, void *ext, void *fct)
{
	ega->set_irq_ext = ext;
	ega->set_irq = fct;
}

void ega_free (ega_t *ega)
{
	mem_blk_del (ega->memblk);
	mem_blk_del (ega->regblk);
}

void ega_del (ega_t *ega)
{
	if (ega != NULL) {
		ega_free (ega);
		free (ega);
	}
}

void ega_init (ega_t *ega, unsigned long io, unsigned long addr)
{
	unsigned i;

	pce_video_init (&ega->video);

	ega->video.ext = ega;
	ega->video.del = (void *) ega_del;
	ega->video.set_msg = (void *) ega_set_msg;
	ega->video.set_terminal = (void *) ega_set_terminal;
	ega->video.get_mem = (void *) ega_get_mem;
	ega->video.get_reg = (void *) ega_get_reg;
	ega->video.print_info = (void *) ega_print_info;
	ega->video.redraw = (void *) ega_redraw;
	ega->video.clock = (void *) ega_clock;

	ega->term = NULL;

	ega->mem = malloc (256UL * 1024UL);
	if (ega->mem == NULL) {
		return;
	}

	ega->memblk = mem_blk_new (addr, 128UL * 1024UL, 0);
	ega->memblk->ext = ega;
	mem_blk_set_data (ega->memblk, ega->mem, 1);
	ega->memblk->set_uint8 = (void *) ega_mem_set_uint8;
	ega->memblk->set_uint16 = (void *) ega_mem_set_uint16;
	ega->memblk->get_uint8 = (void *) ega_mem_get_uint8;
	ega->memblk->get_uint16 = (void *) ega_mem_get_uint16;
	ega->mem = ega->memblk->data;
	mem_blk_clear (ega->memblk, 0x00);

	ega->regblk = mem_blk_new (io, 0x30, 1);
	ega->regblk->ext = ega;
	ega->regblk->set_uint8 = (void *) ega_reg_set_uint8;
	ega->regblk->set_uint16 = (void *) ega_reg_set_uint16;
	ega->regblk->get_uint8 = (void *) ega_reg_get_uint8;
	ega->regblk->get_uint16 = (void *) ega_reg_get_uint16;
	ega->reg = ega->regblk->data;
	mem_blk_clear (ega->regblk, 0x00);

	for (i = 0; i < 5; i++) {
		ega->reg_seq[i] = 0;
	}

	for (i = 0; i < 9; i++) {
		ega->reg_grc[i] = 0;
	}

	for (i = 0; i < 22; i++) {
		ega->reg_atc[i] = 0;
	}

	for (i = 0; i < 25; i++) {
		ega->reg_crt[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		ega->latch[i] = 0;
	}

	ega->latch_addr = 0;
	ega->latch_hpp = 0;

	ega->atc_flipflop = 0;

	ega->switches = 0x09;
	ega->monitor = EGA_MONITOR_ECD;

	ega->blink_on = 1;
	ega->blink_cnt = 0;
	ega->blink_freq = 16;

	ega->clk_ht = 0;
	ega->clk_hd = 0;
	ega->clk_vt = 0;
	ega->clk_vd = 0;

	ega->bufmax = 0;
	ega->buf = NULL;

	ega->update_state = 0;

	ega->set_irq_ext = NULL;
	ega->set_irq = NULL;
	ega->set_irq_val = 0;
}

ega_t *ega_new (unsigned long io, unsigned long addr)
{
	ega_t *ega;

	ega = malloc (sizeof (ega_t));
	if (ega == NULL) {
		return (NULL);
	}

	ega_init (ega, io, addr);

	return (ega);
}

video_t *ega_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      switches;
	unsigned      blink;
	ega_t         *ega;

	ini_get_uint32 (sct, "io", &io, 0x3b0);
	ini_get_uint32 (sct, "address", &addr, 0xa0000);
	ini_get_uint16 (sct, "switches", &switches, 0x09);
	ini_get_uint16 (sct, "blink", &blink, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "EGA io=0x%04lx addr=0x%05lx switches=%02X\n",
		io, addr, switches
	);

	ega = ega_new (io, addr);
	if (ega == NULL) {
		return (NULL);
	}

	ega_set_switches (ega, switches);
	ega_set_blink_rate (ega, blink);

	return (&ega->video);
}
