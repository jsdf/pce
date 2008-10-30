/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/terminal/sdl.c                                           *
 * Created:     2003-09-15 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <SDL.h>

#include <terminal/terminal.h>
#include <terminal/keys.h>
#include <terminal/font.h>
#include <terminal/sdl.h>


#define PCESDL_UPDATE_NONE  0
#define PCESDL_UPDATE_NOW   1
#define PCESDL_UPDATE_DELAY 2


typedef struct {
	SDLKey    sdlkey;
	pce_key_t pcekey;
} sdl_keymap_t;


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
	{ SDLK_LALT,         PCE_KEY_LALT },
	{ SDLK_SPACE,        PCE_KEY_SPACE },
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
unsigned char sdl_colmap[16][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x0a, 0x0a, 0xb9 },
	{ 0x0a, 0xc3, 0x0a },
	{ 0x14, 0xa0, 0xa0 },
	{ 0xa7, 0x0a, 0x0a },
	{ 0xa7, 0x00, 0xa7 },
	{ 0xa5, 0xa5, 0x28 },
	{ 0xc5, 0xc5, 0xc5 },
	{ 0x64, 0x64, 0x64 },
	{ 0x0a, 0x0a, 0xff },
	{ 0x0a, 0xff, 0x0a },
	{ 0x0a, 0xff, 0xff },
	{ 0xff, 0x0a, 0x0a },
	{ 0xff, 0x0a, 0xff },
	{ 0xff, 0xff, 0x00 },
	{ 0xff, 0xff, 0xff }
};


static
int sdl_set_font_psf (sdl_t *sdl, const char *fname)
{
	FILE          *fp;
	unsigned char buf[4];

	fp = fopen (fname, "rb");
	if (fp == NULL) {
		return (1);
	}

	if (fread (buf, 1, 4, fp) != 4) {
		fclose (fp);
		return (1);
	}

	if ((buf[0] != 0x36) || (buf[1] != 0x04)) {
		fclose (fp);
		return (1);
	}

	sdl->font_w = 8;
	sdl->font_h = buf[3];
	sdl->font = (unsigned char *) malloc (256 * sdl->font_h);

	fread (sdl->font, 1, 256 * sdl->font_h, fp);

	fclose (fp);

	return (0);
}

static
Uint32 sdl_get_col (sdl_t *sdl, unsigned idx)
{
	Uint32 col;

	idx &= 0xff;

	col = SDL_MapRGB (sdl->scr->format,
		sdl->colmap[idx][0], sdl->colmap[idx][1], sdl->colmap[idx][2]
	);

	return (col);
}

static
void sdl_set_upd_rct (sdl_t *sdl, unsigned x, unsigned y, unsigned w, unsigned h)
{
	if (x < sdl->upd_x1) {
		sdl->upd_x1 = x;
	}

	if (y < sdl->upd_y1) {
		sdl->upd_y1 = y;
	}

	if ((x + w) > sdl->upd_x2) {
		sdl->upd_x2 = x + w - 1;
	}

	if ((y + h) > sdl->upd_y2) {
		sdl->upd_y2 = y + h - 1;
	}
}

static
void sdl_clr_upd_rct (sdl_t *sdl)
{
	sdl->upd_x1 = sdl->pxl_w;
	sdl->upd_y1 = sdl->pxl_h;
	sdl->upd_x2 = 0;
	sdl->upd_y2 = 0;
}

static
void sdl_update (sdl_t *sdl)
{
	unsigned w, h;

	if ((sdl->upd_x2 < sdl->upd_x1) || (sdl->upd_y2 < sdl->upd_y1)) {
		return;
	}

	w = sdl->upd_x2 - sdl->upd_x1 + 1;
	h = sdl->upd_y2 - sdl->upd_y1 + 1;
	SDL_UpdateRect (sdl->scr, sdl->upd_x1, sdl->upd_y1, w, h);

	sdl_clr_upd_rct (sdl);
}

