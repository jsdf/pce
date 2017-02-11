/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/adb_keyboard.c                              *
 * Created:     2010-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2017 Hampa Hug <hampa@hampa.ch>                     *
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
#include "adb.h"
#include "adb_keyboard.h"

#include <stdlib.h>
#include <string.h>

#include <lib/log.h>


static adb_kbd_map_t keymap_default[] = {
	{ PCE_KEY_ESC,       0x35, 0 },
	{ PCE_KEY_F1,        0x7a },
	{ PCE_KEY_F2,        0x78 },
	{ PCE_KEY_F3,        0x63 },
	{ PCE_KEY_F4,        0x76 },
	{ PCE_KEY_F5,        0x60 },
	{ PCE_KEY_F6,        0x61 },
	{ PCE_KEY_F7,        0x62 },
	{ PCE_KEY_F8,        0x64 },
	{ PCE_KEY_F9,        0x65 },
	{ PCE_KEY_F10,       0x6d },
	{ PCE_KEY_F11,       0x67 },
	{ PCE_KEY_F12,       0x6f },
	{ PCE_KEY_BACKQUOTE, 0x32 },
	{ PCE_KEY_1,         0x12 },
	{ PCE_KEY_2,         0x13 },
	{ PCE_KEY_3,         0x14 },
	{ PCE_KEY_4,         0x15 },
	{ PCE_KEY_5,         0x17 },
	{ PCE_KEY_6,         0x16 },
	{ PCE_KEY_7,         0x1a },
	{ PCE_KEY_8,         0x1c },
	{ PCE_KEY_9,         0x19 },
	{ PCE_KEY_0,         0x1d },
	{ PCE_KEY_MINUS,     0x1b },
	{ PCE_KEY_EQUAL,     0x18 },
	{ PCE_KEY_BACKSPACE, 0x33 },
	{ PCE_KEY_TAB,       0x30 },
	{ PCE_KEY_Q,         0x0c },
	{ PCE_KEY_W,         0x0d },
	{ PCE_KEY_E,         0x0e },
	{ PCE_KEY_R,         0x0f },
	{ PCE_KEY_T,         0x11 },
	{ PCE_KEY_Y,         0x10 },
	{ PCE_KEY_U,         0x20 },
	{ PCE_KEY_I,         0x22 },
	{ PCE_KEY_O,         0x1f },
	{ PCE_KEY_P,         0x23 },
	{ PCE_KEY_LBRACKET,  0x21 },
	{ PCE_KEY_RBRACKET,  0x1e },
	{ PCE_KEY_BACKSLASH, 0x2a },
	{ PCE_KEY_CAPSLOCK,  0x39 },
	{ PCE_KEY_A,         0x00 },
	{ PCE_KEY_S,         0x01 },
	{ PCE_KEY_D,         0x02 },
	{ PCE_KEY_F,         0x03 },
	{ PCE_KEY_G,         0x05 },
	{ PCE_KEY_H,         0x04 },
	{ PCE_KEY_J,         0x26 },
	{ PCE_KEY_K,         0x28 },
	{ PCE_KEY_L,         0x25 },
	{ PCE_KEY_SEMICOLON, 0x29 },
	{ PCE_KEY_QUOTE,     0x27 },
	{ PCE_KEY_RETURN,    0x24 },
	{ PCE_KEY_LSHIFT,    0x38 },
	{ PCE_KEY_Z,         0x06 },
	{ PCE_KEY_X,         0x07 },
	{ PCE_KEY_C,         0x08 },
	{ PCE_KEY_V,         0x09 },
	{ PCE_KEY_B,         0x0b },
	{ PCE_KEY_N,         0x2d },
	{ PCE_KEY_M,         0x2e },
	{ PCE_KEY_COMMA,     0x2b },
	{ PCE_KEY_PERIOD,    0x2f },
	{ PCE_KEY_SLASH,     0x2c },
	{ PCE_KEY_RSHIFT,    0x38 },
	{ PCE_KEY_LCTRL,     0x36 },
	{ PCE_KEY_LSUPER,    0x3a },
	{ PCE_KEY_MODE,      0x3a },
	{ PCE_KEY_LALT,      0x37 },
	{ PCE_KEY_SPACE,     0x31 },
	{ PCE_KEY_RALT,      0x37 },
	{ PCE_KEY_RSUPER,    0x3a },
	{ PCE_KEY_RCTRL,     0x36 },

	{ PCE_KEY_INS,       0x72 },
	{ PCE_KEY_HOME,      0x73 },
	{ PCE_KEY_PAGEUP,    0x74 },
	{ PCE_KEY_DEL,       0x75 },
	{ PCE_KEY_END,       0x77 },
	{ PCE_KEY_PAGEDN,    0x79 },

	{ PCE_KEY_UP,        0x3e },
	{ PCE_KEY_LEFT,      0x3b },
	{ PCE_KEY_RIGHT,     0x3c },
	{ PCE_KEY_DOWN,      0x3d },

	{ PCE_KEY_NUMLOCK,   0x47 },
	{ PCE_KEY_KP_SLASH,  0x4b },
	{ PCE_KEY_KP_STAR,   0x43 },
	{ PCE_KEY_KP_7,      0x59 },
	{ PCE_KEY_KP_8,      0x5b },
	{ PCE_KEY_KP_9,      0x5c },
	{ PCE_KEY_KP_MINUS,  0x4e },
	{ PCE_KEY_KP_4,      0x56 },
	{ PCE_KEY_KP_5,      0x57 },
	{ PCE_KEY_KP_6,      0x58 },
	{ PCE_KEY_KP_PLUS,   0x45 },
	{ PCE_KEY_KP_1,      0x53 },
	{ PCE_KEY_KP_2,      0x54 },
	{ PCE_KEY_KP_3,      0x55 },
	{ PCE_KEY_KP_ENTER,  0x4c },
	{ PCE_KEY_KP_0,      0x52 },
	{ PCE_KEY_KP_PERIOD, 0x41 },

	{ PCE_KEY_NONE,      0 }
};

