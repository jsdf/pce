/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/keybd.c                                       *
 * Created:     2020-04-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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
#include "keybd.h"
#include "msg.h"
#include "vic20.h"

#include <drivers/video/keys.h>


typedef struct {
	pce_key_t     key;

	unsigned char a1;
	unsigned char b1;

	unsigned char a2;
	unsigned char b2;
} vic20_key_map_t;


static vic20_key_map_t key_map[] = {
	{ PCE_KEY_1,         0, 0x01, 0, 0x00 },
	{ PCE_KEY_3,         0, 0x02, 0, 0x00 },
	{ PCE_KEY_5,         0, 0x04, 0, 0x00 },
	{ PCE_KEY_7,         0, 0x08, 0, 0x00 },
	{ PCE_KEY_9,         0, 0x10, 0, 0x00 },
	{ PCE_KEY_MINUS,     0, 0x20, 0, 0x00 }, /* "+" */
	{ PCE_KEY_KP_STAR,   0, 0x40, 0, 0x00 }, /* GBP */
	{ PCE_KEY_BACKSPACE, 0, 0x80, 0, 0x00 }, /* del */

	{ PCE_KEY_BACKQUOTE, 1, 0x01, 0, 0x00 }, /* "<-" */
	{ PCE_KEY_W,         1, 0x02, 0, 0x00 },
	{ PCE_KEY_R,         1, 0x04, 0, 0x00 },
	{ PCE_KEY_Y,         1, 0x08, 0, 0x00 },
	{ PCE_KEY_I,         1, 0x10, 0, 0x00 },
	{ PCE_KEY_P,         1, 0x20, 0, 0x00 },
	{ PCE_KEY_RBRACKET,  1, 0x40, 0, 0x00 }, /* "*" */
	{ PCE_KEY_RETURN,    1, 0x80, 0, 0x00 },

	{ PCE_KEY_LCTRL,     2, 0x01, 0, 0x00 }, /* "ctrl" */
	{ PCE_KEY_A,         2, 0x02, 0, 0x00 },
	{ PCE_KEY_D,         2, 0x04, 0, 0x00 },
	{ PCE_KEY_G,         2, 0x08, 0, 0x00 },
	{ PCE_KEY_J,         2, 0x10, 0, 0x00 },
	{ PCE_KEY_L,         2, 0x20, 0, 0x00 },
	{ PCE_KEY_QUOTE,     2, 0x40, 0, 0x00 }, /* ";" */
	{ PCE_KEY_RIGHT,     2, 0x80, 0, 0x00 },

	{ PCE_KEY_PAGEDN,    3, 0x01, 0, 0x00 }, /* run/stop */
	{ PCE_KEY_LSHIFT,    3, 0x02, 0, 0x00 },
	{ PCE_KEY_X,         3, 0x04, 0, 0x00 },
	{ PCE_KEY_V,         3, 0x08, 0, 0x00 },
	{ PCE_KEY_N,         3, 0x10, 0, 0x00 },
	{ PCE_KEY_COMMA,     3, 0x20, 0, 0x00 },
	{ PCE_KEY_SLASH,     3, 0x40, 0, 0x00 },
	{ PCE_KEY_DOWN,      3, 0x80, 0, 0x00 },

	{ PCE_KEY_SPACE,     4, 0x01, 0, 0x00 },
	{ PCE_KEY_Z,         4, 0x02, 0, 0x00 },
	{ PCE_KEY_C,         4, 0x04, 0, 0x00 },
	{ PCE_KEY_B,         4, 0x08, 0, 0x00 },
	{ PCE_KEY_M,         4, 0x10, 0, 0x00 },
	{ PCE_KEY_PERIOD,    4, 0x20, 0, 0x00 },
	{ PCE_KEY_RSHIFT,    4, 0x40, 0, 0x00 },
	{ PCE_KEY_F1,        4, 0x80, 0, 0x00 },

	{ PCE_KEY_LALT ,     5, 0x01, 0, 0x00 }, /* C= */
	{ PCE_KEY_S,         5, 0x02, 0, 0x00 },
	{ PCE_KEY_F,         5, 0x04, 0, 0x00 },
	{ PCE_KEY_H,         5, 0x08, 0, 0x00 },
	{ PCE_KEY_K,         5, 0x10, 0, 0x00 },
	{ PCE_KEY_SEMICOLON, 5, 0x20, 0, 0x00 }, /* : */
	{ PCE_KEY_BACKSLASH, 5, 0x40, 0, 0x00 }, /* = */
	{ PCE_KEY_F3,        5, 0x80, 0, 0x00 },

	{ PCE_KEY_Q,         6, 0x01, 0, 0x00 },
	{ PCE_KEY_E,         6, 0x02, 0, 0x00 },
	{ PCE_KEY_T,         6, 0x04, 0, 0x00 },
	{ PCE_KEY_U,         6, 0x08, 0, 0x00 },
	{ PCE_KEY_O,         6, 0x10, 0, 0x00 },
	{ PCE_KEY_LBRACKET,  6, 0x20, 0, 0x00 }, /* "@" */
	{ PCE_KEY_KP_SLASH,  6, 0x40, 0, 0x00 }, /* up arrow */
	{ PCE_KEY_F5,        6, 0x80, 0, 0x00 },

	{ PCE_KEY_2,         7, 0x01, 0, 0x00 },
	{ PCE_KEY_4,         7, 0x02, 0, 0x00 },
	{ PCE_KEY_6,         7, 0x04, 0, 0x00 },
	{ PCE_KEY_8,         7, 0x08, 0, 0x00 },
	{ PCE_KEY_0,         7, 0x10, 0, 0x00 },
	{ PCE_KEY_EQUAL,     7, 0x20, 0, 0x00 },
	{ PCE_KEY_HOME,      7, 0x40, 0, 0x00 }, /* clr/home */
	{ PCE_KEY_F7,        7, 0x80, 0, 0x00 },

	{ PCE_KEY_TAB,       2, 0x01, 0, 0x00 }, /* "ctrl" */

	{ PCE_KEY_LEFT,      2, 0x80, 3, 0x02 },
	{ PCE_KEY_UP,        3, 0x80, 3, 0x02 },
	{ PCE_KEY_INS,       0, 0x80, 3, 0x02 }, /* shift+del */
	{ PCE_KEY_DEL,       0, 0x80, 0, 0x00 }, /* del */

	{ PCE_KEY_KP_2,      3, 0x80, 0, 0x00 }, /* down */
	{ PCE_KEY_KP_4,      2, 0x80, 3, 0x02 }, /* shfit+right */
	{ PCE_KEY_KP_6,      2, 0x80, 0, 0x00 }, /* right */
	{ PCE_KEY_KP_8,      3, 0x80, 3, 0x02 }, /* shift+down */

	{ PCE_KEY_KP_0,      0, 0x80, 3, 0x02 }, /* shift+del */
	{ PCE_KEY_KP_PERIOD, 0, 0x80, 0, 0x00 }, /* del */

	{ PCE_KEY_KP_MINUS,  7, 0x20, 0, 0x00 }, /* "-" */
	{ PCE_KEY_KP_PLUS,   0, 0x20, 0, 0x00 }, /* "+" */
	{ PCE_KEY_KP_ENTER,  1, 0x80, 0, 0x00 },

	{ PCE_KEY_KP_3,      3, 0x01, 0, 0x00 }, /* run/stop */
	{ PCE_KEY_KP_7,      7, 0x40, 0, 0x00 }, /* clr/home */

	{ PCE_KEY_F2,        4, 0x80, 3, 0x02 }, /* shift+f1 */
	{ PCE_KEY_F4,        5, 0x80, 3, 0x02 },
	{ PCE_KEY_F6,        6, 0x80, 3, 0x02 },
	{ PCE_KEY_F8,        7, 0x80, 3, 0x02 },

	{ PCE_KEY_NONE,      0, 0, 0, 0 },
};

