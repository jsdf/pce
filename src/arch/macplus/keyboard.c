/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/keyboard.c                                  *
 * Created:     2007-11-20 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "keyboard.h"

#include <stdlib.h>

#include <lib/log.h>

#include <drivers/video/keys.h>


static mac_kbd_map_t key_map_us[] = {
	{ PCE_KEY_BACKQUOTE,  1, { 0x65 }, 1, { 0xe5 } },
	{ PCE_KEY_1,          1, { 0x25 }, 1, { 0xa5 } },
	{ PCE_KEY_2,          1, { 0x27 }, 1, { 0xa7 } },
	{ PCE_KEY_3,          1, { 0x29 }, 1, { 0xa9 } },
	{ PCE_KEY_4,          1, { 0x2b }, 1, { 0xab } },
	{ PCE_KEY_5,          1, { 0x2f }, 1, { 0xaf } },
	{ PCE_KEY_6,          1, { 0x2d }, 1, { 0xad } },
	{ PCE_KEY_7,          1, { 0x35 }, 1, { 0xb5 } },
	{ PCE_KEY_8,          1, { 0x39 }, 1, { 0xb9 } },
	{ PCE_KEY_9,          1, { 0x33 }, 1, { 0xb3 } },
	{ PCE_KEY_0,          1, { 0x3b }, 1, { 0xbb } },
	{ PCE_KEY_MINUS,      1, { 0x37 }, 1, { 0xb7 } },
	{ PCE_KEY_EQUAL,      1, { 0x31 }, 1, { 0xb1 } },
	{ PCE_KEY_BACKSPACE,  1, { 0x67 }, 1, { 0xe7 } },
	{ PCE_KEY_TAB,        1, { 0x61 }, 1, { 0xe1 } },
	{ PCE_KEY_Q,          1, { 0x19 }, 1, { 0x99 } },
	{ PCE_KEY_W,          1, { 0x1b }, 1, { 0x9b } },
	{ PCE_KEY_E,          1, { 0x1d }, 1, { 0x9d } },
	{ PCE_KEY_R,          1, { 0x1f }, 1, { 0x9f } },
	{ PCE_KEY_T,          1, { 0x23 }, 1, { 0xa3 } },
	{ PCE_KEY_Y,          1, { 0x21 }, 1, { 0xa1 } },
	{ PCE_KEY_U,          1, { 0x41 }, 1, { 0xc1 } },
	{ PCE_KEY_I,          1, { 0x45 }, 1, { 0xc5 } },
	{ PCE_KEY_O,          1, { 0x3f }, 1, { 0xbf } },
	{ PCE_KEY_P,          1, { 0x47 }, 1, { 0xc7 } },
	{ PCE_KEY_LBRACKET,   1, { 0x43 }, 1, { 0xc3 } },
	{ PCE_KEY_RBRACKET,   1, { 0x3d }, 1, { 0xbd } },
	{ PCE_KEY_BACKSLASH,  1, { 0x55 }, 1, { 0xd5 } },
	{ PCE_KEY_RETURN,     1, { 0x49 }, 1, { 0xc9 } },
	{ PCE_KEY_CAPSLOCK,   1, { 0x73 }, 1, { 0xf3 } },
	{ PCE_KEY_A,          1, { 0x01 }, 1, { 0x81 } },
	{ PCE_KEY_S,          1, { 0x03 }, 1, { 0x83 } },
	{ PCE_KEY_D,          1, { 0x05 }, 1, { 0x85 } },
	{ PCE_KEY_F,          1, { 0x07 }, 1, { 0x87 } },
	{ PCE_KEY_G,          1, { 0x0b }, 1, { 0x8b } },
	{ PCE_KEY_H,          1, { 0x09 }, 1, { 0x89 } },
	{ PCE_KEY_J,          1, { 0x4d }, 1, { 0xcd } },
	{ PCE_KEY_K,          1, { 0x51 }, 1, { 0xd1 } },
	{ PCE_KEY_L,          1, { 0x4b }, 1, { 0xcb } },
	{ PCE_KEY_SEMICOLON,  1, { 0x53 }, 1, { 0xd3 } },
	{ PCE_KEY_QUOTE,      1, { 0x4f }, 1, { 0xcf } },
	{ PCE_KEY_LSHIFT,     1, { 0x71 }, 1, { 0xf1 } },
	{ PCE_KEY_RSHIFT,     1, { 0x71 }, 1, { 0xf1 } },
	{ PCE_KEY_Z,          1, { 0x0d }, 1, { 0x8d } },
	{ PCE_KEY_X,          1, { 0x0f }, 1, { 0x8f } },
	{ PCE_KEY_C,          1, { 0x11 }, 1, { 0x91 } },
	{ PCE_KEY_V,          1, { 0x13 }, 1, { 0x93 } },
	{ PCE_KEY_B,          1, { 0x17 }, 1, { 0x97 } },
	{ PCE_KEY_N,          1, { 0x5b }, 1, { 0xdb } },
	{ PCE_KEY_M,          1, { 0x5d }, 1, { 0xdd } },
	{ PCE_KEY_COMMA,      1, { 0x57 }, 1, { 0xd7 } },
	{ PCE_KEY_PERIOD,     1, { 0x5f }, 1, { 0xdf } },
	{ PCE_KEY_SLASH,      1, { 0x59 }, 1, { 0xd9 } },
	{ PCE_KEY_LCTRL,      1, { 0x75 }, 1, { 0xf5 } },
	{ PCE_KEY_RCTRL,      1, { 0x75 }, 1, { 0xf5 } },
	{ PCE_KEY_LALT,       1, { 0x6f }, 1, { 0xef } },
	{ PCE_KEY_RALT,       1, { 0x69 }, 1, { 0xe9 } },
	{ PCE_KEY_SPACE,      1, { 0x63 }, 1, { 0xe3 } },
	{ PCE_KEY_NUMLOCK,    2, { 0x79, 0x0f }, 2, { 0x79, 0x8f } },
	{ PCE_KEY_KP_SLASH,   3, { 0x71, 0x79, 0x1b }, 3, { 0x79, 0x9b, 0xf1 } },
	{ PCE_KEY_KP_STAR,    3, { 0x71, 0x79, 0x05 }, 3, { 0x79, 0x85, 0xf1 } },
	{ PCE_KEY_KP_MINUS,   2, { 0x79, 0x1d }, 2, { 0x79, 0x9d } },
	{ PCE_KEY_KP_7,       2, { 0x79, 0x33 }, 2, { 0x79, 0xb3 } },
	{ PCE_KEY_KP_8,       2, { 0x79, 0x37 }, 2, { 0x79, 0xb7 } },
	{ PCE_KEY_KP_9,       2, { 0x79, 0x39 }, 2, { 0x79, 0xb9 } },
	{ PCE_KEY_KP_PLUS,    3, { 0x71, 0x79, 0x0d }, 3, { 0x79, 0x8d, 0xf1 } },
	{ PCE_KEY_KP_4,       2, { 0x79, 0x2d }, 2, { 0x79, 0xad } },
	{ PCE_KEY_KP_5,       2, { 0x79, 0x2f }, 2, { 0x79, 0xaf } },
	{ PCE_KEY_KP_6,       2, { 0x79, 0x31 }, 2, { 0x79, 0xb1 } },
	{ PCE_KEY_KP_1,       2, { 0x79, 0x27 }, 2, { 0x79, 0xa7 } },
	{ PCE_KEY_KP_2,       2, { 0x79, 0x29 }, 2, { 0x79, 0xa9 } },
	{ PCE_KEY_KP_3,       2, { 0x79, 0x2b }, 2, { 0x79, 0xab } },
	{ PCE_KEY_KP_ENTER,   2, { 0x79, 0x19 }, 2, { 0x79, 0x99 } },
	{ PCE_KEY_KP_0,       2, { 0x79, 0x25 }, 2, { 0x79, 0xa5 } },
	{ PCE_KEY_KP_PERIOD,  2, { 0x79, 0x03 }, 2, { 0x79, 0x83 } },
	{ PCE_KEY_UP,         2, { 0x79, 0x1b }, 2, { 0x79, 0x9b } },
	{ PCE_KEY_LEFT,       2, { 0x79, 0x0d }, 2, { 0x79, 0x8d } },
	{ PCE_KEY_RIGHT,      2, { 0x79, 0x05 }, 2, { 0x79, 0x85 } },
	{ PCE_KEY_DOWN,       2, { 0x79, 0x11 }, 2, { 0x79, 0x91 } },

	{ PCE_KEY_HOME,       2, { 0x79, 0x67 }, 2, { 0x79, 0xe7 } },
	{ PCE_KEY_END,        2, { 0x79, 0x6f }, 2, { 0x79, 0xef } },
	{ PCE_KEY_PAGEUP,     2, { 0x79, 0x69 }, 2, { 0x79, 0xe9 } },
	{ PCE_KEY_PAGEDN,     2, { 0x79, 0x73 }, 2, { 0x79, 0xf3 } },
	{ PCE_KEY_INS,        2, { 0x79, 0x65 }, 2, { 0x79, 0x65 } },
	{ PCE_KEY_DEL,        2, { 0x79, 0x6b }, 2, { 0x79, 0xeb } },

	{ PCE_KEY_NONE,       0, { 0 },    0, { 0 } }
};

