/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/ikbd.c                                      *
 * Created:     2013-06-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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
#include "ikbd.h"

#include <drivers/video/keys.h>
#include <lib/log.h>

#include <string.h>
#include <time.h>

#ifndef DEBUG_KBD
#define DEBUG_KBD 0
#endif


typedef struct {
	pce_key_t      pcekey;

	unsigned short down_cnt;
	unsigned char  down[4];

	unsigned short up_cnt;
	unsigned char  up[4];

	char           isdown;
} st_keymap_t;


typedef struct {
	pce_key_t     key;
	unsigned char msk;
} st_joymap_t;


static st_keymap_t keymap[] = {
	{ PCE_KEY_ESC,       1, { 0x01 }, 1, { 0x81 }, 0 },
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
	{ PCE_KEY_TAB,       1, { 0x0f }, 1, { 0x8f }, 0 },
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
	{ PCE_KEY_LBRACKET,  1, { 0x1a }, 1, { 0x9a }, 0 },
	{ PCE_KEY_RBRACKET,  1, { 0x1b }, 1, { 0x9b }, 0 },
	{ PCE_KEY_RETURN,    1, { 0x1c }, 1, { 0x9c }, 0 },
	{ PCE_KEY_LCTRL,     1, { 0x1d }, 1, { 0x9d }, 0 },
	{ PCE_KEY_RCTRL,     1, { 0x1d }, 1, { 0x9d }, 0 },
	{ PCE_KEY_A,         1, { 0x1e }, 1, { 0x9e }, 0 },
	{ PCE_KEY_S,         1, { 0x1f }, 1, { 0x9f }, 0 },
	{ PCE_KEY_D,         1, { 0x20 }, 1, { 0xa0 }, 0 },
	{ PCE_KEY_F,         1, { 0x21 }, 1, { 0xa1 }, 0 },
	{ PCE_KEY_G,         1, { 0x22 }, 1, { 0xa2 }, 0 },
	{ PCE_KEY_H,         1, { 0x23 }, 1, { 0xa3 }, 0 },
	{ PCE_KEY_J,         1, { 0x24 }, 1, { 0xa4 }, 0 },
	{ PCE_KEY_K,         1, { 0x25 }, 1, { 0xa5 }, 0 },
	{ PCE_KEY_L,         1, { 0x26 }, 1, { 0xa6 }, 0 },
	{ PCE_KEY_SEMICOLON, 1, { 0x27 }, 1, { 0xa7 }, 0 },
	{ PCE_KEY_QUOTE,     1, { 0x28 }, 1, { 0xa8 }, 0 },
	{ PCE_KEY_BACKQUOTE, 1, { 0x29 }, 1, { 0xa9 }, 0 },
	{ PCE_KEY_LSHIFT,    1, { 0x2a }, 1, { 0xaa }, 0 },
	{ PCE_KEY_BACKSLASH, 1, { 0x2b }, 1, { 0xab }, 0 },
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
	{ PCE_KEY_LALT,      1, { 0x38 }, 1, { 0xb8 }, 0 },
	{ PCE_KEY_SPACE,     1, { 0x39 }, 1, { 0xb9 }, 0 },
	{ PCE_KEY_CAPSLOCK,  1, { 0x3a }, 1, { 0xba }, 0 },
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
/*	{ PCE_KEY_NUMLOCK,   1, { 0x45 }, 1, { 0xc5 }, 0 }, */
/*	{ PCE_KEY_SCRLK,     1, { 0x46 }, 1, { 0xc6 }, 0 }, */

	{ PCE_KEY_HOME,      1, { 0x47 }, 1, { 0xc7 }, 0 },
	{ PCE_KEY_UP,        1, { 0x48 }, 1, { 0xc8 }, 0 },
	{ PCE_KEY_KP_MINUS,  1, { 0x4a }, 1, { 0xca }, 0 },
	{ PCE_KEY_LEFT,      1, { 0x4b }, 1, { 0xcb }, 0 },
	{ PCE_KEY_RIGHT,     1, { 0x4d }, 1, { 0xcd }, 0 },
	{ PCE_KEY_KP_PLUS,   1, { 0x4e }, 1, { 0xce }, 0 },
	{ PCE_KEY_DOWN,      1, { 0x50 }, 1, { 0xd0 }, 0 },
	{ PCE_KEY_INS,       1, { 0x52 }, 1, { 0xd2 }, 0 },
	{ PCE_KEY_DEL,       1, { 0x53 }, 1, { 0xd3 }, 0 },
	{ PCE_KEY_LESS,      1, { 0x60 }, 1, { 0xe0 }, 0 },
	{ PCE_KEY_PAGEDN,    1, { 0x61 }, 1, { 0xe1 }, 0 },
	{ PCE_KEY_END,       1, { 0x62 }, 1, { 0xe2 }, 0 },

	{ PCE_KEY_KP_SLASH,  1, { 0x65 }, 1, { 0xe5 }, 0 },
	{ PCE_KEY_KP_STAR,   1, { 0x66 }, 1, { 0xe6 }, 0 },
	{ PCE_KEY_KP_7,      1, { 0x67 }, 1, { 0xe7 }, 0 },
	{ PCE_KEY_KP_8,      1, { 0x68 }, 1, { 0xe8 }, 0 },
	{ PCE_KEY_KP_9,      1, { 0x69 }, 1, { 0xe9 }, 0 },
	{ PCE_KEY_KP_4,      1, { 0x6a }, 1, { 0xea }, 0 },
	{ PCE_KEY_KP_5,      1, { 0x6b }, 1, { 0xeb }, 0 },
	{ PCE_KEY_KP_6,      1, { 0x6c }, 1, { 0xec }, 0 },
	{ PCE_KEY_KP_1,      1, { 0x6d }, 1, { 0xed }, 0 },
	{ PCE_KEY_KP_2,      1, { 0x6e }, 1, { 0xee }, 0 },
	{ PCE_KEY_KP_3,      1, { 0x6f }, 1, { 0xef }, 0 },
	{ PCE_KEY_KP_0,      1, { 0x70 }, 1, { 0xf0 }, 0 },
	{ PCE_KEY_KP_PERIOD, 1, { 0x71 }, 1, { 0xf1 }, 0 },
	{ PCE_KEY_KP_ENTER,  1, { 0x72 }, 1, { 0xf2 }, 0 },

	{ PCE_KEY_NONE,      0, { 0x00 }, 0, { 0x00 }, 0 }
};

