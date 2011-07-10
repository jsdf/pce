/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/x11.c                                      *
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


#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <drivers/video/terminal.h>
#include <drivers/video/x11.h>


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
	{ XK_equal,       PCE_KEY_EQUAL },
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
	{ XK_Super_L,     PCE_KEY_LSUPER },
	{ XK_Alt_L,       PCE_KEY_LALT },
	{ XK_Meta_L,      PCE_KEY_LALT },
	{ XK_Mode_switch, PCE_KEY_MODE },
	{ XK_space,       PCE_KEY_SPACE },
	{ XK_Alt_R,       PCE_KEY_RALT },
	{ XK_Meta_R,      PCE_KEY_RALT },
	{ XK_Super_R,     PCE_KEY_RSUPER },
	{ XK_Menu,        PCE_KEY_MENU },
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
	{ XK_KP_6,        PCE_KEY_KP_6 },
	{ XK_KP_1,        PCE_KEY_KP_1 },
	{ XK_KP_2,        PCE_KEY_KP_2 },
	{ XK_KP_3,        PCE_KEY_KP_3 },
	{ XK_KP_Enter,    PCE_KEY_KP_ENTER },
	{ XK_KP_0,        PCE_KEY_KP_0 },
	{ XK_KP_Decimal,  PCE_KEY_KP_PERIOD },
	{ XK_KP_Home,     PCE_KEY_KP_7 },
	{ XK_KP_Up,       PCE_KEY_KP_8 },
	{ XK_KP_Prior,    PCE_KEY_KP_9 },
	{ XK_KP_Left,     PCE_KEY_KP_4 },
	{ XK_KP_Begin,    PCE_KEY_KP_5 },
	{ XK_KP_Right,    PCE_KEY_KP_6 },
	{ XK_KP_End,      PCE_KEY_KP_1 },
	{ XK_KP_Down,     PCE_KEY_KP_2 },
	{ XK_KP_Next,     PCE_KEY_KP_3 },
	{ XK_KP_Insert,   PCE_KEY_KP_0 },
	{ XK_KP_Delete,   PCE_KEY_KP_PERIOD },
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
void xt_grab_mouse (xterm_t *xt, int grab)
{
	if ((xt->grab != 0) == (grab != 0)) {
		return;
	}

	if (grab) {
		int cx, cy;

#if 0
		fprintf (stderr, "xt: grabbing the mouse\n");
#endif

		xt->grab = 1;

		cx = xt->display_w / 2;
		cy = xt->display_h / 2;

		xt->mse_x = cx;
		xt->mse_y = cy;

		XDefineCursor (xt->display, xt->wdw, xt->empty_cursor);

		XGrabPointer (xt->display, xt->wdw, True,
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync, None,  None, CurrentTime
		);

		XWarpPointer (xt->display, None, xt->root, 0, 0, 0, 0, cx, cy);
	}
	else {
#if 0
		fprintf (stderr, "xt: releasing the mouse\n");
#endif

		xt->grab = 0;

		XWarpPointer (xt->display, None, xt->wdw, 0, 0, 0, 0,
			xt->wdw_w / 2, xt->wdw_h / 2
		);

		XUngrabPointer (xt->display, CurrentTime);
		XUndefineCursor (xt->display, xt->wdw);
	}

	XFlush (xt->display);
}

static
void xt_set_window_title (xterm_t *xt, const char *str)
{
	XTextProperty name;

	if (XStringListToTextProperty ((char **) &str, 1, &name) == 0) {
		return;
	}

	XSetWMName (xt->display, xt->wdw, &name);

	XFree (name.value);

	XFlush (xt->display);
}

/*
 * Allocate the backing image
 */
static
int xt_image_alloc (xterm_t *xt, unsigned w, unsigned h)
{
	Visual            *vis;
	XWindowAttributes attrib;
	unsigned          depth;

	vis = DefaultVisual (xt->display, xt->screen);

	XGetWindowAttributes (xt->display, xt->wdw, &attrib);

	depth = attrib.depth;

	xt->img = XCreateImage (xt->display, vis, depth, ZPixmap, 0, NULL, w, h, 8, 0);
	xt->img_buf = malloc (xt->img->bytes_per_line * h);
	xt->img->data = (char *) xt->img_buf;

	return (0);
}