static mac_kbd_map_t key_map_intl[] = {
	{ PCE_KEY_BACKQUOTE,  1, { 0x65 }, 1, { 0xe5 } },
	{ PCE_KEY_1,          1, { 0x27 }, 1, { 0xa7 } },
	{ PCE_KEY_2,          1, { 0x27 }, 1, { 0xa7 } },
	{ PCE_KEY_3,          1, { 0x29 }, 1, { 0xa9 } },
	{ PCE_KEY_4,          1, { 0x2b }, 1, { 0xab } },
	{ PCE_KEY_5,          1, { 0x2f }, 1, { 0xaf } },
	{ PCE_KEY_6,          1, { 0x2d }, 1, { 0xad } },
	{ PCE_KEY_7,          1, { 0x35 }, 1, { 0xb5 } },
	{ PCE_KEY_8,          1, { 0x39 }, 1, { 0xb9 } },
	{ PCE_KEY_9,          1, { 0x33 }, 1, { 0xb3 } },
	{ PCE_KEY_0,          1, { 0x3b }, 1, { 0xbb } },
	{ PCE_KEY_MINUS,      1, { 0x37 }, 1, { 0xb7 } },
	{ PCE_KEY_EQUAL,      1, { 0x31 }, 1, { 0xb1 } },
	{ PCE_KEY_BACKSPACE,  1, { 0x67 }, 1, { 0xe7 } },
	{ PCE_KEY_TAB,        1, { 0x61 }, 1, { 0xe1 } },
	{ PCE_KEY_Q,          1, { 0x19 }, 1, { 0x99 } },
	{ PCE_KEY_W,          1, { 0x1b }, 1, { 0x9b } },
	{ PCE_KEY_E,          1, { 0x1d }, 1, { 0x9d } },
	{ PCE_KEY_R,          1, { 0x1f }, 1, { 0x9f } },
	{ PCE_KEY_T,          1, { 0x23 }, 1, { 0xa3 } },
	{ PCE_KEY_Y,          1, { 0x21 }, 1, { 0xa1 } },
	{ PCE_KEY_U,          1, { 0x41 }, 1, { 0xc1 } },
	{ PCE_KEY_I,          1, { 0x45 }, 1, { 0xc5 } },
	{ PCE_KEY_O,          1, { 0x3f }, 1, { 0xbf } },
	{ PCE_KEY_P,          1, { 0x47 }, 1, { 0xc7 } },
	{ PCE_KEY_LBRACKET,   1, { 0x43 }, 1, { 0xc3 } },
	{ PCE_KEY_RBRACKET,   1, { 0x3d }, 1, { 0xbd } },
	{ PCE_KEY_RETURN,     1, { 0x55 }, 1, { 0xd5 } },
	{ PCE_KEY_CAPSLOCK,   1, { 0x73 }, 1, { 0xf3 } },
	{ PCE_KEY_A,          1, { 0x01 }, 1, { 0x81 } },
	{ PCE_KEY_S,          1, { 0x03 }, 1, { 0x83 } },
	{ PCE_KEY_D,          1, { 0x05 }, 1, { 0x85 } },
	{ PCE_KEY_F,          1, { 0x07 }, 1, { 0x87 } },
	{ PCE_KEY_G,          1, { 0x0b }, 1, { 0x8b } },
	{ PCE_KEY_H,          1, { 0x09 }, 1, { 0x89 } },
	{ PCE_KEY_J,          1, { 0x4d }, 1, { 0xcd } },
	{ PCE_KEY_K,          1, { 0x51 }, 1, { 0xd1 } },
	{ PCE_KEY_L,          1, { 0x4b }, 1, { 0xcb } },
	{ PCE_KEY_SEMICOLON,  1, { 0x53 }, 1, { 0xd3 } },
	{ PCE_KEY_QUOTE,      1, { 0x4f }, 1, { 0xcf } },
	{ PCE_KEY_BACKSLASH,  1, { 0x49 }, 1, { 0xc9 } },
	{ PCE_KEY_LSHIFT,     1, { 0x71 }, 1, { 0xf1 } },
	{ PCE_KEY_RSHIFT,     1, { 0x71 }, 1, { 0xf1 } },
	{ PCE_KEY_LESS,       1, { 0x0d }, 1, { 0x8d } },
	{ PCE_KEY_Z,          1, { 0x0f }, 1, { 0x8f } },
	{ PCE_KEY_X,          1, { 0x11 }, 1, { 0x91 } },
	{ PCE_KEY_C,          1, { 0x13 }, 1, { 0x93 } },
	{ PCE_KEY_V,          1, { 0x17 }, 1, { 0x97 } },
	{ PCE_KEY_B,          1, { 0x5b }, 1, { 0xdb } },
	{ PCE_KEY_N,          1, { 0x5d }, 1, { 0xdd } },
	{ PCE_KEY_M,          1, { 0x57 }, 1, { 0xd7 } },
	{ PCE_KEY_COMMA,      1, { 0x5f }, 1, { 0xdf } },
	{ PCE_KEY_PERIOD,     1, { 0x59 }, 1, { 0xd9 } },
	{ PCE_KEY_SLASH,      1, { 0x15 }, 1, { 0x95 } },
	{ PCE_KEY_LCTRL,      1, { 0x75 }, 1, { 0xf5 } },
	{ PCE_KEY_RCTRL,      1, { 0x75 }, 1, { 0xf5 } },
	{ PCE_KEY_LALT,       1, { 0x6f }, 1, { 0xef } },
	{ PCE_KEY_RALT,       1, { 0x63 }, 1, { 0xe3 } },
	{ PCE_KEY_SPACE,      1, { 0x69 }, 1, { 0xe9 } },
	{ PCE_KEY_NUMLOCK,    2, { 0x79, 0x0f }, 2, { 0x79, 0x8f } },
	{ PCE_KEY_KP_SLASH,   3, { 0x71, 0x79, 0x1b }, 3, { 0x79, 0x9b, 0xf1 } },
	{ PCE_KEY_KP_STAR,    3, { 0x71, 0x79, 0x05 }, 3, { 0x79, 0x85, 0xf1 } },
	{ PCE_KEY_KP_MINUS,   2, { 0x79, 0x1d }, 2, { 0x79, 0x9d } },
	{ PCE_KEY_KP_7,       2, { 0x79, 0x33 }, 2, { 0x79, 0xb3 } },
	{ PCE_KEY_KP_8,       2, { 0x79, 0x37 }, 2, { 0x79, 0xb7 } },
	{ PCE_KEY_KP_9,       2, { 0x79, 0x39 }, 2, { 0x79, 0xb9 } },
	{ PCE_KEY_KP_PLUS,    3, { 0x71, 0x79, 0x0d }, 3, { 0x79, 0x8d, 0xf1 } },
	{ PCE_KEY_KP_4,       2, { 0x79, 0x2d }, 2, { 0x79, 0xad } },
	{ PCE_KEY_KP_5,       2, { 0x79, 0x2f }, 2, { 0x79, 0xaf } },
	{ PCE_KEY_KP_6,       2, { 0x79, 0x31 }, 2, { 0x79, 0xb1 } },
	{ PCE_KEY_KP_1,       2, { 0x79, 0x27 }, 2, { 0x79, 0xa7 } },
	{ PCE_KEY_KP_2,       2, { 0x79, 0x29 }, 2, { 0x79, 0xa9 } },
	{ PCE_KEY_KP_3,       2, { 0x79, 0x2b }, 2, { 0x79, 0xab } },
	{ PCE_KEY_KP_ENTER,   2, { 0x79, 0x19 }, 2, { 0x79, 0x99 } },
	{ PCE_KEY_KP_0,       2, { 0x79, 0x25 }, 2, { 0x79, 0xa5 } },
	{ PCE_KEY_KP_PERIOD,  2, { 0x79, 0x03 }, 2, { 0x79, 0x83 } },
	{ PCE_KEY_UP,         2, { 0x79, 0x1b }, 2, { 0x79, 0x9b } },
	{ PCE_KEY_LEFT,       2, { 0x79, 0x0d }, 2, { 0x79, 0x8d } },
	{ PCE_KEY_RIGHT,      2, { 0x79, 0x05 }, 2, { 0x79, 0x85 } },
	{ PCE_KEY_DOWN,       2, { 0x79, 0x11 }, 2, { 0x79, 0x91 } },

	{ PCE_KEY_HOME,       2, { 0x79, 0x67 }, 2, { 0x79, 0xe7 } },
	{ PCE_KEY_END,        2, { 0x79, 0x6f }, 2, { 0x79, 0xef } },
	{ PCE_KEY_PAGEUP,     2, { 0x79, 0x69 }, 2, { 0x79, 0xe9 } },
	{ PCE_KEY_PAGEDN,     2, { 0x79, 0x73 }, 2, { 0x79, 0xf3 } },
	{ PCE_KEY_INS,        2, { 0x79, 0x65 }, 2, { 0x79, 0xe5 } },
	{ PCE_KEY_DEL,        2, { 0x79, 0x6b }, 2, { 0x79, 0xeb } },

	{ PCE_KEY_NONE,       0, { 0x00 }, 0, { 0x00 } }
};

