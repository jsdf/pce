/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/vga.h                                      *
 * Created:     2004-03-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_VGA_H
#define PCE_VIDEO_VGA_H 1


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>


typedef struct vga_s {
	video_t       video;

	terminal_t    *term;

	mem_blk_t     *memblk;
	unsigned char *mem;

	mem_blk_t     *regblk;
	unsigned char *reg;

	unsigned char reg_seq[5];
	unsigned char reg_grc[9];
	unsigned char reg_atc[21];
	unsigned char reg_crt[25];
	unsigned char reg_dac[768];

	unsigned      latch_addr;
	unsigned      latch_hpp;

	char          atc_flipflop;

	unsigned      dac_addr_read;
	unsigned      dac_addr_write;
	unsigned char dac_state;

	unsigned char latch[4];

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
} vga_t;


void vga_set_irq_fct (vga_t *ega, void *ext, void *fct);

void vga_init (vga_t *vga, unsigned long io, unsigned long addr);

void vga_free (vga_t *vga);

vga_t *vga_new (unsigned long io, unsigned long addr);

void vga_del (vga_t *vga);

video_t *vga_new_ini (ini_sct_t *sct);


#endif