/*
 * Free the backing image
 */
static
void xt_image_free (xterm_t *xt)
{
	XDestroyImage (xt->img);

	xt->img = None;
	xt->img_buf = NULL;
}

/*
 * Decode a bit mask into the first set bit and the number of set bits
 */
static
void xt_decode_mask (unsigned long mask, unsigned *i, unsigned *n)
{
	*i = 0;
	*n = 0;

	while ((mask & 1) == 0) {
		mask = mask >> 1;
		*i += 1;
	}

	while (mask & 1) {
		mask = mask >> 1;
		*n += 1;
	}
}

static inline
unsigned long xt_get_pixel (const unsigned char *src,
	unsigned ri, unsigned rn, unsigned gi, unsigned gn, unsigned bi, unsigned bn)
{
	unsigned long val, tmp;

	tmp = src[0];
	tmp = (tmp << 8) | tmp;
	val = (tmp >> (16 - rn)) << ri;

	tmp = src[1];
	tmp = (tmp << 8) | tmp;
	val |= (tmp >> (16 - gn)) << gi;

	tmp = src[2];
	tmp = (tmp << 8) | tmp;
	val |= (tmp >> (16 - bn)) << bi;

	return (val);
}

/*
 * Render the terminal buffer into the backing image
 */
static
void xt_image_draw (xterm_t *xt, const unsigned char *src, unsigned x, unsigned y, unsigned w, unsigned h)
{
	unsigned char *dst;
	unsigned      i, j, k;
	unsigned      si, di;
	unsigned      ri, rn, gi, gn, bi, bn;
	unsigned      bpp;
	unsigned long val;

	src = src + 3 * w * y;
	dst = xt->img_buf + xt->img->bytes_per_line * y;

	xt_decode_mask (xt->img->red_mask, &ri, &rn);
	xt_decode_mask (xt->img->green_mask, &gi, &gn);
	xt_decode_mask (xt->img->blue_mask, &bi, &bn);

	bpp = xt->img->bits_per_pixel / 8;

	for (j = 0; j < h; j++) {
		si = 3 * x;
		di = bpp * x;

		switch ((bpp << 1) | (xt->img->byte_order == MSBFirst)) {
		case ((2 << 1) | 0):
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				dst[di + 0] = val & 0xff;
				dst[di + 1] = (val >> 8) & 0xff;

				si += 3;
				di += 2;
			}
			break;

		case ((2 << 1) | 1):
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				dst[di + 0] = (val >> 8) & 0xff;
				dst[di + 1] = val & 0xff;

				si += 3;
				di += 2;
			}
			break;

		case ((3 << 1) | 0):
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				dst[di + 0] = val & 0xff;
				dst[di + 1] = (val >> 8) & 0xff;
				dst[di + 2] = (val >> 16) & 0xff;

				si += 3;
				di += 3;
			}
			break;

		case ((3 << 1) | 1):
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				dst[di + 0] = (val >> 16) & 0xff;
				dst[di + 1] = (val >> 8) & 0xff;
				dst[di + 2] = val & 0xff;

				si += 3;
				di += 3;
			}
			break;

		case ((4 << 1) | 0):
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				dst[di + 0] = val & 0xff;
				dst[di + 1] = (val >> 8) & 0xff;
				dst[di + 2] = (val >> 16) & 0xff;
				dst[di + 3] = (val >> 24) & 0xff;

				si += 3;
				di += 4;
			}
			break;

		case ((4 << 1) | 1):
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				dst[di + 0] = (val >> 24) & 0xff;
				dst[di + 1] = (val >> 16) & 0xff;
				dst[di + 2] = (val >> 8) & 0xff;
				dst[di + 3] = val & 0xff;

				si += 3;
				di += 4;
			}
			break;

		default:
			for (i = 0; i < w; i++) {
				val = xt_get_pixel (src + si, ri, rn, gi, gn, bi, bn);

				if (xt->img->byte_order == MSBFirst) {
					for (k = 0; k < bpp; k++) {
						dst[di + bpp - k - 1] = val & 0xff;
						val = val >> 8;
					}
				}
				else {
					for (k = 0; k < bpp; k++) {
						dst[di + k] = val & 0xff;
						val = val >> 8;
					}
				}

				si += 3;
				di += bpp;
			}
			break;
		}

		src += 3 * w;
		dst += xt->img->bytes_per_line;
	}
}