static st_joymap_t joymap[] = {
	{ PCE_KEY_KP_7, 0x05 },
	{ PCE_KEY_KP_8, 0x01 },
	{ PCE_KEY_KP_9, 0x09 },
	{ PCE_KEY_KP_4, 0x04 },
	{ PCE_KEY_KP_5, 0x80 },
	{ PCE_KEY_KP_6, 0x08 },
	{ PCE_KEY_KP_1, 0x06 },
	{ PCE_KEY_KP_2, 0x02 },
	{ PCE_KEY_KP_3, 0x0a },
	{ PCE_KEY_KP_0, 0x80 },
	{ PCE_KEY_NONE, 0 }
};


void st_kbd_init (st_kbd_t *kbd)
{
	kbd->idle = 1;

	kbd->cmd_cnt = 0;

	kbd->paused = 0;
	kbd->disabled = 0;
	kbd->joy_report = 1;

	kbd->mouse_dx = 0;
	kbd->mouse_dy = 0;
	kbd->mouse_but[0] = 0;
	kbd->mouse_but[1] = 0;

	kbd->joy[0] = 0;
	kbd->joy[1] = 0;
	kbd->keypad_joy = 0;

	kbd->buf_hd = 0;
	kbd->buf_tl = 0;
}

int st_kbd_buf_put (st_kbd_t *kbd, unsigned char val)
{
	unsigned i;

	i = kbd->buf_hd + 1;

	if (i >= sizeof (kbd->buf)) {
		i = 0;
	}

	if (i == kbd->buf_tl) {
		st_log_deb ("ikbd: buffer overflow\n");
		return (1);
	}

	kbd->buf[kbd->buf_hd] = val;
	kbd->buf_hd = i;

	return (0);
}

int st_kbd_buf_get (st_kbd_t *kbd, unsigned char *val)
{
	if (kbd->buf_hd == kbd->buf_tl) {
		return (1);
	}

	*val = kbd->buf[kbd->buf_tl];

	kbd->buf_tl += 1;

	if (kbd->buf_tl >= sizeof (kbd->buf)) {
		kbd->buf_tl = 0;
	}

	return (0);
}

