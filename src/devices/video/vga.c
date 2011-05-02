/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/vga.c                                      *
 * Created:     2003-09-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include <devices/video/vga.h>


#define VGA_IFREQ  1193182

#define VGA_PFREQ0 25175000
#define VGA_PFREQ1 28322000


#define VGA_ATC_INDEX     0x10		/* attribute controller index/data register */
#define VGA_ATC_DATA      0x11		/* attribute controller data register */
#define VGA_STATUS0       0x12		/* input status register 0 */
#define VGA_MOUT_W        0x12		/* miscellaneous output register */
#define VGA_SEQ_INDEX     0x14		/* sequencer index register */
#define VGA_SEQ_DATA      0x15		/* sequencer data register */
#define VGA_DAC_PEL_MASK  0x16
#define VGA_DAC_ADDR_RD   0x17
#define VGA_DAC_STATE     0x17
#define VGA_DAC_ADDR_WR   0x18
#define VGA_DAC_DATA      0x19
#define VGA_MOUT          0x1c		/* miscellaneous output register */
#define VGA_GRC_INDEX     0x1e		/* graphics controller index register */
#define VGA_GRC_DATA      0x1f		/* graphics controller data register */
#define VGA_CRT_INDEX     0x24		/* crtc index register */
#define VGA_CRT_DATA      0x25		/* crtc data register */
#define VGA_STATUS1       0x2a		/* input status register 1 */


#define VGA_ATC_MODE      16		/* attribute mode control register */
#define VGA_ATC_CPE       18		/* color plane enable register */
#define VGA_ATC_HPP       19            /* horizontal pel panning register */
#define VGA_ATC_CS        20		/* color select register */

#define VGA_SEQ_RESET     0		/* reset register */
#define VGA_SEQ_CLOCK     1		/* clocking mode register */
#define VGA_SEQ_MAPMASK   2		/* map mask register */
#define VGA_SEQ_CMAPSEL   3		/* character map select register */
#define VGA_SEQ_MODE      4		/* memory mode register */

#define VGA_GRC_SETRESET  0		/* set/reset register */
#define VGA_GRC_ENABLESR  1		/* enable set/reset register */
#define VGA_GRC_COLCMP    2		/* color compare register */
#define VGA_GRC_ROTATE    3		/* data rotate register */
#define VGA_GRC_READMAP   4		/* read map select register */
#define VGA_GRC_MODE      5		/* graphics mode register */
#define VGA_GRC_MISC      6		/* miscellaneous register */
#define VGA_GRC_CDC       7		/* color don't care register */
#define VGA_GRC_BITMASK   8		/* bitmask register */

#define VGA_CRT_HT        0		/* horizontal total */
#define VGA_CRT_HD        1		/* horizontal display enable end */
#define VGA_CRT_HBS       2		/* start horizontal blanking */
#define VGA_CRT_HBE       3		/* end horizontal blanking */
#define VGA_CRT_HRS       4		/* start horizontal retrace pulse */
#define VGA_CRT_HRE       5		/* end horizontal retrace pulse */
#define VGA_CRT_VT        6		/* vertical total */
#define VGA_CRT_OF        7		/* overflow */
#define VGA_CRT_MS        9		/* maximum scan line */
#define VGA_CRT_CS        10		/* cursor start */
#define VGA_CRT_CE        11            /* cursor end */
#define VGA_CRT_SAH       12		/* start address high */
#define VGA_CRT_SAL       13		/* start address low */
#define VGA_CRT_CLH       14		/* cursor location high */
#define VGA_CRT_CLL       15		/* cursor location low */
#define VGA_CRT_VRS       16		/* vertical retrace start */
#define VGA_CRT_VRE       17		/* vertical retrace end */
#define VGA_CRT_VD        18		/* vertical display enable end */
#define VGA_CRT_OFS       19		/* offset */
#define VGA_CRT_ULL       20		/* underline location */
#define VGA_CRT_MODE      23		/* crt mode control */
#define VGA_CRT_LC        24            /* line compare */

#define VGA_STATUS0_SS    0x10		/* switch sense */
#define VGA_STATUS0_CI    0x80		/* crt interrupt */

#define VGA_MOUT_IOS      0x01		/* I/O address select */
#define VGA_MOUT_ERAM     0x02		/* enable ram */
#define VGA_MOUT_CS       0x0c		/* clock select */
#define VGA_MOUT_HSP      0x40		/* horizontal sync polarity */
#define VGA_MOUT_VSP      0x80		/* vertical sync polarity */

#define VGA_STATUS1_DE    0x01		/* display enable */
#define VGA_STATUS1_VR    0x08

#define VGA_ATC_MODE_G    0x01		/* graphics / alpha-numeric mode */
#define VGA_ATC_MODE_ME   0x02		/* mono emulation */
#define VGA_ATC_MODE_ELG  0x04		/* enable line graphics */
#define VGA_ATC_MODE_EB   0x08		/* enable blinking */
#define VGA_ATC_MODE_PP   0x20		/* pel panning compatibility */
#define VGA_ATC_MODE_PS   0x80		/* P5, P4 select */

#define VGA_SEQ_RESET_ASR 0x01		/* asynchronous reset */
#define VGA_SEQ_RESET_SR  0x02		/* synchronous reset */

#define VGA_SEQ_CLOCK_D89 0x01		/* 8/9 dot clocks */
#define VGA_SEQ_CLOCK_SL  0x04		/* shift load */
#define VGA_SEQ_CLOCK_DC  0x08		/* dot clock */
#define VGA_SEQ_CLOCK_SH4 0x10		/* shift 4 */
#define VGA_SEQ_CLOCK_SO  0x20		/* screen off */

#define VGA_SEQ_MODE_EM   0x02		/* extended memory */
#define VGA_SEQ_MODE_OE   0x04		/* odd/even */
#define VGA_SEQ_MODE_CH4  0x08		/* chain 4 */

#define VGA_GRC_MODE_WM   0x03		/* write mode */
#define VGA_GRC_MODE_RM   0x08		/* read mode */
#define VGA_GRC_MODE_OE   0x10		/* odd/even mode */
#define VGA_GRC_MODE_SR   0x20
#define VGA_GRC_MODE_C256 0x40		/* 256 color mode */

#define VGA_GRC_MISC_GM   0x01		/* graphics mode */
#define VGA_GRC_MISC_OE   0x02		/* odd/even mode */
#define VGA_GRC_MISC_MM   0x0c		/* memory map */

#define VGA_CRT_OF_VT8    0x01		/* vertical total bit 8 */
#define VGA_CRT_OF_VD8    0x02		/* vertical display enable end bit 8 */
#define VGA_CRT_OF_VRS8   0x04		/* vertical retrace start bit 8 */
#define VGA_CRT_OF_VBS8   0x08		/* vertical blanking start bit 8 */
#define VGA_CRT_OF_LC8    0x10		/* line compare bit 8 */
#define VGA_CRT_OF_VT9    0x20		/* vertical total bit 9 */
#define VGA_CRT_OF_VD9    0x40		/* vertical display enable end bit 9 */
#define VGA_CRT_OF_VRS9   0x80		/* vertical retrace start bit 9 */