static
int sdl_set_window_size (sdl_t *sdl, unsigned w, unsigned h)
{
	if (sdl->scr != NULL) {
		if ((sdl->wdw_w == w) && (sdl->wdw_h == h)) {
			return (0);
		}

		SDL_FreeSurface (sdl->scr);
	}

	sdl->scr = NULL;

	if (sdl->dsp_bpp == 2) {
		sdl->scr = SDL_SetVideoMode (w, h, 16, SDL_HWSURFACE | SDL_RESIZABLE);
		sdl->scr_bpp = 2;
	}
	else if (sdl->dsp_bpp == 4) {
		sdl->scr = SDL_SetVideoMode (w, h, 32, SDL_HWSURFACE | SDL_RESIZABLE);
		sdl->scr_bpp = 4;
	}

	if (sdl->scr == NULL) {
		sdl->scr = SDL_SetVideoMode (w, h, 16, SDL_SWSURFACE | SDL_RESIZABLE);
		sdl->scr_bpp = 2;
	}

	if (sdl->scr == NULL) {
		return (1);
	}

	sdl->wdw_w = w;
	sdl->wdw_h = h;

	trm_smap_set_map (&sdl->smap, sdl->pxl_w, sdl->pxl_h, w, h);

	return (0);
}

static
void sdl_grab_mouse (sdl_t *sdl, int grab)
{
	if (grab) {
		if (sdl->grab == 0) {
			sdl->grab = 1;
			SDL_ShowCursor (0);
			SDL_WM_GrabInput (SDL_GRAB_ON);
		}
	}
	else {
		if (sdl->grab) {
			sdl->grab = 0;
			SDL_ShowCursor (1);
			SDL_WM_GrabInput (SDL_GRAB_OFF);
		}
	}
}

static
void sdl_init_font (sdl_t *sdl, ini_sct_t *sct)
{
	const char *str;

	sdl->font = NULL;

	ini_get_string (sct, "font", &str, NULL);
	if (str != NULL) {
		if (sdl_set_font_psf (sdl, str)) {
			fprintf (stderr, "sdl: loading PSF font %s failed\n", str);
		}
	}

	if (sdl->font == NULL) {
		sdl->font_w = 8;
		sdl->font_h = 16;
		sdl->font = malloc (256 * 16);
		memcpy (sdl->font, fnt_8x16, 256 * 16);
	}
}

terminal_t *sdl_new (ini_sct_t *sct)
{
	sdl_t               *sdl;
	const char          *str;
	const SDL_VideoInfo *inf;

	sdl = (sdl_t *) malloc (sizeof (sdl_t));
	if (sdl == NULL) {
		return (NULL);
	}

	trm_init (&sdl->trm);

	sdl->trm.ext = sdl;

	sdl->trm.del = (trm_del_f) &sdl_del;
	sdl->trm.set_mode = (trm_set_mode_f) &sdl_set_mode;
	sdl->trm.set_size = (trm_set_size_f) &sdl_set_size;
	sdl->trm.set_map = (trm_set_map_f) &sdl_set_map;
	sdl->trm.set_col = (trm_set_col_f) &sdl_set_col;
	sdl->trm.set_crs = (trm_set_crs_f) &sdl_set_crs;
	sdl->trm.set_pos = (trm_set_pos_f) &sdl_set_pos;
	sdl->trm.set_chr = (trm_set_chr_f) &sdl_set_chr;
	sdl->trm.set_pxl = (trm_set_pxl_f) &sdl_set_pxl;
	sdl->trm.set_rct = (trm_set_rct_f) &sdl_set_rct;
	sdl->trm.check = (trm_check_f) &sdl_check;

	if (SDL_Init (SDL_INIT_VIDEO) < 0) {
		return (NULL);
	}

	sdl->scr = NULL;

	sdl->mode = TERM_MODE_TEXT;
	sdl->txt_w = 80;
	sdl->txt_h = 25;
	sdl->txt_buf = (unsigned char *) malloc (3 * 80 * 25);
	memset (sdl->txt_buf, 0, 3 * 80 * 25);

	sdl_init_font (sdl, sct);

	sdl->crs_x = 0;
	sdl->crs_y = 0;
	sdl->crs_y1 = sdl->font_h - 2;
	sdl->crs_y2 = sdl->font_h - 1;
	sdl->crs_on = 0;

	memcpy (sdl->colmap, sdl_colmap, 16 * 3);

	sdl->fgidx = 7;
	sdl->bgidx = 0;

	sdl->pxl_w = sdl->txt_w * sdl->font_w;
	sdl->pxl_h = sdl->txt_h * sdl->font_h;

	sdl->wdw_w = sdl->pxl_w;
	sdl->wdw_h = sdl->pxl_h;

	trm_smap_init (&sdl->smap);

	sdl_clr_upd_rct (sdl);

	sdl->magic = 0;

	sdl->grab = 0;

	ini_get_uint16 (sct, "update_delay", &sdl->upd_freq, 100);

	ini_get_string (sct, "update_text", &str, "now");
	if (strcmp (str, "now") == 0) {
		sdl->upd_text = PCESDL_UPDATE_NOW;
	}
	else {
		sdl->upd_text = PCESDL_UPDATE_DELAY;
	}

	ini_get_string (sct, "update_graph", &str, "delay");
	if (strcmp (str, "now") == 0) {
		sdl->upd_graph = PCESDL_UPDATE_NOW;
	}
	else {
		sdl->upd_graph = PCESDL_UPDATE_DELAY;
	}

	inf = SDL_GetVideoInfo();
	sdl->dsp_bpp = inf->vfmt->BytesPerPixel;

	if (sdl_set_window_size (sdl, sdl->pxl_w, sdl->pxl_h)) {
		free (sdl);
		return (NULL);
	}

	sdl->fg = SDL_MapRGB (sdl->scr->format, 0xc0, 0xc0, 0xc0);
	sdl->fg = SDL_MapRGB (sdl->scr->format, 0xff, 0xff, 0xff);

	SDL_WM_SetCaption ("pce", "pce");
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EventState (SDL_MOUSEMOTION, SDL_ENABLE);

	return (&sdl->trm);
}