static
unsigned char st_kbd_get_int (int *val)
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
void st_kbd_check_mouse (st_kbd_t *kbd)
{
	unsigned char val;

	if (kbd->disabled || kbd->abs_pos) {
		return;
	}

	if ((kbd->mouse_dx == 0) && (kbd->mouse_dy == 0) && (kbd->mouse_but[0] == kbd->mouse_but[1])) {
		return;
	}

	kbd->mouse_but[0] = kbd->mouse_but[1];

	val = 0xf8;

	if (kbd->mouse_but[1] & 1) {
		val |= 2;
	}

	if (kbd->mouse_but[1] & 2) {
		val |= 1;
	}

	st_kbd_buf_put (kbd, val);
	st_kbd_buf_put (kbd, st_kbd_get_int (&kbd->mouse_dx));
	st_kbd_buf_put (kbd, st_kbd_get_int (&kbd->mouse_dy));
}

void st_kbd_set_mouse (st_kbd_t *kbd, int dx, int dy, unsigned but)
{
	unsigned tx, ty;

	if (kbd->abs_pos) {
		if (dx < 0) {
			tx = -dx;

			if (tx > kbd->cur_x) {
				tx = kbd->cur_x;
			}

			kbd->cur_x -= tx;
		}
		else {
			tx = dx;

			if (tx > (kbd->max_x - kbd->cur_x)) {
				tx = kbd->max_x - kbd->cur_x;
			}

			kbd->cur_x += tx;
		}

		if (dy < 0) {
			ty = -dy;

			if (ty > kbd->cur_y) {
				ty = kbd->cur_y;
			}

			kbd->cur_y -= ty;
		}
		else {
			ty = dy;

			if (ty > (kbd->max_y - kbd->cur_y)) {
				ty = kbd->max_y - kbd->cur_y;
			}

			kbd->cur_y += ty;
		}

		if ((kbd->mouse_but[1] ^ but) & 1) {
			if (but & 1) {
				kbd->button_delta |= 4;
			}
			else {
				kbd->button_delta |= 8;
			}
		}

		if ((kbd->mouse_but[1] ^ but) & 2) {
			if (but & 2) {
				kbd->button_delta |= 1;
			}
			else {
				kbd->button_delta |= 2;
			}
		}

		kbd->mouse_but[1] = but;
	}
	else {
		kbd->mouse_dx += dx;
		kbd->mouse_dy += kbd->y0_at_top ? dy : -dy;
		kbd->mouse_but[1] = but;

		st_kbd_check_mouse (kbd);
	}
}

static
int st_kbd_set_joy (st_kbd_t *kbd, unsigned idx, unsigned event, pce_key_t key)
{
	unsigned char val;
	st_joymap_t   *map;

	if ((event != PCE_KEY_EVENT_DOWN) && (event != PCE_KEY_EVENT_UP)) {
		return (1);
	}

	idx &= 1;

	map = joymap;

	while (map->key != PCE_KEY_NONE) {
		if (map->key == key) {
			break;
		}

		map += 1;
	}

	if (map->key == PCE_KEY_NONE) {
		return (1);
	}

	if (kbd->joy_report == 0) {
		return (0);
	}

	val = kbd->joy[idx];

	if (event == PCE_KEY_EVENT_DOWN) {
		val |= map->msk;
	}
	else {
		val &= ~map->msk;
	}

	if (kbd->joy[idx] == val) {
		return (0);
	}

	kbd->joy[idx] = val;

	st_kbd_buf_put (kbd, 0xfe | (idx & 1));
	st_kbd_buf_put (kbd, val);

	return (0);
}

/*
 * write a key code sequence into the key buffer
 */
static
void st_kbd_set_sequence (st_kbd_t *kbd, unsigned char *buf, unsigned cnt)
{
	unsigned i;

	for (i = 0; i < cnt; i++) {
		if (st_kbd_buf_put (kbd, buf[i])) {
			return;
		}
	}
}

