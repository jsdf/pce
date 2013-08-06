/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/keyboard.c                                    *
 * Created:     2012-07-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/log.h>

#include <drivers/video/terminal.h>


#define RC759_KBD_DELAY 6000000


typedef struct {
	pce_key_t      pcekey;

	unsigned short down_cnt;
	unsigned char  down[4];

	unsigned short up_cnt;
	unsigned char  up[4];

	char           isdown;
} rc759_keymap_t;


static rc759_keymap_t keymap[] = {
	{ PCE_KEY_ESC,       1, { 0x0f }, 1, { 0x8f }, 0 },
	{ PCE_KEY_F1,        1, { 0x3b }, 1, { 0xbb }, 0 },
	{ PCE_KEY_F2,        1, { 0x3c }, 1, { 0xbc }, 0 },
	{ PCE_KEY_F3,        1, { 0x3d }, 1, { 0xbd }, 0 },
	{ PCE_KEY_F4,        1, { 0x3e }, 1, { 0xbe }, 0 },
	{ PCE_KEY_F5,        1, { 0x3f }, 1, { 0xbf }, 0 },
	{ PCE_KEY_F6,        1, { 0x40 }, 1, { 0xc0 }, 0 },
	{ PCE_KEY_F7,        1, { 0x41 }, 1, { 0xc1 }, 0 },
	{ PCE_KEY_F8,        1, { 0x42 }, 1, { 0xc2 }, 0 },
	{ PCE_KEY_F9,        1, { 0x43 }, 1, { 0xc3 }, 0 },
	{ PCE_KEY_F10,       1, { 0x44 }, 1, { 0xc4 }, 0 },
	{ PCE_KEY_F11,       1, { 0x45 }, 1, { 0xc5 }, 0 },
	{ PCE_KEY_F12,       1, { 0x46 }, 1, { 0xc6 }, 0 },

	{ PCE_KEY_BACKQUOTE, 1, { 0x3a }, 1, { 0xba }, 0 },
	{ PCE_KEY_1,         1, { 0x02 }, 1, { 0x82 }, 0 },
	{ PCE_KEY_2,         1, { 0x03 }, 1, { 0x83 }, 0 },
	{ PCE_KEY_3,         1, { 0x04 }, 1, { 0x84 }, 0 },
	{ PCE_KEY_4,         1, { 0x05 }, 1, { 0x85 }, 0 },
	{ PCE_KEY_5,         1, { 0x06 }, 1, { 0x86 }, 0 },
	{ PCE_KEY_6,         1, { 0x07 }, 1, { 0x87 }, 0 },
	{ PCE_KEY_7,         1, { 0x08 }, 1, { 0x88 }, 0 },
	{ PCE_KEY_8,         1, { 0x09 }, 1, { 0x89 }, 0 },
	{ PCE_KEY_9,         1, { 0x0a }, 1, { 0x8a }, 0 },
	{ PCE_KEY_0,         1, { 0x0b }, 1, { 0x8b }, 0 },
	{ PCE_KEY_MINUS,     1, { 0x0c }, 1, { 0x8c }, 0 },
	{ PCE_KEY_EQUAL,     1, { 0x0d }, 1, { 0x8d }, 0 },
	{ PCE_KEY_BACKSPACE, 1, { 0x0e }, 1, { 0x8e }, 0 },

	{ PCE_KEY_TAB,       1, { 0x01 }, 1, { 0x81 }, 0 },
	{ PCE_KEY_Q,         1, { 0x10 }, 1, { 0x90 }, 0 },
	{ PCE_KEY_W,         1, { 0x11 }, 1, { 0x91 }, 0 },
	{ PCE_KEY_E,         1, { 0x12 }, 1, { 0x92 }, 0 },
	{ PCE_KEY_R,         1, { 0x13 }, 1, { 0x93 }, 0 },
	{ PCE_KEY_T,         1, { 0x14 }, 1, { 0x94 }, 0 },
	{ PCE_KEY_Y,         1, { 0x15 }, 1, { 0x95 }, 0 },
	{ PCE_KEY_U,         1, { 0x16 }, 1, { 0x96 }, 0 },
	{ PCE_KEY_I,         1, { 0x17 }, 1, { 0x97 }, 0 },
	{ PCE_KEY_O,         1, { 0x18 }, 1, { 0x98 }, 0 },
	{ PCE_KEY_P,         1, { 0x19 }, 1, { 0x99 }, 0 },
	{ PCE_KEY_LBRACKET,  1, { 0x1b }, 1, { 0x9b }, 0 },
	{ PCE_KEY_RBRACKET,  1, { 0x27 }, 1, { 0xa7 }, 0 },
	{ PCE_KEY_RETURN,    1, { 0x1c }, 1, { 0x9c }, 0 },

	{ PCE_KEY_CAPSLOCK,  1, { 0x28 }, 1, { 0xa8 }, 0 },
	{ PCE_KEY_A,         1, { 0x1e }, 1, { 0x9e }, 0 },
	{ PCE_KEY_S,         1, { 0x1f }, 1, { 0x9f }, 0 },
	{ PCE_KEY_D,         1, { 0x20 }, 1, { 0xa0 }, 0 },
	{ PCE_KEY_F,         1, { 0x21 }, 1, { 0xa1 }, 0 },
	{ PCE_KEY_G,         1, { 0x22 }, 1, { 0xa2 }, 0 },
	{ PCE_KEY_H,         1, { 0x23 }, 1, { 0xa3 }, 0 },
	{ PCE_KEY_J,         1, { 0x24 }, 1, { 0xa4 }, 0 },
	{ PCE_KEY_K,         1, { 0x25 }, 1, { 0xa5 }, 0 },
	{ PCE_KEY_L,         1, { 0x26 }, 1, { 0xa6 }, 0 },
	{ PCE_KEY_SEMICOLON, 1, { 0x1a }, 1, { 0x9a }, 0 },
	{ PCE_KEY_QUOTE,     1, { 0x2b }, 1, { 0xab }, 0 },
	{ PCE_KEY_BACKSLASH, 1, { 0x29 }, 1, { 0xa9 }, 0 },

	{ PCE_KEY_LSHIFT,    1, { 0x2a }, 1, { 0xaa }, 0 },
	{ PCE_KEY_LESS,      1, { 0x38 }, 1, { 0xb8 }, 0 },
	{ PCE_KEY_Z,         1, { 0x2c }, 1, { 0xac }, 0 },
	{ PCE_KEY_X,         1, { 0x2d }, 1, { 0xad }, 0 },
	{ PCE_KEY_C,         1, { 0x2e }, 1, { 0xae }, 0 },
	{ PCE_KEY_V,         1, { 0x2f }, 1, { 0xaf }, 0 },
	{ PCE_KEY_B,         1, { 0x30 }, 1, { 0xb0 }, 0 },
	{ PCE_KEY_N,         1, { 0x31 }, 1, { 0xb1 }, 0 },
	{ PCE_KEY_M,         1, { 0x32 }, 1, { 0xb2 }, 0 },
	{ PCE_KEY_COMMA,     1, { 0x33 }, 1, { 0xb3 }, 0 },
	{ PCE_KEY_PERIOD,    1, { 0x34 }, 1, { 0xb4 }, 0 },
	{ PCE_KEY_SLASH,     1, { 0x35 }, 1, { 0xb5 }, 0 },
	{ PCE_KEY_RSHIFT,    1, { 0x36 }, 1, { 0xb6 }, 0 },

	{ PCE_KEY_LCTRL,     1, { 0x1d }, 1, { 0x9d }, 0 },
	{ PCE_KEY_LALT,      1, { 0x37 }, 1, { 0xb7 }, 0 },
	{ PCE_KEY_SPACE,     1, { 0x39 }, 1, { 0xb9 }, 0 },
	{ PCE_KEY_RCTRL,     1, { 0x1d }, 1, { 0x9d }, 0 },

	{ PCE_KEY_KP_MINUS,  1, { 0x56 }, 1, { 0xd6 }, 0 },
	{ PCE_KEY_KP_7,      1, { 0x53 }, 1, { 0xd3 }, 0 },
	{ PCE_KEY_KP_8,      1, { 0x54 }, 1, { 0xd4 }, 0 },
	{ PCE_KEY_KP_9,      1, { 0x55 }, 1, { 0xd5 }, 0 },
	{ PCE_KEY_KP_PLUS,   1, { 0x57 }, 1, { 0xd7 }, 0 },
	{ PCE_KEY_KP_4,      1, { 0x58 }, 1, { 0xd8 }, 0 },
	{ PCE_KEY_KP_5,      1, { 0x59 }, 1, { 0xd9 }, 0 },
	{ PCE_KEY_KP_6,      1, { 0x5a }, 1, { 0xda }, 0 },
	{ PCE_KEY_KP_1,      1, { 0x5d }, 1, { 0xdd }, 0 },
	{ PCE_KEY_KP_2,      1, { 0x5e }, 1, { 0xde }, 0 },
	{ PCE_KEY_KP_3,      1, { 0x5f }, 1, { 0xdf }, 0 },
	{ PCE_KEY_KP_ENTER,  1, { 0x60 }, 1, { 0xe0 }, 0 },
	{ PCE_KEY_KP_0,      1, { 0x61 }, 1, { 0xe1 }, 0 },
	{ PCE_KEY_KP_PERIOD, 1, { 0x62 }, 1, { 0xe2 }, 0 },

	{ PCE_KEY_INS,       1, { 0x47 }, 1, { 0xc7 }, 0 },
	{ PCE_KEY_HOME,      1, { 0x51 }, 1, { 0xd1 }, 0 },
	{ PCE_KEY_DEL,       1, { 0x52 }, 1, { 0xd2 }, 0 },
	{ PCE_KEY_UP,        1, { 0x4f }, 1, { 0xcf }, 0 },
	{ PCE_KEY_LEFT,      1, { 0x4d }, 1, { 0xcd }, 0 },
	{ PCE_KEY_DOWN,      1, { 0x50 }, 1, { 0xd0 }, 0 },
	{ PCE_KEY_RIGHT,     1, { 0x4e }, 1, { 0xce }, 0 },

	{ PCE_KEY_KP_SLASH,  1, { 0x5c }, 2, { 0xdc }, 0 },
	{ PCE_KEY_KP_STAR,   1, { 0x5b }, 1, { 0x5b }, 0 },

	{ PCE_KEY_PAGEUP,    1, { 0x48 }, 1, { 0xc8 }, 0 },
	{ PCE_KEY_PAGEDN,    1, { 0x49 }, 1, { 0xc9 }, 0 },
	{ PCE_KEY_END,       1, { 0x4a }, 1, { 0xca }, 0 },
	{ PCE_KEY_NUMLOCK,   1, { 0x4b }, 1, { 0xcb }, 0 },

	{ PCE_KEY_NONE,      0, { 0x00 }, 0, { 0x00 }, 0 }
};