/* keypad to motion, motion keys to keypad */
static mac_kbd_map_t key_fix_keypad1[] = {
	{ PCE_KEY_KP_7,       2, { 0x79, 0x67 }, 2, { 0x79, 0xe7 } },
	{ PCE_KEY_KP_8,       2, { 0x79, 0x1b }, 2, { 0x79, 0x9b } },
	{ PCE_KEY_KP_9,       2, { 0x79, 0x69 }, 2, { 0x79, 0xe9 } },
	{ PCE_KEY_KP_4,       2, { 0x79, 0x0d }, 2, { 0x79, 0x8d } },
	{ PCE_KEY_KP_5,       2, { 0x79, 0x19 }, 2, { 0x79, 0x99 } },
	{ PCE_KEY_KP_6,       2, { 0x79, 0x05 }, 2, { 0x79, 0x85 } },
	{ PCE_KEY_KP_1,       2, { 0x79, 0x6f }, 2, { 0x79, 0xef } },
	{ PCE_KEY_KP_2,       2, { 0x79, 0x11 }, 2, { 0x79, 0x91 } },
	{ PCE_KEY_KP_3,       2, { 0x79, 0x73 }, 2, { 0x79, 0xf3 } },
	{ PCE_KEY_KP_0,       2, { 0x79, 0x65 }, 2, { 0x79, 0x65 } },
	{ PCE_KEY_KP_PERIOD,  2, { 0x79, 0x6b }, 2, { 0x79, 0xeb } },
	{ PCE_KEY_KP_ENTER,   1, { 0x49 }, 1, { 0xc9 } },

	{ PCE_KEY_HOME,       2, { 0x79, 0x33 }, 2, { 0x79, 0xb3 } },
	{ PCE_KEY_UP,         2, { 0x79, 0x37 }, 2, { 0x79, 0xb7 } },
	{ PCE_KEY_PAGEUP,     2, { 0x79, 0x39 }, 2, { 0x79, 0xb9 } },
	{ PCE_KEY_LEFT,       2, { 0x79, 0x2d }, 2, { 0x79, 0xad } },
	{ PCE_KEY_RIGHT,      2, { 0x79, 0x31 }, 2, { 0x79, 0xb1 } },
	{ PCE_KEY_END,        2, { 0x79, 0x27 }, 2, { 0x79, 0xa7 } },
	{ PCE_KEY_DOWN,       2, { 0x79, 0x29 }, 2, { 0x79, 0xa9 } },
	{ PCE_KEY_PAGEDN,     2, { 0x79, 0x2b }, 2, { 0x79, 0xab } },
	{ PCE_KEY_INS,        2, { 0x79, 0x25 }, 2, { 0x79, 0xa5 } },
	{ PCE_KEY_DEL,        2, { 0x79, 0x03 }, 2, { 0x79, 0x83 } },

	{ PCE_KEY_NONE,       0, { 0 },    0, { 0 } }
};