void st_kbd_set_key (st_kbd_t *kbd, unsigned event, pce_key_t key)
{
	st_keymap_t *map;

	if (event == PCE_KEY_EVENT_MAGIC) {
		if (key == PCE_KEY_KP_5) {
			if (++kbd->keypad_joy > 2) {
				kbd->keypad_joy = 0;
				st_log_deb ("keypad: keyboard\n");
			}
			else {
				st_log_deb ("keypad: joystick %u\n",
					(kbd->keypad_joy & 1) + 1
				);
			}
		}
		else {
			pce_log (MSG_INF, "unhandled magic key (%u)\n",
				(unsigned) key
			);
		}

		return;
	}

	if (kbd->keypad_joy > 0) {
		if (st_kbd_set_joy (kbd, kbd->keypad_joy & 1, event, key) == 0) {
			return;
		}
	}

	map = keymap;

	while (map->pcekey != PCE_KEY_NONE) {
		if (map->pcekey == key) {
			break;
		}

		map += 1;
	}

	if (map->pcekey == PCE_KEY_NONE) {
		if (event == PCE_KEY_EVENT_DOWN) {
			pce_log (MSG_INF, "unknown key code (%u)\n", key);
		}
		return;
	}

	switch (event) {
	case PCE_KEY_EVENT_DOWN:
		map->isdown = 1;
		st_kbd_set_sequence (kbd, map->down, map->down_cnt);
		break;

	case PCE_KEY_EVENT_UP:
		if (map->isdown) {
			map->isdown = 0;
			st_kbd_set_sequence (kbd, map->up, map->up_cnt);
		}
		break;
	}
}

static
unsigned get_bcd_8 (unsigned n)
{
	return ((n % 10) + 16 * (n / 10));
}

/*
 * 07: SET MOUSE BUTTON ACTION
 */
static
void st_kbd_cmd_07 (st_kbd_t *kbd)
{
	if (kbd->cmd_cnt < 2) {
		return;
	}

#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET MOUSE BUTTON ACTION: %02X\n", kbd->cmd[1]);
#endif

	kbd->cmd_cnt = 0;
}

/*
 * 08: SET RELATIVE MOUSE POSITION REPORTING
 */
static
void st_kbd_cmd_08 (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET RELATIVE MOUSE POSITION REPORTING\n");
#endif

	kbd->abs_pos = 0;
	kbd->disabled = 0;

	kbd->cmd_cnt = 0;
}

/*
 * 09: SET ABSOLUTE MOUSE POSITION REPORTING
 */
static
void st_kbd_cmd_09 (st_kbd_t *kbd)
{
	if (kbd->cmd_cnt < 5) {
		return;
	}

	kbd->max_x = (kbd->cmd[1] << 8) | kbd->cmd[2];
	kbd->max_y = (kbd->cmd[3] << 8) | kbd->cmd[4];

#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET ABSOLUTE MOUSE POSITION REPORTING (%u / %u)\n",
		kbd->max_x, kbd->max_y
	);
#endif

	kbd->abs_pos = 1;
	kbd->disabled = 0;

	kbd->cur_x = 0;
	kbd->cur_y = 0;
	kbd->button_delta = 0;

	kbd->cmd_cnt = 0;
}

/*
 * 0B: SET MOUSE THRESHOLD
 */
static
void st_kbd_cmd_0b (st_kbd_t *kbd)
{
	if (kbd->cmd_cnt < 3) {
		return;
	}

#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET MOUSE THRESHOLD: %u / %u\n", kbd->cmd[1], kbd->cmd[2]);
#endif

	kbd->cmd_cnt = 0;
}

/*
 * 0C: SET MOUSE SCALE
 */
static
void st_kbd_cmd_0c (st_kbd_t *kbd)
{
	if (kbd->cmd_cnt < 3) {
		return;
	}

	kbd->scale_x = kbd->cmd[1];
	kbd->scale_y = kbd->cmd[2];

#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET MOUSE SCALE (%u / %u)\n",
		kbd->scale_x, kbd->scale_y
	);
#endif

	kbd->cmd_cnt = 0;
}

/*
 * 0D: INTERROGATE MOUSE POSITION
 */
static
void st_kbd_cmd_0d (st_kbd_t *kbd)
{
	if (kbd->abs_pos == 0) {
		kbd->cmd_cnt = 0;
		return;
	}

#if DEBUG_KBD >= 2
	st_log_deb ("IKBD: INTERROGATE MOUSE POSITION: %u / %u / %u\n",
		kbd->cur_x, kbd->cur_y, kbd->button_delta
	);
#endif

	st_kbd_buf_put (kbd, 0xf7);
	st_kbd_buf_put (kbd, kbd->button_delta);
	st_kbd_buf_put (kbd, (kbd->cur_x >> 8) & 0xff);
	st_kbd_buf_put (kbd, kbd->cur_x & 0xff);
	st_kbd_buf_put (kbd, (kbd->cur_y >> 8) & 0xff);
	st_kbd_buf_put (kbd, kbd->cur_y & 0xff);

	kbd->button_delta = 0;

	kbd->cmd_cnt = 0;
}

