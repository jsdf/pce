/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6560.h                                          *
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


/* MOS 6560/6561 VIC */


#ifndef PCE_CHIPSET_E6560_H
#define PCE_CHIPSET_E6560_H 1


#include <stdint.h>


typedef struct {
	unsigned long cnt;
	unsigned long div;
	short         smp;
} e6560_chn_t;


typedef struct {
	void *hsync_ext;
	void (*hsync) (void *ext, unsigned y, unsigned w, const unsigned char *buf);

	void *vsync_ext;
	void (*vsync) (void *ext);

	void *sound_ext;
	void (*sound) (void *ext, uint16_t *buf, unsigned cnt, int done);

	const unsigned char *memmap[64];
	const unsigned char *colram;

	unsigned char reg[16];

	unsigned      w;
	unsigned      h;

	unsigned      x;
	unsigned      y;
	unsigned      start_x;
	unsigned      start_y;

	unsigned      col;
	unsigned      row;
	unsigned      col_cnt;
	unsigned      row_cnt;

	unsigned      line;
	unsigned      line_cnt;
	unsigned      line_shift;

	unsigned      vbase;
	unsigned      cbase;

	unsigned      addr1;
	unsigned      addr2;

	unsigned char next;
	unsigned char chr;
	unsigned char color;

	unsigned long frame;

	unsigned      reverse;
	unsigned char colmap1[2];
	unsigned char colmap2[4];

	unsigned char *ptr;
	unsigned char buf[284];

	unsigned      snd_enable;
	unsigned long snd_rem;
	unsigned long snd_lfsr;

	unsigned long snd_clk_inp;
	unsigned long snd_clk_out;
	unsigned long snd_clk_cnt;

	e6560_chn_t   chn[4];

	unsigned      snd_buf_cnt;
	unsigned      snd_buf_max;
	uint16_t      snd_buf[256];
} e6560_t;


void e6560_init (e6560_t *vic);
void e6560_free (e6560_t *vic);

void e6560_set_hsync_fct (e6560_t *vic, void *ext, void *fct);
void e6560_set_vsync_fct (e6560_t *vic, void *ext, void *fct);
void e6560_set_sound_fct (e6560_t *vic, void *ext, void *fct);

void e6560_set_memmap (e6560_t *vic, unsigned pa, unsigned pn, const unsigned char *ptr);
void e6560_set_colram (e6560_t *vic, const unsigned char *ptr);

void e6560_set_pal (e6560_t *vic, int val);

void e6560_set_clock (e6560_t *vic, unsigned long val);

void e6560_set_srate (e6560_t *vic, unsigned long val);

unsigned char e6560_get_reg (e6560_t *vic, unsigned long addr);
void e6560_set_reg (e6560_t *vic, unsigned long addr, unsigned char val);

void e6560_reset (e6560_t *vic);

void e6560_clock (e6560_t *vic);


#endif