/* keypad to keypad, motion to motion */
static mac_kbd_map_t key_fix_keypad2[] = {
	{ PCE_KEY_KP_7,       2, { 0x79, 0x33 }, 2, { 0x79, 0xb3 } },
	{ PCE_KEY_KP_8,       2, { 0x79, 0x37 }, 2, { 0x79, 0xb7 } },
	{ PCE_KEY_KP_9,       2, { 0x79, 0x39 }, 2, { 0x79, 0xb9 } },
	{ PCE_KEY_KP_4,       2, { 0x79, 0x2d }, 2, { 0x79, 0xad } },
	{ PCE_KEY_KP_5,       2, { 0x79, 0x2f }, 2, { 0x79, 0xaf } },
	{ PCE_KEY_KP_6,       2, { 0x79, 0x31 }, 2, { 0x79, 0xb1 } },
	{ PCE_KEY_KP_1,       2, { 0x79, 0x27 }, 2, { 0x79, 0xa7 } },
	{ PCE_KEY_KP_2,       2, { 0x79, 0x29 }, 2, { 0x79, 0xa9 } },
	{ PCE_KEY_KP_3,       2, { 0x79, 0x2b }, 2, { 0x79, 0xab } },
	{ PCE_KEY_KP_0,       2, { 0x79, 0x25 }, 2, { 0x79, 0xa5 } },
	{ PCE_KEY_KP_PERIOD,  2, { 0x79, 0x03 }, 2, { 0x79, 0x83 } },
	{ PCE_KEY_KP_ENTER,   2, { 0x79, 0x19 }, 2, { 0x79, 0x99 } },

	{ PCE_KEY_HOME,       2, { 0x79, 0x67 }, 2, { 0x79, 0xe7 } },
	{ PCE_KEY_UP,         2, { 0x79, 0x1b }, 2, { 0x79, 0x9b } },
	{ PCE_KEY_PAGEUP,     2, { 0x79, 0x69 }, 2, { 0x79, 0xe9 } },
	{ PCE_KEY_LEFT,       2, { 0x79, 0x0d }, 2, { 0x79, 0x8d } },
	{ PCE_KEY_RIGHT,      2, { 0x79, 0x05 }, 2, { 0x79, 0x85 } },
	{ PCE_KEY_END,        2, { 0x79, 0x6f }, 2, { 0x79, 0xef } },
	{ PCE_KEY_DOWN,       2, { 0x79, 0x11 }, 2, { 0x79, 0x91 } },
	{ PCE_KEY_PAGEDN,     2, { 0x79, 0x73 }, 2, { 0x79, 0xf3 } },
	{ PCE_KEY_INS,        2, { 0x79, 0x65 }, 2, { 0x79, 0x65 } },
	{ PCE_KEY_DEL,        2, { 0x79, 0x6b }, 2, { 0x79, 0xeb } },

	{ PCE_KEY_NONE,       0, { 0 },    0, { 0 } }
};


