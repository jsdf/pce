/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/sdl.c                                         *
 * Created:       2003-09-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-19 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: sdl.c,v 1.4 2003/09/19 15:09:19 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h>

#include <terminal/terminal.h>
#include <terminal/font.h>
#include <terminal/sdl.h>


#define PCESDL_UPDATE_NONE  0
#define PCESDL_UPDATE_NOW   1
#define PCESDL_UPDATE_DELAY 2


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

terminal_t *sdl_new (ini_sct_t *sct)
{
  sdl_t *sdl;
  char  *str;

  sdl = (sdl_t *) malloc (sizeof (sdl_t));
  if (sdl == NULL) {
    return (NULL);
  }

  trm_init (&sdl->trm);

  sdl->trm.ext = sdl;

  sdl->trm.del = (trm_del_f) &sdl_del;
  sdl->trm.set_size = (trm_set_size_f) &sdl_set_size;
  sdl->trm.set_map = (trm_set_map_f) &sdl_set_map;
  sdl->trm.set_col = (trm_set_col_f) &sdl_set_col;
  sdl->trm.set_crs = (trm_set_crs_f) &sdl_set_crs;
  sdl->trm.set_pos = (trm_set_pos_f) &sdl_set_pos;
  sdl->trm.set_chr = (trm_set_chr_f) &sdl_set_chr;
  sdl->trm.set_pxl = (trm_set_pxl_f) &sdl_set_pxl;
  sdl->trm.check = (trm_check_f) &sdl_check;

  if (SDL_Init (SDL_INIT_VIDEO) < 0) {
    return (NULL);
  }

  sdl->mode = TERM_MODE_TEXT;
  sdl->scr_w = 80;
  sdl->scr_h = 25;

  sdl->txt_w = 80;
  sdl->txt_h = 25;
  sdl->txt_buf = (unsigned char *) malloc (3 * 80 * 25);
  memset (sdl->txt_buf, 0, 3 * 80 * 25);

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
    sdl->font = (unsigned char *) malloc (256 * 16);
    memcpy (sdl->font, fnt_8x16, 256 * 16);
  }

  sdl->crs_x = 0;
  sdl->crs_y = 0;
  sdl->crs_y1 = (6 * 255) / 7;
  sdl->crs_y2 = (7 * 255) / 7;
  sdl->crs_on = 0;

  memcpy (sdl->colmap, sdl_colmap, 16 * 3);

  sdl->fgidx = 7;
  sdl->bgidx = 0;

  sdl->pxl_w = sdl->scr_w * sdl->font_w;
  sdl->pxl_h = sdl->scr_h * sdl->font_h;

  sdl->upd_x1 = sdl->pxl_w;
  sdl->upd_y1 = sdl->pxl_h;
  sdl->upd_x2 = 0;
  sdl->upd_y2 = 0;

  sdl->grab = 0;

  ini_get_uint (sct, "update_delay", &sdl->upd_freq, 100);

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

  sdl->scr = SDL_SetVideoMode (sdl->pxl_w, sdl->pxl_h, 32, SDL_SWSURFACE);
  if (sdl->scr == NULL) {
    free (sdl);
    return (NULL);
  }

  sdl->fg = SDL_MapRGB (sdl->scr->format, 0xc0, 0xc0, 0xc0);
  sdl->fg = SDL_MapRGB (sdl->scr->format, 0xff, 0xff, 0xff);

  SDL_WM_SetCaption ("pce", "pce");
  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EventState (SDL_MOUSEMOTION, SDL_ENABLE);

  fprintf (stderr, "sdl: bpp = %u\n", sdl->scr->format->BytesPerPixel);

  return (&sdl->trm);
}

void sdl_del (sdl_t *sdl)
{
  if (sdl != NULL) {
    if (sdl->scr != NULL) {
      SDL_FreeSurface (sdl->scr);
    }

    free (sdl->txt_buf);
    free (sdl->font);
    free (sdl);
  }

  SDL_Quit();
}

