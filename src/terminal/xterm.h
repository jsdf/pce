/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/xterm.h                                       *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-25 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: xterm.h,v 1.2 2003/04/25 02:30:18 hampa Exp $ */


#ifndef PCE_TERMINAL_XTERM_H
#define PCE_TERMINAL_XTERM_H 1


#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <terminal/terminal.h>


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
  GC            crs_gc;

  XFontStruct   *font;
  int           font_w;
  int           font_h;
  int           font_d;

  unsigned      wdw_w;
  unsigned      wdw_h;

  unsigned      scn_w;
  unsigned      scn_h;
  unsigned char *scn;

  unsigned      crs_y1;
  unsigned      crs_y2;
  unsigned      crs_x;
  unsigned      crs_y;
  Pixmap        crs_bg;

  unsigned      fg;
  unsigned      bg;

  XColor        col[16];
} xterm_t;


int xt_init (xterm_t *xt);
terminal_t *xt_new (void);

void xt_free (xterm_t *xt);
void xt_del (xterm_t *xt);

void xt_set_size (xterm_t *xt, unsigned w, unsigned h);
void xt_set_col (xterm_t *xt, unsigned fg, unsigned bg);
void xt_set_crs (xterm_t *xt, unsigned y1, unsigned y2);
void xt_set_pos (xterm_t *xt, unsigned x, unsigned y);
void xt_set_chr (xterm_t *xt, unsigned x, unsigned y, unsigned char c);
void xt_check (xterm_t *xt);


#endif
