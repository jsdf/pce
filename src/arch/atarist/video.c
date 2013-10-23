/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/video.c                                     *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>

#include <devices/memory.h>

#ifndef DEBUG_VIDEO
#define DEBUG_VIDEO 0
#endif


static unsigned char st_video_get_uint8 (st_video_t *vid, unsigned long addr);
static unsigned short st_video_get_uint16 (st_video_t *vid, unsigned long addr);
static unsigned long st_video_get_uint32 (st_video_t *vid, unsigned long addr);
static void st_video_set_uint8 (st_video_t *vid, unsigned long addr, unsigned char val);
static void st_video_set_uint16 (st_video_t *vid, unsigned long addr, unsigned short val);
static void st_video_set_uint32 (st_video_t *vid, unsigned long addr, unsigned long val);


int st_video_init (st_video_t *vid, unsigned long addr, int mono)
{
	vid->mem = NULL;
	vid->base = 0;
	vid->addr = 0;

	if (mem_blk_init (&vid->reg, addr, 128, 0)) {
		return (1);
	}

	mem_blk_set_fct (&vid->reg, vid,
		st_video_get_uint8, st_video_get_uint16, st_video_get_uint32,
		st_video_set_uint8, st_video_set_uint16, st_video_set_uint32
	);

	vid->mono = (mono != 0);

	vid->rgb = malloc (3UL * 640UL * 400UL);

	if (vid->rgb == NULL) {
		return (1);
	}

	vid->src = NULL;
	vid->dst = vid->rgb;

	vid->frame_skip = 0;
	vid->frame_skip_max = 1;

	vid->hb_val = 0;
	vid->hb_ext = NULL;
	vid->set_hb = NULL;

	vid->vb_val = 0;
	vid->vb_ext = NULL;
	vid->set_vb = NULL;

	return (0);
}

void st_video_free (st_video_t *vid)
{
	mem_blk_free (&vid->reg);
}

st_video_t *st_video_new (unsigned long addr, int mono)
{
	st_video_t *vid;

	if ((vid = malloc (sizeof (st_video_t))) == NULL) {
		return (NULL);
	}

	if (st_video_init (vid, addr, mono)) {
		free (vid);
		return (NULL);
	}

	return (vid);
}

void st_video_del (st_video_t *vid)
{
	if (vid != NULL) {
		st_video_free (vid);
		free (vid);
	}
}

void st_video_set_memory (st_video_t *vid, memory_t *mem)
{
	vid->mem = mem;
}

void st_video_set_hb_fct (st_video_t *vid, void *ext, void *fct)
{
	vid->hb_ext = ext;
	vid->set_hb = fct;
}

void st_video_set_vb_fct (st_video_t *vid, void *ext, void *fct)
{
	vid->vb_ext = ext;
	vid->set_vb = fct;
}

void st_video_set_terminal (st_video_t *vid, terminal_t *trm)
{
	vid->trm = trm;

	if (vid->trm != NULL) {
		if (vid->mono) {
			trm_open (vid->trm, 640, 400);
		}
		else {
			trm_open (vid->trm, 320, 200);
		}
	}
}

void st_video_set_frame_skip (st_video_t *vid, unsigned skip)
{
	vid->frame_skip_max = skip;
}

static
void st_video_set_hb (st_video_t *vid, unsigned char val)
{
	if (vid->hb_val != val) {
		vid->hb_val = val;

		if (vid->set_hb != NULL) {
			vid->set_hb (vid->hb_ext, val);
		}
	}
}

static
void st_video_set_vb (st_video_t *vid, unsigned char val)
{
	if (vid->vb_val != val) {
		vid->vb_val = val;

		if (vid->set_vb != NULL) {
			vid->set_vb (vid->vb_ext, val);
		}
	}
}