/*
 * Set the window size and reallocate the backing image
 */
static
void xt_set_window_size (xterm_t *xt, unsigned w, unsigned h)
{
	XSizeHints size;

	if ((xt->wdw_w == w) && (xt->wdw_h == h)) {
		return;
	}

	size.flags = PMinSize | PMaxSize;
	size.max_width = w;
	size.min_width = w;
	size.max_height = h;
	size.min_height = h;

	XSetWMNormalHints (xt->display, xt->wdw, &size);

	XResizeWindow (xt->display, xt->wdw, w, h);

	xt->wdw_w = w;
	xt->wdw_h = h;

	xt_image_free (xt);
	xt_image_alloc (xt, w, h);
}

/*
 * Update the window from the terminal buffer
 */
static
void xt_update (xterm_t *xt)
{
	terminal_t          *trm;
	const unsigned char *buf;
	unsigned            fx, fy;
	unsigned            dw, dh;
	unsigned            ux, uy, uw, uh;

	trm = &xt->trm;

	trm_get_scale (trm, trm->w, trm->h, &fx, &fy);

	dw = fx * trm->w;
	dh = fy * trm->h;

	xt_set_window_size (xt, dw, dh);

	buf = trm_scale (trm, trm->buf, trm->w, trm->h, fx, fy);

	ux = fx * trm->update_x;
	uy = fy * trm->update_y;
	uw = fx * trm->update_w;
	uh = fy * trm->update_h;

	xt_image_draw (xt, buf, ux, uy, uw, uh);

	XPutImage (xt->display, xt->wdw, xt->gc, xt->img, ux, uy, ux, uy, uw, uh);
}

/*
 * Map an X11 key symbol to a PCE key symbol
 */
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

/*
 * Send a key event to the emulator core
 */
static
void xt_key_send (xterm_t *xt, KeySym sym, int press)
{
	pce_key_t key;

	key = xt_key_map (xt, sym);

#if 0
	fprintf (stderr, "xt: key map %u -> %u (%s)\n",
		(unsigned) sym, (unsigned) key,
		pce_key_to_string (key)
	);
#endif

	if (key == PCE_KEY_NONE) {
		fprintf (stderr, "xt: unknown key: 0x%04x\n", (unsigned) sym);
	}

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

	if ((sym == XK_grave) && (evt->xkey.state & ControlMask)) {
		xt_grab_mouse (xt, 0);
		trm_set_msg_emu (&xt->trm, "emu.stop", "1");
		return;
	}
	else if ((sym == XK_Print) && (evt->xkey.state == 0)) {
		trm_screenshot (&xt->trm, NULL);
		return;
	}

	xt_key_send (xt, sym, 1);
}

static
void xt_event_keyup (xterm_t *xt, XEvent *evt)
{
	KeySym sym;

	sym = XLookupKeysym (&evt->xkey, 0);

	xt_key_send (xt, sym, 0);
}

static
void xt_event_expose (xterm_t *xt, XEvent *event)
{
	XExposeEvent *evt;

	evt = (XExposeEvent *) event;

	XPutImage (xt->display, xt->wdw, xt->gc, xt->img,
		evt->x, evt->y, evt->x, evt->y, evt->width, evt->height
	);
}