static
void mac_kbd_fix_map (mac_kbd_map_t *dst, const mac_kbd_map_t *src)
{
	mac_kbd_map_t *tmp;

	while (src->pcekey != PCE_KEY_NONE) {
		tmp = dst;
		while (tmp->pcekey != PCE_KEY_NONE) {
			if (tmp->pcekey == src->pcekey) {
				*tmp = *src;
				break;
			}

			tmp += 1;
		}

		src += 1;
	}
}

void mac_kbd_init (mac_kbd_t *kbd)
{
	kbd->set_data_ext = NULL;
	kbd->set_data = NULL;

	kbd->set_intr_ext = NULL;
	kbd->set_intr = NULL;

	kbd->data = 0;
	kbd->timeout = 0;

	kbd->send_byte = 0;

	kbd->buf_i = 0;
	kbd->buf_n = 0;

	kbd->model = 0;

	kbd->keypad_mode = 0;

	kbd->keymap = key_map_us;
}

void mac_kbd_free (mac_kbd_t *kbd)
{
}

mac_kbd_t *mac_kbd_new (void)
{
	mac_kbd_t *kbd;

	kbd = malloc (sizeof (mac_kbd_t));

	if (kbd == NULL) {
		return (NULL);
	}

	mac_kbd_init (kbd);

	return (kbd);
}

