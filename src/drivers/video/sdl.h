/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/sdl.h                                      *
 * Created:     2003-09-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_SDL_H
#define PCE_VIDEO_SDL_H 1


#include <stdio.h>

#include <drivers/video/terminal.h>

#include <libini/libini.h>

#include <SDL.h>


typedef struct {
	SDLKey    sdlkey;
	pce_key_t pcekey;
} sdl_keymap_t;


/*!***************************************************************************
 * @short The SDL terminal structure
 *****************************************************************************/
typedef struct {
	terminal_t    trm;

	SDL_Surface   *scr;

	unsigned      wdw_w;
	unsigned      wdw_h;

	unsigned      button;

	char          fullscreen;

	unsigned      dsp_bpp;
	unsigned      scr_bpp;

	unsigned      border[4];

	char          grab;

	char          report_keys;

	unsigned      keymap_cnt;
	sdl_keymap_t  *keymap;
} sdl_t;


/*!***************************************************************************
 * @short Create a new SDL terminal
 *****************************************************************************/
terminal_t *sdl_new (ini_sct_t *ini);


#endif