/*
 * 0E: LOAD MOUSE POSITION
 */
static
void st_kbd_cmd_0e (st_kbd_t *kbd)
{
	unsigned x, y;

	if (kbd->cmd_cnt < 6) {
		return;
	}

	x = kbd->cmd[2];
	x = (x << 8) | kbd->cmd[3];
	y = kbd->cmd[4];
	y = (y << 8) | kbd->cmd[5];

#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: LOAD MOUSE POSITION (%u, %u)\n", x, y);
#endif

	kbd->cur_x = (x <= kbd->max_x) ? x : kbd->max_x;
	kbd->cur_y = (y <= kbd->max_y) ? y : kbd->max_y;

	kbd->cmd_cnt = 0;
}

/*
 * 0F: SET Y=0 AT BOTTOM
 */
static
void st_kbd_cmd_0f (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET Y=0 AT BOTTOM\n");
#endif

	kbd->y0_at_top = 0;
	kbd->cmd_cnt = 0;
}

/*
 * 10: SET Y=0 AT TOP
 */
static
void st_kbd_cmd_10 (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET Y=0 AT TOP\n");
#endif

	kbd->y0_at_top = 1;
	kbd->cmd_cnt = 0;
}

/*
 * 11: RESUME
 */
static
void st_kbd_cmd_11 (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: RESUME\n");
#endif

	kbd->paused = 0;
	kbd->cmd_cnt = 0;
}

/*
 * 12: DISABLE MOUSE
 */
static
void st_kbd_cmd_12 (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: DISABLE MOUSE\n");
#endif

	kbd->disabled = 1;
	kbd->mouse_dx = 0;
	kbd->mouse_dy = 0;
	kbd->cmd_cnt = 0;
}

/*
 * 13: PAUSE OUTPUT
 */
static
void st_kbd_cmd_13 (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: PAUSE OUTPUT\n");
#endif

	kbd->paused = 1;
	kbd->cmd_cnt = 0;
}

/*
 * 16: JOYSTICK INTERROGATE
 */
static
void st_kbd_cmd_16 (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: JOYSTICK INTERROGATE\n");
#endif

	st_kbd_buf_put (kbd, 0xfd);
	st_kbd_buf_put (kbd, kbd->joy[0]);
	st_kbd_buf_put (kbd, kbd->joy[1]);
}

/*
 * 1A: DISABLE JOYSTICKS
 */
static
void st_kbd_cmd_1a (st_kbd_t *kbd)
{
#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: DISABLE JOYSTICKS\n");
#endif

	kbd->cmd_cnt = 0;
}

/*
 * 1B: SET TIME-OF-DAY CLOCK
 */
static
void st_kbd_cmd_1b (st_kbd_t *kbd)
{
	if (kbd->cmd_cnt < 7) {
		return;
	}

#if DEBUG_KBD >= 1
	st_log_deb ("IKBD: SET TIME-OF-DAY CLOCK: %02X-%02X-%02X %02X:%02X:%02X\n",
		kbd->cmd[1], kbd->cmd[2], kbd->cmd[3],
		kbd->cmd[4], kbd->cmd[5], kbd->cmd[6]
	);
#endif

	kbd->cmd_cnt = 0;
}

/*
 * 1C: INTERROGATE TIME-OF-DAY CLOCK
 */
static
void st_kbd_cmd_1c (st_kbd_t *kbd)
{
	time_t    tm;
	struct tm *tval;

	tm = time (NULL);
	tval = localtime (&tm);

	st_kbd_buf_put (kbd, 0xfc);
	st_kbd_buf_put (kbd, get_bcd_8 (tval->tm_year));
	st_kbd_buf_put (kbd, get_bcd_8 (tval->tm_mon + 1));
	st_kbd_buf_put (kbd, get_bcd_8 (tval->tm_mday));
	st_kbd_buf_put (kbd, get_bcd_8 (tval->tm_hour));
	st_kbd_buf_put (kbd, get_bcd_8 (tval->tm_min));
	st_kbd_buf_put (kbd, get_bcd_8 (tval->tm_sec));

#if DEBUG_KBD >= 1
	st_log_deb (
		"IKBD: INTERROGATE TIME-OF-DAY CLOCK:"
		" %02u-%02u-%02u %02u:%02u:%02u\n",
		tval->tm_year + 1900, tval->tm_mon + 1, tval->tm_mday,
		tval->tm_hour, tval->tm_min, tval->tm_sec
	);
#endif

	kbd->cmd_cnt = 0;
}