#if 0
	{ PCE_KEY_SCRLK,     1, { 0x46 }, 1, { 0xc6 }, 0 },
#endif

void rc759_kbd_init (rc759_kbd_t *kbd)
{
	kbd->enable = 0;

	kbd->delay = 0;

	kbd->key = 0;
	kbd->key_valid = 0;

	kbd->key_i = 0;
	kbd->key_j = 0;

	kbd->mouse_dx = 0;
	kbd->mouse_dy = 0;
	kbd->mouse_but[0] = 0;
	kbd->mouse_but[1] = 0;

	kbd->irq_ext = NULL;
	kbd->irq = NULL;
}

void rc759_kbd_set_irq_fct (rc759_kbd_t *kbd, void *ext, void *fct)
{
	kbd->irq_ext = ext;
	kbd->irq = fct;
}

static
void rc759_kbd_set_irq (rc759_kbd_t *kbd, unsigned char val)
{
	if (kbd->irq != NULL) {
		kbd->irq (kbd->irq_ext, val);
	}
}

void rc759_kbd_reset (rc759_kbd_t *kbd)
{
	rc759_keymap_t *map;

	sim_log_deb ("reset keyboard\n");

	map = keymap;

	while (map->pcekey != PCE_KEY_NONE) {
		map->isdown = 0;
		map += 1;
	}

	kbd->key = 0x00;
	kbd->key_valid = 1;

	kbd->delay = RC759_KBD_DELAY / 10;
	kbd->delay = 0;

	/*
	 * FF: no error
	 * FE: PROM checksum error
	 * FD: Key depressed
	 * FC: Microcontroller failure
	 *
	 * E4-F3: Nationality codes
	 */
	kbd->key_i = 0;
	kbd->key_j = 2;
	kbd->key_buf[0] = 0xff;
	kbd->key_buf[1] = 0xe4;

	kbd->mouse_dx = 0;
	kbd->mouse_dy = 0;
	kbd->mouse_but[0] = kbd->mouse_but[1];

	rc759_kbd_set_irq (kbd, 0);
}