void sdl_del (sdl_t *sdl)
{
	if (sdl != NULL) {
		sdl_grab_mouse (sdl, 0);

		if (sdl->scr != NULL) {
			SDL_FreeSurface (sdl->scr);
		}

		trm_smap_free (&sdl->smap);

		free (sdl->txt_buf);
		free (sdl->font);
		free (sdl);
	}

	SDL_Quit();
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
void sdl_magic (sdl_t *sdl, SDLKey key)
{
	if (key == SDLK_f) {
		if (sdl->scr != NULL) {
			SDL_WM_ToggleFullScreen (sdl->scr);
		}
	}
	else if (key == SDLK_i) {
		trm_set_msg (&sdl->trm, "emu.idle.toggle", "");
	}
	else if (key == SDLK_p) {
		trm_set_msg (&sdl->trm, "emu.pause.toggle", "");
	}
	else if (key == SDLK_q) {
		sdl_grab_mouse (sdl, 0);
		trm_set_msg (&sdl->trm, "emu.exit", "1");
	}
	else if (key == SDLK_r) {
		trm_set_msg (&sdl->trm, "video.redraw", "1");
	}
	else if (key == SDLK_s) {
		sdl_grab_mouse (sdl, 0);
		trm_set_msg (&sdl->trm, "emu.stop", "1");
	}
	else if (key == SDLK_t) {
		trm_set_msg (&sdl->trm, "emu.realtime.toggle", "");
	}
}

static
void sdls_set_rct (sdl_t *sdl, unsigned x, unsigned y, unsigned w, unsigned h, Uint32 col)
{
	unsigned i, j;
	Uint8    *p;

	if (sdl->scr_bpp == 1) {
		p = (Uint8 *) sdl->scr->pixels + y * sdl->scr->pitch
			+ x * sdl->scr->format->BytesPerPixel;

		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				*p = col;
				p += 1;
			}

			p += sdl->scr->pitch - w;
		}
	}
	else if (sdl->scr_bpp == 2) {
		p = (Uint8 *) sdl->scr->pixels + y * sdl->scr->pitch
			+ x * sdl->scr->format->BytesPerPixel;

		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				*(Uint16 *)p = col;
				p += 2;
			}

			p += sdl->scr->pitch - (w << 1);
		}
	}
	else if (sdl->scr_bpp == 4) {
		p = (Uint8 *) sdl->scr->pixels + y * sdl->scr->pitch
			+ x * sdl->scr->format->BytesPerPixel;

		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				*(Uint32 *)p = col;
				p += 4;
			}

			p += sdl->scr->pitch - (w << 2);
		}
	}
}

static
void sdl_set_pixel (sdl_t *sdl, unsigned x, unsigned y, Uint32 col)
{
	unsigned sx, sy, sw, sh;

	trm_smap_get_pixel (&sdl->smap, x, y, &sx, &sy, &sw, &sh);

	sdl_set_upd_rct (sdl, sx, sy, sw, sh);

	sdls_set_rct (sdl, sx, sy, sw, sh, col);
}

