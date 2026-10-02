/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6560.c                                          *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#include <stdlib.h>
#include <stdio.h>

#include "e6560.h"


void e6560_init (e6560_t *vic)
{
	unsigned i;

	vic->hsync_ext = NULL;
	vic->hsync = NULL;

	vic->vsync_ext = NULL;
	vic->vsync = NULL;

	vic->sound_ext = NULL;
	vic->sound = NULL;

	for (i = 0; i < 64; i++) {
		vic->memmap[i] = NULL;
	}

	vic->colram = NULL;

	vic->w = 260;
	vic->h = 261;

	vic->frame = 0;

	vic->ptr = vic->buf;

	vic->snd_enable = 0;
	vic->snd_rem = 0;

	vic->snd_clk_inp = 1000000;
	vic->snd_clk_out = 44100;
	vic->snd_clk_cnt = 0;

	vic->snd_buf_cnt = 0;
	vic->snd_buf_max = 256;
}

void e6560_free (e6560_t *vic)
{
	if (vic->sound != NULL) {
		vic->sound (vic->sound_ext, vic->snd_buf, 0, 1);
	}
}

void e6560_set_hsync_fct (e6560_t *vic, void *ext, void *fct)
{
	vic->hsync_ext = ext;
	vic->hsync = fct;
}

void e6560_set_vsync_fct (e6560_t *vic, void *ext, void *fct)
{
	vic->vsync_ext = ext;
	vic->vsync = fct;
}

void e6560_set_sound_fct (e6560_t *vic, void *ext, void *fct)
{
	vic->sound_ext = ext;
	vic->sound = fct;
}

void e6560_set_memmap (e6560_t *vic, unsigned pa, unsigned pn, const unsigned char *ptr)
{
	while ((pa < 64) && (pn > 0)) {
		vic->memmap[pa] = ptr;

		pa += 1;
		pn -= 1;

		if (ptr != NULL) {
			ptr += 256;
		}
	}
}

void e6560_set_colram (e6560_t *vic, const unsigned char *ptr)
{
	vic->colram = ptr;
}

void e6560_set_pal (e6560_t *vic, int val)
{
	if (val) {
		vic->w = 284;
		vic->h = 312;
	}
	else {
		vic->w = 260;
		vic->h = 261;
	}
}

void e6560_set_clock (e6560_t *vic, unsigned long val)
{
	vic->snd_clk_inp = val;
}

void e6560_set_srate (e6560_t *vic, unsigned long val)
{
	if (val == 0) {
		return;
	}

	vic->snd_clk_out = val;
	vic->snd_clk_cnt = 0;
}

static
void vic_sound_start (e6560_t *vic)
{
	unsigned i;

	vic->snd_buf_cnt = 0;
	vic->snd_rem = 2 * vic->snd_clk_out;
	vic->snd_clk_cnt = 0;

	if (vic->sound == NULL) {
		return;
	}

	for (i = 0; i < vic->snd_buf_max; i++) {
		vic->snd_buf[i] = 0x8000;
	}

	vic->sound (vic->sound_ext, vic->snd_buf, vic->snd_buf_max, 0);
	vic->sound (vic->sound_ext, vic->snd_buf, vic->snd_buf_max, 0);
}

static
void vic_sound_stop (e6560_t *vic)
{
	if (vic->sound != NULL) {
		vic->sound (vic->sound_ext, vic->snd_buf, vic->snd_buf_cnt, 1);
	}

	vic->snd_buf_cnt = 0;
}

static
void vic_sound_enable (e6560_t *vic, unsigned chn, unsigned char val)
{
	e6560_chn_t *c;

	c = vic->chn + (chn & 3);

	if (val & 0x80) {
		vic->snd_enable |= 1 << chn;

		c->div = (128 - ((val + 1) & 0x7f)) << (7 - chn);
		c->cnt = c->div;
		c->smp = -64;

		if (vic->snd_rem == 0) {
			vic_sound_start (vic);
		}
	}
	else {
		vic->snd_enable &= ~(1 << chn);

		c->smp = 0;
	}
}