static
void st_video_set_timing (st_video_t *vid)
{
	int pal;

	pal = vid->sync_mode & 2;

	switch (vid->shift_mode & 3) {
	case 0:
	case 1:
		if (pal) {
			vid->hb1 = 320;
			vid->hb2 = 320 + 192;
			vid->vb1 = 200;
			vid->vb2 = 200 + 113;
		}
		else {
			vid->hb1 = 320;
			vid->hb2 = 320 + 188;
			vid->vb1 = 200;
			vid->vb2 = 200 + 63;
		}
		break;

	default:
		vid->hb1 = 640 / 4;
		vid->hb2 = (640 + 256) / 4;
		vid->vb1 = 400;
		vid->vb2 = 400 + 101;
		break;
	}

	vid->frame_rate[0] = 8000000;
	vid->frame_rate[1] = (unsigned long) vid->hb2 * vid->vb2;
}

static
void st_video_set_shift_mode (st_video_t *vid, unsigned char val)
{
	if (vid->shift_mode == val) {
		return;
	}

	vid->shift_mode = val;
	vid->mode = val & 3;

#if DEBUG_VIDEO >= 1
	st_log_deb ("video: shift mode = 0x%02X\n", val);
#endif

	st_video_set_timing (vid);
}

static
void st_video_set_sync_mode (st_video_t *vid, unsigned char val)
{
	if (vid->sync_mode == val) {
		return;
	}

	vid->sync_mode = val;

#if DEBUG_VIDEO >= 1
	st_log_deb ("video: sync mode = 0x%02X\n", val);
#endif

	st_video_set_timing (vid);
}

static
void st_video_set_palette (st_video_t *vid, unsigned idx, unsigned short val)
{
	unsigned char tmp;
	unsigned char *pal;

	vid->palette[idx] = val;

	pal = vid->pal_col[idx];

	pal[0] = (val >> 3) & 0xe0;
	pal[1] = (val << 1) & 0xe0;
	pal[2] = (val << 5) & 0xe0;

	pal[0] |= (pal[0] >> 3) | (pal[0] >> 6);
	pal[1] |= (pal[1] >> 3) | (pal[1] >> 6);
	pal[2] |= (pal[2] >> 3) | (pal[2] >> 6);

	if (idx == 0) {
		tmp = (val & 1) ? 0xff : 0x00;

		vid->pal_mono[0][0] = ~tmp;
		vid->pal_mono[0][1] = ~tmp;
		vid->pal_mono[0][2] = ~tmp;
		vid->pal_mono[1][0] = tmp;
		vid->pal_mono[1][1] = tmp;
		vid->pal_mono[1][2] = tmp;
	}
}

static
unsigned char st_video_get_uint8 (st_video_t *vid, unsigned long addr)
{
	unsigned char val;

	switch (addr) {
	case 0x01:
		val = (vid->base >> 16) & 0xff;
		break;

	case 0x03:
		val = (vid->base >> 8) & 0xff;
		break;

	case 0x05:
		val = (vid->addr >> 16) & 0xff;
		break;

	case 0x07:
		val = (vid->addr >> 8) & 0xff;
		break;

	case 0x09:
		val = vid->addr & 0xff;
		break;

	case 0x0a:
		val = vid->sync_mode;
		break;

	case 0x0d:
		/* vaddr low byte */
		val = 0;
		break;

	case 0x40: /* palette */
	case 0x5f:
		val = 0;
		break;

	case 0x60:
		val = vid->shift_mode;
		break;

	default:
		st_log_deb ("video: get 8: %06lX -> %04X\n", addr, 0);
		val = 0;
		break;
	}

	return (val);
}

static
unsigned short st_video_get_uint16 (st_video_t *vid, unsigned long addr)
{
	unsigned short val;

	if ((addr >= 0x0040) && (addr < 0x0060)) {
		val = vid->palette[(addr - 64) >> 1];
	}
	else {
		st_log_deb ("video: get 16: %06lX -> %04X\n", addr, 0);
		val = 0;
	}

	return (val);
}

static
unsigned long st_video_get_uint32 (st_video_t *vid, unsigned long addr)
{
	unsigned long val;

	if ((addr >= 0x40) && (addr < 0x60)) {
		val = st_video_get_uint16 (vid, addr);
		val = (val << 16) | st_video_get_uint16 (vid, addr + 2);
	}
	else {
		val = 0;
		st_log_deb ("video: get 32: %06lX -> %04X\n", addr, 0);
	}

	return (val);
}