static
void sdl_set_rect (sdl_t *sdl, unsigned x, unsigned y, unsigned w, unsigned h, Uint32 col)
{
	unsigned x1, y1, w1, h1;
	unsigned x2, y2, w2, h2;

	trm_smap_get_pixel (&sdl->smap, x, y, &x1, &y1, &w1, &h1);
	trm_smap_get_pixel (&sdl->smap, x + w - 1, y + h - 1, &x2, &y2, &w2, &h2);

	w1 = x2 - x1 + w2;
	h1 = y2 - y1 + h2;

	sdl_set_upd_rct (sdl, x1, y1, w1, h1);

	sdls_set_rct (sdl, x1, y1, w1, h1, col);
}

static
void sdl_set_chr_xyc (sdl_t *sdl, unsigned x, unsigned y, unsigned c,
	Uint32 fg, Uint32 bg)
{
	unsigned      i, j;
	unsigned char val;
	unsigned char *fnt;
	Uint8         *p;

	x *= sdl->font_w;
	y *= sdl->font_h;

	fnt = sdl->font + c * sdl->font_h;

	val = 0;

	if ((sdl->pxl_w == sdl->wdw_w) && (sdl->pxl_h == sdl->wdw_h)) {
		if (sdl->scr_bpp == 2) {
			p = (Uint8 *) sdl->scr->pixels + y * sdl->scr->pitch
				+ x * sdl->scr->format->BytesPerPixel;

			for (j = 0; j < sdl->font_h; j++) {
				for (i = 0; i < sdl->font_w; i++) {
					if ((i & 7) == 0) {
						val = *(fnt++);
					}

					*(Uint16 *)p = (val & 0x80) ? fg : bg;
					p += 2;

					val = (val & 0x7f) << 1;
				}

				p += sdl->scr->pitch - (sdl->font_w << 1);
			}

			sdl_set_upd_rct (sdl, x, y, sdl->font_w, sdl->font_h);

			return;
		}
	}

	for (j = 0; j < sdl->font_h; j++) {
		for (i = 0; i < sdl->font_w; i++) {
			if ((i & 7) == 0) {
				val = *(fnt++);
			}

			sdl_set_pixel (sdl, x + i, y + j, (val & 0x80) ? fg : bg);

			val = (val & 0x7f) << 1;
		}
	}
}

static
void sdl_crs_draw (sdl_t *sdl, unsigned x, unsigned y)
{
	unsigned i, h;
	Uint32   col;

	if (sdl->crs_y2 < sdl->crs_y1) {
		return;
	}

	if ((x >= sdl->txt_w) || (y >= sdl->txt_h)) {
		return;
	}

	i = 3 * (sdl->txt_w * y + x);
	col = sdl_get_col (sdl, sdl->txt_buf[i + 1]);

	x *= sdl->font_w;
	y = sdl->font_h * y + sdl->crs_y1;
	h = sdl->crs_y2 - sdl->crs_y1 + 1;

	sdl_set_rect (sdl, x, y, sdl->font_w, h, col);
}

static
void sdl_crs_erase (sdl_t *sdl, unsigned x, unsigned y)
{
	unsigned i;
	unsigned chr;
	Uint32   fg, bg;

	if ((x >= sdl->txt_w) || (y >= sdl->txt_h)) {
		return;
	}

	i = 3 * (sdl->txt_w * y + x);

	chr = sdl->txt_buf[i];
	fg = sdl_get_col (sdl, sdl->txt_buf[i + 1]);
	bg = sdl_get_col (sdl, sdl->txt_buf[i + 2]);

	sdl_set_chr_xyc (sdl, x, y, chr, fg, bg);
}

void sdl_set_mode (sdl_t *sdl, unsigned m, unsigned w, unsigned h)
{
	if (m == TERM_MODE_TEXT) {
		sdl->mode = TERM_MODE_TEXT;
		sdl->txt_w = w;
		sdl->txt_h = h;
		sdl->pxl_w = w * sdl->font_w;
		sdl->pxl_h = h * sdl->font_h;
		sdl->txt_buf = (unsigned char *) realloc (sdl->txt_buf, 3 * w * h);
		sdl->crs_x = 0;
		sdl->crs_y = 0;
	}
	else {
		sdl->mode = TERM_MODE_GRAPH;
		sdl->pxl_w = w;
		sdl->pxl_h = h;
		sdl->crs_on = 0;
	}

	trm_smap_set_map (&sdl->smap, sdl->pxl_w, sdl->pxl_h, sdl->wdw_w, sdl->wdw_h);

	sdl_clr_upd_rct (sdl);
}

void sdl_set_size (sdl_t *sdl, unsigned w, unsigned h)
{
	if ((w > 0) && (h > 0)) {
		sdl_set_window_size (sdl, w, h);
	}
}

