/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/x11.c                                         *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-21 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: x11.c,v 1.7 2003/09/21 04:04:22 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <terminal/terminal.h>
#include <terminal/x11.h>


static
struct {  unsigned short r, g, b; } rgb[16] = {
  { 0x0000, 0x0000, 0x0000 },
  { 0x0a0a, 0x0a0a, 0xb9b9 },
  { 0x0a0a, 0xc3c3, 0x0a0a },
  { 0x1414, 0xa0a0, 0xa0a0 },
  { 0xa7a7, 0x0a0a, 0x0a0a },
  { 0xa7a7, 0x0000, 0xa7a7 },
  { 0xa5a5, 0xa5a5, 0x2828 },
  { 0xc5c5, 0xc5c5, 0xc5c5 },
  { 0x6464, 0x6464, 0x6464 },
  { 0x0a0a, 0x0a0a, 0xffff },
  { 0x0a0a, 0xffff, 0x0a0a },
  { 0x0a0a, 0xffff, 0xffff },
  { 0xffff, 0x0a0a, 0x0a0a },
  { 0xffff, 0x0a0a, 0xffff },
  { 0xffff, 0xffff, 0x0000 },
  { 0xffff, 0xffff, 0xffff }
};

static
unsigned char col_alloc[256];


static
int xt_init_colors (xterm_t *xt)
{
  unsigned i;

  for (i = 0; i < 16; i++) {
    xt->col[i].flags = DoRed | DoGreen | DoBlue;

    xt->col[i].red = rgb[i].r;
    xt->col[i].green = rgb[i].g;
    xt->col[i].blue = rgb[i].b;

    XAllocColor (xt->display, DefaultColormap (xt->display, xt->screen), &xt->col[i]);
    col_alloc[i] = 1;
  }

  for (i = 16; i < 256; i++) {
    col_alloc[i] = 0;
  }

  return (0);
}

int xt_init_cursor (xterm_t *xt)
{
  xt->init_cursor = 1;

  xt->crs_on = 0;

  xt->crs_y1 = 0;
  xt->crs_y2 = 0;

  xt->crs_x = 0;
  xt->crs_y = 0;

  return (0);
}

int xt_init_pointer (xterm_t *xt)
{
  Pixmap               mask;
  static unsigned char map[16] = { 0 };

  mask = XCreatePixmapFromBitmapData (xt->display, xt->wdw, map,
    1, 1, 0, 0, 1
  );

  xt->crs_none = XCreatePixmapCursor (xt->display, mask, mask,
    (XColor *) map, (XColor *) map, 0, 0
  );

  XFreePixmap (xt->display, mask);

  return (0);
}

int xt_init_display (xterm_t *xt)
{
  xt->display = XOpenDisplay (NULL);
  if (xt->display == NULL) {
    return (1);
  }

  xt->init_display = 1;

  xt->screen = DefaultScreen (xt->display);
  xt->display_w = DisplayWidth (xt->display, xt->screen);
  xt->display_h = DisplayHeight (xt->display, xt->screen);

  return (0);
}

int xt_init_font (xterm_t *xt, ini_sct_t *ini)
{
  char *fname;

  ini_get_string (ini, "font", &fname, "vga");

  xt->font = XLoadQueryFont (xt->display, fname);

  if (xt->font == NULL) {
    xt->font = XLoadQueryFont (xt->display, "fixed");
  }

  if (xt->font == NULL) {
    return (1);
  }

  xt->init_font = 1;

  xt->font_w = XTextWidth (xt->font, "W", 1);
  xt->font_h = xt->font->ascent + xt->font->descent;
  xt->font_d = xt->font->descent;

  return (0);
}

