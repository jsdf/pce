/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/cga.h                                      *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_CGA_H
#define PCE_VIDEO_CGA_H 1


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>


typedef struct cga_s {
	video_t       video;

	void          *ext;

	/* update the internal screen buffer */
	void          (*update) (struct cga_s *ext);

	mem_blk_t     *memblk;
	unsigned char *mem;

	mem_blk_t     *regblk;
	unsigned char *reg;

	terminal_t    *term;

	unsigned char reg_crt[18];

	unsigned char pal[4];

	unsigned char *font;

	/* composite mode color lookup table */
	char          comp_tab_ok;
	unsigned char *comp_tab;

	char          blink_on;
	unsigned      blink_cnt;
	unsigned      blink_freq;

	/* these are derived from the crtc registers */
	unsigned      w;
	unsigned      h;
	unsigned      ch;

	unsigned long clk_ht;
	unsigned long clk_vt;
	unsigned long clk_hd;
	unsigned long clk_vd;

	unsigned      buf_w;
	unsigned      buf_h;
	unsigned long bufmax;
	unsigned char *buf;

	unsigned char update_state;
} cga_t;


extern unsigned char cga_rgb[16][3];


void cga_set_blink_rate (cga_t *cga, unsigned freq);
unsigned cga_get_start (cga_t *cga);
unsigned cga_get_cursor (cga_t *cga);
int cga_get_position (cga_t *cga, unsigned *x, unsigned *y);
int cga_set_buf_size (cga_t *cga, unsigned w, unsigned h);
void cga_update (cga_t *cga);
unsigned char cga_reg_get_uint8 (cga_t *cga, unsigned long addr);
unsigned short cga_reg_get_uint16 (cga_t *cga, unsigned long addr);
void cga_reg_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val);
void cga_reg_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val);


void cga_init (cga_t *cga, unsigned long io, unsigned long addr, unsigned long size);

void cga_free (cga_t *cga);

cga_t *cga_new (unsigned long io, unsigned long addr, unsigned long size);

void cga_del (cga_t *cga);

video_t *cga_new_ini (ini_sct_t *sct);


#endif
