/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/keyboard.c                                 *
 * Created:     2022-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "keyboard.h"

#include <stdlib.h>
#include <string.h>

#include <drivers/video/keys.h>


struct spec_key_s {
	pce_key_t     key;
	unsigned char idx1;
	unsigned char msk1;
	unsigned char idx2;
	unsigned char msk2;
};


struct spec_key_s spec_keys[] = {
	{ PCE_KEY_LSHIFT,	0, 0x01 },
	{ PCE_KEY_RSHIFT,	0, 0x01 },
	{ PCE_KEY_Z,		0, 0x02 },
	{ PCE_KEY_X,		0, 0x04 },
	{ PCE_KEY_C,		0, 0x08 },
	{ PCE_KEY_V,		0, 0x10 },

	{ PCE_KEY_A,		1, 0x01 },
	{ PCE_KEY_S,		1, 0x02 },
	{ PCE_KEY_D,		1, 0x04 },
	{ PCE_KEY_F,		1, 0x08 },
	{ PCE_KEY_G,		1, 0x10 },

	{ PCE_KEY_Q,		2, 0x01 },
	{ PCE_KEY_W,		2, 0x02 },
	{ PCE_KEY_E,		2, 0x04 },
	{ PCE_KEY_R,		2, 0x08 },
	{ PCE_KEY_T,		2, 0x10 },

	{ PCE_KEY_1,		3, 0x01 },
	{ PCE_KEY_2,		3, 0x02 },
	{ PCE_KEY_3,		3, 0x04 },
	{ PCE_KEY_4,		3, 0x08 },
	{ PCE_KEY_5,		3, 0x10 },

	{ PCE_KEY_0,		4, 0x01 },
	{ PCE_KEY_9,		4, 0x02 },
	{ PCE_KEY_8,		4, 0x04 },
	{ PCE_KEY_7,		4, 0x08 },
	{ PCE_KEY_6,		4, 0x10 },

	{ PCE_KEY_P,		5, 0x01 },
	{ PCE_KEY_O,		5, 0x02 },
	{ PCE_KEY_I,		5, 0x04 },
	{ PCE_KEY_U,		5, 0x08 },
	{ PCE_KEY_Y,		5, 0x10 },

	{ PCE_KEY_RETURN,	6, 0x01 },
	{ PCE_KEY_KP_ENTER,	6, 0x01 },
	{ PCE_KEY_L,		6, 0x02 },
	{ PCE_KEY_K,		6, 0x04 },
	{ PCE_KEY_J,		6, 0x08 },
	{ PCE_KEY_H,		6, 0x10 },

	{ PCE_KEY_SPACE,	7, 0x01 },
	{ PCE_KEY_LCTRL,	7, 0x02 },
	{ PCE_KEY_M,		7, 0x04 },
	{ PCE_KEY_N,		7, 0x08 },
	{ PCE_KEY_B,		7, 0x10 },

	{ PCE_KEY_NONE }
};

struct spec_key_s spec_keys_ext[] = {
	{ PCE_KEY_BACKQUOTE,	7, 0x02, 0, 0x04 },
	{ PCE_KEY_MINUS,	7, 0x02, 6, 0x08 },
	{ PCE_KEY_EQUAL,	7, 0x02, 6, 0x02 },
	{ PCE_KEY_BACKSPACE,	0, 0x01, 4, 0x01 },
	{ PCE_KEY_SEMICOLON,	7, 0x02, 5, 0x02 },
	{ PCE_KEY_QUOTE,	7, 0x02, 4, 0x08 },
	{ PCE_KEY_COMMA,	7, 0x02, 7, 0x08 },
	{ PCE_KEY_PERIOD,	7, 0x02, 7, 0x04 },
	{ PCE_KEY_SLASH,	7, 0x02, 0, 0x10 },
	{ PCE_KEY_KP_PLUS,	7, 0x02, 6, 0x04 },
	{ PCE_KEY_KP_MINUS,	7, 0x02, 6, 0x08 },
	{ PCE_KEY_KP_STAR,	7, 0x02, 7, 0x10 },
	{ PCE_KEY_NONE }
};

