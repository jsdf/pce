/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/terminal/x11.c                                           *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
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


typedef struct {
	KeySym    sym;
	pce_key_t key;
} xt_keymap_t;


static xt_keymap_t keymap[] = {
	{ XK_Escape,      PCE_KEY_ESC },
	{ XK_F1,          PCE_KEY_F1 },
	{ XK_F2,          PCE_KEY_F2 },
	{ XK_F3,          PCE_KEY_F3 },
	{ XK_F4,          PCE_KEY_F4 },
	{ XK_F5,          PCE_KEY_F5 },
	{ XK_F6,          PCE_KEY_F6 },
	{ XK_F7,          PCE_KEY_F7 },
	{ XK_F8,          PCE_KEY_F8 },
	{ XK_F9,          PCE_KEY_F9 },
	{ XK_F10,         PCE_KEY_F10 },
	{ XK_F11,         PCE_KEY_F11 },
	{ XK_F12,         PCE_KEY_F12 },

	{ XK_Print,       PCE_KEY_PRTSCN },
	{ XK_Scroll_Lock, PCE_KEY_SCRLK },
	{ XK_Pause,       PCE_KEY_PAUSE },

	{ XK_grave,       PCE_KEY_BACKQUOTE },
	{ XK_1,           PCE_KEY_1 },
	{ XK_2,           PCE_KEY_2 },
	{ XK_3,           PCE_KEY_3 },
	{ XK_4,           PCE_KEY_4 },
	{ XK_5,           PCE_KEY_5 },
	{ XK_6,           PCE_KEY_6 },
	{ XK_7,           PCE_KEY_7 },
	{ XK_8,           PCE_KEY_8 },
	{ XK_9,           PCE_KEY_9 },
	{ XK_0,           PCE_KEY_0 },
	{ XK_minus,       PCE_KEY_MINUS },
	{ XK_plus,        PCE_KEY_EQUAL },
	{ XK_BackSpace,   PCE_KEY_BACKSPACE },

	{ XK_Tab,         PCE_KEY_TAB },
	{ XK_q,           PCE_KEY_Q },
	{ XK_w,           PCE_KEY_W },
	{ XK_e,           PCE_KEY_E },
	{ XK_r,           PCE_KEY_R },
	{ XK_t,           PCE_KEY_T },
	{ XK_y,           PCE_KEY_Y },
	{ XK_u,           PCE_KEY_U },
	{ XK_i,           PCE_KEY_I },
	{ XK_o,           PCE_KEY_O },
	{ XK_p,           PCE_KEY_P },
	{ XK_parenleft,   PCE_KEY_LBRACKET },
	{ XK_parenright,  PCE_KEY_RBRACKET },
	{ XK_Return,      PCE_KEY_RETURN },

	{ XK_Caps_Lock,   PCE_KEY_CAPSLOCK },
	{ XK_a,           PCE_KEY_A },
	{ XK_s,           PCE_KEY_S },
	{ XK_d,           PCE_KEY_D },
	{ XK_f,           PCE_KEY_F },
	{ XK_g,           PCE_KEY_G },
	{ XK_h,           PCE_KEY_H },
	{ XK_j,           PCE_KEY_J },
	{ XK_k,           PCE_KEY_K },
	{ XK_l,           PCE_KEY_L },
	{ XK_semicolon,   PCE_KEY_SEMICOLON },
	{ XK_apostrophe,  PCE_KEY_QUOTE },
	{ XK_backslash,   PCE_KEY_BACKSLASH },

	{ XK_Shift_L,     PCE_KEY_LSHIFT },
	{ XK_less,        PCE_KEY_LESS },
	{ XK_z,           PCE_KEY_Z },
	{ XK_x,           PCE_KEY_X },
	{ XK_c,           PCE_KEY_C },
	{ XK_v,           PCE_KEY_V },
	{ XK_b,           PCE_KEY_B },
	{ XK_n,           PCE_KEY_N },
	{ XK_m,           PCE_KEY_M },
	{ XK_comma,       PCE_KEY_COMMA },
	{ XK_period,      PCE_KEY_PERIOD },
	{ XK_slash,       PCE_KEY_SLASH },
	{ XK_Shift_R,     PCE_KEY_RSHIFT },

	{ XK_Control_L,   PCE_KEY_LCTRL },
	{ XK_Alt_L,       PCE_KEY_LALT },
	{ XK_Meta_L,      PCE_KEY_LALT },
	{ XK_space,       PCE_KEY_SPACE },
	{ XK_Alt_R,       PCE_KEY_RALT },
	{ XK_Meta_R,      PCE_KEY_RALT },
	{ XK_Control_R,   PCE_KEY_RCTRL },

	{ XK_Num_Lock,    PCE_KEY_NUMLOCK },
	{ XK_KP_Divide,   PCE_KEY_KP_SLASH },
	{ XK_KP_Multiply, PCE_KEY_KP_STAR },
	{ XK_KP_Subtract, PCE_KEY_KP_MINUS },
	{ XK_KP_7,        PCE_KEY_KP_7 },
	{ XK_KP_8,        PCE_KEY_KP_8 },
	{ XK_KP_9,        PCE_KEY_KP_9 },
	{ XK_KP_Add,      PCE_KEY_KP_PLUS },
	{ XK_KP_4,        PCE_KEY_KP_4 },
	{ XK_KP_5,        PCE_KEY_KP_5 },
	{ XK_KP_Begin,    PCE_KEY_KP_5 },
	{ XK_KP_6,        PCE_KEY_KP_6 },
	{ XK_KP_1,        PCE_KEY_KP_1 },
	{ XK_KP_2,        PCE_KEY_KP_2 },
	{ XK_KP_3,        PCE_KEY_KP_3 },
	{ XK_KP_Enter,    PCE_KEY_KP_ENTER },
	{ XK_KP_0,        PCE_KEY_KP_0 },
	{ XK_KP_Decimal,  PCE_KEY_KP_PERIOD },
	{ XK_Insert,      PCE_KEY_INS },
	{ XK_Home,        PCE_KEY_HOME },
	{ XK_Prior,       PCE_KEY_PAGEUP },
	{ XK_Delete,      PCE_KEY_DEL },
	{ XK_End,         PCE_KEY_END },
	{ XK_Next,        PCE_KEY_PAGEDN },
	{ XK_Up,          PCE_KEY_UP },
	{ XK_Left,        PCE_KEY_LEFT },
	{ XK_Down,        PCE_KEY_DOWN },
	{ XK_Right,       PCE_KEY_RIGHT },
	{ PCE_KEY_NONE,   0 }
};


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

	mask = XCreatePixmapFromBitmapData (xt->display, xt->wdw, (char *) map,
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
	const char *fname;

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
/*  wm_hints.icon_pixmap = icon_pixmap; */
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
	xt->trm.set_mode = (trm_set_mode_f) &xt_set_mode;
	xt->trm.set_map = (trm_set_map_f) &xt_set_map;
	xt->trm.set_col = (trm_set_col_f) &xt_set_col;
	xt->trm.set_crs = (trm_set_crs_f) &xt_set_crs;
	xt->trm.set_pos = (trm_set_pos_f) &xt_set_pos;
	xt->trm.set_chr = (trm_set_chr_f) &xt_set_chr;
	xt->trm.set_pxl = (trm_set_pxl_f) &xt_set_pxl;
	xt->trm.set_rct = (trm_set_rct_f) &xt_set_rct;
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

static
void xt_crs_draw (xterm_t *xt, unsigned x, unsigned y)
{
	unsigned col;

	xt->crs_x = x;
	xt->crs_y = y;

	if (xt->mode != 0) {
		return;
	}

	if (xt->crs_on == 0) {
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

void xt_set_mode (xterm_t *xt, unsigned m, unsigned w, unsigned h)
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
		scrx, scry + xt->font_h - xt->font_d, (char *) &c, 1
	);

	XCopyArea (xt->display, xt->back, xt->wdw, xt->gc,
		scrx, scry, xt->font_w, xt->font_h, scrx, scry
	);

	if ((xt->crs_x == x) && (xt->crs_y == y)) {
		xt_crs_draw (xt, xt->crs_x, xt->crs_y);
	}
}

void xt_set_pxl (xterm_t *xt, unsigned x, unsigned y)
{
	XDrawPoint (xt->display, xt->back, xt->back_gc, x, y);

	if (x < xt->flush_x1) {
		xt->flush_x1 = x;
	}
	if (y < xt->flush_y1) {
		xt->flush_y1 = y;
	}
	if (x >= xt->flush_x2) {
		xt->flush_x2 = x;
	}
	if (y >= xt->flush_y2) {
		xt->flush_y2 = y;
	}
}

void xt_set_rct (xterm_t *xt, unsigned x, unsigned y, unsigned w, unsigned h)
{
	XFillRectangle (xt->display, xt->back, xt->back_gc, x, y, w, h);

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


static
pce_key_t xt_key_map (xterm_t *xt, KeySym sym)
{
	xt_keymap_t *map;

	map = keymap;

	while (map->key != PCE_KEY_NONE) {
		if (map->sym == sym) {
			return (map->key);
		}

		map += 1;
	}

	return (PCE_KEY_NONE);
}

static
void xt_key_send (xterm_t *xt, KeySym sym, int press)
{
	pce_key_t key;

	key = xt_key_map (xt, sym);

	fprintf (stderr, "xt: map %u -> %u\n", (unsigned) sym, (unsigned) key);

	if (key == PCE_KEY_NONE) {
		return;
	}

	if (press) {
		trm_set_key (&xt->trm, PCE_KEY_EVENT_DOWN, key);
	}
	else {
		trm_set_key (&xt->trm, PCE_KEY_EVENT_UP, key);
	}
}

static
void xt_event_keydown (xterm_t *xt, XEvent *evt)
{
	KeySym sym;

	sym = XLookupKeysym (&evt->xkey, 0);

	if ((sym == XK_grave) && (evt->xkey.state & Mod1Mask)) {
		trm_set_msg (&xt->trm, "emu.exit", "1");
	}
	else if ((sym == XK_grave) && (evt->xkey.state & ControlMask)) {
		trm_set_msg (&xt->trm, "emu.stop", "1");
	}
	else if (sym == XK_Pause) {
		trm_set_msg (&xt->trm, "emu.exit", "1");
	}
	else if ((sym == XK_Print) && (evt->xkey.state == 0)) {
		trm_set_msg (&xt->trm, "video.screenshot", "");
	}
	else {
		xt_key_send (xt, sym, 1);
	}
}

static
void xt_event_keyup (xterm_t *xt, XEvent *evt)
{
	KeySym sym;

	sym = XLookupKeysym (&evt->xkey, 0);

	xt_key_send (xt, sym, 0);
}

void xt_check (xterm_t *xt)
{
	XEvent          event;
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
				xt_event_keydown (xt, &event);
				break;

			case KeyRelease:
				xt_event_keyup (xt, &event);
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