/* keypad to motion keys */
static adb_kbd_map_t keymap_motion[] = {
	{ PCE_KEY_KP_7,      0x73 },
	{ PCE_KEY_KP_8,      0x3e },
	{ PCE_KEY_KP_9,      0x74 },
	{ PCE_KEY_KP_4,      0x3b },
	{ PCE_KEY_KP_5,      0x57 },
	{ PCE_KEY_KP_6,      0x3c },
	{ PCE_KEY_KP_1,      0x77 },
	{ PCE_KEY_KP_2,      0x3d },
	{ PCE_KEY_KP_3,      0x79 },
	{ PCE_KEY_KP_0,      0x72 },
	{ PCE_KEY_KP_PERIOD, 0x75 },
	{ PCE_KEY_KP_ENTER,  0x24 },
	{ PCE_KEY_NONE,      0 }
};

/* keypad to keypad */
static adb_kbd_map_t keymap_keypad[] = {
	{ PCE_KEY_KP_7,      0x59 },
	{ PCE_KEY_KP_8,      0x5b },
	{ PCE_KEY_KP_9,      0x5c },
	{ PCE_KEY_KP_4,      0x56 },
	{ PCE_KEY_KP_5,      0x57 },
	{ PCE_KEY_KP_6,      0x58 },
	{ PCE_KEY_KP_1,      0x53 },
	{ PCE_KEY_KP_2,      0x54 },
	{ PCE_KEY_KP_3,      0x55 },
	{ PCE_KEY_KP_0,      0x52 },
	{ PCE_KEY_KP_PERIOD, 0x41 },
	{ PCE_KEY_KP_ENTER,  0x4c },
	{ PCE_KEY_NONE,      0 }
};


static
adb_kbd_map_t *adb_kbd_clone_map (const adb_kbd_map_t *src)
{
	unsigned      n;
	adb_kbd_map_t *dst;

	n = 0;
	while (src[n].key != PCE_KEY_NONE) {
		n += 1;
	}

	n += 1;

	dst = malloc (n * sizeof (adb_kbd_map_t));

	if (dst == NULL) {
		return (NULL);
	}

	memcpy (dst, src, n * sizeof (adb_kbd_map_t));

	return (dst);
}

static
void adb_kbd_fix_map (adb_kbd_map_t *dst, const adb_kbd_map_t *src)
{
	adb_kbd_map_t *tmp;

	while (src->key != PCE_KEY_NONE) {
		tmp = dst;
		while (tmp->key != PCE_KEY_NONE) {
			if (tmp->key == src->key) {
				*tmp = *src;
			}

			tmp += 1;
		}

		src += 1;
	}
}


static
void adb_kbd_add_buf (adb_kbd_t *kbd, unsigned char code)
{
	unsigned k;

	k = (kbd->buf_j + 1) & (ADB_KBD_BUF - 1);

	if (k == kbd->buf_i) {
		return;
	}

	kbd->buf[kbd->buf_j] = code;
	kbd->buf_j = k;
}

static
int adb_kbd_get_buf (adb_kbd_t *kbd, unsigned char *code)
{
	if (kbd->buf_i == kbd->buf_j) {
		*code = 0;
		return (1);
	}

	*code = kbd->buf[kbd->buf_i];

	kbd->buf_i = (kbd->buf_i + 1) & (ADB_KBD_BUF - 1);

	return (0);
}

static
void adb_kbd_set_service_request (adb_kbd_t *kbd)
{
	kbd->dev.service_request = (kbd->buf_i != kbd->buf_j);
}


static
void adb_kbd_del (adb_dev_t *dev)
{
	adb_kbd_t *kbd;

	kbd = dev->ext;

	free (kbd->keymap);
	free (kbd);
}

static
void adb_kbd_reset (adb_dev_t *dev)
{
	adb_kbd_t *kbd;

	kbd = dev->ext;

	adb_dev_reset (dev);

	kbd->talking = 0;

	kbd->buf_i = 0;
	kbd->buf_j = 0;
}