int xt_init_window (xterm_t *xt)
{
  char          *name = "pce";
  char          *progname = "pce";
  XSizeHints    size;
  XWMHints      wm;
  XClassHint    cls;
  XTextProperty windowName;

  xt->wdw_w = xt->scn_w * xt->font_w;
  xt->wdw_h = xt->scn_h * xt->font_h;

  xt->wdw = XCreateSimpleWindow (xt->display,
    RootWindow (xt->display, xt->screen),
    0, 0, xt->wdw_w, xt->wdw_h, 0,
    WhitePixel (xt->display, xt->screen),
    BlackPixel (xt->display, xt->screen)
  );

  xt->init_window = 1;

  if (XStringListToTextProperty (&name, 1, &windowName) == 0) {
    return (1);
  }

  size.flags = PSize | PMinSize | PMaxSize;
  size.max_width = xt->wdw_w;
  size.min_width = xt->wdw_w;
  size.max_height = xt->wdw_h;
  size.min_height = xt->wdw_h;

  wm.initial_state = NormalState;
  wm.input = True;
//  wm_hints.icon_pixmap = icon_pixmap;
  wm.icon_pixmap = None;
  wm.flags = StateHint | IconPixmapHint | InputHint;

  cls.res_name = "pce";
  cls.res_class = "PCE";

  XSetWMProperties (xt->display, xt->wdw, &windowName, NULL,
    &progname, 1, &size, &wm, &cls
  );

  XSelectInput (xt->display, xt->wdw,
    ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
    StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask
  );

  xt->back = XCreatePixmap (xt->display, xt->wdw, xt->wdw_w, xt->wdw_h,
    DefaultDepth (xt->display, xt->screen)
  );

  return (0);
}

int xt_init_gc (xterm_t *xt)
{
  XGCValues     values;
  unsigned int  line_width = 6;
  int           line_style = LineOnOffDash;
  int           cap_style = CapRound;
  int           join_style = JoinRound;

  xt->gc = XCreateGC (xt->display, xt->wdw, 0, &values);
  XSetForeground (xt->display, xt->gc, xt->col[0].pixel);
  XSetBackground (xt->display, xt->gc, xt->col[0].pixel);
  XSetFont (xt->display, xt->gc, xt->font->fid);
  XSetLineAttributes (xt->display, xt->gc, line_width, line_style, cap_style, join_style);
  XSetFillStyle (xt->display, xt->gc, FillSolid);

  xt->back_gc = XCreateGC (xt->display, xt->back, 0, &values);
  XSetForeground (xt->display, xt->back_gc, xt->col[0].pixel);
  XSetBackground (xt->display, xt->back_gc, xt->col[0].pixel);
  XSetFont (xt->display, xt->back_gc, xt->font->fid);
  XSetLineAttributes (xt->display, xt->back_gc, line_width, line_style, cap_style, join_style);
  XDrawRectangle (xt->display, xt->back, xt->back_gc, 0, 0, xt->wdw_w, xt->wdw_h);
  XSetFillStyle (xt->display, xt->back_gc, FillSolid);

  xt->crs_gc = XCreateGC (xt->display, xt->wdw, 0, &values);
  XSetForeground (xt->display, xt->crs_gc, WhitePixel (xt->display, xt->screen));
  XSetFillStyle (xt->display, xt->crs_gc, FillSolid);

  xt->fg = 0;
  xt->bg = 0;
  xt->crs_fg = 15;

  xt->init_gc = 1;

  return (0);
}