#define VGA_CRT_MS_DSC    0x80		/* double scanning */

#define VGA_CRT_VRE_PR    0x80		/* protect register 0-7 */

#define VGA_CRT_VRE_CVI   0x10		/* clear vertical interrupt */
#define VGA_CRT_VRE_EVI   0x20		/* enable vertical interrupt */

#define VGA_CRT_ULL_CB4   0x20		/* count by 4 */
#define VGA_CRT_ULL_DW    0x40		/* double word mode */

#define VGA_CRT_MODE_CMS0 0x01
#define VGA_CRT_MODE_WB   0x40		/* byte/word mode */

#define VGA_UPDATE_DIRTY   1
#define VGA_UPDATE_RETRACE 2


static void vga_clock (vga_t *vga, unsigned long cnt);


static
void vga_set_irq (vga_t *vga, unsigned char val)
{
	val = (val != 0);

	if (vga->set_irq_val == val) {
		return;
	}

	if (val) {
		vga->reg[VGA_STATUS0] |= VGA_STATUS0_CI;
	}
	else {
		vga->reg[VGA_STATUS0] &= ~VGA_STATUS0_CI;
	}

	vga->set_irq_val = val;

	if (vga->set_irq != NULL) {
		vga->set_irq (vga->set_irq_ext, val);
	}
}

/*
 * Set the blink frequency
 */
static
void vga_set_blink_rate (vga_t *vga, unsigned freq)
{
	vga->blink_on = 1;
	vga->blink_cnt = freq;
	vga->blink_freq = freq;
}

/*
 * Get the displayed width in pixels
 */
static
unsigned vga_get_w (vga_t *vga)
{
	unsigned val;

	val = vga->reg_crt[VGA_CRT_HD] + 1;

	val *= (vga->reg_seq[VGA_SEQ_CLOCK] & VGA_SEQ_CLOCK_D89) ? 8 : 9;

	return (val);
}

/*
 * Get the displayed height in pixels
 */
static
unsigned vga_get_h (vga_t *vga)
{
	unsigned h;

	h = vga->reg_crt[VGA_CRT_VD];
	h |= (vga->reg_crt[VGA_CRT_OF] << 7) & 0x100;
	h |= (vga->reg_crt[VGA_CRT_OF] << 3) & 0x200;
	h += 1;

	return (h);
}

/*
 * Get the character width in pixels
 */
static
unsigned vga_get_cw (vga_t *vga)
{
	if ((vga->reg_seq[VGA_SEQ_CLOCK] & VGA_SEQ_CLOCK_D89) == 0) {
		return (9);
	}

	return (8);
}

/*
 * Get the character height in pixels
 */
static
unsigned vga_get_ch (vga_t *vga)
{
	return ((vga->reg_crt[VGA_CRT_MS] & 0x1f) + 1);
}

/*
 * Get CRT start offset
 */
static
unsigned vga_get_start (vga_t *vga)
{
	unsigned val;

	val = vga->reg_crt[VGA_CRT_SAH];
	val = (val << 8) | vga->reg_crt[VGA_CRT_SAL];

	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned vga_get_cursor (vga_t *vga)
{
	unsigned val;

	val = vga->reg_crt[VGA_CRT_CLH];
	val = (val << 8) | vga->reg_crt[VGA_CRT_CLL];

	return (val);
}

/*
 * Get the line compare register
 */
static
unsigned vga_get_line_compare (const vga_t *vga)
{
	unsigned val;

	val = vga->reg_crt[VGA_CRT_LC];

	if (vga->reg_crt[VGA_CRT_OF] & 0x10) {
		val += 256;
	}

	if (vga->reg_crt[VGA_CRT_MS] & 0x40) {
		val += 512;
	}

	return (val);
}

/*
 * Get a palette entry
 */
static
void vga_get_palette (vga_t *vga, unsigned idx,
	unsigned char *r, unsigned char *g, unsigned char *b)
{
	if (vga->reg_grc[VGA_GRC_MODE] & VGA_GRC_MODE_C256) {
		idx = 3 * (idx & 0xff);
		*r = vga->reg_dac[idx + 0];
		*g = vga->reg_dac[idx + 1];
		*b = vga->reg_dac[idx + 2];
		return;
	}

	idx &= vga->reg_atc[VGA_ATC_CPE];
	idx = vga->reg_atc[idx & 0x0f];

	if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_PS) {
		idx &= 0x0f;
		idx |= (vga->reg_atc[VGA_ATC_CS] & 0x03) << 4;
	}

	idx &= 0x3f;
	idx |= (vga->reg_atc[VGA_ATC_CS] & 0xc0) << 4;
	idx *= 3;

	*r = vga->reg_dac[idx + 0];
	*g = vga->reg_dac[idx + 1];
	*b = vga->reg_dac[idx + 2];
}

/*
 * Get a transformed CRTC address
 */
static
unsigned vga_get_crtc_addr (vga_t *vga, unsigned addr, unsigned row)
{
	if (vga->reg_crt[VGA_CRT_ULL] & VGA_CRT_ULL_DW) {
		/* double word mode */
		addr = ((addr << 2) | (addr >> 14)) & 0xffff;
	}
	else if ((vga->reg_crt[VGA_CRT_MODE] & VGA_CRT_MODE_WB) == 0) {
		/* word mode */
		addr = ((addr << 1) | (addr >> 15)) & 0xffff;
	}

	if ((vga->reg_crt[VGA_CRT_MODE] & VGA_CRT_MODE_CMS0) == 0) {
		/* CGA 8K bank simulation */
		addr = (addr & ~0x2000) | ((row & 1) << 13);
	}

	return (addr);
}

/*
 * Set the timing values from the CRT registers
 */
static
void vga_set_timing (vga_t *vga)
{
	int           d;
	unsigned      cw, of;
	unsigned long v;

	cw = vga_get_cw (vga);
	of = vga->reg_crt[VGA_CRT_OF];

	v = cw * (vga->reg_crt[VGA_CRT_HT] + 5);
	d = (vga->clk_ht != v);
	vga->clk_ht = v;

	v = cw * (vga->reg_crt[VGA_CRT_HD] + 1);
	d |= (vga->clk_hd != v);
	vga->clk_hd = v;

	v = vga->reg_crt[VGA_CRT_VT];
	v |= (of << 8) & 0x100;
	v |= (of << 4) & 0x200;
	v = (v + 2) * vga->clk_ht;
	d |= (vga->clk_vt != v);
	vga->clk_vt = v;

	v = vga->reg_crt[VGA_CRT_VD];
	v |= (of << 7) & 0x100;
	v |= (of << 3) & 0x200;
	v = (v + 1) * vga->clk_ht;
	d |= (vga->clk_vd != v);
	vga->clk_vd = v;

	if (d) {
		vga->update_state |= VGA_UPDATE_DIRTY;
	}
}

/*
 * Get the dot clock
 */