static
void vic_sound_out (e6560_t *vic)
{
	int         val;
	uint16_t    tmp;
	e6560_chn_t *chn;

	if (vic->snd_rem == 0) {
		return;
	}

	vic->snd_clk_cnt += vic->snd_clk_out;

	if (vic->snd_clk_cnt < vic->snd_clk_inp) {
		return;
	}

	vic->snd_clk_cnt -= vic->snd_clk_inp;

	chn = vic->chn;

	val = (chn[0].smp + chn[1].smp + chn[2].smp + chn[3].smp);
	val *= ((vic->reg[14] & 15) + 1) * 4;
	tmp = val;

	vic->snd_buf[vic->snd_buf_cnt++] = tmp ^ 0x8000;

	if (vic->snd_buf_cnt >= vic->snd_buf_max) {
		if (vic->sound != NULL) {
			vic->sound (vic->sound_ext, vic->snd_buf, vic->snd_buf_max, 0);
		}

		vic->snd_buf_cnt = 0;
	}

	if (--vic->snd_rem == 0) {
		vic_sound_stop (vic);
	}
}

static
void vic_sound_clock (e6560_t *vic)
{
	unsigned    i;
	e6560_chn_t *chn;

	if (vic->snd_enable == 0) {
		return;
	}

	vic->snd_rem = 2 * vic->snd_clk_out;

	for (i = 0; i < 4; i++) {
		if (~vic->snd_enable & (1 << i)) {
			continue;
		}

		chn = vic->chn + i;

		if (--chn->cnt > 0) {
			continue;
		}

		chn->cnt = chn->div;
		chn->smp = -chn->smp;

		if (i == 3) {
			if (vic->snd_lfsr & 1) {
				vic->snd_lfsr = (vic->snd_lfsr >> 1) ^ 0x80000057;
				chn->smp = -chn->smp;
			}
			else {
				vic->snd_lfsr = vic->snd_lfsr >> 1;
			}
		}
	}
}

static
void vic_update_line (e6560_t *vic)
{
	vic->reg[3] = (vic->reg[3] & 0x7f) | ((vic->y & 1) << 7);
	vic->reg[4] = vic->y >> 1;
}

static
void vic_update_colmap (e6560_t *vic)
{
	vic->reverse = (vic->reg[15] >> 3) & 1;

	vic->colmap1[vic->reverse] = 0;
	vic->colmap1[vic->reverse ^ 1] = (vic->reg[15] >> 4) & 0x0f;

	vic->colmap2[0] = (vic->reg[15] >> 4) & 0x0f;
	vic->colmap2[1] = vic->reg[15] & 0x07;
	vic->colmap2[2] = 0;
	vic->colmap2[3] = (vic->reg[14] >> 4) & 0x0f;
}

static
void vic_update_vaddr (e6560_t *vic)
{
	vic->vbase = (vic->reg[5] << 6) & 0x3c00;
	vic->vbase |= (vic->reg[2] << 2) & 0x200;
	vic->cbase = (vic->reg[5] << 10) & 0x3c00;
}

unsigned char e6560_get_reg (e6560_t *vic, unsigned long addr)
{
	if ((addr == 3) || (addr == 4)) {
		vic_update_line (vic);
	}

	if (addr < 16) {
		return (vic->reg[addr]);
	}

	return (0xaa);
}

void e6560_set_reg (e6560_t *vic, unsigned long addr, unsigned char val)
{
	if (addr >= 16) {
		return;
	}

	vic->reg[addr] = val;

	if (addr == 0) {
		vic->start_x = (val & 0x7f) << 2;
	}

	if (addr == 1) {
		vic->start_y = (val & 0xff) << 1;
	}

	if ((addr == 2) || (addr == 5)) {
		vic_update_vaddr (vic);
	}

	if (addr == 3) {
		vic->line_shift = 3 + (val & 1);
		vic->line_cnt = 1 << vic->line_shift;
	}

	if ((addr == 14) || (addr == 15)) {
		vic_update_colmap (vic);
	}

	if ((addr >= 10) && (addr <= 13)) {
		vic_sound_enable (vic, addr - 10, vic->reg[addr]);
	}
}