int xt_init (xterm_t *xt, ini_sct_t *ini)
{
  trm_init (&xt->trm);

  xt->trm.ext = xt;

  xt->trm.del = (trm_del_f) &xt_del;
  xt->trm.set_size = (trm_set_size_f) &xt_set_size;
  xt->trm.set_map = (trm_set_map_f) &xt_set_map;
  xt->trm.set_col = (trm_set_col_f) &xt_set_col;
  xt->trm.set_crs = (trm_set_crs_f) &xt_set_crs;
  xt->trm.set_pos = (trm_set_pos_f) &xt_set_pos;
  xt->trm.set_chr = (trm_set_chr_f) &xt_set_chr;
  xt->trm.set_pxl = (trm_set_pxl_f) &xt_set_pxl;
  xt->trm.check = (trm_check_f) &xt_check;

  xt->init_display = 0;
  xt->init_font = 0;
  xt->init_window = 0;
  xt->init_gc = 0;

  if (xt_init_display (xt)) {
    fprintf (stderr, "xt_init_display()\n");
    return (1);
  }

  if (xt_init_font (xt, ini)) {
    fprintf (stderr, "xt_init_font()\n");
    return (1);
  }

  xt_init_colors (xt);

  xt->mode = 0;

  xt->scn_w = 80;
  xt->scn_h = 25;
  xt->scn = (unsigned char *) malloc (xt->scn_w * xt->scn_h * 2);

  if (xt_init_window (xt)) {
    fprintf (stderr, "xt_init_window()\n");
    return (1);
  }

  if (xt_init_gc (xt)) {
    fprintf (stderr, "xt_init_gc()\n");
    return (1);
  }

  xt_init_cursor (xt);
  xt_init_pointer (xt);

  xt->flush_x1 = xt->wdw_w;
  xt->flush_y1 = xt->wdw_h;
  xt->flush_x2 = 0;
  xt->flush_y2 = 0;

  XMapWindow (xt->display, xt->wdw);

  return (0);
}

terminal_t *xt_new (ini_sct_t *ini)
{
  xterm_t *xt;

  xt = (xterm_t *) malloc (sizeof (xterm_t));
  if (xt == NULL) {
    return (NULL);
  }

  if (xt_init (xt, ini)) {
    xt_free (xt);
    free (xt);
    return (NULL);
  }

  return (&xt->trm);
}

void xt_free (xterm_t *xt)
{
  if (xt == NULL) {
    return;
  }

  if (xt->init_window) {
    XFreePixmap (xt->display, xt->back);
  }

  if (xt->init_font) {
    XUnloadFont (xt->display, xt->font->fid);
  }

  if (xt->init_gc) {
    XFreeGC (xt->display, xt->gc);
    XFreeGC (xt->display, xt->back_gc);
    XFreeGC (xt->display, xt->crs_gc);
  }

  if (xt->init_display) {
    XCloseDisplay (xt->display);
  }
}

void xt_del (xterm_t *xt)
{
  if (xt != NULL) {
    xt_free (xt);
    free (xt);
  }
}

static
void xt_crs_restore (xterm_t *xt)
{
  int x, y;

  if (xt->crs_on) {
    x = xt->crs_x * xt->font_w;
    y = xt->crs_y * xt->font_h;
    XCopyArea (xt->display, xt->back, xt->wdw, xt->gc,
      x, y, xt->font_w, xt->font_h, x, y
    );
  }
}

void xt_crs_draw (xterm_t *xt, unsigned x, unsigned y)
{
  unsigned col;

  xt->crs_x = x;
  xt->crs_y = y;

  if (xt->mode != 0) {
    return;
  }

  if (xt->crs_y2 < xt->crs_y1) {
    return;
  }

  col = xt->scn[2 * (xt->scn_w * y) + 1] & 0x0f;
  if (col != xt->crs_fg) {
    xt->crs_fg = col;
    XSetForeground (xt->display, xt->crs_gc, xt->col[col].pixel);
  }

  XFillRectangle (xt->display, xt->wdw, xt->crs_gc,
    x * xt->font_w, y * xt->font_h + xt->crs_y1,
    xt->font_w, xt->crs_y2 - xt->crs_y1 + 1
  );
}