static
unsigned long vga_get_dotclock (vga_t *vga)
{
	unsigned long long clk;

	clk = vga->video.dotclk[0];

	switch ((vga->reg[VGA_MOUT] >> 2) & 3) {
	case 0:
		clk *= VGA_PFREQ0;
		break;

	case 1:
		clk *= VGA_PFREQ1;
		break;

	case 2:
	case 3:
		/* test */
		clk *= VGA_PFREQ1;
		break;
	}

	if (vga->reg_seq[VGA_SEQ_CLOCK] & VGA_SEQ_CLOCK_DC) {
		clk = clk / (2 * VGA_IFREQ);
	}
	else {
		clk = clk / VGA_IFREQ;
	}

	return (clk);
}

/*
 * Get a pointer to the bitmap for a character
 */
static
const unsigned char *vga_get_font (vga_t *vga, unsigned chr, unsigned atr)
{
	const unsigned char *fnt;
	unsigned char       sel;
	unsigned            ofs;

	sel = vga->reg_seq[VGA_SEQ_CMAPSEL];

	if (atr & 0x08) {
		ofs = ((sel >> 1) & 0x06) | ((sel >> 5) & 0x01);
	}
	else {
		ofs = ((sel << 1) & 0x06) | ((sel >> 4) & 0x01);
	}

	fnt = vga->mem + 0x20000 + (8192 * ofs) + (32 * (chr & 0xff));

	return (fnt);
}

/*
 * Set the internal screen buffer size
 */
static
int vga_set_buf_size (vga_t *vga, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > vga->bufmax) {
		tmp = realloc (vga->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		vga->buf = tmp;
		vga->bufmax = cnt;
	}

	vga->buf_w = w;
	vga->buf_h = h;

	return (0);
}

/*
 * Draw a character in the internal buffer
 */