void mac_kbd_del (mac_kbd_t *kbd)
{
	if (kbd != NULL) {
		mac_kbd_free (kbd);
		free (kbd);
	}
}

void mac_kbd_set_data_fct (mac_kbd_t *kbd, void *ext, void *fct)
{
	kbd->set_data_ext = ext;
	kbd->set_data = fct;
}

void mac_kbd_set_intr_fct (mac_kbd_t *kbd, void *ext, void *fct)
{
	kbd->set_intr_ext = ext;
	kbd->set_intr = fct;
}

int mac_kbd_set_model (mac_kbd_t *kbd, unsigned model, int intl)
{
	kbd->model = model;

	if (intl) {
		kbd->keymap = key_map_intl;
	}
	else {
		kbd->keymap = key_map_us;
	}

	return (0);
}

void mac_kbd_set_keypad_mode (mac_kbd_t *kbd, int motion)
{
	if (motion) {
		mac_kbd_fix_map (kbd->keymap, key_fix_keypad1);
		kbd->keypad_mode = 0;
	}
	else {
		mac_kbd_fix_map (kbd->keymap, key_fix_keypad2);
		kbd->keypad_mode = 1;
	}
}

static
void mac_kbd_set_data_out (mac_kbd_t *kbd, unsigned char val)
{
	if (kbd->set_data != NULL) {
		kbd->set_data (kbd->set_data_ext, val);
	}
}