struct spec_key_s spec_keys_alt[] = {
	{ PCE_KEY_1,		7, 0x02, 3, 0x01 },
	{ PCE_KEY_2,		7, 0x02, 3, 0x02 },
	{ PCE_KEY_3,		7, 0x02, 3, 0x04 },
	{ PCE_KEY_4,		7, 0x02, 3, 0x08 },
	{ PCE_KEY_5,		7, 0x02, 3, 0x10 },
	{ PCE_KEY_6,		7, 0x02, 6, 0x10 },
	{ PCE_KEY_7,		7, 0x02, 4, 0x10 },
	{ PCE_KEY_8,		7, 0x02, 7, 0x10 },
	{ PCE_KEY_9,		7, 0x02, 4, 0x04 },
	{ PCE_KEY_0,		7, 0x02, 4, 0x02 },

	{ PCE_KEY_MINUS,	7, 0x02, 4, 0x01 },
	{ PCE_KEY_EQUAL,	7, 0x02, 6, 0x04 },
	{ PCE_KEY_SEMICOLON,	7, 0x02, 0, 0x02 },
	{ PCE_KEY_QUOTE,	7, 0x02, 5, 0x01 },
	{ PCE_KEY_COMMA,	7, 0x02, 2, 0x08 },
	{ PCE_KEY_PERIOD,	7, 0x02, 2, 0x10 },
	{ PCE_KEY_SLASH,	7, 0x02, 0, 0x08 },

	{ PCE_KEY_NONE }
};

struct spec_key_s spec_cursor[] = {
	{ PCE_KEY_KP_4,		3, 0x10 },
	{ PCE_KEY_KP_6,		4, 0x04 },
	{ PCE_KEY_KP_2,		4, 0x10 },
	{ PCE_KEY_KP_8,		4, 0x08 },

	{ PCE_KEY_KP_1,		4, 0x10, 3, 0x10 },
	{ PCE_KEY_KP_3,		4, 0x10, 4, 0x04 },
	{ PCE_KEY_KP_7,		3, 0x10, 4, 0x08 },
	{ PCE_KEY_KP_9,		4, 0x04, 4, 0x08 },

	{ PCE_KEY_LEFT,		3, 0x10 },
	{ PCE_KEY_RIGHT,	4, 0x04 },
	{ PCE_KEY_DOWN,		4, 0x10 },
	{ PCE_KEY_UP,		4, 0x08 },

	{ PCE_KEY_KP_0,		4, 0x01 },
	{ PCE_KEY_TAB,		4, 0x01 },

	{ PCE_KEY_NONE }
};

struct spec_key_s spec_joy_1[] = {
	{ PCE_KEY_KP_4,		4, 0x10 },
	{ PCE_KEY_KP_6,		4, 0x08 },
	{ PCE_KEY_KP_2,		4, 0x04 },
	{ PCE_KEY_KP_8,		4, 0x02 },

	{ PCE_KEY_KP_1,		4, 0x04, 4, 0x10 },
	{ PCE_KEY_KP_3,		4, 0x04, 4, 0x08 },
	{ PCE_KEY_KP_7,		4, 0x10, 4, 0x02 },
	{ PCE_KEY_KP_9,		4, 0x08, 4, 0x02 },

	{ PCE_KEY_LEFT,		4, 0x10 },
	{ PCE_KEY_RIGHT,	4, 0x08 },
	{ PCE_KEY_DOWN,		4, 0x04 },
	{ PCE_KEY_UP,		4, 0x02 },

	{ PCE_KEY_KP_0,		4, 0x01 },
	{ PCE_KEY_TAB,		4, 0x01 },

	{ PCE_KEY_NONE }
};

struct spec_key_s spec_joy_2[] = {
	{ PCE_KEY_KP_4,		3, 0x01 },
	{ PCE_KEY_KP_6,		3, 0x02 },
	{ PCE_KEY_KP_2,		3, 0x04 },
	{ PCE_KEY_KP_8,		3, 0x08 },

	{ PCE_KEY_KP_1,		3, 0x04, 3, 0x01 },
	{ PCE_KEY_KP_3,		3, 0x04, 3, 0x02 },
	{ PCE_KEY_KP_7,		3, 0x01, 3, 0x08 },
	{ PCE_KEY_KP_9,		3, 0x02, 3, 0x08 },

	{ PCE_KEY_LEFT,		3, 0x01 },
	{ PCE_KEY_RIGHT,	3, 0x02 },
	{ PCE_KEY_DOWN,		3, 0x04 },
	{ PCE_KEY_UP,		3, 0x08 },

	{ PCE_KEY_KP_0,		3, 0x10 },
	{ PCE_KEY_TAB,		3, 0x10 },

	{ PCE_KEY_NONE }
};


void spec_kbd_reset (spectrum_t *sim)
{
	unsigned i;

	for (i = 0; i < 8; i++) {
		sim->keys[i] = 0;
	}
}