Uint32 sdl_get_col (sdl_t *sdl, unsigned idx)
{
  Uint32 col;

  idx &= 0xff;

  col = SDL_MapRGB (sdl->scr->format,
    sdl->colmap[idx][0], sdl->colmap[idx][1], sdl->colmap[idx][2]
  );

  return (col);
}

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

void sdl_clr_upd_rct (sdl_t *sdl)
{
  sdl->upd_x1 = sdl->pxl_w;
  sdl->upd_y1 = sdl->pxl_h;
  sdl->upd_x2 = 0;
  sdl->upd_y2 = 0;
}

void sdl_set_chr_xyc (sdl_t *sdl, unsigned x, unsigned y, unsigned c,
  Uint32 fg, Uint32 bg, unsigned update)
{
  unsigned      i, j;
  Uint8         *p;
  unsigned char *fnt;

  x *= sdl->font_w;
  y *= sdl->font_h;

  fnt = sdl->font + c * sdl->font_h;

  if (SDL_MUSTLOCK (sdl->scr)) {
    if (SDL_LockSurface (sdl->scr) < 0) {
      return;
    }
  }

  for (j = 0; j < sdl->font_h; j++) {
    p = (Uint8 *) sdl->scr->pixels + (y + j) * sdl->scr->pitch
      + x * sdl->scr->format->BytesPerPixel;

    for (i = 0; i < sdl->font_w; i++) {
      if (*fnt & (0x80 >> i)) {
        *(Uint32 *)p = fg;
      }
      else {
        *(Uint32 *)p = bg;
      }

      p += sdl->scr->format->BytesPerPixel;
    }

    fnt += 1;
  }

  if (SDL_MUSTLOCK (sdl->scr)) {
    SDL_UnlockSurface (sdl->scr);
  }

  switch (update) {
    case PCESDL_UPDATE_NOW:
      SDL_UpdateRect (sdl->scr, x, y, sdl->font_w, sdl->font_h);
      break;

    case PCESDL_UPDATE_DELAY:
      sdl_set_upd_rct (sdl, x, y, sdl->font_w, sdl->font_h);
      break;
  }
}

void sdl_set_rct (sdl_t *sdl, unsigned x, unsigned y, unsigned w, unsigned h,
  Uint32 col, unsigned update)
{
  unsigned i, j;
  Uint8    *p;

  if (SDL_MUSTLOCK (sdl->scr)) {
    if (SDL_LockSurface (sdl->scr) < 0) {
      return;
    }
  }

  for (j = 0; j < h; j++) {
    p = (Uint8 *) sdl->scr->pixels + (y + j) * sdl->scr->pitch
      + x * sdl->scr->format->BytesPerPixel;

    for (i = 0; i < w; i++) {
      *(Uint32 *) p = col;

      p += sdl->scr->format->BytesPerPixel;
    }
  }

  if (SDL_MUSTLOCK (sdl->scr)) {
    SDL_UnlockSurface (sdl->scr);
  }

  switch (update) {
    case PCESDL_UPDATE_NOW:
      SDL_UpdateRect (sdl->scr, x, y, w, h);
      break;

    case PCESDL_UPDATE_DELAY:
      sdl_set_upd_rct (sdl, x, y, w, h);
      break;
  }
}

void sdl_set_size (sdl_t *sdl, unsigned m, unsigned w, unsigned h)
{
  if (sdl->scr != NULL) {
    SDL_FreeSurface (sdl->scr);
  }

  if (m == TERM_MODE_TEXT) {
    sdl->mode = TERM_MODE_TEXT;
    sdl->scr_w = w;
    sdl->scr_h = h;
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
    sdl->scr_w = w;
    sdl->scr_h = h;
    sdl->pxl_w = w;
    sdl->pxl_h = h;
    sdl->crs_on = 0;
  }

  sdl->scr = SDL_SetVideoMode (sdl->pxl_w, sdl->pxl_h, 32, SDL_SWSURFACE);

  sdl->upd_x1 = sdl->pxl_w;
  sdl->upd_y1 = sdl->pxl_h;
  sdl->upd_x2 = 0;
  sdl->upd_y2 = 0;
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
  col = sdl_get_col (sdl, 7);

  x *= sdl->font_w;
  y = sdl->font_h * (y + 1) - sdl->crs_y2 - 1;
  h = sdl->crs_y2 - sdl->crs_y1 + 1;

  sdl_set_rct (sdl, x, y, sdl->font_w, h, col, sdl->upd_text);
}

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

  sdl_set_chr_xyc (sdl, x, y, chr, fg, bg, sdl->upd_text);
}