static
void xt_event_button_press (xterm_t *xt, XEvent *event)
{
	unsigned     b;
	XButtonEvent *evt = (XButtonEvent *) event;

	if (xt->grab == 0) {
		xt_grab_mouse (xt, 1);
		return;
	}

	b = evt->state;
	b = ((b & Button1Mask) ? 0x01 : 0) | ((b & Button3Mask) ? 0x02 : 0);
	b ^= (evt->button == Button1) ? 0x01 : 0x00;
	b ^= (evt->button == Button3) ? 0x02 : 0x00;

	trm_set_mouse (&xt->trm, 0, 0, b);
}

static
void xt_event_button_release (xterm_t *xt, XEvent *event)
{
	unsigned     b;
	XButtonEvent *evt = (XButtonEvent *) event;

	b = evt->state;
	b = ((b & Button1Mask) ? 0x01 : 0) | ((b & Button3Mask) ? 0x02 : 0);
	b ^= (evt->button == Button1) ? 0x01 : 0x00;
	b ^= (evt->button == Button3) ? 0x02 : 0x00;

	trm_set_mouse (&xt->trm, 0, 0, b);
}

static
void xt_event_motion (xterm_t *xt, XEvent *event)
{
	int          cx, cy;
	int          dx, dy;
	unsigned     b;
	XMotionEvent *evt;

	if (xt->grab == 0) {
		return;
	}

	evt = (XMotionEvent *) event;

	cx = xt->display_w / 2;
	cy = xt->display_h / 2;

	dx = evt->x_root - xt->mse_x;
	dy = evt->y_root - xt->mse_y;

	if ((dx == 0) && (dy == 0)) {
		return;
	}

	b = evt->state;
	b = ((b & Button1Mask) ? 1 : 0) | ((b & Button2Mask) ? 2 : 0);

	trm_set_mouse (&xt->trm, dx, dy, b);

	XWarpPointer (xt->display, None, xt->root, 0, 0, 0, 0, cx, cy);

	xt->mse_x = cx;
	xt->mse_y = cy;
}

static
void xt_check (xterm_t *xt)
{
	XEvent event;

	while (XPending (xt->display) > 0) {
		XNextEvent (xt->display, &event);

		switch (event.type) {
		case Expose:
			xt_event_expose (xt, &event);
			break;

		case KeyPress:
			xt_event_keydown (xt, &event);
			break;

		case KeyRelease:
			xt_event_keyup (xt, &event);
			break;

		case ButtonPress:
			xt_event_button_press (xt, &event);
			break;

		case ButtonRelease:
			xt_event_button_release (xt, &event);
			break;

		case MotionNotify:
			xt_event_motion (xt, &event);
			break;

		case NoExpose:
		case ConfigureNotify:
			break;

		default:
			break;
		}
	}
}

static
int xt_set_msg_trm (xterm_t *xt, const char *msg, const char *val)
{
	if (val == NULL) {
		val = "";
	}

	if (strcmp (msg, "term.fullscreen") == 0) {
#if 0
		int v;

		v = strtol (val, NULL, 0);

		xt_set_fullscreen (xt, v != 0);
#endif

		return (0);
	}
	else if (strcmp (msg, "term.fullscreen.toggle") == 0) {
		/* xt_set_fullscreen (xt, !xt->fullscreen); */
		return (0);
	}
	else if (strcmp (msg, "term.grab") == 0) {
		xt_grab_mouse (xt, 1);
		return (0);
	}
	else if (strcmp (msg, "term.release") == 0) {
		xt_grab_mouse (xt, 0);
		return (0);
	}
	else if (strcmp (msg, "term.title") == 0) {
		xt_set_window_title (xt, val);
		return (0);
	}

	return (-1);
}

static
void xt_del (xterm_t *xt)
{
	free (xt);
}

static
int xt_init_pointer (xterm_t *xt)
{
	Pixmap               mask;
	static unsigned char map[16] = { 0 };

	mask = XCreatePixmapFromBitmapData (xt->display, xt->wdw, (char *) map,
		1, 1, 0, 0, 1
	);

	xt->empty_cursor = XCreatePixmapCursor (xt->display, mask, mask,
		(XColor *) map, (XColor *) map, 0, 0
	);

	XFreePixmap (xt->display, mask);

	return (0);
}