void xt_set_size (xterm_t *xt, unsigned m, unsigned w, unsigned h)
{
  XSizeHints size;

  xt->mode = m;

  if (m == TERM_MODE_TEXT) {
    xt->scn_w = w;
    xt->scn_h = h;

    xt->wdw_w = w * xt->font_w;
    xt->wdw_h = h * xt->font_h;
  }
  else {
    xt->scn_w = w / xt->font_w;
    xt->scn_h = h / xt->font_h;

    xt->wdw_w = w;
    xt->wdw_h = h;

    xt->crs_on = 0;
  }

  xt->scn = (unsigned char *) realloc (xt->scn, 2 * xt->scn_w * xt->scn_h);

  XResizeWindow (xt->display, xt->wdw, xt->wdw_w, xt->wdw_h);

  XFreePixmap (xt->display, xt->back);
  xt->back = XCreatePixmap (xt->display, xt->wdw, xt->wdw_w, xt->wdw_h,
    DefaultDepth (xt->display, xt->screen)
  );

  size.flags = PSize | PMinSize | PMaxSize;
  size.min_width = xt->wdw_w;
  size.max_width = xt->wdw_w;
  size.min_height = xt->wdw_h;
  size.max_height = xt->wdw_h;

  XSetWMNormalHints (xt->display, xt->wdw, &size);

  xt->flush_x1 = xt->wdw_w;
  xt->flush_y1 = xt->wdw_h;
  xt->flush_x2 = 0;
  xt->flush_y2 = 0;

  xt_set_col (xt, 0, 0);
  xt_clear (xt);
}

void xt_set_map (xterm_t *xt, unsigned idx, unsigned r, unsigned g, unsigned b)
{
  Colormap cmap;

  if (idx >= 256) {
    return;
  }

  cmap = DefaultColormap (xt->display, xt->screen);

  if (col_alloc[idx]) {
    XFreeColors (xt->display, cmap, &xt->col[idx].pixel, 1, 0);
  }

  xt->col[idx].flags = DoRed | DoGreen | DoBlue;

  xt->col[idx].red = r;
  xt->col[idx].green = g;
  xt->col[idx].blue = b;

  XAllocColor (xt->display, cmap, &xt->col[idx]);

  col_alloc[idx] = 1;

  if (idx == xt->fg) {
    XSetForeground (xt->display, xt->gc, xt->col[xt->fg].pixel);
    XSetForeground (xt->display, xt->back_gc, xt->col[xt->fg].pixel);
  }

  if (idx == xt->bg) {
    XSetBackground (xt->display, xt->gc, xt->col[xt->bg].pixel);
    XSetBackground (xt->display, xt->back_gc, xt->col[xt->bg].pixel);
  }
}

void xt_set_col (xterm_t *xt, unsigned fg, unsigned bg)
{
  if ((xt->fg == fg) && (xt->bg == bg)) {
    return;
  }

  xt->fg = fg & 0x0f;
  xt->bg = bg & 0x0f;

  XSetForeground (xt->display, xt->gc, xt->col[xt->fg].pixel);
  XSetBackground (xt->display, xt->gc, xt->col[xt->bg].pixel);

  XSetForeground (xt->display, xt->back_gc, xt->col[xt->fg].pixel);
  XSetBackground (xt->display, xt->back_gc, xt->col[xt->bg].pixel);
}

void xt_set_crs (xterm_t *xt, unsigned y1, unsigned y2, int show)
{
  xt_crs_restore (xt);

  xt->crs_on = (show != 0);

  y1 = (y1 <= 255) ? y1 : 255;
  y2 = (y2 <= 255) ? y2 : 255;

  xt->crs_y1 = (xt->font_h * y1) / 256;
  xt->crs_y2 = (xt->font_h * y2) / 256;

  xt_crs_draw (xt, xt->crs_x, xt->crs_y);
}

void xt_set_pos (xterm_t *xt, unsigned x, unsigned y)
{
  xt_crs_restore (xt);
  xt_crs_draw (xt, x, y);
}