void sdl_set_crs (sdl_t *sdl, unsigned y1, unsigned y2)
{
  if (sdl->crs_on) {
    sdl_crs_erase (sdl, sdl->crs_x, sdl->crs_y);
  }

  sdl->crs_y1 = (sdl->font_w * y1) / 255;
  sdl->crs_y2 = (sdl->font_h * y2) / 255;

  sdl->crs_on = (sdl->crs_y1 <= sdl->crs_y2);

  if (sdl->crs_on) {
    sdl_crs_draw (sdl, sdl->crs_x, sdl->crs_y);
  }
}

void sdl_set_pos (sdl_t *sdl, unsigned x, unsigned y)
{
  if (sdl->crs_on) {
    sdl_crs_erase (sdl, sdl->crs_x, sdl->crs_y);
  }

  sdl->crs_x = x;
  sdl->crs_y = y;

  if (sdl->crs_on) {
    sdl_crs_draw (sdl, sdl->crs_x, sdl->crs_y);
  }
}

void sdl_set_chr (sdl_t *sdl, unsigned x, unsigned y, unsigned char c)
{
  unsigned i;

  if ((x >= sdl->txt_w) || (y >= sdl->txt_h)) {
    return;
  }

  i = 3 * (sdl->txt_w * y + x);

  sdl->txt_buf[i] = c;
  sdl->txt_buf[i + 1] = sdl->fgidx;
  sdl->txt_buf[i + 2] = sdl->bgidx;

  sdl_set_chr_xyc (sdl, x, y, c, sdl->fg, sdl->bg, sdl->upd_text);

  if (sdl->crs_on && (sdl->crs_x == x) && (sdl->crs_y == y)) {
    sdl_crs_draw (sdl, x, y);
  }
}

void sdl_set_pxl (sdl_t *sdl, unsigned x, unsigned y, unsigned w, unsigned h)
{
  sdl_set_rct (sdl, x, y, w, h, sdl->fg, sdl->upd_graph);
}

void sdl_send_key_code (sdl_t *sdl, unsigned long code)
{
  if (sdl->trm.set_key == NULL) {
    return;
  }

  while (code != 0) {
    sdl->trm.set_key (sdl->trm.key_ext, code & 0xff);
    code = code >> 8;
  }
}