static
int xt_open_window (xterm_t *xt, unsigned w, unsigned h)
{
	char        *argv0 = "pce";
	XSizeHints  size;
	XWMHints    wm;
	XClassHint  cls;
	unsigned    fx, fy;

	if ((w == 0) || (h == 0)) {
		w = 640;
		h = 480;
	}

	trm_get_scale (&xt->trm, w, h, &fx, &fy);

	w *= fx;
	h *= fy;

	xt->wdw_w = w;
	xt->wdw_h = h;

	xt->wdw = XCreateSimpleWindow (xt->display,
		RootWindow (xt->display, xt->screen),
		0, 0, w, h, 0,
		WhitePixel (xt->display, xt->screen),
		BlackPixel (xt->display, xt->screen)
	);

	xt_image_alloc (xt, xt->wdw_w, xt->wdw_h);

	size.flags = PMinSize | PMaxSize;
	size.max_width = xt->wdw_w;
	size.min_width = xt->wdw_w;
	size.max_height = xt->wdw_h;
	size.min_height = xt->wdw_h;

	wm.flags = StateHint | IconPixmapHint | InputHint;
	wm.initial_state = NormalState;
	wm.input = True;
	wm.icon_pixmap = None;

	cls.res_name = "pce";
	cls.res_class = "PCE";

	XSetWMProperties (xt->display, xt->wdw, NULL, NULL, &argv0, 1,
		&size, &wm, &cls
	);

	xt_set_window_title (xt, "pce");

	XSelectInput (xt->display, xt->wdw,
		ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
		StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask
	);

	return (0);
}

static
int xt_open (xterm_t *xt, unsigned w, unsigned h)
{
	XGCValues values;

	xt->mse_x = 0;
	xt->mse_y = 0;

	xt->grab = 0;

	xt->display = XOpenDisplay (NULL);
	if (xt->display == NULL) {
		return (1);
	}

	xt->screen = DefaultScreen (xt->display);
	xt->display_w = DisplayWidth (xt->display, xt->screen);
	xt->display_h = DisplayHeight (xt->display, xt->screen);
	xt->root = RootWindow (xt->display, xt->screen);

	if (xt_open_window (xt, w, h)) {
		XCloseDisplay (xt->display);
		return (1);
	}

	xt->gc = XCreateGC (xt->display, xt->wdw, 0, &values);

	xt_init_pointer (xt);

	XMapWindow (xt->display, xt->wdw);

	XFlush (xt->display);

	return (0);
}

static
int xt_close (xterm_t *xt)
{
	xt_grab_mouse (xt, 0);

	xt_image_free (xt);

	XFreeCursor (xt->display, xt->empty_cursor);
	XFreeGC (xt->display, xt->gc);
	XDestroyWindow (xt->display, xt->wdw);
	XCloseDisplay (xt->display);

	xt->display = NULL;
	xt->root = None;
	xt->wdw = None;
	xt->gc = None;
	xt->empty_cursor = None;

	return (0);
}

static
void xt_init (xterm_t *xt, ini_sct_t *ini)
{
	trm_init (&xt->trm, xt);

	xt->trm.del = (void *) xt_del;
	xt->trm.open = (void *) xt_open;
	xt->trm.close = (void *) xt_close;
	xt->trm.set_msg_trm = (void *) xt_set_msg_trm;
	xt->trm.update = (void *) xt_update;
	xt->trm.check = (void *) xt_check;

	xt->display = NULL;
	xt->root = None;
	xt->wdw = None;
	xt->gc = None;
	xt->img = NULL;
	xt->img_buf = NULL;

	xt->empty_cursor = None;

	xt->wdw_w = 0;
	xt->wdw_h = 0;

	xt->mse_x = 0;
	xt->mse_y = 0;

	xt->grab = 0;
}

terminal_t *xt_new (ini_sct_t *ini)
{
	xterm_t *xt;

	xt = malloc (sizeof (xterm_t));
	if (xt == NULL) {
		return (NULL);
	}

	xt_init (xt, ini);

	return (&xt->trm);
}
