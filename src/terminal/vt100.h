/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/vt100.h                                       *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-11-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: vt100.h,v 1.6 2003/11/18 00:31:39 hampa Exp $ */


#ifndef PCE_TERMINAL_VT100_H
#define PCE_TERMINAL_VT100_H 1


#include <stdio.h>

#include <libini/libini.h>
#include <terminal/terminal.h>


#define VT100_KEY_MAX 16


typedef struct vt100_keymap_t {
  unsigned      key_cnt;
  unsigned char key[VT100_KEY_MAX];

  unsigned      seq_cnt;
  unsigned char seq[VT100_KEY_MAX];
} vt100_keymap_t;


typedef struct {
  terminal_t     trm;

  unsigned       fg;
  unsigned       bg;

  unsigned       scn_x;
  unsigned       scn_y;

  unsigned       crs_x;
  unsigned       crs_y;

  char           crs_on;
  unsigned       crs_y1;
  unsigned       crs_y2;

  unsigned       key_cnt;
  vt100_keymap_t *keymap;

  int            fd_inp;
  int            fd_out;
} vt100_t;


void vt100_init (vt100_t *vt, ini_sct_t *ini, int inp, int out);
terminal_t *vt100_new (ini_sct_t *ini, int inp, int out);

void vt100_free (vt100_t *vt);
void vt100_del (vt100_t *vt);

void vt100_set_size (vt100_t *vt, unsigned m, unsigned w, unsigned h);
void vt100_set_col (vt100_t *vt, unsigned fg, unsigned bg);
void vt100_set_crs (vt100_t *vt, unsigned y1, unsigned y2, int show);
void vt100_set_pos (vt100_t *vt, unsigned x, unsigned y);
void vt100_set_chr (vt100_t *vt, unsigned x, unsigned y, unsigned char c);
void vt100_check (vt100_t *vt);


#endif