static
void st_video_set_uint8 (st_video_t *vid, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0x01:
		vid->base &= 0x00ffff;
		vid->base |= (unsigned long) (val & 0xff) << 16;
#if DEBUG_VIDEO >= 1
		st_log_deb ("video: base = 0x%06lX\n", vid->base);
#endif
		break;

	case 0x03:
		vid->base &= 0xff00ff;
		vid->base |= (val & 0xff) << 8;
#if DEBUG_VIDEO >= 1
		st_log_deb ("video: base = 0x%06lX\n", vid->base);
#endif
		break;

	case 0x0a:
		st_video_set_sync_mode (vid, val);
		break;

	case 0x60:
		st_video_set_shift_mode (vid, val);
		break;
	}
}

static
void st_video_set_uint16 (st_video_t *vid, unsigned long addr, unsigned short val)
{
	if ((addr >= 0x0040) && (addr < 0x0060)) {
		st_video_set_palette (vid, (addr - 64) >> 1, val);
	}
	else {
		st_log_deb ("video: set 16: %06lX <- %04X\n", addr, val);
	}
}

static
void st_video_set_uint32 (st_video_t *vid, unsigned long addr, unsigned long val)
{
	if (addr == 0) {
		vid->base = (val & 0xff0000) | ((val << 8) & 0x00ff00);
#if DEBUG_VIDEO >= 1
		st_log_deb ("video: base = 0x%06lX\n", vid->base);
#endif
	}
	else if ((addr >= 0x40) && (addr < 0x60)) {
		st_video_set_uint16 (vid, addr, val >> 16);
		st_video_set_uint16 (vid, addr + 2, val);
	}
	else {
		st_log_deb ("video: set 32: %06lX <- %04lX\n", addr, val);
	}
}

static
void st_video_update_line_0 (st_video_t *vid)
{
	unsigned            i, j, idx;
	unsigned short      val[4];
	const unsigned char *src, *col;
	unsigned char       *dst;

	if (vid->src == NULL) {
		return;
	}

	src = vid->src;
	dst = vid->dst;

	for (i = 0; i < 20; i++) {
		val[0] = (src[0] << 8) | src[1];
		val[1] = (src[2] << 8) | src[3];
		val[2] = (src[4] << 8) | src[5];
		val[3] = (src[6] << 8) | src[7];

		for (j = 0; j < 16; j++) {
			idx = (val[3] >> 12) & 8;
			idx |= (val[2] >> 13) & 4;
			idx |= (val[1] >> 14) & 2;
			idx |= (val[0] >> 15) & 1;

			val[0] <<= 1;
			val[1] <<= 1;
			val[2] <<= 1;
			val[3] <<= 1;

			col = vid->pal_col[idx];

			dst[0] = col[0];
			dst[1] = col[1];
			dst[2] = col[2];

			dst += 3;
		}

		src += 8;
	}

	vid->src = src;
	vid->dst = dst;
	vid->addr += 160;
}

static
void st_video_update_line_1 (st_video_t *vid)
{
	unsigned            i, j, idx;
	unsigned short      val[2];
	const unsigned char *src, *col;
	unsigned char       *dst;

	if (vid->src == NULL) {
		return;
	}

	src = vid->src;
	dst = vid->dst;

	for (i = 0; i < 40; i++) {
		val[0] = (src[0] << 8) | src[1];
		val[1] = (src[2] << 8) | src[3];

		for (j = 0; j < 16; j++) {
			idx = (val[1] >> 14) & 2;
			idx |= (val[0] >> 15) & 1;

			val[0] <<= 1;
			val[1] <<= 1;

			col = vid->pal_col[idx];

			dst[0] = col[0];
			dst[1] = col[1];
			dst[2] = col[2];

			dst += 3;
		}

		src += 4;
	}

	vid->src = src;
	vid->dst = dst;
	vid->addr += 160;
}