static
void vga_mode0_update_char (vga_t *vga, unsigned char *dst, unsigned w,
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

	if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_EB) {
		if (a & 0x80) {
			blk = !vga->blink_on;
		}

		a &= 0x7f;
	}

	vga_get_palette (vga, a & 0x0f, fg, fg + 1, fg + 2);
	vga_get_palette (vga, (a >> 4) & 0x0f, bg, bg + 1, bg + 2);

	c1 = vga->reg_crt[VGA_CRT_CS] & 0x1f;
	c2 = vga->reg_crt[VGA_CRT_CE] & 0x1f;
	crs = (crs && (c1 <= c2) && vga->blink_on);
	incrs = 0;

	elg = 0;

	if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_ELG) {
		if ((c >= 0xc0) && (c <= 0xdf)) {
			elg = 1;
		}
	}

	ull = 0xffff;

	if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_ME) {
		if ((a & 0x07) == 1) {
			ull = vga->reg_crt[VGA_CRT_ULL] & 0x1f;
		}
	}

	fnt = vga_get_font (vga, c, a);

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

			if (incrs) {
				val = 0xffff;
			}

			if (y == c2) {
				incrs = !incrs;
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
void vga_update_text (vga_t *vga)
{
	unsigned            x, y, w, h, cw, ch;
	unsigned            w2, h2;
	unsigned            addr, rptr, rofs, p;
	unsigned            cpos;
	const unsigned char *src;
	unsigned char       *dst;

	w = vga_get_w (vga);
	h = vga_get_h (vga);
	cw = vga_get_cw (vga);
	ch = vga_get_ch (vga);

	if (vga_set_buf_size (vga, w, h)) {
		return;
	}

	src = vga->mem;
	dst = vga->buf;

	addr = vga->latch_addr;
	rofs = 2 * vga->reg_crt[VGA_CRT_OFS];
	cpos = vga_get_cursor (vga);

	y = 0;

	while (y < h) {
		h2 = h - y;

		if (h2 > ch) {
			h2 = ch;
		}

		dst = vga->buf + 3UL * y * w;

		rptr = addr;

		x = 0;
		while (x < w) {
			w2 = w - x;

			if (w2 > cw) {
				w2 = cw;
			}

			p = vga_get_crtc_addr (vga, rptr, 0);

			vga_mode0_update_char (vga, dst + 3 * x, w, w2, h2,
				src[p], src[p + 0x10000], rptr == cpos
			);

			rptr = (rptr + 1) & 0xffff;

			x += w2;
		}

		if (rofs == 0) {
			addr = rptr;
		}
		else {
			addr = (addr + rofs) & 0xffff;
		}

		y += h2;
	}
}

/*
 * Update graphics modes
 *
 * There's lots of room for optimizations here.
 */
static
void vga_update_graphics (vga_t *vga)
{
	unsigned            x, y, w, h;
	unsigned            row0, row1, col, cw, ch, dsr;
	unsigned            addr, rptr, rofs, ptr;
	unsigned            lcmp, hpp;
	unsigned char       blink1, blink2;
	unsigned            msk, bit;
	unsigned            idx;
	unsigned char       buf[4];
	int                 m256, mcga;
	const unsigned char *src;
	unsigned char       *dst;

	w = vga_get_w (vga);
	h = vga_get_h (vga);
	cw = vga_get_cw (vga);
	ch = vga_get_ch (vga);

	lcmp = vga_get_line_compare (vga);

	dsr = (vga->reg_crt[VGA_CRT_MS] & VGA_CRT_MS_DSC) ? 2 : 1;

	if ((dsr == 2) && ((h & 1) == 0)) {
		dsr = 1;
		h >>= 1;
		lcmp >>= 1;
	}

	if (vga->reg_grc[VGA_GRC_MODE] & VGA_GRC_MODE_C256) {
		if (((cw & 1) == 0) && ((w & 1) == 0)) {
			cw >>= 1;
			w >>= 1;
		}
	}

	if (vga->reg_crt[VGA_CRT_MODE] & VGA_CRT_MODE_CMS0) {
		if (((ch & 1) == 0) && ((h & 1) == 0)) {
			ch >>= 1;
			h >>= 1;
		}
	}

	if (vga_set_buf_size (vga, w, h)) {
		return;
	}

	if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_EB) {
		blink1 = 0xff;
		blink2 = vga->blink_on ? 0xff : 0x00;
	}
	else {
		blink1 = 0x00;
		blink2 = 0x00;
	}

	hpp = vga->latch_hpp;

	src = vga->mem;
	dst = vga->buf;

	m256 = ((vga->reg_grc[VGA_GRC_MODE] & VGA_GRC_MODE_C256) != 0);
	mcga = ((vga->reg_grc[VGA_GRC_MODE] & VGA_GRC_MODE_SR) != 0);

	addr = vga->latch_addr;
	rofs = 2 * vga->reg_crt[VGA_CRT_OFS];

	msk = 0;
	bit = 0;

	row0 = 0;
	row1 = 0;
	y = 0;

	while (y < h) {
		if (y == lcmp) {
			addr = 0;

			if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_PP) {
				hpp = 0;
			}
		}

		dst = vga->buf + 3UL * y * w;

		rptr = addr;

		col = 0;
		x = 0;

		ptr = vga_get_crtc_addr (vga, rptr, row1);

		buf[0] = src[ptr + 0x00000];
		buf[1] = src[ptr + 0x10000];
		buf[2] = src[ptr + 0x20000];
		buf[3] = (src[ptr + 0x30000] ^ blink1) | blink2;

		msk = 0x80 >> (hpp & 7);

		if (m256) {
			bit = (hpp >> 1) & 3;
		}
		else if (mcga) {
			bit = (2 * hpp) & 6;
		}

		col = hpp;

		while (x < w) {
			if (col >= cw) {
				rptr = (rptr + 1) & 0xffff;

				ptr = vga_get_crtc_addr (vga, rptr, row1);

				buf[0] = src[ptr + 0x00000];
				buf[1] = src[ptr + 0x10000];
				buf[2] = src[ptr + 0x20000];
				buf[3] = (src[ptr + 0x30000] ^ blink1) | blink2;

				msk = 0x80;
				bit = 0;
				col = 0;
			}

			if (m256) {
				/* VGA 256 color mode */

				idx = buf[bit & 3];
				bit += 1;
			}
			else if (mcga) {
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
				/* EGA 16 color mode */

				idx = (buf[0] & msk) ? 0x01 : 0x00;
				idx |= (buf[1] & msk) ? 0x02 : 0x00;
				idx |= (buf[2] & msk) ? 0x04 : 0x00;
				idx |= (buf[3] & msk) ? 0x08 : 0x00;
				msk >>= 1;
			}

			vga_get_palette (vga, idx, dst, dst + 1, dst + 2);

			dst += 3;
			col += 1;
			x += 1;
		}

		row0 += 1;

		if (row0 >= dsr) {
			row0 = 0;

			row1 += 1;
			if (row1 >= ch) {
				row1 = 0;

				if (rofs == 0) {
					addr = rptr;
				}
				else {
					addr = (addr + rofs) & 0xffff;
				}
			}

		}

		y += 1;
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void vga_update_blank (vga_t *vga)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	if ((vga->buf_w == 0) || (vga->buf_h == 0)) {
		vga_set_buf_size (vga, 320, 200);
	}

	dst = vga->buf;

	for (y = 0; y < vga->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < vga->buf_w; x++) {
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
void vga_update (vga_t *vga)
{
	int      show;
	unsigned w, h;

	w = vga_get_w (vga);
	h = vga_get_h (vga);

	show = 1;

	if (vga->reg_seq[VGA_SEQ_CLOCK] & VGA_SEQ_CLOCK_SO) {
		show = 0;
	}
	else if ((vga->reg_seq[VGA_SEQ_RESET] & VGA_SEQ_RESET_SR) == 0) {
		show = 0;
	}
	else if ((vga->reg_seq[VGA_SEQ_RESET] & VGA_SEQ_RESET_ASR) == 0) {
		show = 0;
	}
	else if ((w < 16) || (h < 16)) {
		show = 0;
	}

	if (show == 0) {
		vga_update_blank (vga);
		return;
	}

	if (vga->reg_grc[VGA_GRC_MISC] & VGA_GRC_MISC_GM) {
		vga_update_graphics (vga);
	}
	else {
		vga_update_text (vga);
	}
}


static
unsigned char vga_mem_get_uint8 (vga_t *vga, unsigned long addr)
{
	unsigned rdmode;
	unsigned a0;

	if ((vga->reg[VGA_MOUT] & VGA_MOUT_ERAM) == 0) {
		return (0xff);
	}

	switch ((vga->reg_grc[VGA_GRC_MISC] >> 2) & 3) {
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

	a0 = addr & 3;

	if (vga->reg_seq[VGA_SEQ_MODE] & VGA_SEQ_MODE_CH4) {
		addr &= 0xfffc;
	}
	else if ((vga->reg_seq[VGA_SEQ_MODE] & VGA_SEQ_MODE_OE) == 0) {
		addr &= 0xfffe;
	}

	rdmode = (vga->reg_grc[VGA_GRC_MODE] >> 3) & 1;

	vga->latch[0] = vga->mem[addr + 0x00000];
	vga->latch[1] = vga->mem[addr + 0x10000];
	vga->latch[2] = vga->mem[addr + 0x20000];
	vga->latch[3] = vga->mem[addr + 0x30000];

	if (rdmode == 0) {
		unsigned map;

		map = vga->reg_grc[VGA_GRC_READMAP] & 3;

		if (vga->reg_seq[VGA_SEQ_MODE] & VGA_SEQ_MODE_CH4) {
			map = a0;
		}
		else if ((vga->reg_seq[VGA_SEQ_MODE] & VGA_SEQ_MODE_OE) == 0) {
			map = (map & 0x02) + (a0 & 1);
		}

		return (vga->latch[map]);
	}
	else {
		unsigned char msk, val, c1, c2;

		val = 0x00;
		msk = 0x80;

		while (msk != 0) {
			c1 = (vga->latch[0] & msk) ? 0x01 : 0x00;
			c1 |= (vga->latch[1] & msk) ? 0x02 : 0x00;
			c1 |= (vga->latch[2] & msk) ? 0x04 : 0x00;
			c1 |= (vga->latch[3] & msk) ? 0x08 : 0x00;
			c1 &= vga->reg_grc[VGA_GRC_CDC];

			c2 = vga->reg_grc[VGA_GRC_COLCMP];
			c2 &= vga->reg_grc[VGA_GRC_CDC];

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
unsigned short vga_mem_get_uint16 (vga_t *vga, unsigned long addr)
{
	unsigned short val;

	if ((vga->reg[VGA_MOUT] & VGA_MOUT_ERAM) == 0) {
		return (0);
	}

	val = vga_mem_get_uint8 (vga, addr);
	val |= vga_mem_get_uint8 (vga, addr + 1) << 8;

	return (val);
}

static
void vga_mem_set_uint8 (vga_t *vga, unsigned long addr, unsigned char val)
{
	unsigned      wrmode;
	unsigned      rot;
	unsigned char mapmsk, bitmsk;
	unsigned char esr, set;
	unsigned char col[4];

	if ((vga->reg[VGA_MOUT] & VGA_MOUT_ERAM) == 0) {
		return;
	}

	switch ((vga->reg_grc[VGA_GRC_MISC] >> 2) & 3) {
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

	wrmode = vga->reg_grc[VGA_GRC_MODE] & 3;
	mapmsk = vga->reg_seq[VGA_SEQ_MAPMASK];
	bitmsk = vga->reg_grc[VGA_GRC_BITMASK];

	if (vga->reg_seq[VGA_SEQ_MODE] & VGA_SEQ_MODE_CH4) {
		mapmsk = 1 << (addr & 3);
		addr &= 0xfffc;
	}
	else if ((vga->reg_seq[VGA_SEQ_MODE] & VGA_SEQ_MODE_OE) == 0) {
		mapmsk &= (addr & 1) ? 0x0a : 0x05;
		addr &= 0xfffe;
	}

	switch (wrmode) {
	case 0: /* write mode 0 */
		rot = vga->reg_grc[VGA_GRC_ROTATE] & 7;
		val = ((val >> rot) | (val << (8 - rot))) & 0xff;

		esr = vga->reg_grc[VGA_GRC_ENABLESR];
		set = vga->reg_grc[VGA_GRC_SETRESET];

		col[0] = (esr & 0x01) ? ((set & 0x01) ? 0xff : 0x00) : val;
		col[1] = (esr & 0x02) ? ((set & 0x02) ? 0xff : 0x00) : val;
		col[2] = (esr & 0x04) ? ((set & 0x04) ? 0xff : 0x00) : val;
		col[3] = (esr & 0x08) ? ((set & 0x08) ? 0xff : 0x00) : val;
		break;

	case 1: /* write mode 1 */
		col[0] = vga->latch[0];
		col[1] = vga->latch[1];
		col[2] = vga->latch[2];
		col[3] = vga->latch[3];
		break;

	case 2: /* write mode 2 */
		col[0] = (val & 0x01) ? 0xff : 0x00;
		col[1] = (val & 0x02) ? 0xff : 0x00;
		col[2] = (val & 0x04) ? 0xff : 0x00;
		col[3] = (val & 0x08) ? 0xff : 0x00;
		break;

	case 3: /* write mode 3 */
		rot = vga->reg_grc[VGA_GRC_ROTATE] & 7;
		val = ((val >> rot) | (val << (8 - rot))) & 0xff;

		bitmsk &= val;

		set = vga->reg_grc[VGA_GRC_SETRESET];

		col[0] = (set & 0x01) ? 0xff : 0x00;
		col[1] = (set & 0x02) ? 0xff : 0x00;
		col[2] = (set & 0x04) ? 0xff : 0x00;
		col[3] = (set & 0x08) ? 0xff : 0x00;
		break;

	default:
		/* not reached */
		return;
	}

	if (wrmode != 1) {
		switch ((vga->reg_grc[VGA_GRC_ROTATE] >> 3) & 3) {
		case 0: /* copy */
			break;

		case 1: /* and */
			col[0] &= vga->latch[0];
			col[1] &= vga->latch[1];
			col[2] &= vga->latch[2];
			col[3] &= vga->latch[3];
			break;

		case 2: /* or */
			col[0] |= vga->latch[0];
			col[1] |= vga->latch[1];
			col[2] |= vga->latch[2];
			col[3] |= vga->latch[3];
			break;

		case 3: /* xor */
			col[0] ^= vga->latch[0];
			col[1] ^= vga->latch[1];
			col[2] ^= vga->latch[2];
			col[3] ^= vga->latch[3];
			break;
		}
	}

	col[0] = (col[0] & bitmsk) | (vga->latch[0] & ~bitmsk);
	col[1] = (col[1] & bitmsk) | (vga->latch[1] & ~bitmsk);
	col[2] = (col[2] & bitmsk) | (vga->latch[2] & ~bitmsk);
	col[3] = (col[3] & bitmsk) | (vga->latch[3] & ~bitmsk);

	if (mapmsk & 0x01) {
		vga->mem[addr + 0x00000] = col[0];
	}

	if (mapmsk & 0x02) {
		vga->mem[addr + 0x10000] = col[1];
	}

	if (mapmsk & 0x04) {
		vga->mem[addr + 0x20000] = col[2];
	}

	if (mapmsk & 0x08) {
		vga->mem[addr + 0x30000] = col[3];
	}

	vga->update_state |= VGA_UPDATE_DIRTY;
}

static
void vga_mem_set_uint16 (vga_t *vga, unsigned long addr, unsigned short val)
{
	if ((vga->reg[VGA_MOUT] & VGA_MOUT_ERAM) == 0) {
		return;
	}

	vga_mem_set_uint8 (vga, addr, val & 0xff);
	vga_mem_set_uint8 (vga, addr + 1, (val >> 8) & 0xff);
}


/*
 * Get an attribute controller register
 */
static
unsigned char vga_atc_get_reg (vga_t *vga, unsigned reg)
{
	if (reg > 20) {
		return (0xff);
	}

	return (vga->reg_atc[reg]);
}

/*
 * Set an attribute controller register
 */
static
void vga_atc_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
	if (reg > 20) {
		return;
	}

	if (vga->reg_atc[reg] == val) {
		return;
	}

	vga->reg_atc[reg] = val;

	vga->update_state |= VGA_UPDATE_DIRTY;
}


/*
 * Get a sequencer register
 */
static
unsigned char vga_seq_get_reg (vga_t *vga, unsigned reg)
{
	if (reg > 4) {
		return (0xff);
	}

	return (vga->reg_seq[reg]);
}

/*
 * Set a sequencer register
 */
static
void vga_seq_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
	if (reg > 4) {
		return;
	}

	if (vga->reg_seq[reg] == val) {
		return;
	}

	switch (reg) {
	case VGA_SEQ_RESET: /* 0 */
		vga->reg_seq[VGA_SEQ_RESET] = val;
		break;

	case VGA_SEQ_CLOCK: /* 1 */
		vga->reg_seq[VGA_SEQ_CLOCK] = val;
		vga->update_state |= VGA_UPDATE_DIRTY;
		break;

	case VGA_SEQ_MAPMASK: /* 2 */
		vga->reg_seq[VGA_SEQ_MAPMASK] = val;
		break;

	case VGA_SEQ_CMAPSEL: /* 3 */
		vga->reg_seq[VGA_SEQ_CMAPSEL] = val;
		vga->update_state |= VGA_UPDATE_DIRTY;
		break;

	case VGA_SEQ_MODE: /* 4 */
		vga->reg_seq[VGA_SEQ_MODE] = val;
		break;
	}
}


/*
 * Get a graphics controller register
 */
static
unsigned char vga_grc_get_reg (vga_t *vga, unsigned reg)
{
	if (reg > 8) {
		return (0xff);
	}

	return vga->reg_grc[reg];
}

/*
 * Set a graphics controller register
 */
static
void vga_grc_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
	if (reg > 8) {
		return;
	}

	if (vga->reg_grc[reg] == val) {
		return;
	}

	vga->reg_grc[reg] = val;

	if (reg == VGA_GRC_MODE) {
		vga->update_state |= VGA_UPDATE_DIRTY;
	}
}


/*
 * Get a CRTC register
 */
static
unsigned char vga_crtc_get_reg (vga_t *vga, unsigned reg)
{
	if (reg > 24) {
		return (0xff);
	}

	return (vga->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void vga_crtc_set_reg (vga_t *vga, unsigned reg, unsigned char val)
{
	if (reg > 24) {
		return;
	}

	if (reg < 8) {
		if (vga->reg_crt[VGA_CRT_VRE] & VGA_CRT_VRE_PR) {
			if (reg == VGA_CRT_OF) {
				/* overflow/lc8 is not protected */
				val &= VGA_CRT_OF_LC8;
				val |= vga->reg_crt[VGA_CRT_OF] & ~VGA_CRT_OF_LC8;
			}
			else {
				return;
			}
		}
	}

	if (reg == VGA_CRT_VRE) {
		if ((val & VGA_CRT_VRE_CVI) == 0) {
			vga_set_irq (vga, 0);
		}
	}

	if (vga->reg_crt[reg] == val) {
		return;
	}

	vga->reg_crt[reg] = val;

	vga_set_timing (vga);

	vga->update_state |= VGA_UPDATE_DIRTY;
}


/*
 * Get the DAC write address register
 */
static
unsigned char vga_get_dac_addr_write (vga_t *vga)
{
	return (vga->dac_addr_write / 3);
}

/*
 * Get the DAC state register
 */
static
unsigned char vga_get_dac_state (vga_t *vga)
{
	return (vga->dac_state);
}

/*
 * Get the DAC data register
 */
static
unsigned char vga_get_dac_data (vga_t *vga)
{
	unsigned char val;

	if ((vga->dac_state & 3) == 0) {
		return (0);
	}

	val = vga->reg_dac[vga->dac_addr_read] >> 2;

	vga->dac_addr_read += 1;

	if (vga->dac_addr_read >= 768) {
		vga->dac_addr_read = 0;
	}

	return (val);
}

/*
 * Get the DAC PEL mask register
 */
static
unsigned char vga_get_dac_pel_mask (vga_t *vga)
{
	return (0xff);
}

/*
 * Get the attribute controller index register
 */
static
unsigned char vga_get_atc_index (vga_t *vga)
{
	return (vga->reg[VGA_ATC_INDEX]);
}

/*
 * Get the attribute controller data register
 */
static
unsigned char vga_get_atc_data (vga_t *vga)
{
	return (vga_atc_get_reg (vga, vga->reg[VGA_ATC_INDEX] & 0x1f));
}

/*
 * Get the sequencer index register
 */
static
unsigned char vga_get_seq_index (vga_t *vga)
{
	return (vga->reg[VGA_SEQ_INDEX]);
}

/*
 * Get the sequencer data register
 */
static
unsigned char vga_get_seq_data (vga_t *vga)
{
	return (vga_seq_get_reg (vga, vga->reg[VGA_SEQ_INDEX] & 7));
}

/*
 * Get the graphics controller index register
 */
static
unsigned char vga_get_grc_index (vga_t *vga)
{
	return (vga->reg[VGA_GRC_INDEX]);
}

/*
 * Get the graphics controller data register
 */
static
unsigned char vga_get_grc_data (vga_t *vga)
{
	return (vga_grc_get_reg (vga, vga->reg[VGA_GRC_INDEX] & 0x0f));
}

/*
 * Get the CRTC index register
 */
static
unsigned char vga_get_crtc_index (vga_t *vga)
{
	return (vga->reg[VGA_CRT_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char vga_get_crtc_data (vga_t *vga)
{
	return (vga_crtc_get_reg (vga, vga->reg[VGA_CRT_INDEX] & 0x1f));
}

/*
 * Get the switch sense input
 *
 * This is reverse engineered from the IBM VGA BIOS. I have no
 * idea how and why this works.
 */
static
int vga_get_input_status_0_ss (vga_t *vga)
{
	const unsigned char *p;
	unsigned char       dac[3];

	static unsigned char vals[] = {
		0x12, 0x12, 0x12, 0x10,
		0x14, 0x14, 0x14, 0x10,
		0x2d, 0x14, 0x14, 0x00,
		0x14, 0x2d, 0x14, 0x00,
		0x14, 0x14, 0x2d, 0x00,
		0x2d, 0x2d, 0x2d, 0x00,
		0x00, 0x00, 0x00, 0xff
	};

	dac[0] = vga->reg_dac[0] >> 2;
	dac[1] = vga->reg_dac[1] >> 2;
	dac[2] = vga->reg_dac[2] >> 2;

	p = vals;

	while (p[3] != 0xff) {
		if ((p[0] == dac[0]) && (p[1] == dac[1]) && (p[2] == dac[2])) {
			return (p[3] != 0);
		}

		p += 4;
	}

	return (1);
}

/*
 * Get the input status register 0
 */
static
unsigned char vga_get_input_status_0 (vga_t *vga)
{
	if (vga_get_input_status_0_ss (vga)) {
		vga->reg[VGA_STATUS0] |= VGA_STATUS0_SS;
	}
	else {
		vga->reg[VGA_STATUS0] &= ~VGA_STATUS0_SS;
	}

	return (vga->reg[VGA_STATUS0]);
}

/*
 * Get the miscellaneous output register
 */
static
unsigned char vga_get_misc_out (vga_t *vga)
{
	return (vga->reg[VGA_MOUT]);
}

/*
 * Get the input status register 1
 */
static
unsigned char vga_get_input_status_1 (vga_t *vga)
{
	unsigned char val;
	unsigned long clk;

	vga_clock (vga, 0);

	vga->atc_flipflop = 0;

	clk = vga_get_dotclock (vga);

	val = vga->reg[VGA_STATUS1];
	val &= ~(VGA_STATUS1_DE | VGA_STATUS1_VR);

	if (clk >= vga->clk_vd) {
		val |= (VGA_STATUS1_DE | VGA_STATUS1_VR);
	}
	else if ((clk % vga->clk_ht) >= vga->clk_hd) {
		val |= VGA_STATUS1_DE;
	}

	/* don't know what this is, but the bios needs it */
	val = (val & ~0x30) | ((val + 0x10) & 0x30);

	vga->reg[VGA_STATUS1] = val;

	return (val);
}

/*
 * Get a VGA register
 */
static
unsigned char vga_reg_get_uint8 (vga_t *vga, unsigned long addr)
{
	if (addr < 0x10) {
		if (vga->reg[VGA_MOUT] & VGA_MOUT_IOS) {
			return (0xff);
		}

		addr += 0x20;
	}
	else if (addr > 0x20) {
		if ((vga->reg[VGA_MOUT] & VGA_MOUT_IOS) == 0) {
			return (0xff);
		}
	}

	switch (addr) {
	case VGA_ATC_INDEX: /* 3C0 */
		return (vga_get_atc_index (vga));

	case VGA_ATC_DATA: /* 3C1 */
		return (vga_get_atc_data (vga));

	case VGA_STATUS0: /* 3C2 */
		return (vga_get_input_status_0 (vga));

	case VGA_SEQ_INDEX: /* 3C4 */
		return (vga_get_seq_index (vga));

	case VGA_SEQ_DATA: /* 3C5 */
		return (vga_get_seq_data (vga));

	case VGA_DAC_PEL_MASK: /* 3C6 */
		return (vga_get_dac_pel_mask (vga));

	case VGA_DAC_STATE: /* 3C7 */
		return (vga_get_dac_state (vga));

	case VGA_DAC_ADDR_WR: /* 3C8 */
		return (vga_get_dac_addr_write (vga));

	case VGA_DAC_DATA: /* 3C9 */
		return (vga_get_dac_data (vga));

	case VGA_MOUT: /* 3CC */
		return (vga_get_misc_out (vga));

	case VGA_GRC_INDEX: /* 3CE */
		return (vga_get_grc_index (vga));

	case VGA_GRC_DATA: /* 3CF */
		return (vga_get_grc_data (vga));

	case VGA_CRT_INDEX: /* 3D4 */
		return (vga_get_crtc_index (vga));

	case VGA_CRT_DATA: /* 3D5 */
		return (vga_get_crtc_data (vga));

	case VGA_STATUS1: /* 3DA */
		return (vga_get_input_status_1 (vga));
	}

	return (0xff);
}

static
unsigned short vga_reg_get_uint16 (vga_t *vga, unsigned long addr)
{
	unsigned short ret;

	ret = vga_reg_get_uint8 (vga, addr);
	ret |= vga_reg_get_uint8 (vga, addr + 1) << 8;

	return (ret);
}


/*
 * Set the DAC read address register
 */
static
void vga_set_dac_addr_read (vga_t *vga, unsigned char val)
{
	vga->dac_addr_read = 3 * (val & 0xff);
	vga->dac_state |= 3;
}

/*
 * Set the DAC write address register
 */
static
void vga_set_dac_addr_write (vga_t *vga, unsigned char val)
{
	vga->dac_addr_write = 3 * (val & 0xff);
	vga->dac_state &= ~3;
}

/*
 * Set the DAC data register
 */
static
void vga_set_dac_data (vga_t *vga, unsigned char val)
{
	if (vga->dac_state & 3) {
		return;
	}

	val &= 0x3f;
	val = (val << 2) | (val >> 4);

	if (vga->reg_dac[vga->dac_addr_write] != val) {
		vga->reg_dac[vga->dac_addr_write] = val;
		vga->update_state |= VGA_UPDATE_DIRTY;
	}

	vga->dac_addr_write += 1;

	if (vga->dac_addr_write >= 768) {
		vga->dac_addr_write = 0;
	}
}

/*
 * Set the attribute controller index/data register
 */
static
void vga_set_atc_index (vga_t *vga, unsigned char val)
{
	if (vga->atc_flipflop == 0) {
		vga->reg[VGA_ATC_INDEX] = val;
		vga->atc_flipflop = 1;
	}
	else {
		vga_atc_set_reg (vga, vga->reg[VGA_ATC_INDEX] & 0x1f, val);
		vga->atc_flipflop = 0;
	}
}

/*
 * Set the sequencer index register
 */
static
void vga_set_seq_index (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_SEQ_INDEX] = val;
}

/*
 * Set the sequencer data register
 */
static
void vga_set_seq_data (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_SEQ_DATA] = val;

	vga_seq_set_reg (vga, vga->reg[VGA_SEQ_INDEX] & 0x3f, val);
}

/*
 * Set the graphics controller index register
 */
static
void vga_set_grc_index (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_GRC_INDEX] = val;
}

/*
 * Set the graphics controller data register
 */
static
void vga_set_grc_data (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_GRC_DATA] = val;

	vga_grc_set_reg (vga, vga->reg[VGA_GRC_INDEX] & 0x3f, val);
}

/*
 * Set the CRTC index register
 */
static
void vga_set_crtc_index (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_CRT_INDEX] = val;
}

/*
 * Set the CRTC data register
 */
static
void vga_set_crtc_data (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_CRT_DATA] = val;

	vga_crtc_set_reg (vga, vga->reg[VGA_CRT_INDEX] & 0x3f, val);
}

/*
 * Set the miscellaneous output register
 */
static
void vga_set_misc_out (vga_t *vga, unsigned char val)
{
	vga->reg[VGA_MOUT] = val;

	vga->update_state |= VGA_UPDATE_DIRTY;
}

/*
 * Set a VGA register
 */
static
void vga_reg_set_uint8 (vga_t *vga, unsigned long addr, unsigned char val)
{
	if (addr < 0x10) {
		if (vga->reg[VGA_MOUT] & VGA_MOUT_IOS) {
			return;
		}

		addr += 0x20;
	}
	else if (addr > 0x20) {
		if ((vga->reg[VGA_MOUT] & VGA_MOUT_IOS) == 0) {
			return;
		}
	}

	switch (addr) {
	case VGA_ATC_INDEX: /* 3C0 */
		vga_set_atc_index (vga, val);
		break;

	case VGA_ATC_DATA: /* 3C1 */
		vga_set_atc_index (vga, val);
		break;

	case VGA_MOUT_W: /* 3C2 */
		vga_set_misc_out (vga, val);
		break;

	case VGA_SEQ_INDEX: /* 3C4 */
		vga_set_seq_index (vga, val);
		break;

	case VGA_SEQ_DATA: /* 3C5 */
		vga_set_seq_data (vga, val);
		break;

	case VGA_DAC_ADDR_RD: /* 3C7 */
		vga_set_dac_addr_read (vga, val);
		break;

	case VGA_DAC_ADDR_WR: /* 3C8 */
		vga_set_dac_addr_write (vga, val);
		break;

	case VGA_DAC_DATA: /* 3C8 */
		vga_set_dac_data (vga, val);
		break;

	case VGA_MOUT: /* 3CC */
		break;

	case VGA_GRC_INDEX: /* 3CE */
		vga_set_grc_index (vga, val);
		break;

	case VGA_GRC_DATA: /* 3CF */
		vga_set_grc_data (vga, val);
		break;

	case VGA_CRT_INDEX: /* 3D4 */
		vga_set_crtc_index (vga, val);
		break;

	case VGA_CRT_DATA: /* 3D5 */
		vga_set_crtc_data (vga, val);
		break;

	case VGA_STATUS1: /* 3DA */
		break;

	default:
		break;
	}
}

static
void vga_reg_set_uint16 (vga_t *vga, unsigned long addr, unsigned short val)
{
	vga_reg_set_uint8 (vga, addr, val & 0xff);
	vga_reg_set_uint8 (vga, addr + 1, val >> 8);
}


static
int vga_set_msg (vga_t *vga, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}

		vga_set_blink_rate (vga, freq);

		return (0);
	}

	return (-1);
}

static
void vga_set_terminal (vga_t *vga, terminal_t *trm)
{
	vga->term = trm;

	if (vga->term != NULL) {
		trm_open (vga->term, 720, 400);
	}
}

static
mem_blk_t *vga_get_mem (vga_t *vga)
{
	return (vga->memblk);
}

static
mem_blk_t *vga_get_reg (vga_t *vga)
{
	return (vga->regblk);
}

static
void vga_print_regs (vga_t *vga, FILE *fp, const char *name, const unsigned char *reg, unsigned cnt)
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
void vga_print_info (vga_t *vga, FILE *fp)
{
	fprintf (fp, "DEV: VGA\n");

	fprintf (fp, "VGA: ADDR=%04X  LOFS=%04X  CURS=%04X  LCMP=%04X\n",
		vga_get_start (vga),
		2 * vga->reg_crt[VGA_CRT_OFS],
		vga_get_cursor (vga),
		vga_get_line_compare (vga)
	);

	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		vga_get_dotclock (vga),
		vga->clk_ht, vga->clk_hd,
		vga->clk_vt, vga->clk_vd
	);

	fprintf (fp, "MOUT=%02X  ST0=%02X  ST1=%02X\n",
		vga->reg[VGA_MOUT],
		vga->reg[VGA_STATUS0],
		vga->reg[VGA_STATUS1]
	);

	vga_print_regs (vga, fp, "CRT", vga->reg_crt, 25);
	vga_print_regs (vga, fp, "ATC", vga->reg_atc, 21);
	vga_print_regs (vga, fp, "SEQ", vga->reg_seq, 5);
	vga_print_regs (vga, fp, "GRC", vga->reg_grc, 9);

	fflush (fp);
}

