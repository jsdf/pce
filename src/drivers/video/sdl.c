/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/sdl.c                                      *
 * Created:     2003-09-15 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include <drivers/video/terminal.h>
#include <drivers/video/keys.h>
#include <drivers/video/sdl.h>


typedef struct {
	SDLKey    sdlkey;
	pce_key_t pcekey;
} sdl_keymap_t;


static int sdl_set_window_size (sdl_t *sdl, unsigned w, unsigned h, int force);


static sdl_keymap_t keymap[] = {
	{ SDLK_ESCAPE,       PCE_KEY_ESC },
	{ SDLK_F1,           PCE_KEY_F1 },
	{ SDLK_F2,           PCE_KEY_F2 },
	{ SDLK_F3,           PCE_KEY_F3 },
	{ SDLK_F4,           PCE_KEY_F4 },
	{ SDLK_F5,           PCE_KEY_F5 },
	{ SDLK_F6,           PCE_KEY_F6 },
	{ SDLK_F7,           PCE_KEY_F7 },
	{ SDLK_F8,           PCE_KEY_F8 },
	{ SDLK_F9,           PCE_KEY_F9 },
	{ SDLK_F10,          PCE_KEY_F10 },
	{ SDLK_F11,          PCE_KEY_F11 },
	{ SDLK_F12,          PCE_KEY_F12 },

	{ SDLK_PRINT,        PCE_KEY_PRTSCN },
	{ SDLK_SCROLLOCK,    PCE_KEY_SCRLK },
	{ SDLK_PAUSE,        PCE_KEY_PAUSE },

	{ SDLK_BACKQUOTE,    PCE_KEY_BACKQUOTE },
	{ SDLK_1,            PCE_KEY_1 },
	{ SDLK_2,            PCE_KEY_2 },
	{ SDLK_3,            PCE_KEY_3 },
	{ SDLK_4,            PCE_KEY_4 },
	{ SDLK_5,            PCE_KEY_5 },
	{ SDLK_6,            PCE_KEY_6 },
	{ SDLK_7,            PCE_KEY_7 },
	{ SDLK_8,            PCE_KEY_8 },
	{ SDLK_9,            PCE_KEY_9 },
	{ SDLK_0,            PCE_KEY_0 },
	{ SDLK_MINUS,        PCE_KEY_MINUS },
	{ SDLK_EQUALS,       PCE_KEY_EQUAL },
	{ SDLK_BACKSPACE,    PCE_KEY_BACKSPACE },

	{ SDLK_TAB,          PCE_KEY_TAB },
	{ SDLK_q,            PCE_KEY_Q },
	{ SDLK_w,            PCE_KEY_W },
	{ SDLK_e,            PCE_KEY_E },
	{ SDLK_r,            PCE_KEY_R },
	{ SDLK_t,            PCE_KEY_T },
	{ SDLK_y,            PCE_KEY_Y },
	{ SDLK_u,            PCE_KEY_U },
	{ SDLK_i,            PCE_KEY_I },
	{ SDLK_o,            PCE_KEY_O },
	{ SDLK_p,            PCE_KEY_P },
	{ SDLK_LEFTBRACKET,  PCE_KEY_LBRACKET },
	{ SDLK_RIGHTBRACKET, PCE_KEY_RBRACKET },
	{ SDLK_RETURN,       PCE_KEY_RETURN },

	{ SDLK_CAPSLOCK,     PCE_KEY_CAPSLOCK },
	{ SDLK_a,            PCE_KEY_A },
	{ SDLK_s,            PCE_KEY_S },
	{ SDLK_d,            PCE_KEY_D },
	{ SDLK_f,            PCE_KEY_F },
	{ SDLK_g,            PCE_KEY_G },
	{ SDLK_h,            PCE_KEY_H },
	{ SDLK_j,            PCE_KEY_J },
	{ SDLK_k,            PCE_KEY_K },
	{ SDLK_l,            PCE_KEY_L },
	{ SDLK_SEMICOLON,    PCE_KEY_SEMICOLON },
	{ SDLK_QUOTE,        PCE_KEY_QUOTE },
	{ SDLK_BACKSLASH,    PCE_KEY_BACKSLASH },

	{ SDLK_LSHIFT,       PCE_KEY_LSHIFT },
	{ SDLK_LESS,         PCE_KEY_LESS },
	{ SDLK_z,            PCE_KEY_Z },
	{ SDLK_x,            PCE_KEY_X },
	{ SDLK_c,            PCE_KEY_C },
	{ SDLK_v,            PCE_KEY_V },
	{ SDLK_b,            PCE_KEY_B },
	{ SDLK_n,            PCE_KEY_N },
	{ SDLK_m,            PCE_KEY_M },
	{ SDLK_COMMA,        PCE_KEY_COMMA },
	{ SDLK_PERIOD,       PCE_KEY_PERIOD },
	{ SDLK_SLASH,        PCE_KEY_SLASH },
	{ SDLK_RSHIFT,       PCE_KEY_RSHIFT },

	{ SDLK_LCTRL,        PCE_KEY_LCTRL },
	{ SDLK_LMETA,        PCE_KEY_LMETA },
	{ SDLK_LSUPER,       PCE_KEY_LSUPER },
	{ SDLK_LALT,         PCE_KEY_LALT },
	{ SDLK_MODE,         PCE_KEY_MODE },
	{ SDLK_SPACE,        PCE_KEY_SPACE },
	{ SDLK_RALT,         PCE_KEY_RALT },
	{ SDLK_RMETA,        PCE_KEY_RMETA },
	{ SDLK_RSUPER,       PCE_KEY_RSUPER },
	{ SDLK_MENU,         PCE_KEY_MENU },
	{ SDLK_RCTRL,        PCE_KEY_RCTRL },

	{ SDLK_NUMLOCK,      PCE_KEY_NUMLOCK },
	{ SDLK_KP_DIVIDE,    PCE_KEY_KP_SLASH },
	{ SDLK_KP_MULTIPLY,  PCE_KEY_KP_STAR },
	{ SDLK_KP_MINUS,     PCE_KEY_KP_MINUS },
	{ SDLK_KP7,          PCE_KEY_KP_7 },
	{ SDLK_KP8,          PCE_KEY_KP_8 },
	{ SDLK_KP9,          PCE_KEY_KP_9 },
	{ SDLK_KP_PLUS,      PCE_KEY_KP_PLUS },
	{ SDLK_KP4,          PCE_KEY_KP_4 },
	{ SDLK_KP5,          PCE_KEY_KP_5 },
	{ SDLK_KP6,          PCE_KEY_KP_6 },
	{ SDLK_KP1,          PCE_KEY_KP_1 },
	{ SDLK_KP2,          PCE_KEY_KP_2 },
	{ SDLK_KP3,          PCE_KEY_KP_3 },
	{ SDLK_KP_ENTER,     PCE_KEY_KP_ENTER },
	{ SDLK_KP0,          PCE_KEY_KP_0 },
	{ SDLK_KP_PERIOD,    PCE_KEY_KP_PERIOD },
	{ SDLK_INSERT,       PCE_KEY_INS },
	{ SDLK_HOME,         PCE_KEY_HOME },
	{ SDLK_PAGEUP,       PCE_KEY_PAGEUP },
	{ SDLK_DELETE,       PCE_KEY_DEL },
	{ SDLK_END,          PCE_KEY_END },
	{ SDLK_PAGEDOWN,     PCE_KEY_PAGEDN },
	{ SDLK_UP,           PCE_KEY_UP },
	{ SDLK_LEFT,         PCE_KEY_LEFT },
	{ SDLK_DOWN,         PCE_KEY_DOWN },
	{ SDLK_RIGHT,        PCE_KEY_RIGHT },
	{ 0,                 PCE_KEY_NONE }
};