void e6560_reset (e6560_t *vic)
{
	unsigned i;

	for (i = 0; i < 16; i++) {
		vic->reg[i] = 0;
	}

	vic->x = 0;
	vic->y = 0;

	vic->start_x = 0;
	vic->start_y = 0;

	vic->col = 0;
	vic->row = 0;
	vic->col_cnt = 0;
	vic->row_cnt = 0;

	vic->line = 0;
	vic->line_cnt = 8;
	vic->line_shift = 3;

	vic->vbase = 0;
	vic->cbase = 0;

	vic->addr1 = 0;
	vic->addr2 = 0;

	vic->next = 0;

	vic->ptr = vic->buf;

	vic->colmap1[0] = 0;
	vic->colmap1[1] = 0;

	for (i = 0; i < 4; i++) {
		vic->colmap2[i] = 0;
	}

	vic->snd_enable = 0;
	vic->snd_lfsr = 1;
	vic->snd_clk_cnt = 0;

	for (i = 0; i < 4; i++) {
		vic->chn[i].cnt = 0;
		vic->chn[i].div = 0;
		vic->chn[i].smp = 0;
	}
}

static
unsigned char vic_fetch_data (e6560_t *vic, unsigned addr)
{
	const unsigned char *ptr;

	ptr = vic->memmap[(addr & 16383) >> 8];

	if (ptr != NULL) {
		return (ptr[addr & 0xff]);
	}

	return (0);
}

static
unsigned char vic_fetch_color (e6560_t *vic, unsigned addr)
{
	if (vic->colram == NULL) {
		return (0);
	}

	return (vic->colram[addr & 0x3ff]);
}

static
void e6560_clock_char (e6560_t *vic)
{
	unsigned      i;
	unsigned      addr;
	unsigned char val, col;
	unsigned char *map;

	if ((vic->col_cnt == 0) || (vic->row_cnt == 0)) {
		/* in border */

		val = vic->reg[15] & 7;

		for (i = 0; i < 4; i++) {
			*(vic->ptr++) = val;
		}

		return;
	}

	if (vic->next == 0) {
		/* fetch character code */

		vic->chr = vic_fetch_data (vic, vic->vbase + vic->addr2);
		vic->color = vic_fetch_color (vic, vic->vbase + vic->addr2);

		vic->next = 1;
		vic->addr2 += 1;

		return;
	}

	/* update one line of one character */

	addr = vic->cbase + (vic->chr << vic->line_shift) + vic->line;
	val = vic_fetch_data (vic, addr);

	vic->next = 0;
	vic->col += 1;
	vic->col_cnt -= 1;

	if (vic->color & 0x08) {
		map = vic->colmap2;
		map[2] = vic->color & 7;

		for (i = 0; i < 4; i++) {
			col = map[(val >> 6) & 3];
			*(vic->ptr++) = col;
			*(vic->ptr++) = col;
			val <<= 2;
		}
	}
	else {
		map = vic->colmap1;
		map[vic->reverse] = vic->color & 7;

		for (i = 0; i < 8; i++) {
			*(vic->ptr++) = map[(val >> 7) & 1];
			val <<= 1;
		}
	}
}

void e6560_clock (e6560_t *vic)
{
	if (vic->snd_rem > 0) {
		vic_sound_clock (vic);
		vic_sound_out (vic);
	}

	if (vic->x == vic->start_x) {
		vic->col_cnt = vic->reg[2] & 0x7f;
	}

	e6560_clock_char (vic);

	vic->x += 4;

	if (vic->x < vic->w) {
		return;
	}

	if (vic->hsync != NULL) {
		vic->hsync (vic->hsync_ext, vic->y, vic->w, vic->buf);
	}

	vic->x = 0;
	vic->next = 0;
	vic->ptr = vic->buf;
	vic->col = 0;
	vic->col_cnt = 0;

	vic->line += 1;

	if (vic->line >= vic->line_cnt) {
		vic->line = 0;

		if (vic->row_cnt > 0) {
			vic->row += 1;
			vic->row_cnt -= 1;
		}

		vic->addr1 = vic->addr2;
	}
	else {
		vic->addr2 = vic->addr1;
	}

	vic->y += 1;

	if (vic->y == vic->start_y) {
		vic->row = 0;
		vic->line = 0;
		vic->row_cnt = (vic->reg[3] >> 1) & 0x3f;
		vic->addr1 = 0;
		vic->addr2 = 0;
	}

	if (vic->y < vic->h) {
		return;
	}

	if (vic->vsync != NULL) {
		vic->vsync (vic->vsync_ext);
	}

	vic->y = 0;
	vic->row = 0;
	vic->line = 0;
	vic->row_cnt = 0;
	vic->addr1 = 0;
	vic->addr2 = 0;
	vic->frame += 1;
}