static vic20_key_map_t joy_map[] = {
	{ PCE_KEY_KP_0,      0x10, 0x00, 0, 0 },
	{ PCE_KEY_KP_1,      0x09, 0x06, 0, 0 },
	{ PCE_KEY_KP_2,      0x08, 0x04, 0, 0 },
	{ PCE_KEY_KP_3,      0x0a, 0x05, 0, 0 },
	{ PCE_KEY_KP_4,      0x01, 0x02, 0, 0 },
	{ PCE_KEY_KP_5,      0x10, 0x00, 0, 0 },
	{ PCE_KEY_KP_6,      0x02, 0x01, 0, 0 },
	{ PCE_KEY_KP_7,      0x05, 0x0a, 0, 0 },
	{ PCE_KEY_KP_8,      0x04, 0x08, 0, 0 },
	{ PCE_KEY_KP_9,      0x06, 0x09, 0, 0 },
	{ PCE_KEY_LSUPER,    0x10, 0x00, 0, 0 },
	{ PCE_KEY_NONE,      0, 0, 0, 0 },
};


void v20_keybd_matrix (vic20_t *sim, unsigned char *row, unsigned char *col)
{
	unsigned      i;
	unsigned char c, r;

	c = 0;
	r = 0;

	for (i = 0; i < 8 ; i++) {
		if (*row & (1 << i)) {
			c |= sim->keymat[i];
		}
	}

	for (i = 0; i < 8; i++) {
		if (*col & sim->keymat[i]) {
			r |= (1 << i);
		}
	}

	*row = r;
	*col = c;
}