unsigned long sdl_get_key_code (sdl_t *sdl, SDLKey key, int make)
{
  unsigned long ret, tmp, msk;

  msk = make ? 0x00 : 0x80;

  switch (key) {
    case SDLK_ESCAPE:       ret = 0x01; break;
    case SDLK_1:            ret = 0x02; break;
    case SDLK_2:            ret = 0x03; break;
    case SDLK_3:            ret = 0x04; break;
    case SDLK_4:            ret = 0x05; break;
    case SDLK_5:            ret = 0x06; break;
    case SDLK_6:            ret = 0x07; break;
    case SDLK_7:            ret = 0x08; break;
    case SDLK_8:            ret = 0x09; break;
    case SDLK_9:            ret = 0x0a; break;
    case SDLK_0:            ret = 0x0b; break;
    case SDLK_MINUS:        ret = 0x0c; break;
    case SDLK_EQUALS:       ret = 0x0d; break;
    case SDLK_BACKSPACE:    ret = 0x0e; break;
    case SDLK_TAB:          ret = 0x0f; break;
    case SDLK_q:            ret = 0x10; break;
    case SDLK_w:            ret = 0x11; break;
    case SDLK_e:            ret = 0x12; break;
    case SDLK_r:            ret = 0x13; break;
    case SDLK_t:            ret = 0x14; break;
    case SDLK_y:            ret = 0x15; break;
    case SDLK_u:            ret = 0x16; break;
    case SDLK_i:            ret = 0x17; break;
    case SDLK_o:            ret = 0x18; break;
    case SDLK_p:            ret = 0x19; break;
    case SDLK_LEFTBRACKET:  ret = 0x1a; break;
    case SDLK_RIGHTBRACKET: ret = 0x1b; break;
    case SDLK_RETURN:       ret = 0x1c; break;
    case SDLK_LCTRL:        ret = 0x1d; break;
    case SDLK_a:            ret = 0x1e; break;
    case SDLK_s:            ret = 0x1f; break;
    case SDLK_d:            ret = 0x20; break;
    case SDLK_f:            ret = 0x21; break;
    case SDLK_g:            ret = 0x22; break;
    case SDLK_h:            ret = 0x23; break;
    case SDLK_j:            ret = 0x24; break;
    case SDLK_k:            ret = 0x25; break;
    case SDLK_l:            ret = 0x26; break;
    case SDLK_SEMICOLON:    ret = 0x27; break;
    case SDLK_QUOTE:        ret = 0x28; break;
    case SDLK_BACKQUOTE:    ret = 0x29; break;
    case SDLK_LSHIFT:       ret = 0x2a; break;
    case SDLK_BACKSLASH:    ret = 0x2b; break;
    case SDLK_z:            ret = 0x2c; break;
    case SDLK_x:            ret = 0x2d; break;
    case SDLK_c:            ret = 0x2e; break;
    case SDLK_v:            ret = 0x2f; break;
    case SDLK_b:            ret = 0x30; break;
    case SDLK_n:            ret = 0x31; break;
    case SDLK_m:            ret = 0x32; break;
    case SDLK_COMMA:        ret = 0x33; break;
    case SDLK_PERIOD:       ret = 0x34; break;
    case SDLK_SLASH:        ret = 0x35; break;
    case SDLK_RSHIFT:       ret = 0x36; break;
    case SDLK_KP_MULTIPLY:  ret = 0x37; break;
    case SDLK_LALT:         ret = 0x38; break;
    case SDLK_SPACE:        ret = 0x39; break;
    case SDLK_CAPSLOCK:     ret = 0xba3a; break;
    case SDLK_F1:           ret = 0x3b; break;
    case SDLK_F2:           ret = 0x3c; break;
    case SDLK_F3:           ret = 0x3d; break;
    case SDLK_F4:           ret = 0x3e; break;
    case SDLK_F5:           ret = 0x3f; break;
    case SDLK_F6:           ret = 0x40; break;
    case SDLK_F7:           ret = 0x41; break;
    case SDLK_F8:           ret = 0x42; break;
    case SDLK_F9:           ret = 0x43; break;
    case SDLK_F10:          ret = 0x44; break;

    case SDLK_NUMLOCK:      ret = 0x45; break;
    case SDLK_SCROLLOCK:    ret = 0x46; break;

    case SDLK_HOME:         ret = 0x47; break;
    case SDLK_UP:           ret = 0x48; break;
    case SDLK_PAGEUP:       ret = 0x49; break;
    case SDLK_KP_MINUS:     ret = 0x4a; break;
    case SDLK_LEFT:         ret = 0x4b; break;
    case SDLK_KP5:          ret = 0x4c; break;
    case SDLK_RIGHT:        ret = 0x4d; break;
    case SDLK_KP_PLUS:      ret = 0x4e; break;
    case SDLK_END:          ret = 0x4f; break;
    case SDLK_DOWN:         ret = 0x50; break;
    case SDLK_PAGEDOWN:     ret = 0x51; break;
    case SDLK_KP0:          ret = 0x52; break;
    case SDLK_DELETE:       ret = 0x53; break;

    case SDLK_KP_DIVIDE:    ret = 0x35e0; break;
    default:                ret = 0x00; break;
  }

  if (ret == 0) {
    fprintf (stderr, "sdl: keysym=%04x\n", (unsigned) key);
  }

  tmp = ret;
  while (tmp != 0) {
    ret |= msk;
    tmp = tmp >> 8;
    msk = msk << 8;
  }

  return (ret);
}

