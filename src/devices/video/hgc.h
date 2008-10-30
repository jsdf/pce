/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/hgc.h                                      *
 * Created:     2003-08-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2008 Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id$ */


#ifndef PCE_HGC_H
#define PCE_HGC_H 1


#include <libini/libini.h>
#include <terminal/terminal.h>
#include <terminal/term-old.h>
#include <devices/video/video.h>


typedef struct {
	video_t       vid;

	mem_blk_t     *mem;
	mem_blk_t     *reg;

	unsigned      mode_80x25_w;
	unsigned      mode_80x25_h;

	unsigned      mode_720x348_w;
	unsigned      mode_720x348_h;

	unsigned char crtc_reg[18];

	unsigned      page_ofs;
	unsigned      crtc_pos;
	unsigned      crtc_ofs;

	unsigned char enable_page1;
	unsigned char enable_graph;

	int           crs_on;

	unsigned long rgb[18];

	unsigned      mode;

	terminal_t    *trmnew;
	term_old_t    trm;
	unsigned long trmclk;
} hgc_t;


video_t *hgc_new (terminal_t *trm, ini_sct_t *ini);

void hgc_del (hgc_t *cga);

void hgc_clock (hgc_t *cga, unsigned long cnt);

void hgc_prt_state (hgc_t *cga, FILE *fp);

int hgc_dump (hgc_t *hgc, FILE *fp);

mem_blk_t *hgc_get_mem (hgc_t *hgc);
mem_blk_t *hgc_get_reg (hgc_t *hgc);

int hgc_screenshot (hgc_t *hgc, FILE *fp, unsigned mode);

void hgc_update (hgc_t *hgc);

void hgc_mem_set_uint8 (hgc_t *hgc, unsigned long addr, unsigned char val);
void hgc_mem_set_uint16 (hgc_t *hgc, unsigned long addr, unsigned short val);

void hgc_reg_set_uint8 (hgc_t *hgc, unsigned long addr, unsigned char val);
void hgc_reg_set_uint16 (hgc_t *hgc, unsigned long addr, unsigned short val);
unsigned char hgc_reg_get_uint8 (hgc_t *hgc, unsigned long addr);
unsigned short hgc_reg_get_uint16 (hgc_t *hgc, unsigned long addr);


#endif
