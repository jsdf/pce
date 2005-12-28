/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/video/vga.h                                    *
 * Created:       2004-03-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_IBMPC_VGA_H
#define PCE_IBMPC_VGA_H 1


#include <libini/libini.h>
#include <terminal/terminal.h>
#include <devices/video/video.h>


typedef struct vga_t {
	video_t       vid;

	mem_blk_t     *mem;
	mem_blk_t     *reg;

	unsigned char *data;

	unsigned long base;
	unsigned long size;

	unsigned      mode_80x25_w;
	unsigned      mode_80x25_h;
	unsigned      mode_320x200_w;
	unsigned      mode_320x200_h;
	unsigned      mode_640x200_w;
	unsigned      mode_640x200_h;
	unsigned      mode_640x350_w;
	unsigned      mode_640x350_h;
	unsigned      mode_640x480_w;
	unsigned      mode_640x480_h;

	unsigned char crtc_reg[24];
	unsigned char ts_reg[5];
	unsigned char gdc_reg[9];
	unsigned char atc_reg[21];
	unsigned char dac_reg[768];

	int           atc_index;

	unsigned      dac_idx;
	unsigned char dac_col_msk;
	unsigned char dac_state;

	unsigned char latch[4];

	void          (*update) (struct vga_t *vga);
	int           (*screenshot) (struct vga_t *vga, FILE *fp);
	void          (*set_latches) (struct vga_t *vga, unsigned long addr, unsigned char val[4]);
	void          (*set_uint8) (struct vga_t *vga, unsigned long addr, unsigned char val);
	unsigned char (*get_uint8) (struct vga_t *vga, unsigned long addr);

	unsigned      crtc_pos;
	unsigned      crtc_ofs;

	int           crs_on;

	unsigned      mode;
	unsigned      mode_w;
	unsigned      mode_h;

	char          dirty;

	terminal_t    *trm;
} vga_t;


video_t *vga_new (terminal_t *trm, ini_sct_t *sct);

void vga_del (vga_t *vga);

void vga_prt_state (vga_t *vga, FILE *fp);

int vga_dump (vga_t *vga, FILE *fp);

mem_blk_t *vga_get_mem (vga_t *cga);
mem_blk_t *vga_get_reg (vga_t *cga);

int vga_screenshot (vga_t *vga, FILE *fp, unsigned mode);

void vga_mem_set_uint8 (vga_t *vga, unsigned long addr, unsigned char val);
void vga_mem_set_uint16 (vga_t *vga, unsigned long addr, unsigned short val);
unsigned char vga_mem_get_uint8 (vga_t *vga, unsigned long addr);
unsigned short vga_mem_get_uint16 (vga_t *vga, unsigned long addr);

void vga_reg_set_uint8 (vga_t *vga, unsigned long addr, unsigned char val);
void vga_reg_set_uint16 (vga_t *vga, unsigned long addr, unsigned short val);
unsigned char vga_reg_get_uint8 (vga_t *vga, unsigned long addr);
unsigned short vga_reg_get_uint16 (vga_t *vga, unsigned long addr);

void vga_clock (vga_t *vga, unsigned long cnt);


#endif