/*
 * Force a screen update
 */
static
void vga_redraw (vga_t *vga, int now)
{
	if (now) {
		if (vga->term != NULL) {
			vga_update (vga);
			trm_set_size (vga->term, vga->buf_w, vga->buf_h);
			trm_set_lines (vga->term, vga->buf, 0, vga->buf_h);
			trm_update (vga->term);
		}
	}

	vga->update_state |= VGA_UPDATE_DIRTY;
}

static
void vga_clock (vga_t *vga, unsigned long cnt)
{
	unsigned      addr;
	unsigned long clk;

	if (vga->clk_vt < 50000) {
		return;
	}

	clk = vga_get_dotclock (vga);

	if (clk < vga->clk_vd) {
		vga->update_state &= ~VGA_UPDATE_RETRACE;
		return;
	}

	if (clk >= vga->clk_vt) {
		vga->video.dotclk[0] = 0;
		vga->video.dotclk[1] = 0;
		vga->video.dotclk[2] = 0;

		vga->latch_hpp = vga->reg_atc[VGA_ATC_HPP] & 0x0f;

		addr = vga_get_start (vga);

		if (vga->latch_addr != addr) {
			vga->latch_addr = addr;
			vga->update_state |= VGA_UPDATE_DIRTY;
		}
	}

	if (vga->update_state & VGA_UPDATE_RETRACE) {
		return;
	}

	if (vga->blink_cnt > 0) {
		vga->blink_cnt -= 1;

		if (vga->blink_cnt == 0) {
			vga->blink_cnt = vga->blink_freq;
			vga->blink_on = !vga->blink_on;

			if ((vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_G) == 0) {
				vga->update_state |= VGA_UPDATE_DIRTY;
			}
			else if (vga->reg_atc[VGA_ATC_MODE] & VGA_ATC_MODE_EB) {
				vga->update_state |= VGA_UPDATE_DIRTY;
			}
		}
	}

	if (vga->term != NULL) {
		if (vga->update_state & VGA_UPDATE_DIRTY) {
			vga_update (vga);
			trm_set_size (vga->term, vga->buf_w, vga->buf_h);
			trm_set_lines (vga->term, vga->buf, 0, vga->buf_h);
		}

		trm_update (vga->term);
	}

	vga->update_state = VGA_UPDATE_RETRACE;

	if ((vga->reg_crt[VGA_CRT_VRE] & VGA_CRT_VRE_EVI) == 0) {
		/* vertical retrace interrupt enabled */
		vga_set_irq (vga, 1);
	}
}