void xt_set_chr (xterm_t *xt, unsigned x, unsigned y, unsigned char c)
{
  unsigned i;
  int      scrx, scry;

  if ((x >= xt->scn_w) || (y >= xt->scn_h)) {
    return;
  }

  i = xt->scn_w * y + x;

  xt->scn[2 * i] = c;
  xt->scn[2 * i + 1] = (xt->bg << 4) | xt->fg;

  if ((xt->crs_x == x) && (xt->crs_y == y)) {
    xt_crs_restore (xt);
  }

  scrx = x * xt->font_w;
  scry = y * xt->font_h;

  XDrawImageString (xt->display, xt->back, xt->back_gc,
    scrx, scry + xt->font_h - xt->font_d, &c, 1
  );

  XCopyArea (xt->display, xt->back, xt->wdw, xt->gc,
    scrx, scry, xt->font_w, xt->font_h, scrx, scry
  );

  if ((xt->crs_x == x) && (xt->crs_y == y)) {
    xt_crs_draw (xt, xt->crs_x, xt->crs_y);
  }
}

void xt_set_pxl (xterm_t *xt, unsigned x, unsigned y, unsigned w, unsigned h)
{
  if ((w == 1) && (h == 1)) {
    XDrawPoint (xt->display, xt->back, xt->back_gc, x, y);
//    XDrawPoint (xt->display, xt->wdw, xt->gc, x, y);
  }
  else {
    XFillRectangle (xt->display, xt->back, xt->back_gc, x, y, w, h);
//    XFillRectangle (xt->display, xt->wdw, xt->gc, x, y, w, h);
  }

  if (x < xt->flush_x1) {
    xt->flush_x1 = x;
  }
  if (y < xt->flush_y1) {
    xt->flush_y1 = y;
  }
  if ((x + w) > xt->flush_x2) {
    xt->flush_x2 = x + w - 1;
  }
  if ((y + h) > xt->flush_y2) {
    xt->flush_y2 = y + h - 1;
  }
}

void xt_clear (xterm_t *xt)
{
  XFillRectangle (xt->display, xt->back, xt->back_gc, 0, 0, xt->wdw_w, xt->wdw_h);
  XFillRectangle (xt->display, xt->wdw, xt->gc, 0, 0, xt->wdw_w, xt->wdw_h);
}

void xt_update (xterm_t *xt, int x, int y, int w, int h)
{
  XCopyArea (xt->display, xt->back, xt->wdw, xt->gc,
    x, y, w, h, x, y
  );

  xt_crs_draw (xt, xt->crs_x, xt->crs_y);
}


#define key_table1_n (sizeof (key_table1) / sizeof (key_table1[0]))
#define key_table2_n (sizeof (key_table2) / sizeof (key_table2[0]))

static
unsigned char key_table1[] = {
  0x39, 0x02,
#ifdef KBUK             /* double quotes, hash symbol */
  0x03, 0x2b,
#else
  0x28, 0x04,
#endif
  0x05, 0x06, 0x08, 0x28,
  0x0a, 0x0b, 0x09, 0x0d, 0x33, 0x0c, 0x34, 0x35,
  0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x27, 0x27, 0x33, 0x0d, 0x34, 0x35,
#ifdef KBUK             /* at symbol */
  0x28,
#else
  0x03,
#endif
  0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22,
  0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
  0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11,
  0x2d, 0x15, 0x2c, 0x1a,
#ifdef KBUK             /* backslash */
  0x56,
#else
  0x2b,
#endif
  0x1b, 0x07, 0x0c,
  0x29, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22,
  0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
  0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11,
  0x2d, 0x15, 0x2c, 0x1a,
#ifdef KBUK             /* vertical bar */
  0x56,
#else
  0x2b,
#endif

  0x1b,

#ifdef KBUK             /* tilde */
  0x2b,
#else
  0x29,
#endif
  0
};