static
void sdl_grab_mouse (sdl_t *sdl, int grab)
{
	if (grab) {
		sdl->grab = 1;
		SDL_ShowCursor (0);
		SDL_WM_GrabInput (SDL_GRAB_ON);
	}
	else {
		sdl->grab = 0;
		SDL_ShowCursor (1);
		SDL_WM_GrabInput (SDL_GRAB_OFF);
	}
}

static
void sdl_set_fullscreen (sdl_t *sdl, int val)
{
	if ((val != 0) == (sdl->fullscreen != 0)) {
		return;
	}

	if (sdl->scr != NULL) {
		SDL_WM_ToggleFullScreen (sdl->scr);
	}

	sdl->fullscreen = (val != 0);

#ifdef PCE_HOST_WINDOWS
	/*
	 * SDL under windows does not support toggling full screen mode
	 * after a surface has been created. Setting the window size
	 * to the current size will free and then reallocate the
	 * surface.
	 */
	sdl_set_window_size (sdl, sdl->wdw_w, sdl->wdw_h, 1);

	/* Invalidate the entire terminal to force an update. */
	sdl->trm.update_x = 0;
	sdl->trm.update_y = 0;
	sdl->trm.update_w = sdl->trm.w;
	sdl->trm.update_h = sdl->trm.h;

	trm_update (&sdl->trm);
#endif
}

