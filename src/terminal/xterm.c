/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/xterm.c                                       *
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

/* $Id: xterm.c,v 1.1 2003/04/24 23:18:17 hampa Exp $ */


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
#include <terminal/xterm.h>


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
int xt_init_colors (xterm_t *xt)
{
  unsigned i;

  for (i = 0; i < 16; i++) {
    xt->col[i].flags = DoRed | DoGreen | DoBlue;

    xt->col[i].red = rgb[i].r;
    xt->col[i].green = rgb[i].g;
    xt->col[i].blue = rgb[i].b;

    XAllocColor (xt->display, DefaultColormap (xt->display, xt->screen), &xt->col[i]);
  }

  return (0);
}

int xt_init_cursor (xterm_t *xt)
{
  xt->crs_bg = XCreatePixmap (xt->display, xt->wdw, xt->font_w, xt->font_h,
    DefaultDepth (xt->display, xt->screen)
  );

  xt->crs_y1 = 32;
  xt->crs_y2 = 0;

  xt->crs_x = 0;
  xt->crs_y = 0;

  return (0);
}

int xt_init_display (xterm_t *xt)
{
  xt->display = XOpenDisplay (NULL);
  if (xt->display == NULL) {
    return (1);
  }

  xt->screen = DefaultScreen (xt->display);
  xt->display_w = DisplayWidth (xt->display, xt->screen);
  xt->display_h = DisplayHeight (xt->display, xt->screen);

  return (0);
}

int xt_init_font (xterm_t *xt)
{
  xt->font = XLoadQueryFont (xt->display, "vga");

  if (xt->font == NULL) {
    xt->font = XLoadQueryFont (xt->display, "fixed");
  }

  if (xt->font == NULL) {
    return (1);
  }

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
    ExposureMask | KeyPressMask | KeyReleaseMask |
    StructureNotifyMask | ButtonPressMask | ButtonReleaseMask
  );

  return (0);
}

int xt_init_gc (xterm_t *xt)
{
  unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
  XGCValues     values;
  unsigned int  line_width = 6;
  int           line_style = LineOnOffDash;
  int           cap_style = CapRound;
  int           join_style = JoinRound;

  xt->gc = XCreateGC (xt->display, xt->wdw, valuemask, &values);

  xt->crs_gc = XCreateGC (xt->display, xt->wdw, valuemask, &values);

  XSetFont (xt->display, xt->gc, xt->font->fid);

  XSetForeground (xt->display, xt->gc, WhitePixel (xt->display, xt->screen));
  XSetBackground (xt->display, xt->gc, BlackPixel (xt->display, xt->screen));

  XSetLineAttributes (xt->display, xt->gc, line_width, line_style, cap_style, join_style);

  XSetForeground (xt->display, xt->crs_gc, WhitePixel (xt->display, xt->screen));
  XSetBackground (xt->display, xt->crs_gc, BlackPixel (xt->display, xt->screen));

  return (0);
}

int xt_init (xterm_t *xt)
{
  trm_init (&xt->trm);

  xt->trm.ext = xt;

  xt->trm.del = (trm_del_f) &xt_del;
  xt->trm.set_size = (trm_set_size_f) &xt_set_size;
  xt->trm.set_col = (trm_set_col_f) &xt_set_col;
  xt->trm.set_crs = (trm_set_crs_f) &xt_set_crs;
  xt->trm.set_pos = (trm_set_pos_f) &xt_set_pos;
  xt->trm.set_chr = (trm_set_chr_f) &xt_set_chr;
  xt->trm.check = (trm_check_f) &xt_check;

  xt->display = NULL;
  xt->screen = 0;
  xt->font = NULL;
  xt->wdw = None;
  xt->scn = NULL;

  if (xt_init_display (xt)) {
    fprintf (stderr, "xt_init_display()\n");
    return (1);
  }

  if (xt_init_font (xt)) {
    fprintf (stderr, "xt_init_font()\n");
    return (1);
  }

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

  xt_init_colors (xt);

  XMapWindow (xt->display, xt->wdw);

  return (0);
}

void xt_flush (xterm_t *xt)
{
  XFlush (xt->display);
}

terminal_t *xt_new (void)
{
  xterm_t *xt;

  xt = (xterm_t *) malloc (sizeof (xterm_t));
  if (xt == NULL) {
    return (NULL);
  }

  if (xt_init (xt)) {
    free (xt);
    return (NULL);
  }

  xt_flush (xt);

  return (&xt->trm);
}

