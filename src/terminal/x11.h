/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/x11.h                                         *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-05-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_TERMINAL_X11_H
#define PCE_TERMINAL_X11_H 1


#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <terminal/terminal.h>
#include <libini/libini.h>


typedef struct {
  terminal_t    trm;

  char          init_display;
  char          init_font;
  char          init_window;
  char          init_cursor;
  char          init_gc;

  Display       *display;
  int           screen;
  int           display_w;
  int           display_h;

  Window        wdw;
  GC            gc;

  Pixmap        back;
  GC            back_gc;

  GC            crs_gc;

  Cursor        crs_none;

  unsigned      mode;

  XFontStruct   *font;
  int           font_w;
  int           font_h;
  int           font_d;

  unsigned      wdw_w;
  unsigned      wdw_h;

  unsigned      scn_w;
  unsigned      scn_h;
  unsigned char *scn;

  unsigned      flush_x1;
  unsigned      flush_y1;
  unsigned      flush_x2;
  unsigned      flush_y2;

  char          crs_on;
  unsigned      crs_y1;
  unsigned      crs_y2;
  unsigned      crs_x;
  unsigned      crs_y;
  unsigned      crs_fg;

  int           grab;

  int           mse_x;
  int           mse_y;

  unsigned      fg;
  unsigned      bg;

  XColor        col[256];
} xterm_t;


int xt_init (xterm_t *xt, ini_sct_t *ini);
terminal_t *xt_new (ini_sct_t *ini);

void xt_free (xterm_t *xt);
void xt_del (xterm_t *xt);

void xt_set_mode (xterm_t *xt, unsigned m, unsigned w, unsigned h);
void xt_set_map (xterm_t *xt, unsigned idx, unsigned r, unsigned g, unsigned b);
void xt_set_col (xterm_t *xt, unsigned fg, unsigned bg);
void xt_set_crs (xterm_t *xt, unsigned y1, unsigned y2, int show);
void xt_set_pos (xterm_t *xt, unsigned x, unsigned y);
void xt_set_chr (xterm_t *xt, unsigned x, unsigned y, unsigned char c);
void xt_set_pxl (xterm_t *xt, unsigned x, unsigned y, unsigned w, unsigned h);
void xt_clear (xterm_t *xt);
void xt_check (xterm_t *xt);


#endif
