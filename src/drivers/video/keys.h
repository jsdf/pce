/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/keys.h                                     *
 * Created:     2007-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_KEYS_H
#define PCE_VIDEO_KEYS_H 1


typedef enum {
	PCE_KEY_NONE,

	PCE_KEY_ESC,

	PCE_KEY_F1,
	PCE_KEY_F2,
	PCE_KEY_F3,
	PCE_KEY_F4,
	PCE_KEY_F5,
	PCE_KEY_F6,
	PCE_KEY_F7,
	PCE_KEY_F8,
	PCE_KEY_F9,
	PCE_KEY_F10,
	PCE_KEY_F11,
	PCE_KEY_F12,

	PCE_KEY_PRTSCN,
	PCE_KEY_SCRLK,
	PCE_KEY_PAUSE,

	PCE_KEY_BACKQUOTE,
	PCE_KEY_1,
	PCE_KEY_2,
	PCE_KEY_3,
	PCE_KEY_4,
	PCE_KEY_5,
	PCE_KEY_6,
	PCE_KEY_7,
	PCE_KEY_8,
	PCE_KEY_9,
	PCE_KEY_0,
	PCE_KEY_MINUS,
	PCE_KEY_EQUAL,
	PCE_KEY_BACKSPACE,

	PCE_KEY_TAB,
	PCE_KEY_Q,
	PCE_KEY_W,
	PCE_KEY_E,
	PCE_KEY_R,
	PCE_KEY_T,
	PCE_KEY_Y,
	PCE_KEY_U,
	PCE_KEY_I,
	PCE_KEY_O,
	PCE_KEY_P,
	PCE_KEY_LBRACKET,
	PCE_KEY_RBRACKET,
	PCE_KEY_RETURN,

	PCE_KEY_CAPSLOCK,
	PCE_KEY_A,
	PCE_KEY_S,
	PCE_KEY_D,
	PCE_KEY_F,
	PCE_KEY_G,
	PCE_KEY_H,
	PCE_KEY_J,
	PCE_KEY_K,
	PCE_KEY_L,
	PCE_KEY_SEMICOLON,
	PCE_KEY_QUOTE,
	PCE_KEY_BACKSLASH,

	PCE_KEY_LSHIFT,
	PCE_KEY_LESS,
	PCE_KEY_Z,
	PCE_KEY_X,
	PCE_KEY_C,
	PCE_KEY_V,
	PCE_KEY_B,
	PCE_KEY_M,
	PCE_KEY_N,
	PCE_KEY_COMMA,
	PCE_KEY_PERIOD,
	PCE_KEY_SLASH,
	PCE_KEY_RSHIFT,

	PCE_KEY_LCTRL,
	PCE_KEY_LSUPER,
	PCE_KEY_LMETA,
	PCE_KEY_MODE,
	PCE_KEY_LALT,
	PCE_KEY_SPACE,
	PCE_KEY_RALT,
	PCE_KEY_RMETA,
	PCE_KEY_RSUPER,
	PCE_KEY_MENU,
	PCE_KEY_RCTRL,

	PCE_KEY_NUMLOCK,
	PCE_KEY_KP_SLASH,
	PCE_KEY_KP_STAR,
	PCE_KEY_KP_MINUS,
	PCE_KEY_KP_7,
	PCE_KEY_KP_8,
	PCE_KEY_KP_9,
	PCE_KEY_KP_PLUS,
	PCE_KEY_KP_4,
	PCE_KEY_KP_5,
	PCE_KEY_KP_6,
	PCE_KEY_KP_1,
	PCE_KEY_KP_2,
	PCE_KEY_KP_3,
	PCE_KEY_KP_ENTER,
	PCE_KEY_KP_0,
	PCE_KEY_KP_PERIOD,

	PCE_KEY_INS,
	PCE_KEY_HOME,
	PCE_KEY_PAGEUP,
	PCE_KEY_DEL,
	PCE_KEY_END,
	PCE_KEY_PAGEDN,
	PCE_KEY_UP,
	PCE_KEY_LEFT,
	PCE_KEY_DOWN,
	PCE_KEY_RIGHT,

	PCE_KEY_EXTRA1,
	PCE_KEY_EXTRA2,
	PCE_KEY_EXTRA3,
	PCE_KEY_EXTRA4,
	PCE_KEY_EXTRA5,
	PCE_KEY_EXTRA6,
	PCE_KEY_EXTRA7,
	PCE_KEY_EXTRA8,
	PCE_KEY_EXTRA9,
	PCE_KEY_EXTRA10,
	PCE_KEY_EXTRA11,
	PCE_KEY_EXTRA12,
	PCE_KEY_EXTRA13,
	PCE_KEY_EXTRA14,
	PCE_KEY_EXTRA15,
	PCE_KEY_EXTRA16
} pce_key_t;


#define PCE_KEY_EVENT_DOWN  1
#define PCE_KEY_EVENT_UP    2
#define PCE_KEY_EVENT_MAGIC 3


typedef struct {
	unsigned short event;
	pce_key_t      key;
} pce_key_evt_t;


/*!***************************************************************************
 * @short  Convert a string to a PCE key symbol
 * @param  str A key name
 * @return The PCE key symbol or PCE_KEY_NONE if the key is not found
 *****************************************************************************/
pce_key_t pce_key_from_string (const char *str);

/*!***************************************************************************
 * @short  Convert a PCE key symbol to a string
 * @param  key A PCE key symbol
 * @return The key name or NULL
 *****************************************************************************/
const char *pce_key_to_string (pce_key_t key);

/*!***************************************************************************
 * @short  Get a PCE key map
 * @param  map  A string of the form "<integer> = <pce key name>"
 * @retval src  The integer part of the mapping
 * @retval dst  The PCE key name part of the mapping
 * @return Non-zero on error
 *****************************************************************************/
int pce_key_get_map (const char *map, unsigned long *src, pce_key_t *dst);


#endif