void sdl_set_map (sdl_t *sdl, unsigned i, unsigned r, unsigned g, unsigned b)
{
	if (i >= 256) {
		return;
	}

	sdl->colmap[i][0] = (r >> 8) & 0xff;
	sdl->colmap[i][1] = (g >> 8) & 0xff;
	sdl->colmap[i][2] = (b >> 8) & 0xff;

	if (i == sdl->fgidx) {
		sdl->fg = sdl_get_col (sdl, i);
	}

	if (i == sdl->bgidx) {
		sdl->bg = sdl_get_col (sdl, i);
	}
}

void sdl_set_col (sdl_t *sdl, unsigned fg, unsigned bg)
{
	sdl->fgidx = fg;
	sdl->bgidx = bg;

	sdl->fg = sdl_get_col (sdl, fg);
	sdl->bg = sdl_get_col (sdl, bg);
}

void sdl_set_crs (sdl_t *sdl, unsigned y1, unsigned y2, int show)
{
	if (SDL_MUSTLOCK (sdl->scr)) {
		if (SDL_LockSurface (sdl->scr) < 0) {
			return;
		}
	}

	if (sdl->crs_on) {
		sdl_crs_erase (sdl, sdl->crs_x, sdl->crs_y);
	}

	y1 = (y1 <= 255) ? y1 : 255;
	y2 = (y2 <= 255) ? y2 : 255;

	sdl->crs_y1 = (sdl->font_h * y1) / 256;
	sdl->crs_y2 = (sdl->font_h * y2) / 256;

	sdl->crs_on = (show != 0);

	if (sdl->crs_on) {
		sdl_crs_draw (sdl, sdl->crs_x, sdl->crs_y);
	}

	if (SDL_MUSTLOCK (sdl->scr)) {
		SDL_UnlockSurface (sdl->scr);
	}

	if (sdl->upd_text == PCESDL_UPDATE_NOW) {
		sdl_update (sdl);
	}
}

void sdl_set_pos (sdl_t *sdl, unsigned x, unsigned y)
{
	if (SDL_MUSTLOCK (sdl->scr)) {
		if (SDL_LockSurface (sdl->scr) < 0) {
			return;
		}
	}

	if (sdl->crs_on) {
		sdl_crs_erase (sdl, sdl->crs_x, sdl->crs_y);
	}

	sdl->crs_x = x;
	sdl->crs_y = y;

	if (sdl->crs_on) {
		sdl_crs_draw (sdl, sdl->crs_x, sdl->crs_y);
	}

	if (SDL_MUSTLOCK (sdl->scr)) {
		SDL_UnlockSurface (sdl->scr);
	}

	if (sdl->upd_text == PCESDL_UPDATE_NOW) {
		sdl_update (sdl);
	}
}

void sdl_set_chr (sdl_t *sdl, unsigned x, unsigned y, unsigned char c)
{
	unsigned i;

	if ((x >= sdl->txt_w) || (y >= sdl->txt_h)) {
		return;
	}

	if (SDL_MUSTLOCK (sdl->scr)) {
		if (SDL_LockSurface (sdl->scr) < 0) {
			return;
		}
	}

	i = 3 * (sdl->txt_w * y + x);

	sdl->txt_buf[i] = c;
	sdl->txt_buf[i + 1] = sdl->fgidx;
	sdl->txt_buf[i + 2] = sdl->bgidx;

	sdl_set_chr_xyc (sdl, x, y, c, sdl->fg, sdl->bg);

	if (sdl->crs_on && (sdl->crs_x == x) && (sdl->crs_y == y)) {
		sdl_crs_draw (sdl, x, y);
	}

	if (SDL_MUSTLOCK (sdl->scr)) {
		SDL_UnlockSurface (sdl->scr);
	}

	if (sdl->upd_text == PCESDL_UPDATE_NOW) {
		sdl_update (sdl);
	}
}

void sdl_set_pxl (sdl_t *sdl, unsigned x, unsigned y)
{
	if (SDL_MUSTLOCK (sdl->scr)) {
		if (SDL_LockSurface (sdl->scr) < 0) {
			return;
		}
	}

	sdl_set_pixel (sdl, x, y, sdl->fg);

	if (SDL_MUSTLOCK (sdl->scr)) {
		SDL_UnlockSurface (sdl->scr);
	}

	if (sdl->upd_graph == PCESDL_UPDATE_NOW) {
		sdl_update (sdl);
	}
}