static struct {
  KeySym        key;
  unsigned long code;
} key_table2[] = {
    { XK_BackSpace,     0x0e },
    { XK_Tab,           0x0f },
    { XK_Return,        0x1c },
    { XK_Escape,        0x01 },
    { XK_Delete,        0x53e0 },

    { XK_Home,          0x47e0 },
    { XK_Left,          0x4be0 },
    { XK_Up,            0x48e0 },
    { XK_Right,         0x4de0 },
    { XK_Down,          0x50e0 },
    { XK_Prior,         0x49e0 },
    { XK_Next,          0x51e0 },
    { XK_End,           0x4fe0 },
    { XK_Insert,        0x52e0 },
    { XK_Print,         0x37e0 },
    { XK_Num_Lock,      0x45 },

    { XK_KP_Enter,      0x1ce0 },
    { XK_KP_Multiply,   0x37 },
    { XK_KP_Add,        0x4e },
    { XK_KP_Subtract,   0x4a },
    { XK_KP_Divide,     0x36e0 },
    { XK_KP_Decimal,    0x53 },

    { XK_KP_Insert,     0x52 },
    { XK_KP_Begin,      0x4c },

    { XK_KP_0,          0x52 },
    { XK_KP_1,          0x4f },
    { XK_KP_2,          0x50 },
    { XK_KP_3,          0x51 },
    { XK_KP_4,          0x4b },
    { XK_KP_5,          0x4c },
    { XK_KP_6,          0x4d },
    { XK_KP_7,          0x47 },
    { XK_KP_8,          0x48 },
    { XK_KP_9,          0x49 },

    { XK_F1,            0x3b },
    { XK_F2,            0x3c },
    { XK_F3,            0x3d },
    { XK_F4,            0x3e },
    { XK_F5,            0x3f },
    { XK_F6,            0x40 },
    { XK_F7,            0x41 },
    { XK_F8,            0x42 },
    { XK_F9,            0x43 },
    { XK_F10,           0x44 },
    { XK_F11,           0x57 },
    { XK_F12,           0x58 },

    { XK_Shift_L,       0x2a },
    { XK_Shift_R,       0x36 },
    { XK_Control_L,     0x1d },
    { XK_Control_R,     0x1de0 },
    { XK_Meta_L,        0x38 },
    { XK_Alt_L,         0x38 },
    { XK_Meta_R,        0x38e0 },
    { XK_Alt_R,         0x38e0 },

    { XK_Scroll_Lock,   0x46 },
    { XK_Caps_Lock,     0xba3a }
};

void xt_send_key_code (xterm_t *xt, unsigned long code)
{
  if (xt->trm.set_key == NULL) {
    return;
  }

  while (code != 0) {
    xt->trm.set_key (xt->trm.key_ext, code & 0xff);
    code = code >> 8;
  }
}

unsigned long xt_get_key_code (xterm_t *xt, KeySym key, int make)
{
  unsigned      i;
  unsigned long ret, tmp, msk;

  msk = make ? 0x00 : 0x80;

  if ((key >= 32) && (key < (32 + key_table1_n))) {
    return (key_table1[key - 32] | msk);
  }

  for (i = 0; i < key_table2_n; i++) {
    if (key_table2[i].key == key) {
      tmp = key_table2[i].code;
      ret = key_table2[i].code;

      while (tmp != 0) {
        ret |= msk;
        tmp = tmp >> 8;
        msk = msk << 8;
      }

      return (ret);
    }
  }

  fprintf (stderr, "unmatched key: %04X\n", (unsigned) key);

  return (0);
}

