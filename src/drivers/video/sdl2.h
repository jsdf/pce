/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/sdl2.h                                     *
 * Created:     2015-06-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_SDL2_H
#define PCE_VIDEO_SDL2_H 1


#include <stdio.h>

#include <drivers/video/terminal.h>

#include <libini/libini.h>

#include <SDL.h>


typedef struct {
	SDL_Scancode sdlkey;
	pce_key_t    pcekey;
} sdl2_keymap_t;


/*!***************************************************************************
 * @short The SDL2 terminal structure
 *****************************************************************************/
typedef struct {
	terminal_t    trm;

	SDL_Window    *window;
	SDL_Renderer  *render;
	SDL_Texture   *texture;

	unsigned      txt_w;
	unsigned      txt_h;

	unsigned      wdw_w;
	unsigned      wdw_h;

	unsigned      button;

	char          update;
	char          fullscreen;
	char          grab;
	char          report_keys;
	char          autosize;
	char          grave_down;
	char          ignore_keys;

	unsigned      keymap_cnt;
	sdl2_keymap_t *keymap;
} sdl2_t;


/*!***************************************************************************
 * @short Create a new SDL2 terminal
 *****************************************************************************/
terminal_t *sdl2_new (ini_sct_t *ini);


#endif
