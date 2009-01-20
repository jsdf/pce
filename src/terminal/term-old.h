/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/terminal/term-old.h                                      *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_TERMINAL_TERM_OLD_H
#define PCE_TERMINAL_TERM_OLD_H 1


#include <stdio.h>


#define TERM_MODE_TEXT  0
#define TERM_MODE_GRAPH 1


typedef void (*trm_del_f) (void *ext);
typedef void (*trm_set_mse_f) (void *ext, int dx, int dy, unsigned b);
typedef void (*trm_set_mode_f) (void *ext, unsigned m, unsigned w, unsigned h);
typedef void (*trm_set_size_f) (void *ext, unsigned w, unsigned h);
typedef void (*trm_set_map_f) (void *ext, unsigned i, unsigned r, unsigned g, unsigned b);
typedef void (*trm_set_col_f) (void *ext, unsigned fg, unsigned bg);
typedef void (*trm_set_crs_f) (void *ext, unsigned y1, unsigned y2, int show);
typedef void (*trm_set_pos_f) (void *ext, unsigned x, unsigned y);
typedef void (*trm_set_chr_f) (void *ext, unsigned x, unsigned y, unsigned char c);
typedef void (*trm_set_pxl_f) (void *ext, unsigned x, unsigned y);
typedef void (*trm_set_rct_f) (void *ext, unsigned x, unsigned y, unsigned w, unsigned h);
typedef void (*trm_check_f) (void *ext);


typedef struct {
	void (*del) (void *ext);

	void *msg_ext;
	int  (*set_msg) (void *ext, const char *msg, const char *val);
	int  (*get_msgul) (void *ext, const char *msg, unsigned long *val);

	void *key_ext;
	void (*set_key) (void *ext, unsigned char val);

	void *mse_ext;
	void (*set_mse) (void *ext, int dx, int dy, unsigned b);

	trm_set_mode_f set_mode;
	trm_set_size_f set_size;

	trm_set_map_f  set_map;
	trm_set_col_f  set_col;
	trm_set_crs_f  set_crs;
	trm_set_pos_f  set_pos;

	trm_set_chr_f  set_chr;
	trm_set_pxl_f  set_pxl;
	trm_set_rct_f  set_rct;

	trm_check_f    check;

	terminal_t     *trm;

	unsigned       mode;
	unsigned       w;
	unsigned       h;
	unsigned char  *gbuf;
	unsigned char  *tbuf;

	unsigned char  colmap[3 * 16];

	unsigned       cursor_x;
	unsigned       cursor_y;
	unsigned       cursor_y1;
	unsigned       cursor_y2;
	int            cursor_show;

	unsigned       fg;
	unsigned       bg;

	unsigned       update_y1;
	unsigned       update_y2;

	void           *ext;
} term_old_t;


void trm_old_init (term_old_t *trm, terminal_t *trmnew);
void trm_old_free (term_old_t *trm);

void trm_old_del (term_old_t *trm);

void trm_old_set_msg_fct (term_old_t *trm, void *ext, void *fct);
void trm_old_set_key_fct (term_old_t *trm, void *ext, void *fct);

int trm_old_set_msg (term_old_t *trm, const char *msg, const char *val);

void trm_old_set_mode (term_old_t *trm, unsigned m, unsigned w, unsigned h);
void trm_old_set_size (term_old_t *trm, unsigned w, unsigned h);

void trm_old_set_map (term_old_t *trm, unsigned i, unsigned r, unsigned g, unsigned b);
void trm_old_set_col (term_old_t *trm, unsigned fg, unsigned bg);

void trm_old_set_crs (term_old_t *trm, unsigned y1, unsigned y2, int show);

void trm_old_set_pos (term_old_t *trm, unsigned x, unsigned y);

void trm_old_set_chr (term_old_t *trm, unsigned x, unsigned y, unsigned char c);

void trm_old_set_pxl (term_old_t *trm, unsigned x, unsigned y);

void trm_old_set_rct (term_old_t *trm, unsigned x, unsigned y, unsigned w, unsigned h);

void trm_old_update (term_old_t *trm);


#endif
