/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/ega.h                                      *
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


#ifndef PCE_VIDEO_EGA_H
#define PCE_VIDEO_EGA_H 1


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>


typedef struct ega_s {
	video_t       video;

	terminal_t    *term;

	mem_blk_t     *memblk;
	unsigned char *mem;

	mem_blk_t     *regblk;
	unsigned char *reg;

	unsigned char reg_seq[5];
	unsigned char reg_grc[9];
	unsigned char reg_atc[22];
	unsigned char reg_crt[25];

	unsigned      latch_addr;
	unsigned      latch_hpp;

	char          atc_flipflop;

	unsigned char latch[4];

	/* the switch settings */
	unsigned char switches;

	/* the monitor type, derived from the switches */
	unsigned      monitor;

	char          blink_on;
	unsigned      blink_cnt;
	unsigned      blink_freq;

	/* these are derived from the crtc registers */
	unsigned long clk_ht;
	unsigned long clk_hd;
	unsigned long clk_vt;
	unsigned long clk_vd;

	unsigned      buf_w;
	unsigned      buf_h;
	unsigned long bufmax;
	unsigned char *buf;

	unsigned char update_state;

	void          *set_irq_ext;
	void          (*set_irq) (void *ext, unsigned char val);
	unsigned char set_irq_val;
} ega_t;


void ega_set_irq_fct (ega_t *ega, void *ext, void *fct);

void ega_init (ega_t *ega, unsigned long io, unsigned long addr);

void ega_free (ega_t *ega);

ega_t *ega_new (unsigned long io, unsigned long addr);

void ega_del (ega_t *ega);

video_t *ega_new_ini (ini_sct_t *sct);


#endif
