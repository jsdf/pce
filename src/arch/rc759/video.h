/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/video.h                                       *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_RC759_VIDEO_H
#define PCE_RC759_VIDEO_H 1


#include <drivers/video/terminal.h>


typedef struct {
	char           ready;
	unsigned short cnt;
	unsigned short buf[256];

	unsigned short fullrowdesc_cnt;
	unsigned short fullrowdesc[7];
} e82730_rowbuf_t;

typedef struct {
	unsigned char   scb;
	unsigned long   ibp;
	unsigned long   cbp;
	unsigned long   pixel_mem;

	unsigned short  intmask;

	unsigned short  status;

	char            graphic;
	char            monochrome;

	char            first_ca;
	char            mode_set;
	unsigned char   auto_lf;
	unsigned char   list_switch;
	unsigned short  max_dma;
	unsigned char   frame_int_count[2];
	char            channel_attention;
	char            end_of_frame;

	unsigned short  mode_line_length;
	unsigned short  mode_hfldstrt;
	unsigned short  mode_hfldstp;
	unsigned short  mode_frame_length;
	unsigned short  mode_vfldstrt;
	unsigned short  mode_vfldstp;
	unsigned char   mode_lines_per_row;
	char            blank_row;

	unsigned short  field_attrib;

	char            cursor_enable[2];
	char            cursor_reverse[2];
	char            cursor_blink[2];
	unsigned char   cursor_x[2];
	unsigned char   cursor_y[2];
	unsigned char   cursor_start[2];
	unsigned char   cursor_stop[2];
	unsigned short  cursor_cnt;
	unsigned short  cursor_duty;
	unsigned short  cursor_max;

	unsigned short  cur_line;
	unsigned short  cur_row;
	unsigned short  cur_row_idx;

	unsigned long   lbase;
	unsigned        stridx;
	unsigned long   strptr;

	unsigned long   addr_mask;

	unsigned long   rclk;

	unsigned long   input_clock_mul;
	unsigned long   input_clock_div;

	unsigned char   *rgb;

	unsigned char   *buf;
	unsigned long   buf_cnt;
	unsigned        buf_w;
	unsigned        buf_h;
	unsigned        buf_min_h;
	unsigned        buf_y;

	terminal_t      *trm;

	e82730_rowbuf_t *rbp[2];
	e82730_rowbuf_t rowbuf[2];

	unsigned char   palette[32];

	void            *set_mem_ext;
	void            (*set_mem8) (void *ext, unsigned long addr, unsigned char val);
	void            (*set_mem16) (void *ext, unsigned long addr, unsigned short val);

	void            *get_mem_ext;
	unsigned char   (*get_mem8) (void *ext, unsigned long addr);
	unsigned short  (*get_mem16) (void *ext, unsigned long addr);

	unsigned char   sint_val;
	void            *sint_ext;
	void            (*sint) (void *ext, unsigned char val);
} e82730_t;


void e82730_init (e82730_t *crt);
void e82730_free (e82730_t *crt);

e82730_t *e82730_new (void);
void e82730_del (e82730_t *crt);

void e82730_set_getmem_fct (e82730_t *crt, void *ext, void *get8, void *get16);
void e82730_set_setmem_fct (e82730_t *crt, void *ext, void *set8, void *set16);

void e82730_set_sint_fct (e82730_t *crt, void *ext, void *fct);

void e82730_set_clock (e82730_t *crt, unsigned long mul, unsigned long div);

void e82730_set_terminal (e82730_t *crt, terminal_t *trm);

void e82730_set_monochrome (e82730_t *crt, int val);

void e82730_set_graphic (e82730_t *crt, int val);

void e82730_set_min_h (e82730_t *crt, unsigned val);

void e82730_reset (e82730_t *crt);

void e82730_set_ca (e82730_t *crt);
void e82730_set_srst (e82730_t *crt);

void e82730_set_palette (e82730_t *crt, unsigned idx, unsigned char val);
unsigned char e82730_get_palette (const e82730_t *crt, unsigned idx);

void e82730_clock (e82730_t *crt, unsigned cnt);


#endif