void sdl_check (sdl_t *sdl)
{
  SDL_Event evt;
  static Uint32 ticks1 = 0;
  Uint32        ticks2;

  if ((sdl->upd_x1 <= sdl->upd_x2) && (sdl->upd_y1 <= sdl->upd_y2)) {
    ticks2 = SDL_GetTicks();
    if ((ticks2 < ticks1) || ((ticks2 - ticks1) > sdl->upd_freq)) {
      unsigned w, h;

      ticks1 = ticks2;

      w = sdl->upd_x2 - sdl->upd_x1 + 1;
      h = sdl->upd_y2 - sdl->upd_y1 + 1;
      SDL_UpdateRect (sdl->scr, sdl->upd_x1, sdl->upd_y1, w, h);

      sdl->upd_x1 = sdl->pxl_w;
      sdl->upd_y1 = sdl->pxl_h;
      sdl->upd_x2 = 0;
      sdl->upd_y2 = 0;
    }
  }

  while (SDL_PollEvent (&evt)) {
    switch (evt.type) {
      case SDL_KEYDOWN: {
        SDLMod mod;
        SDLKey key;

        mod = SDL_GetModState();
        key = evt.key.keysym.sym;

        if (key == SDLK_PAUSE) {
          if (sdl->trm.set_brk != NULL) {
            sdl->trm.set_brk (sdl->trm.key_ext, 2);
          }
        }
        else if ((key == SDLK_BACKQUOTE) && (mod & KMOD_LALT)) {
          if (sdl->trm.set_brk != NULL) {
            sdl->trm.set_brk (sdl->trm.key_ext, 2);
          }
        }
        else if ((key == SDLK_BACKQUOTE) && (mod & KMOD_LCTRL)) {
          if (sdl->trm.set_brk != NULL) {
            sdl->trm.set_brk (sdl->trm.key_ext, 1);
          }
        }
        else if ((key == SDLK_PRINT) && (mod == 0)) {
          if (sdl->trm.set_brk != NULL) {
            sdl->trm.set_brk (sdl->trm.key_ext, 3);
          }
        }
        else {
          unsigned long code;

          code = sdl_get_key_code (sdl, evt.key.keysym.sym, 1);
          if (code != 0) {
            sdl_send_key_code (sdl, code);
          }
        }
      }
        break;

      case SDL_KEYUP: {
          unsigned long code;

          code = sdl_get_key_code (sdl, evt.key.keysym.sym, 0);
          if (code != 0) {
            sdl_send_key_code (sdl, code);
          }
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if (sdl->trm.set_mse != NULL) {
          unsigned b;

          if ((evt.type == SDL_MOUSEBUTTONDOWN) && (evt.button.button == SDL_BUTTON_MIDDLE)) {
            if (sdl->grab == 0) {
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
          else if (sdl->grab) {
            b = SDL_GetMouseState (NULL, NULL);
            b = ((b & SDL_BUTTON (1)) ? 0x01 : 0) | ((b & SDL_BUTTON (3)) ? 0x02 : 0);
            sdl->trm.set_mse (sdl->trm.mse_ext, 0, 0, b);
          }
        }
        break;

      case SDL_MOUSEMOTION:
        if ((sdl->trm.set_mse != NULL) && (sdl->grab)) {
          unsigned b;

          b = SDL_GetMouseState (NULL, NULL);
          b = ((b & SDL_BUTTON (1)) ? 0x01 : 0) | ((b & SDL_BUTTON (3)) ? 0x02 : 0);

          sdl->trm.set_mse (sdl->trm.mse_ext, evt.motion.xrel, evt.motion.yrel, b);
        }
        break;

      default:
        break;
    }
  }
}