/*
 * 80: RESET
 */
static
void st_kbd_cmd_80 (st_kbd_t *kbd)
{
	if (kbd->cmd_cnt < 2) {
		return;
	}

	if (kbd->cmd[1] == 0x01) {
#if DEBUG_KBD >= 1
		st_log_deb ("IKBD: RESET\n");
#endif
		kbd->paused = 0;
		kbd->disabled = 0;
		kbd->abs_pos = 0;
		kbd->y0_at_top = 1;
		kbd->joy_report = 1;

		kbd->mouse_dx = 0;
		kbd->mouse_dy = 0;

		kbd->buf_hd = 0;
		kbd->buf_tl = 0;

		st_kbd_buf_put (kbd, 0xf0);
	}

	kbd->cmd_cnt = 0;
}

void st_kbd_set_uint8 (st_kbd_t *kbd, unsigned char val)
{
#if DEBUG_KBD >= 3
		st_log_deb ("IKBD: recv %02X\n", val);
#endif

	if (kbd->cmd_cnt >= sizeof (kbd->cmd)) {
		kbd->cmd_cnt = 0;
		return;
	}

	kbd->cmd[kbd->cmd_cnt++] = val;

	switch (kbd->cmd[0]) {
	case 0x07:
		st_kbd_cmd_07 (kbd);
		break;

	case 0x08:
		st_kbd_cmd_08 (kbd);
		break;

	case 0x09:
		st_kbd_cmd_09 (kbd);
		break;

	case 0x0b:
		st_kbd_cmd_0b (kbd);
		break;

	case 0x0c:
		st_kbd_cmd_0c (kbd);
		break;

	case 0x0d:
		st_kbd_cmd_0d (kbd);
		break;

	case 0x0e:
		st_kbd_cmd_0e (kbd);
		break;

	case 0x0f:
		st_kbd_cmd_0f (kbd);
		break;

	case 0x10:
		st_kbd_cmd_10 (kbd);
		break;

	case 0x11:
		st_kbd_cmd_11 (kbd);
		break;

	case 0x12:
		st_kbd_cmd_12 (kbd);
		break;

	case 0x13:
		st_kbd_cmd_13 (kbd);
		break;

	case 0x16:
		st_kbd_cmd_16 (kbd);
		break;

	case 0x1a:
		st_kbd_cmd_1a (kbd);
		break;

	case 0x1b:
		st_kbd_cmd_1b (kbd);
		break;

	case 0x1c:
		st_kbd_cmd_1c (kbd);
		break;

	case 0x80:
		st_kbd_cmd_80 (kbd);
		break;

	default:
		if (kbd->cmd_cnt == 1) {
			st_log_deb ("IKBD: UNKNOWN COMMAND: %02X\n", val);
			kbd->cmd_cnt = 0;
		}
		break;
	}

	if ((kbd->cmd_cnt == 0) && (kbd->cmd[0] != 0x13)) {
		kbd->paused = 0;
	}
}

int st_kbd_get_uint8 (st_kbd_t *kbd, unsigned char *val)
{
	if (kbd->paused) {
		kbd->idle = 1;
		return (1);
	}

	if (st_kbd_buf_get (kbd, val)) {
		st_kbd_check_mouse (kbd);

		if (st_kbd_buf_get (kbd, val)) {
			kbd->idle = 1;

			return (1);
		}
	}

#if DEBUG_KBD >= 3
	st_log_deb ("IKBD: send %02X\n", *val);
#endif

	kbd->idle = 0;

	return (0);
}

void st_kbd_reset (st_kbd_t *kbd)
{
	kbd->idle = 1;

	kbd->cmd_cnt = 0;

	kbd->paused = 0;
	kbd->disabled = 0;
	kbd->abs_pos = 0;
	kbd->y0_at_top = 1;
	kbd->joy_report = 1;

	kbd->mouse_dx = 0;
	kbd->mouse_dy = 0;
	kbd->mouse_but[0] = 0;
	kbd->mouse_but[1] = 0;

	kbd->joy[0] = 0;
	kbd->joy[1] = 0;

	kbd->cur_x = 0;
	kbd->cur_y = 0;
	kbd->max_x = 255;
	kbd->max_y = 255;
	kbd->scale_x = 1;
	kbd->scale_y = 1;
	kbd->button_delta = 0;

	kbd->buf_hd = 0;
	kbd->buf_tl = 0;
}