void vga_set_irq_fct (vga_t *vga, void *ext, void *fct)
{
	vga->set_irq_ext = ext;
	vga->set_irq = fct;
}

void vga_free (vga_t *vga)
{
	mem_blk_del (vga->memblk);
	mem_blk_del (vga->regblk);
}

void vga_del (vga_t *vga)
{
	if (vga != NULL) {
		vga_free (vga);
		free (vga);
	}
}

void vga_init (vga_t *vga, unsigned long io, unsigned long addr)
{
	unsigned i;

	pce_video_init (&vga->video);

	vga->video.ext = vga;
	vga->video.del = (void *) vga_del;
	vga->video.set_msg = (void *) vga_set_msg;
	vga->video.set_terminal = (void *) vga_set_terminal;
	vga->video.get_mem = (void *) vga_get_mem;
	vga->video.get_reg = (void *) vga_get_reg;
	vga->video.print_info = (void *) vga_print_info;
	vga->video.redraw = (void *) vga_redraw;
	vga->video.clock = (void *) vga_clock;

	vga->term = NULL;

	vga->mem = malloc (256UL * 1024UL);
	if (vga->mem == NULL) {
		return;
	}

	vga->memblk = mem_blk_new (addr, 128UL * 1024UL, 0);
	vga->memblk->ext = vga;
	mem_blk_set_data (vga->memblk, vga->mem, 1);
	vga->memblk->set_uint8 = (void *) vga_mem_set_uint8;
	vga->memblk->set_uint16 = (void *) vga_mem_set_uint16;
	vga->memblk->get_uint8 = (void *) vga_mem_get_uint8;
	vga->memblk->get_uint16 = (void *) vga_mem_get_uint16;
	vga->mem = vga->memblk->data;
	mem_blk_clear (vga->memblk, 0x00);

	vga->regblk = mem_blk_new (io, 0x30, 1);
	vga->regblk->ext = vga;
	vga->regblk->set_uint8 = (void *) vga_reg_set_uint8;
	vga->regblk->set_uint16 = (void *) vga_reg_set_uint16;
	vga->regblk->get_uint8 = (void *) vga_reg_get_uint8;
	vga->regblk->get_uint16 = (void *) vga_reg_get_uint16;
	vga->reg = vga->regblk->data;
	mem_blk_clear (vga->regblk, 0x00);

	for (i = 0; i < 5; i++) {
		vga->reg_seq[i] = 0;
	}

	for (i = 0; i < 9; i++) {
		vga->reg_grc[i] = 0;
	}

	for (i = 0; i < 21; i++) {
		vga->reg_atc[i] = 0;
	}

	for (i = 0; i < 25; i++) {
		vga->reg_crt[i] = 0;
	}

	for (i = 0; i < 768; i++) {
		vga->reg_dac[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		vga->latch[i] = 0;
	}

	vga->latch_addr = 0;
	vga->latch_hpp = 0;

	vga->atc_flipflop = 0;

	vga->dac_addr_read = 0;
	vga->dac_addr_write = 0;
	vga->dac_state = 0;

	vga->latch_addr = 0;

	vga->blink_on = 1;
	vga->blink_cnt = 0;
	vga->blink_freq = 16;

	vga->clk_ht = 0;
	vga->clk_hd = 0;
	vga->clk_vt = 0;
	vga->clk_vd = 0;

	vga->bufmax = 0;
	vga->buf = NULL;
	vga->buf_w = 0;
	vga->buf_h = 0;

	vga->update_state = 0;

	vga->set_irq_ext = NULL;
	vga->set_irq = NULL;
	vga->set_irq_val = 0;
}

vga_t *vga_new (unsigned long io, unsigned long addr)
{
	vga_t *vga;

	vga = malloc (sizeof (vga_t));
	if (vga == NULL) {
		return (NULL);
	}

	vga_init (vga, io, addr);

	return (vga);
}

video_t *vga_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      blink;
	vga_t         *vga;

	ini_get_uint32 (sct, "io", &io, 0x3b0);
	ini_get_uint32 (sct, "address", &addr, 0xa0000);
	ini_get_uint16 (sct, "blink", &blink, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "VGA io=0x%04lx addr=0x%05lx\n",
		io, addr
	);

	vga = vga_new (io, addr);
	if (vga == NULL) {
		return (NULL);
	}

	vga_set_blink_rate (vga, blink);

	return (&vga->video);
}