static
void mac_kbd_buf_set_byte (mac_kbd_t *kbd, unsigned char val)
{
	unsigned i;

	if (kbd->buf_n >= MAC_KBD_BUFSIZE) {
		return;
	}

	i = (kbd->buf_i + kbd->buf_n) % MAC_KBD_BUFSIZE;

	kbd->buf[i] = val;

	kbd->buf_n += 1;
}

static
void mac_kbd_set_sequence (mac_kbd_t *kbd, unsigned char *buf, unsigned cnt)
{
	unsigned i;

	for (i = 0; i < cnt; i++) {
		mac_kbd_buf_set_byte (kbd, buf[i]);
	}
}

void mac_kbd_set_key (mac_kbd_t *kbd, unsigned event, pce_key_t key)
{
	mac_kbd_map_t *map;

#ifdef DEBUG_KBD
	mac_log_deb ("kbd: set key: %u %u\n", event, (unsigned) key);
#endif

	if (key == PCE_KEY_F1) {
		key = PCE_KEY_LCTRL;
	}
	else if (key == PCE_KEY_F2) {
		key = PCE_KEY_LALT;
	}

	map = kbd->keymap;

	while (map->pcekey != PCE_KEY_NONE) {
		if (map->pcekey == key) {
			break;
		}

		map += 1;
	}

	if (map->pcekey == PCE_KEY_NONE) {
		const char *str;

		str = pce_key_to_string (key);

		pce_log (MSG_INF, "ignoring pce key: %04x (%s)\n",
			(unsigned) key,
			(str != NULL) ? str : "<none>"
		);

		return;
	}

	switch (event) {
	case PCE_KEY_EVENT_DOWN:
		mac_kbd_set_sequence (kbd, map->down, map->down_cnt);
		break;

	case PCE_KEY_EVENT_UP:
		mac_kbd_set_sequence (kbd, map->up, map->up_cnt);
		break;
	}
}