void spec_kbd_set_cursor (spectrum_t *sim)
{
	sim->keypad_mode = 0;
	sim_log_deb ("keypad cursor\n");
}

void spec_kbd_set_joy_1 (spectrum_t *sim)
{
	sim->keypad_mode = 1;
	sim_log_deb ("keypad joystick 1\n");
}

void spec_kbd_set_joy_2 (spectrum_t *sim)
{
	sim->keypad_mode = 2;
	sim_log_deb ("keypad joystick 2\n");
}

void spec_kbd_set_kempston (spectrum_t *sim)
{
	sim->keypad_mode = 3;
	sim_log_deb ("keypad kempston joystick\n");
}

void spec_kbd_set_keypad_mode (spectrum_t *sim, unsigned mode)
{
	switch (mode) {
	case 0:
		spec_kbd_set_cursor (sim);
		break;

	case 1:
		spec_kbd_set_joy_1 (sim);
		break;

	case 2:
		spec_kbd_set_joy_2 (sim);
		break;

	default:
		sim_log_deb ("unknown keypad mode (%u)\n", mode);
		sim->keypad_mode = mode;
		break;
	}
}

unsigned spec_kbd_get_keys (spectrum_t *sim, unsigned val)
{
	unsigned      i;
	unsigned char ret;

	ret = 0;

	for (i = 0; i < 8; i++) {
		if (val & (1 << i)) {
			ret |= sim->keys[i];
		}
	}

	return (ret);
}

static
int kbd_set_key_kempston (spectrum_t *sim, int press, pce_key_t key)
{
	unsigned msk;

	switch (key) {
	case PCE_KEY_TAB:
		msk = 0x10;
		break;

	case PCE_KEY_KP_0:
		msk = 0x10;
		break;

	case PCE_KEY_KP_1:
		msk = 0x06;
		break;

	case PCE_KEY_KP_2:
		msk = 0x04;
		break;

	case PCE_KEY_KP_3:
		msk = 0x05;
		break;

	case PCE_KEY_KP_4:
		msk = 0x02;
		break;

	case PCE_KEY_KP_6:
		msk = 0x01;
		break;

	case PCE_KEY_KP_7:
		msk = 0x0a;
		break;

	case PCE_KEY_KP_8:
		msk = 0x08;
		break;

	case PCE_KEY_KP_9:
		msk = 0x09;
		break;

	default:
		return (0);
	}

	if (press) {
		sim->joystate |= msk;
	}
	else {
		sim->joystate &= ~msk;
	}

	return (1);
}

static
int kbd_set_key_lst (spectrum_t *sim, struct spec_key_s *lst, int press, pce_key_t key)
{
	int ret;

	ret = 0;

	while (lst->key != PCE_KEY_NONE) {
		if (lst->key == key) {
			if (press) {
				sim->keys[lst->idx1] |= lst->msk1;
				sim->keys[lst->idx2] |= lst->msk2;
			}
			else {
				sim->keys[lst->idx1] &= ~lst->msk1;
				sim->keys[lst->idx2] &= ~lst->msk2;
			}

			ret = 1;
		}

		lst += 1;
	}

	return (ret);
}

void spec_kbd_set_key (spectrum_t *sim, int press, pce_key_t key)
{
	int        r;
	const char *str;

	if (key == PCE_KEY_LALT) {
		sim->kbd_alt = (press != 0);
		return;
	}

	r = 0;

	if (sim->kbd_alt) {
		r |= kbd_set_key_lst (sim, spec_keys_alt, press, key);
	}
	else {
		r |= kbd_set_key_lst (sim, spec_keys, press, key);
		r |= kbd_set_key_lst (sim, spec_keys_ext, press, key);
	}

	if (sim->keypad_mode == 0) {
		r |= kbd_set_key_lst (sim, spec_cursor, press, key);
	}
	else if (sim->keypad_mode == 1) {
		r |= kbd_set_key_lst (sim, spec_joy_1, press, key);
	}
	else if (sim->keypad_mode == 2) {
		r |= kbd_set_key_lst (sim, spec_joy_2, press, key);
	}
	else if (sim->keypad_mode == 3) {
		r |= kbd_set_key_kempston (sim, press, key);
	}

	if (r == 0) {
		str = pce_key_to_string (key);

		if (str == NULL) {
			str = "<unknown>";
		}

		sim_log_deb ("unhandled key (%u, %s)\n", key, str);
	}
}