void rc759_kbd_set_enable (rc759_kbd_t *kbd, unsigned char val)
{
	val = (val != 0);

	if (kbd->enable == val) {
		return;
	}

	kbd->enable = val;

	if (val) {
		rc759_kbd_reset (kbd);
	}
}

/*
 * write a key code sequence into the key buffer
 */
static
void rc759_kbd_set_sequence (rc759_kbd_t *kbd, unsigned char *buf, unsigned cnt)
{
	unsigned i;
	unsigned next;

	for (i = 0; i < cnt; i++) {
		next = (kbd->key_j + 1) % RC759_KBD_BUF;

		if (next == kbd->key_i) {
			pce_log (MSG_ERR, "keyboard buffer overflow\n");
			return;
		}

		kbd->key_buf[kbd->key_j] = buf[i];
		kbd->key_j = next;
	}
}

static
unsigned char rc759_kbd_get_int8 (int *val)
{
	int           tmp;
	unsigned char ret;

	if (*val < 0) {
		tmp = (*val < -128) ? -128 : *val;
		ret = (~(-tmp) + 1) & 0xff;
	}
	else {
		tmp = (*val > 127) ? 127 : *val;
		ret = tmp;
	}

	*val -= tmp;

	return (ret);
}

static
void rc759_kbd_check_mouse (rc759_kbd_t *kbd)
{
	unsigned char buf[10];

	if (kbd->key_i != kbd->key_j) {
		return;
	}

	if ((kbd->mouse_dx == 0) && (kbd->mouse_dy == 0) && (kbd->mouse_but[0] == kbd->mouse_but[1])) {
		return;
	}

	kbd->mouse_but[0] = kbd->mouse_but[1];

	buf[0] = 0xe3;
	buf[1] = 0x87;
	buf[2] = 0xe3;
	buf[3] = rc759_kbd_get_int8 (&kbd->mouse_dx);
	buf[4] = 0xe3;
	buf[5] = rc759_kbd_get_int8 (&kbd->mouse_dy);
	buf[6] = 0xe3;
	buf[7] = rc759_kbd_get_int8 (&kbd->mouse_dx);
	buf[8] = 0xe3;
	buf[9] = rc759_kbd_get_int8 (&kbd->mouse_dy);

	if (kbd->mouse_but[1] & 1) {
		buf[1] &= ~4;
	}

	if (kbd->mouse_but[1] & 2) {
		buf[1] &= ~2;
	}

	if (kbd->mouse_but[1] & 4) {
		buf[1] &= ~1;
	}

	rc759_kbd_set_sequence (kbd, buf, 10);
}