static
void st_video_update_line_2 (st_video_t *vid)
{
	unsigned            i, j;
	unsigned char       msk;
	const unsigned char *src, *col;
	unsigned char       *dst;

	if (vid->src == NULL) {
		return;
	}

	src = vid->src;
	dst = vid->dst;
	msk = 0x80;

	for (i = 0; i < 80; i++) {
		msk = 0x80;

		for (j = 0; j < 8; j++) {
			if (*src & msk) {
				col = vid->pal_mono[0];
			}
			else {
				col = vid->pal_mono[1];
			}

			dst[0] = col[0];
			dst[1] = col[1];
			dst[2] = col[2];

			msk >>= 1;

			dst += 3;
		}

		src += 1;
	}

	vid->src = src;
	vid->dst = dst;
	vid->addr += 80;
}

void st_video_redraw (st_video_t *vid)
{
}

void st_video_reset (st_video_t *vid)
{
	unsigned i;

	vid->base = 0;
	vid->addr = 0;

	vid->sync_mode = 0;
	vid->shift_mode = 0xff;

	vid->clk = 0;
	vid->line = 0;
	vid->frame = 0;

	vid->src = mem_get_ptr (vid->mem, vid->addr, 32768);
	vid->dst = vid->rgb;

	if (vid->mono) {
		vid->w = 640;
		vid->h = 400;
	}
	else {
		vid->w = 320;
		vid->h = 200;
	}

	st_video_set_shift_mode (vid, vid->mono ? 2 : 0);

	for (i = 0; i < 16; i++) {
		vid->palette[i] = 0;
		vid->pal_col[i][0] = 0;
		vid->pal_col[i][1] = 0;
		vid->pal_col[i][2] = 0;
	}

	for (i = 0; i < 2; i++) {
		vid->pal_mono[i][0] = 0;
		vid->pal_mono[i][1] = 0;
		vid->pal_mono[i][2] = 0;
	}
}

static
void st_video_update_terminal (st_video_t *vid)
{
	if (vid->trm == NULL) {
		return;
	}

	trm_set_size (vid->trm, vid->w, vid->h);
	trm_set_lines (vid->trm, vid->rgb, 0, vid->h);
	trm_update (vid->trm);
}

void st_video_clock (st_video_t *vid, unsigned cnt)
{
	vid->clk += cnt;

	if (vid->clk < vid->hb1) {
		if (vid->hb_val) {
			st_video_set_hb (vid, 0);
		}

		return;
	}

	if (vid->clk < vid->hb2) {
		if (vid->hb_val == 0) {
			st_video_set_hb (vid, 1);

			if (vid->line < vid->vb1) {
				if (vid->frame_skip == 0) {
					if (vid->mode == 0) {
						st_video_update_line_0 (vid);
					}
					else if (vid->mode == 1) {
						st_video_update_line_1 (vid);
					}
					else if (vid->mode == 2) {
						st_video_update_line_2 (vid);
					}
				}
				else {
					if ((vid->mode == 0) || (vid->mode == 1)) {
						vid->addr += 160;
					}
					else if (vid->mode == 2) {
						vid->addr += 80;
					}
				}
			}
		}

		return;
	}

	vid->clk -= vid->hb2;
	vid->line += 1;

	if (vid->line <= vid->vb1) {
		if (vid->vb_val) {
			st_video_set_vb (vid, 0);
		}

		return;
	}

	if (vid->line <= vid->vb2) {
		if (vid->vb_val == 0) {
			st_video_set_vb (vid, 1);

			if (vid->frame_skip == 0) {
				st_video_update_terminal (vid);
			}

			vid->addr = vid->base;
		}

		return;
	}

	vid->frame += 1;

	if (vid->frame_skip > 0) {
		vid->frame_skip -= 1;
	}
	else {
		vid->frame_skip = vid->frame_skip_max;
	}

	st_video_set_vb (vid, 0);

	if (vid->mode == 0) {
		vid->w = 320;
		vid->h = 200;
	}
	else if (vid->mode == 1) {
		vid->w = 640;
		vid->h = 200;
	}
	else if (vid->mode == 2) {
		vid->w = 640;
		vid->h = 400;
	}
	else {
		vid->w = 640;
		vid->h = 400;
	}

	vid->line = 0;
	vid->src = mem_get_ptr (vid->mem, vid->addr, 32768);
	vid->dst = vid->rgb;
}