void mac_kbd_set_uint8 (mac_kbd_t *kbd, unsigned char val)
{
	switch (val) {
	case 0x10: /* inquiry */
		kbd->timeout = MAC_CPU_CLOCK / 4;
		kbd->send_byte = 1;
		break;

	case 0x14: /* instant */
		kbd->timeout = 0;
		kbd->send_byte = 1;
		break;

	case 0x16: /* model number */
#ifdef DEBUG_KBD
		mac_log_deb ("kbd: command: model number (%02X)\n", val);
#endif
		kbd->buf_i = 0;
		kbd->buf_n = 1;
		kbd->buf[0] = ((kbd->model & 7) << 1) | 0x01;
		kbd->timeout = 0;
		kbd->send_byte = 1;
		break;

	default:
		mac_log_deb ("kbd: unknown command (%02X)\n", val);
		break;
	}
}

void mac_kbd_set_data (mac_kbd_t *kbd, unsigned char val)
{
	kbd->data = val;
}

void mac_kbd_clock (mac_kbd_t *kbd, unsigned cnt)
{
	unsigned char val;

	if (kbd->data == 0) {
		return;
	}

	if (kbd->send_byte == 0) {
		return;
	}

	if (kbd->buf_n > 0) {
		val = kbd->buf[kbd->buf_i];

#ifdef DEBUG_KBD
		mac_log_deb ("kbd: send response (%02X)\n", val);
#endif

		mac_kbd_set_data_out (kbd, val);

		kbd->buf_i = (kbd->buf_i + 1) % MAC_KBD_BUFSIZE;
		kbd->buf_n -= 1;

		kbd->send_byte = 0;
		kbd->timeout = 0;
	}
	else {
		if (cnt < kbd->timeout) {
			kbd->timeout -= cnt;
		}
		else {
			mac_kbd_set_data_out (kbd, 0x7b);
			kbd->send_byte = 0;
		}
	}
}