static
int sdl_set_window_size (sdl_t *sdl, unsigned w, unsigned h, int force)
{
	unsigned long flags;

	if (sdl->scr != NULL) {
		if (force == 0) {
			if ((sdl->wdw_w == w) && (sdl->wdw_h == h)) {
				return (0);
			}
		}

		SDL_FreeSurface (sdl->scr);
	}

	sdl->scr = NULL;

	flags = SDL_HWSURFACE;

	if (sdl->fullscreen) {
		flags |= SDL_FULLSCREEN;
	}

	if (sdl->dsp_bpp == 2) {
		sdl->scr = SDL_SetVideoMode (w, h, 16, flags);
		sdl->scr_bpp = 2;
	}
	else if (sdl->dsp_bpp == 4) {
		sdl->scr = SDL_SetVideoMode (w, h, 32, flags);
		sdl->scr_bpp = 4;
	}

	if (sdl->scr == NULL) {
		sdl->scr = SDL_SetVideoMode (w, h, 16, flags);
		sdl->scr_bpp = 2;
	}

	if (sdl->scr == NULL) {
		return (1);
	}

	sdl->wdw_w = w;
	sdl->wdw_h = h;

	return (0);
}

static
unsigned sdl_map_key (SDLKey key)
{
	unsigned i;

	i = 0;
	while (keymap[i].pcekey != PCE_KEY_NONE) {
		if (keymap[i].sdlkey == key) {
			return (keymap[i].pcekey);
		}

		i += 1;
	}

	return (PCE_KEY_NONE);
}

static
void sdl_update (sdl_t *sdl)
{
	SDL_Surface         *s;
	Uint32              rmask, gmask, bmask;
	terminal_t          *trm;
	SDL_Rect            dst;
	const unsigned char *buf;
	unsigned            dw, dh;
	unsigned            ux, uy, uw, uh;
	unsigned            fx, fy;
	unsigned            bx, by;

	trm = &sdl->trm;

	trm_get_scale (&sdl->trm, trm->w, trm->h, &fx, &fy);

	dw = fx * trm->w;
	dh = fy * trm->h;

	bx = sdl->border[0] + sdl->border[2];
	by = sdl->border[1] + sdl->border[3];

	if (sdl_set_window_size (sdl, dw + bx, dh + by, 0)) {
		return;
	}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x00ff0000;
	gmask = 0x0000ff00;
	bmask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
#endif

	buf = trm_scale (trm, trm->buf, trm->w, trm->h, fx, fy);

	ux = fx * trm->update_x;
	uy = fy * trm->update_y;
	uw = fx * trm->update_w;
	uh = fy * trm->update_h;

	s = SDL_CreateRGBSurfaceFrom (
		(char *) buf + 3 * (dw * uy + ux), uw, uh, 24, 3 * dw,
		rmask, gmask, bmask, 0
	);

	dst.x = ux + sdl->border[0];
	dst.y = uy + sdl->border[1];

	if (s == NULL) {
		return;
	}

	if (SDL_BlitSurface (s, NULL, sdl->scr, &dst) != 0) {
		fprintf (stderr, "sdl: blit error\n");
	}

	SDL_FreeSurface (s);

	SDL_Flip (sdl->scr);
}