void xt_free (xterm_t *xt)
{
  if (xt == NULL) {
    return;
  }

  if (xt->display == NULL) {
    return;
  }

  if (xt->font != NULL) {
    XUnloadFont (xt->display, xt->font->fid);
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
  if (xt->crs_y1 <= xt->crs_y2) {
    XCopyArea (xt->display, xt->crs_bg, xt->wdw, xt->crs_gc,
      0, 0, xt->font_w, xt->font_h, xt->crs_x * xt->font_w, xt->crs_y * xt->font_h
    );
  }
}

void xt_crs_draw (xterm_t *xt, unsigned x, unsigned y)
{
  xt->crs_x = x;
  xt->crs_y = y;

  if (xt->crs_y2 < xt->crs_y1) {
    return;
  }

  XCopyArea (xt->display, xt->wdw, xt->crs_bg, xt->crs_gc,
    x * xt->font_w, y * xt->font_h, xt->font_w, xt->font_h, 0, 0
  );

  XFillRectangle (xt->display, xt->wdw, xt->crs_gc,
    x * xt->font_w, (y + 1) * xt->font_h - xt->crs_y2 - 1,
    xt->font_w, xt->crs_y2 - xt->crs_y1 + 1
  );
}

void xt_set_size (xterm_t *xt, unsigned w, unsigned h)
{
  XSizeHints size;

  if ((w == xt->scn_w) && (h == xt->scn_h)) {
    return;
  }

  xt->scn_w = w;
  xt->scn_h = h;
  xt->scn = (unsigned char *) realloc (xt->scn, 2 * w * h);

  xt->wdw_w = w * xt->font_w;
  xt->wdw_h = h * xt->font_h;

  XResizeWindow (xt->display, xt->wdw, xt->wdw_w, xt->wdw_h);

  size.flags = PSize | PMinSize | PMaxSize;
  size.min_width = xt->wdw_w;
  size.max_width = xt->wdw_w;
  size.min_height = xt->wdw_h;
  size.max_height = xt->wdw_h;

  XSetWMNormalHints (xt->display, xt->wdw, &size);
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
}

void xt_set_crs (xterm_t *xt, unsigned y1, unsigned y2)
{
  xt_crs_restore (xt);

  xt->crs_y1 = y1;
  xt->crs_y2 = y2;

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

  if ((x >= xt->scn_w) || (y >= xt->scn_h)) {
    return;
  }

  i = xt->scn_w * y + x;

  xt->scn[2 * i] = c;
  xt->scn[2 * i + 1] = (xt->bg << 4) | xt->fg;

  if ((xt->crs_x == x) && (xt->crs_y == y)) {
    xt_crs_restore (xt);
  }

  XDrawImageString (xt->display, xt->wdw, xt->gc,
    x * xt->font_w, (y + 1) * xt->font_h - xt->font_d,
    &c, 1
  );

  if ((xt->crs_x == x) && (xt->crs_y == y)) {
    xt_crs_draw (xt, xt->crs_x, xt->crs_y);
  }
}

void xt_set_str (xterm_t *xt, unsigned x, unsigned y, const char *str)
{
  unsigned n;

  n = strlen (str);

  XDrawImageString (xt->display, xt->wdw, xt->gc, x * xt->font_w, (y + 1) * xt->font_h, str, n);
}

void xt_clear (xterm_t *xt)
{
  XClearWindow (xt->display, xt->wdw);
}

void xt_update (xterm_t *xt)
{
  unsigned      x, y, i;
  unsigned      fg, bg;
  unsigned char c;

  i = 0;

  xt_crs_restore (xt);

  for (y = 0; y < xt->scn_h; y++) {
    for (x = 0; x < xt->scn_w; x++) {
      c = xt->scn[i];
      fg = xt->scn[i + 1] & 0x0f;
      bg = (xt->scn[i + 1] & 0xf0) >> 4;

      XSetForeground (xt->display, xt->gc, xt->col[fg].pixel);
      XSetBackground (xt->display, xt->gc, xt->col[bg].pixel);

      XDrawImageString (xt->display, xt->wdw, xt->gc,
        x * xt->font_w, (y + 1) * xt->font_h - xt->font_d,
        &c, 1
      );

      i += 2;
    }
  }

  xt_crs_draw (xt, xt->crs_x, xt->crs_y);

  XSetForeground (xt->display, xt->gc, xt->col[xt->fg].pixel);
  XSetBackground (xt->display, xt->gc, xt->col[xt->bg].pixel);
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
  XEvent event;
  KeySym key;
  unsigned long code;

  while (XPending (xt->display) > 0) {
    XNextEvent (xt->display, &event);

    switch (event.type) {
      case Expose:
        xt_update (xt);
        break;

      case KeyPress:
        key = XLookupKeysym (&event.xkey, 0);
        if (key == XK_Print) {
          if (xt->trm.set_brk != NULL) {
            xt->trm.set_brk (xt->trm.key_ext, 0);
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
    }
  }
}