void rc759_kbd_set_mouse (rc759_kbd_t *kbd, int dx, int dy, unsigned but)
{
	kbd->mouse_dx += dx;
	kbd->mouse_dy -= dy;
	kbd->mouse_but[1] = but;

	rc759_kbd_check_mouse (kbd);
}

void rc759_kbd_set_key (rc759_kbd_t *kbd, unsigned event, unsigned key)
{
	rc759_keymap_t *map;

	if (event == PCE_KEY_EVENT_MAGIC) {
		pce_log (MSG_INF, "unhandled magic key (%u)\n",
			(unsigned) key
		);

		return;
	}

	map = keymap;

	while (map->pcekey != PCE_KEY_NONE) {
		if (map->pcekey == key) {
			break;
		}

		map += 1;
	}

	if (map->pcekey == PCE_KEY_NONE) {
		const char *str;

		str = pce_key_to_string (key);

		pce_log (MSG_INF, "ignoring pce key: %u 0x%04x (%s)\n",
			event, (unsigned) key,
			(str != NULL) ? str : "<none>"
		);

		return;
	}

	switch (event) {
	case PCE_KEY_EVENT_DOWN:
		map->isdown = 1;
		rc759_kbd_set_sequence (kbd, map->down, map->down_cnt);
		break;

	case PCE_KEY_EVENT_UP:
		if (map->isdown) {
			map->isdown = 0;
			rc759_kbd_set_sequence (kbd, map->up, map->up_cnt);
		}
		break;
	}
}

void rc759_kbd_set_keycode (rc759_kbd_t *kbd, unsigned char val)
{
	rc759_kbd_set_sequence (kbd, &val, 1);
}

unsigned char rc759_kbd_get_key (rc759_kbd_t *kbd)
{
	if (kbd->key_valid) {
		kbd->key_valid = 0;
		kbd->delay = RC759_KBD_DELAY / 100;
		rc759_kbd_set_irq (kbd, 0);
	}

	return (kbd->key);
}

void rc759_kbd_clock (rc759_kbd_t *kbd, unsigned cnt)
{
	if (kbd->key_i == kbd->key_j) {
		return;
	}

	if (kbd->enable == 0) {
		return;
	}

	if (kbd->delay > cnt) {
		kbd->delay -= cnt;
		return;
	}

	if (kbd->key_valid) {
		return;
	}

	kbd->delay = RC759_KBD_DELAY / 100;

	kbd->key = kbd->key_buf[kbd->key_i];
	kbd->key_valid = 1;

	kbd->key_i = (kbd->key_i + 1) % RC759_KBD_BUF;


	rc759_kbd_check_mouse (kbd);

	rc759_kbd_set_irq (kbd, 1);
}