static
void sdl_event_keydown (sdl_t *sdl, SDLKey key)
{
	SDLMod    mod;
	pce_key_t pcekey;

	mod = SDL_GetModState();

	if ((key == SDLK_BACKQUOTE) && (mod & KMOD_LCTRL)) {
		sdl_grab_mouse (sdl, 0);
		sdl_set_fullscreen (sdl, 0);
		trm_set_msg_emu (&sdl->trm, "emu.stop", "1");
		return;
	}
	else if ((key == SDLK_PRINT) && ((mod && KMOD_SHIFT) == 0)) {
		trm_screenshot (&sdl->trm, NULL);
		return;
	}

	pcekey = sdl_map_key (key);

	if (pcekey == PCE_KEY_NONE) {
		fprintf (stderr, "sdl: key = %04x\n", (unsigned) key);
		return;
	}

	trm_set_key (&sdl->trm, PCE_KEY_EVENT_DOWN, pcekey);

	if (key == SDLK_NUMLOCK) {
		trm_set_key (&sdl->trm, PCE_KEY_EVENT_UP, pcekey);
	}
}

static
void sdl_event_keyup (sdl_t *sdl, SDLKey key)
{
	pce_key_t pcekey;

	pcekey = sdl_map_key (key);

	if (pcekey != PCE_KEY_NONE) {
		if (key == SDLK_NUMLOCK) {
			trm_set_key (&sdl->trm, 1, pcekey);
		}

		trm_set_key (&sdl->trm, 2, pcekey);
	}
}

static
void sdl_event_mouse_button (sdl_t *sdl)
{
	unsigned but, val;

	val = 0;
	but = SDL_GetMouseState (NULL, NULL);

	if (but & SDL_BUTTON (1)) {
		val |= 1;
	}

	if (but & SDL_BUTTON (3)) {
		val |= 2;
	}

	if (sdl->grab == 0) {
		if (but == 0) {
			sdl_grab_mouse (sdl, 1);
		}

		return;
	}

	if (sdl->trm.set_mouse == NULL) {
		return;
	}

	trm_set_mouse (&sdl->trm, 0, 0, val);
}

static
void sdl_event_mouse_motion (sdl_t *sdl, int dx, int dy)
{
	unsigned but, val;

	if (sdl->grab == 0) {
		return;
	}

	if (sdl->trm.set_mouse == NULL) {
		return;
	}

	val = 0;
	but = SDL_GetMouseState (NULL, NULL);

	if (but & SDL_BUTTON (1)) {
		val |= 1;
	}

	if (but & SDL_BUTTON (3)) {
		val |= 2;
	}

	trm_set_mouse (&sdl->trm, dx, dy, val);
}

static
void sdl_check (sdl_t *sdl)
{
	unsigned  i;
	SDL_Event evt;

	i = 0;
	while (SDL_PollEvent (&evt) && (i < 8)) {
		switch (evt.type) {
		case SDL_KEYDOWN:
			sdl_event_keydown (sdl, evt.key.keysym.sym);
			break;

		case SDL_KEYUP:
			sdl_event_keyup (sdl, evt.key.keysym.sym);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			sdl_event_mouse_button (sdl);
			break;

		case SDL_MOUSEMOTION:
			sdl_event_mouse_motion (sdl, evt.motion.xrel, evt.motion.yrel);
			break;

		case SDL_VIDEORESIZE:
			break;

		case SDL_QUIT:
			sdl_grab_mouse (sdl, 0);
			trm_set_msg_emu (&sdl->trm, "emu.exit", "1");
			break;

		default:
			break;
		}

		i += 1;
	}
}