void sdl_set_rct (sdl_t *sdl, unsigned x, unsigned y, unsigned w, unsigned h)
{
	if (SDL_MUSTLOCK (sdl->scr)) {
		if (SDL_LockSurface (sdl->scr) < 0) {
			return;
		}
	}

	sdl_set_rect (sdl, x, y, w, h, sdl->fg);

	if (SDL_MUSTLOCK (sdl->scr)) {
		SDL_UnlockSurface (sdl->scr);
	}

	if (sdl->upd_graph == PCESDL_UPDATE_NOW) {
		sdl_update (sdl);
	}
}

static
void sdl_event_keydown (sdl_t *sdl, SDLKey key)
{
	SDLMod    mod;
	pce_key_t pcekey;

	if ((sdl->magic & 0x03) == 0x03) {
		sdl_magic (sdl, key);
		return;
	}

	if (key == SDLK_LCTRL) {
		sdl->magic |= 0x01;
	}
	else if (key == SDLK_LALT) {
		sdl->magic |= 0x02;
	}

	if ((sdl->magic & 0x03) == 0x03) {
		sdl_grab_mouse (sdl, 0);
	}

	if (key == SDLK_PAUSE) {
		sdl_grab_mouse (sdl, 0);
		trm_set_msg (&sdl->trm, "emu.exit", "1");
		return;
	}

	mod = SDL_GetModState();

	if ((key == SDLK_BACKQUOTE) && (mod & KMOD_LCTRL)) {
		sdl_grab_mouse (sdl, 0);
		trm_set_msg (&sdl->trm, "emu.stop", "1");
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

	if (key == SDLK_LCTRL) {
		sdl->magic &= ~0x01;
	}
	else if (key == SDLK_LALT) {
		sdl->magic &= ~0x02;
	}

	if ((sdl->magic & 0x03) == 0x03) {
		return;
	}

	pcekey = sdl_map_key (key);

	if (pcekey != PCE_KEY_NONE) {
		if (key == SDLK_NUMLOCK) {
			trm_set_key (&sdl->trm, 1, pcekey);
		}

		trm_set_key (&sdl->trm, 2, pcekey);
	}
}

void sdl_check (sdl_t *sdl)
{
	SDL_Event     evt;
	static Uint32 ticks1 = 0;
	Uint32        ticks2;

	if ((sdl->upd_x1 <= sdl->upd_x2) && (sdl->upd_y1 <= sdl->upd_y2)) {
		ticks2 = SDL_GetTicks();
		if ((ticks2 < ticks1) || ((ticks2 - ticks1) > sdl->upd_freq)) {
			ticks1 = ticks2;

			sdl_update (sdl);
			sdl_clr_upd_rct (sdl);
		}
	}

	while (SDL_PollEvent (&evt)) {
		switch (evt.type) {
		case SDL_KEYDOWN:
			sdl_event_keydown (sdl, evt.key.keysym.sym);
			break;

		case SDL_KEYUP:
			sdl_event_keyup (sdl, evt.key.keysym.sym);
			break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (sdl->trm.set_mse != NULL) {
					unsigned b;
					SDLMod   mod;

					mod = SDL_GetModState();

					if (sdl->grab) {
						b = SDL_GetMouseState (NULL, NULL);
						b = ((b & SDL_BUTTON (1)) ? 0x01 : 0) | ((b & SDL_BUTTON (3)) ? 0x02 : 0);
						sdl->trm.set_mse (sdl->trm.mse_ext, 0, 0, b);
					}
					else {
						sdl_grab_mouse (sdl, 1);
					}
				}
				break;

			case SDL_MOUSEMOTION:
				if (sdl->grab && (sdl->trm.set_mse != NULL)) {
					unsigned b;

					b = SDL_GetMouseState (NULL, NULL);
					b = ((b & SDL_BUTTON (1)) ? 0x01 : 0) | ((b & SDL_BUTTON (3)) ? 0x02 : 0);

					sdl->trm.set_mse (sdl->trm.mse_ext, evt.motion.xrel, evt.motion.yrel, b);
				}
				break;

			case SDL_VIDEORESIZE:
				sdl_set_window_size (sdl, evt.resize.w, evt.resize.h);
				trm_set_msg (&sdl->trm, "video.redraw", "1");
				break;

			case SDL_QUIT:
				sdl_grab_mouse (sdl, 0);
				trm_set_msg (&sdl->trm, "emu.exit", "1");
				break;

			default:
				break;
		}
	}
}