void xt_check (xterm_t *xt)
{
  XEvent          event;
  KeySym          key;
  unsigned long   code;
  static unsigned cnt = 0;

  cnt += 1;

  if (cnt > 32) {
    cnt -= 32;

    if ((xt->flush_x1 <= xt->flush_x2) && (xt->flush_y1 <= xt->flush_y2)) {
      unsigned        w, h;

      w = xt->flush_x2 - xt->flush_x1 + 1;
      h = xt->flush_y2 - xt->flush_y1 + 1;
      xt_update (xt, xt->flush_x1, xt->flush_y1, w, h);
      xt->flush_x1 = xt->wdw_w;
      xt->flush_y1 = xt->wdw_h;
      xt->flush_x2 = 0;
      xt->flush_y2 = 0;
    }
  }

  while (XPending (xt->display) > 0) {
    XNextEvent (xt->display, &event);

    switch (event.type) {
      case Expose: {
        XExposeEvent *evt;
        evt = (XExposeEvent *) &event;
        xt_update (xt, evt->x, evt->y, evt->width, evt->height);
        }
        break;

      case KeyPress:
        key = XLookupKeysym (&event.xkey, 0);
        if ((key == XK_grave) && (event.xkey.state & Mod1Mask)) {
          if (xt->trm.set_brk != NULL) {
            xt->trm.set_brk (xt->trm.key_ext, 2);
          }
        }
        else if ((key == XK_grave) && (event.xkey.state & ControlMask)) {
          if (xt->trm.set_brk != NULL) {
            xt->trm.set_brk (xt->trm.key_ext, 1);
          }
        }
        else if (key == XK_Pause) {
          if (xt->trm.set_brk != NULL) {
            xt->trm.set_brk (xt->trm.key_ext, 2);
          }
        }
        else if ((key == XK_Print) && (event.xkey.state == 0)) {
          if (xt->trm.set_brk != NULL) {
            xt->trm.set_brk (xt->trm.key_ext, 3);
          }
        }
        else {
          code = xt_get_key_code (xt, key, 1);
          xt_send_key_code (xt, code);
        }
        break;

      case KeyRelease:
        key = XLookupKeysym (&event.xkey, 0);
        code = xt_get_key_code (xt, key, 0);
        xt_send_key_code (xt, code);
        break;

      case ButtonPress:
      case ButtonRelease:
        if (xt->trm.set_mse != NULL) {
          unsigned     b;
          XButtonEvent *evt = (XButtonEvent *) &event;

          if ((evt->type == ButtonPress) && (evt->button == Button2)) {
            if (xt->grab == 0) {
              xt->grab = 1;
              xt->mse_x = evt->x;
              xt->mse_y = evt->y;
              XDefineCursor (xt->display, xt->wdw, xt->crs_none);
              XGrabPointer (xt->display, xt->wdw, True,
                PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None,  None, CurrentTime
              );
            }
            else {
              xt->grab = 0;
              XUngrabPointer (xt->display, CurrentTime);
              XUndefineCursor (xt->display, xt->wdw);
            }
          }
          else if (xt->grab) {
            b = evt->state;
            b = ((b & Button1Mask) ? 0x01 : 0) | ((b & Button3Mask) ? 0x02 : 0);
            b ^= (evt->button == Button1) ? 0x01 : 0x00;
            b ^= (evt->button == Button3) ? 0x02 : 0x00;
            xt->trm.set_mse (xt->trm.mse_ext, 0, 0, b);
          }
        }
        break;

      case MotionNotify:
        if ((xt->trm.set_mse != NULL) && (xt->grab)) {
          int      cx, cy;
          int      dx, dy;
          unsigned b;
          XMotionEvent *evt = (XMotionEvent *) &event;

          cx = xt->wdw_w / 2;
          cy = xt->wdw_h / 2;

          if ((evt->x == cx) && (evt->y == cy)) {
            xt->mse_x = evt->x;
            xt->mse_y = evt->y;
            break;
          }

          dx = xt->mse_x;
          dy = xt->mse_y;

          xt->mse_x = evt->x;
          xt->mse_y = evt->y;

          b = evt->state;
          b = ((b & Button1Mask) ? 1 : 0) | ((b & Button2Mask) ? 2 : 0);

          xt->trm.set_mse (xt->trm.mse_ext, xt->mse_x - dx, xt->mse_y - dy, b);

          XWarpPointer (xt->display, None, xt->wdw, 0, 0, 0, 0, cx, cy);
        }
        break;

      case NoExpose:
      case ConfigureNotify:
        break;

      default:
        break;
    }
  }
}