static
void adb_kbd_flush (adb_dev_t *dev)
{
	adb_kbd_t *kbd;

	kbd = dev->ext;

	adb_dev_flush (dev);

	kbd->talking = 0;

	kbd->buf_i = 0;
	kbd->buf_j = 0;
}

static
unsigned adb_kbd_talk_0 (adb_dev_t *dev, void *buf)
{
	unsigned char c1, c2;
	unsigned char *dst;
	adb_kbd_t     *kbd;

	kbd = dev->ext;
	dst = buf;

	if (kbd->talking == 0) {
		if (adb_kbd_get_buf (kbd, &c1)) {
			return (0);
		}

		if (adb_kbd_get_buf (kbd, &c2)) {
			c2 = 0xff;
		}

		dev->reg[0] = (c1 << 8) | c2;
	}

	kbd->talking = 1;

	dst[0] = (dev->reg[0] >> 8) & 0xff;
	dst[1] = dev->reg[0] & 0xff;

	return (2);
}

static
unsigned adb_kbd_talk_2 (adb_dev_t *dev, void *buf)
{
	unsigned char *dst;

	dst = buf;

	dst[0] = 0xff;
	dst[1] = 0xff;

	mac_log_deb ("adb-kbd: talk 2\n");

	return (2);
}

static
unsigned adb_kbd_talk (adb_dev_t *dev, unsigned reg, void *buf)
{
	if (reg == 0) {
		return (adb_kbd_talk_0 (dev, buf));
	}
	else if (reg == 2) {
		return (adb_kbd_talk_2 (dev, buf));
	}
	else if (reg == 3) {
		;
	}
	else {
		mac_log_deb ("adb-kbd: talk %u\n", reg);
	}

	return (adb_dev_talk (dev, reg, buf));
}

static
void adb_kbd_talk_done (adb_dev_t *dev, unsigned reg)
{
	adb_kbd_t *kbd;

	kbd = dev->ext;

	if (reg == 0) {
		kbd->talking = 0;
		adb_kbd_set_service_request (kbd);
	}
}

static
void adb_kbd_listen (adb_dev_t *dev, unsigned reg, const void *buf, unsigned cnt)
{
	const unsigned char *src;

	src = buf;

	if (reg != 3) {
		mac_log_deb ("adb-kbd: listen %u\n", reg);
	}

	if ((reg == 2) && (cnt == 2)) {
		dev->reg[2] = (src[0] << 8) | src[1];
	}
	else {
		adb_dev_listen (dev, reg, buf, cnt);
	}
}

void adb_kbd_set_keypad_mode (adb_kbd_t *kbd, int motion)
{
	if (motion) {
		adb_kbd_fix_map (kbd->keymap, keymap_motion);
		kbd->keypad_motion_mode = 1;
	}
	else {
		adb_kbd_fix_map (kbd->keymap, keymap_keypad);
		kbd->keypad_motion_mode = 0;
	}
}

int adb_kbd_get_keypad_mode (const adb_kbd_t *kbd)
{
	return (kbd->keypad_motion_mode);
}

void adb_kbd_set_key (adb_kbd_t *kbd, unsigned event, pce_key_t key)
{
	adb_kbd_map_t *map;

	map = kbd->keymap;

	while (map->key != PCE_KEY_NONE) {
		if (map->key == key) {
			break;
		}

		map += 1;
	}

	if (map->key == PCE_KEY_NONE) {
		const char *str;

		str = pce_key_to_string (key);

		pce_log (MSG_INF, "ignoring pce key: 0x%04x (%s)\n",
			(unsigned) key,
			(str != NULL) ? str : "<none>"
		);

		return;
	}

	switch (event) {
	case PCE_KEY_EVENT_DOWN:
		adb_kbd_add_buf (kbd, map->code);
		break;

	case PCE_KEY_EVENT_UP:
		adb_kbd_add_buf (kbd, map->code | 0x80);
		break;
	}

	adb_kbd_set_service_request (kbd);
}

void adb_kbd_init (adb_kbd_t *kbd)
{
	adb_dev_init (&kbd->dev, 2, 2);

	kbd->dev.ext = kbd;

	kbd->dev.del = adb_kbd_del;

	kbd->dev.reset = adb_kbd_reset;
	kbd->dev.flush = adb_kbd_flush;
	kbd->dev.talk = adb_kbd_talk;
	kbd->dev.talk_done = adb_kbd_talk_done;
	kbd->dev.listen = adb_kbd_listen;

	kbd->buf_i = 0;
	kbd->buf_j = 0;

	kbd->keypad_motion_mode = 0;

	kbd->keymap = adb_kbd_clone_map (keymap_default);
}

adb_kbd_t *adb_kbd_new (void)
{
	adb_kbd_t *kbd;

	kbd = malloc (sizeof (adb_kbd_t));

	if (kbd == NULL) {
		return (NULL);
	}

	adb_kbd_init (kbd);

	return (kbd);
}
