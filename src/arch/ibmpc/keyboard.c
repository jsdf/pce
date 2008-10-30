/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/keyboard.c                                    *
 * Created:     2007-11-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007 Hampa Hug <hampa@hampa.ch>                          *
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


#include "main.h"


typedef struct {
	pce_key_t pcekey;

	unsigned short down_cnt;
	unsigned char  down[4];

	unsigned short up_cnt;
	unsigned char  up[4];
} pc_keymap_t;


static pc_keymap_t keymap[] = {
	{ PCE_KEY_ESC,       1, { 0x01 }, 1, { 0x81 } },
	{ PCE_KEY_1,         1, { 0x02 }, 1, { 0x82 } },
	{ PCE_KEY_2,         1, { 0x03 }, 1, { 0x83 } },
	{ PCE_KEY_3,         1, { 0x04 }, 1, { 0x84 } },
	{ PCE_KEY_4,         1, { 0x05 }, 1, { 0x85 } },
	{ PCE_KEY_5,         1, { 0x06 }, 1, { 0x86 } },
	{ PCE_KEY_6,         1, { 0x07 }, 1, { 0x87 } },
	{ PCE_KEY_7,         1, { 0x08 }, 1, { 0x88 } },
	{ PCE_KEY_8,         1, { 0x09 }, 1, { 0x89 } },
	{ PCE_KEY_9,         1, { 0x0a }, 1, { 0x8a } },
	{ PCE_KEY_0,         1, { 0x0b }, 1, { 0x8b } },
	{ PCE_KEY_MINUS,     1, { 0x0c }, 1, { 0x8c } },
	{ PCE_KEY_EQUAL,     1, { 0x0d }, 1, { 0x8d } },
	{ PCE_KEY_BACKSPACE, 1, { 0x0e }, 1, { 0x8e } },
	{ PCE_KEY_TAB,       1, { 0x0f }, 1, { 0x8f } },
	{ PCE_KEY_Q,         1, { 0x10 }, 1, { 0x90 } },
	{ PCE_KEY_W,         1, { 0x11 }, 1, { 0x91 } },
	{ PCE_KEY_E,         1, { 0x12 }, 1, { 0x92 } },
	{ PCE_KEY_R,         1, { 0x13 }, 1, { 0x93 } },
	{ PCE_KEY_T,         1, { 0x14 }, 1, { 0x94 } },
	{ PCE_KEY_Y,         1, { 0x15 }, 1, { 0x95 } },
	{ PCE_KEY_U,         1, { 0x16 }, 1, { 0x96 } },
	{ PCE_KEY_I,         1, { 0x17 }, 1, { 0x97 } },
	{ PCE_KEY_O,         1, { 0x18 }, 1, { 0x98 } },
	{ PCE_KEY_P,         1, { 0x19 }, 1, { 0x99 } },
	{ PCE_KEY_LBRACKET,  1, { 0x1a }, 1, { 0x9a } },
	{ PCE_KEY_RBRACKET,  1, { 0x1b }, 1, { 0x9b } },
	{ PCE_KEY_RETURN,    1, { 0x1c }, 1, { 0x9c } },
	{ PCE_KEY_LCTRL,     1, { 0x1d }, 1, { 0x9d } },
	{ PCE_KEY_RCTRL,     1, { 0x1d }, 1, { 0x9d } },
	{ PCE_KEY_A,         1, { 0x1e }, 1, { 0x9e } },
	{ PCE_KEY_S,         1, { 0x1f }, 1, { 0x9f } },
	{ PCE_KEY_D,         1, { 0x20 }, 1, { 0xa0 } },
	{ PCE_KEY_F,         1, { 0x21 }, 1, { 0xa1 } },
	{ PCE_KEY_G,         1, { 0x22 }, 1, { 0xa2 } },
	{ PCE_KEY_H,         1, { 0x23 }, 1, { 0xa3 } },
	{ PCE_KEY_J,         1, { 0x24 }, 1, { 0xa4 } },
	{ PCE_KEY_K,         1, { 0x25 }, 1, { 0xa5 } },
	{ PCE_KEY_L,         1, { 0x26 }, 1, { 0xa6 } },
	{ PCE_KEY_SEMICOLON, 1, { 0x27 }, 1, { 0xa7 } },
	{ PCE_KEY_QUOTE,     1, { 0x28 }, 1, { 0xa8 } },
	{ PCE_KEY_BACKQUOTE, 1, { 0x29 }, 1, { 0xa9 } },
	{ PCE_KEY_LSHIFT,    1, { 0x2a }, 1, { 0xaa } },
	{ PCE_KEY_BACKSLASH, 1, { 0x2b }, 1, { 0xab } },
	{ PCE_KEY_Z,         1, { 0x2c }, 1, { 0xac } },
	{ PCE_KEY_X,         1, { 0x2d }, 1, { 0xad } },
	{ PCE_KEY_C,         1, { 0x2e }, 1, { 0xae } },
	{ PCE_KEY_V,         1, { 0x2f }, 1, { 0xaf } },
	{ PCE_KEY_B,         1, { 0x30 }, 1, { 0xb0 } },
	{ PCE_KEY_N,         1, { 0x31 }, 1, { 0xb1 } },
	{ PCE_KEY_M,         1, { 0x32 }, 1, { 0xb2 } },
	{ PCE_KEY_COMMA,     1, { 0x33 }, 1, { 0xb3 } },
	{ PCE_KEY_PERIOD,    1, { 0x34 }, 1, { 0xb4 } },
	{ PCE_KEY_SLASH,     1, { 0x35 }, 1, { 0xb5 } },
	{ PCE_KEY_RSHIFT,    1, { 0x36 }, 1, { 0xb6 } },
	{ PCE_KEY_KP_STAR,   1, { 0x37 }, 1, { 0xb7 } },
	{ PCE_KEY_LALT,      1, { 0x38 }, 1, { 0xb8 } },
	{ PCE_KEY_SPACE,     1, { 0x39 }, 1, { 0xb9 } },
	{ PCE_KEY_CAPSLOCK,  1, { 0x3a }, 1, { 0xba } },
	{ PCE_KEY_F1,        1, { 0x3b }, 1, { 0xbb } },
	{ PCE_KEY_F2,        1, { 0x3c }, 1, { 0xbc } },
	{ PCE_KEY_F3,        1, { 0x3d }, 1, { 0xbd } },
	{ PCE_KEY_F4,        1, { 0x3e }, 1, { 0xbe } },
	{ PCE_KEY_F5,        1, { 0x3f }, 1, { 0xbf } },
	{ PCE_KEY_F6,        1, { 0x40 }, 1, { 0xc0 } },
	{ PCE_KEY_F7,        1, { 0x41 }, 1, { 0xc1 } },
	{ PCE_KEY_F8,        1, { 0x42 }, 1, { 0xc2 } },
	{ PCE_KEY_F9,        1, { 0x43 }, 1, { 0xc3 } },
	{ PCE_KEY_F10,       1, { 0x44 }, 1, { 0xc4 } },
	{ PCE_KEY_NUMLOCK,   1, { 0x45 }, 1, { 0xc5 } },
	{ PCE_KEY_SCRLK,     1, { 0x46 }, 1, { 0xc6 } },
	{ PCE_KEY_KP_7,      1, { 0x47 }, 1, { 0xc7 } },
	{ PCE_KEY_HOME,      1, { 0x47 }, 1, { 0xc7 } },
	{ PCE_KEY_KP_8,      1, { 0x48 }, 1, { 0xc8 } },
	{ PCE_KEY_UP,        1, { 0x48 }, 1, { 0xc8 } },
	{ PCE_KEY_KP_9,      1, { 0x49 }, 1, { 0xc9 } },
	{ PCE_KEY_PAGEUP,    1, { 0x49 }, 1, { 0xc9 } },
	{ PCE_KEY_KP_MINUS,  1, { 0x4a }, 1, { 0xca } },
	{ PCE_KEY_KP_4,      1, { 0x4b }, 1, { 0xcb } },
	{ PCE_KEY_LEFT,      1, { 0x4b }, 1, { 0xcb } },
	{ PCE_KEY_KP_5,      1, { 0x4c }, 1, { 0xcc } },
	{ PCE_KEY_KP_6,      1, { 0x4d }, 1, { 0xcd } },
	{ PCE_KEY_RIGHT,     1, { 0x4d }, 1, { 0xcd } },
	{ PCE_KEY_KP_PLUS,   1, { 0x4e }, 1, { 0xce } },
	{ PCE_KEY_KP_1,      1, { 0x4f }, 1, { 0xcf } },
	{ PCE_KEY_END,       1, { 0x4f }, 1, { 0xcf } },
	{ PCE_KEY_KP_2,      1, { 0x50 }, 1, { 0xd0 } },
	{ PCE_KEY_DOWN,      1, { 0x50 }, 1, { 0xd0 } },
	{ PCE_KEY_KP_3,      1, { 0x51 }, 1, { 0xd1 } },
	{ PCE_KEY_PAGEDN,    1, { 0x51 }, 1, { 0xd1 } },
	{ PCE_KEY_KP_0,      1, { 0x52 }, 1, { 0xd2 } },
	{ PCE_KEY_INS,       1, { 0x52 }, 1, { 0xd2 } },
	{ PCE_KEY_KP_PERIOD, 1, { 0x53 }, 1, { 0xd3 } },
	{ PCE_KEY_DEL,       1, { 0x53 }, 1, { 0xd3 } },
	{ PCE_KEY_KP_SLASH,  2, { 0xe0, 0x35 }, 2, { 0xe0, 0xb5 } },
	{ PCE_KEY_F11,       1, { 0x57 }, 1, { 0xd7 } },
	{ PCE_KEY_F12,       1, { 0x58 }, 1, { 0xd8 } },
	{ PCE_KEY_NONE,      0, { 0x00 }, 0, { 0x00 } }
};

/*
 * write a key code sequence into the key buffer
 */
static
void pc_kbd_set_sequence (ibmpc_t *pc, unsigned char *buf, unsigned cnt)
{
	unsigned i;

	if ((pc->key_j + cnt) > 256) {
		return;
	}

	for (i = 0; i < cnt; i++) {
		pc->key_buf[pc->key_j] = buf[i];
		pc->key_j += 1;
	}
}

void pc_set_key (ibmpc_t *pc, unsigned event, unsigned key)
{
	pc_keymap_t *map;

	map = keymap;

	while (map->pcekey != PCE_KEY_NONE) {
		if (map->pcekey == key) {
			break;
		}

		map += 1;
	}

	if (map->pcekey == PCE_KEY_NONE) {
		return;
	}

	switch (event) {
	case 1:
		pc_kbd_set_sequence (pc, map->down, map->down_cnt);
		break;

	case 2:
		pc_kbd_set_sequence (pc, map->up, map->up_cnt);
		break;
	}
}

void pc_set_keycode (ibmpc_t *pc, unsigned char val)
{
	if (pc->key_j > 255) {
		return;
	}

	pc->key_buf[pc->key_j] = val;
	pc->key_j += 1;
}