void v20_keybd_reset (vic20_t *sim)
{
	unsigned i;

	for (i = 0; i < 8; i++) {
		sim->keymat[i] = 0;
	}
}

static
void v20_keybd_magic (vic20_t *sim, pce_key_t key)
{
	switch (key) {
	case PCE_KEY_F9:
		v20_set_msg (sim, "emu.cas.play", "");
		break;

	case PCE_KEY_F10:
		v20_set_msg (sim, "emu.cas.record", "");
		break;

	case PCE_KEY_F11:
		v20_set_msg (sim, "emu.cas.load", "0");
		break;

	case PCE_KEY_F12:
		v20_set_msg (sim, "emu.cas.stop", "");
		break;

	case PCE_KEY_KP_5:
		v20_set_keypad_mode (sim, -1);
		break;

	default:
		sim_log_deb ("unknown magic key (%u)\n", key);
		break;
	}
}

static
int v20_keybd_extra (vic20_t *sim, pce_key_t key)
{
	switch (key) {
	case PCE_KEY_F9:
		v20_set_msg (sim, "emu.cas.play", "");
		return (1);

	case PCE_KEY_F10:
		v20_set_msg (sim, "emu.cas.record", "");
		return (1);

	case PCE_KEY_F11:
		v20_set_msg (sim, "emu.cas.load", "0");
		return (1);

	case PCE_KEY_F12:
		v20_set_msg (sim, "emu.cas.stop", "");
		return (1);

	default:
		break;
	}

	return (0);
}

static
int kbd_joystick (vic20_t *sim, unsigned event, pce_key_t key)
{
	unsigned i;

	i = 0;

	while (joy_map[i].key != PCE_KEY_NONE) {
		if (joy_map[i].key == key) {
			break;
		}

		i += 1;
	}

	if (joy_map[i].key == PCE_KEY_NONE) {
		return (0);
	}

	if (event == PCE_KEY_EVENT_DOWN) {
		sim->joymat |= joy_map[i].a1;
		sim->joymat &= ~joy_map[i].b1;
	}
	else {
		sim->joymat &= ~joy_map[i].a1;
	}

	v20_update_joystick (sim);

	return (1);
}

void v20_keybd_set_key (vic20_t *sim, unsigned event, pce_key_t key)
{
	unsigned      i;
	unsigned      a1, a2;
	unsigned char b1, b2;

	if (event == PCE_KEY_EVENT_MAGIC) {
		v20_keybd_magic (sim, key);
		return;
	}
	else if (event == PCE_KEY_EVENT_DOWN) {
		if (v20_keybd_extra (sim, key)) {
			return;
		}
	}

	if (sim->keypad_joystick) {
		if (kbd_joystick (sim, event, key)) {
			return;
		}
	}

	if ((key == PCE_KEY_PAGEUP) || (key == PCE_KEY_KP_9)) {
		v20_set_restore (sim, event == PCE_KEY_EVENT_DOWN);
		return;
	}

	i = 0;

	a1 = 0;
	b1 = 0;

	a2 = 0;
	b2 = 0;

	while (key_map[i].key != PCE_KEY_NONE) {
		if (key_map[i].key == key) {
			break;
		}

		i += 1;
	}

	if (key_map[i].key == PCE_KEY_NONE) {
		const char *str;

		if (event != PCE_KEY_EVENT_DOWN) {
			return;
		}

		str = pce_key_to_string (key);

		sim_log_deb ("unhandled key: 0x%04x (%s)\n", key, str);

		return;
	}

	a1 = key_map[i].a1;
	b1 = key_map[i].b1;

	a2 = key_map[i].a2;
	b2 = key_map[i].b2;

	if (event == PCE_KEY_EVENT_DOWN) {
		sim->keymat[a1] |= b1;
		sim->keymat[a2] |= b2;
	}
	else if (event == PCE_KEY_EVENT_UP) {
		sim->keymat[a1] &= ~b1;
		sim->keymat[a2] &= ~b2;
	}

	v20_update_keys (sim);
}