static
int sdl_set_msg_trm (sdl_t *sdl, const char *msg, const char *val)
{
	if (val == NULL) {
		val = "";
	}

	if (strcmp (msg, "term.grab") == 0) {
		sdl_grab_mouse (sdl, 1);
		return (0);
	}
	else if (strcmp (msg, "term.release") == 0) {
		sdl_grab_mouse (sdl, 0);
		return (0);
	}
	else if (strcmp (msg, "term.title") == 0) {
		SDL_WM_SetCaption (val, val);
		return (0);
	}
	else if (strcmp (msg, "term.set_border_x") == 0) {
		sdl->border[0] = strtoul (val, NULL, 0);
		sdl->border[2] = sdl->border[0];
		sdl_update (sdl);
		return (0);
	}
	else if (strcmp (msg, "term.set_border_y") == 0) {
		sdl->border[1] = strtoul (val, NULL, 0);
		sdl->border[3] = sdl->border[0];
		sdl_update (sdl);
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen.toggle") == 0) {
		sdl_set_fullscreen (sdl, !sdl->fullscreen);
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen") == 0) {
		int v;

		v = strtol (val, NULL, 0);

		sdl_set_fullscreen (sdl, v != 0);

		return (0);
	}

	return (-1);
}

static
void sdl_del (sdl_t *sdl)
{
	free (sdl);
}

static
int sdl_open (sdl_t *sdl, unsigned w, unsigned h)
{
	unsigned             fx, fy, bx, by;
	const SDL_VideoInfo *inf;

	if ((w == 0) || (h == 0)) {
		w = 640;
		h = 480;
	}

	trm_get_scale (&sdl->trm, w, h, &fx, &fy);

	bx = sdl->border[0] + sdl->border[2];
	by = sdl->border[1] + sdl->border[3];

	if (SDL_WasInit (SDL_INIT_VIDEO) == 0) {
		if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0) {
			return (1);
		}
	}

	inf = SDL_GetVideoInfo();

	sdl->dsp_bpp = inf->vfmt->BytesPerPixel;
	sdl->scr_bpp = 0;

	SDL_WM_SetCaption ("pce", "pce");
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EventState (SDL_MOUSEMOTION, SDL_ENABLE);

	sdl_set_window_size (sdl, fx * w + bx, fy * h + by, 0);

	return (0);
}

static
int sdl_close (sdl_t *sdl)
{
	sdl_grab_mouse (sdl, 0);

	if (sdl->scr != NULL) {
		SDL_FreeSurface (sdl->scr);

		sdl->scr = NULL;
	}

	return (0);
}

static
void sdl_init (sdl_t *sdl, ini_sct_t *sct)
{
	int fs;

	trm_init (&sdl->trm, sdl);

	sdl->trm.del = (void *) sdl_del;
	sdl->trm.open = (void *) sdl_open;
	sdl->trm.close = (void *) sdl_close;
	sdl->trm.set_msg_trm = (void *) sdl_set_msg_trm;
	sdl->trm.update = (void *) sdl_update;
	sdl->trm.check = (void *) sdl_check;

	sdl->scr = NULL;

	sdl->wdw_w = 0;
	sdl->wdw_h = 0;

	ini_get_bool (sct, "fullscreen", &fs, 0);
	sdl->fullscreen = (fs != 0);

	ini_get_uint16 (sct, "border", &sdl->border[0], 0);
	sdl->border[1] = sdl->border[0];
	sdl->border[2] = sdl->border[0];
	sdl->border[3] = sdl->border[0];

	sdl->grab = 0;

	sdl->dsp_bpp = 0;
	sdl->scr_bpp = 0;
}

terminal_t *sdl_new (ini_sct_t *sct)
{
	sdl_t *sdl;

	sdl = malloc (sizeof (sdl_t));
	if (sdl == NULL) {
		return (NULL);
	}

	sdl_init (sdl, sct);

	return (&sdl->trm);
}
