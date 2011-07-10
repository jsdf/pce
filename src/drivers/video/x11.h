/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/x11.h                                      *
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


#ifndef PCE_VIDEO_X11_H
#define PCE_VIDEO_X11_H 1


#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <drivers/video/terminal.h>

#include <libini/libini.h>


/*!***************************************************************************
 * @short The X11 terminal structure
 *****************************************************************************/
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
	Window        root;

	Window        wdw;
	GC            gc;

	XImage        *img;
	unsigned char *img_buf;

	Cursor        empty_cursor;

	unsigned      wdw_w;
	unsigned      wdw_h;

	int           mse_x;
	int           mse_y;

	int           grab;
} xterm_t;


/*!***************************************************************************
 * @short Create a new X11 terminal
 *****************************************************************************/
terminal_t *xt_new (ini_sct_t *ini);


#endif
