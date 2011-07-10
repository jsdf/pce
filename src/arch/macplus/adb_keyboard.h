/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/adb_keyboard.h                              *
 * Created:     2010-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_ADB_KEYBOARD_H
#define PCE_MACPLUS_ADB_KEYBOARD_H 1


#include "adb.h"

#include <drivers/video/keys.h>

#define ADB_KBD_BUF 256


typedef struct {
	pce_key_t     key;
	unsigned char code;
	unsigned char flags;
} adb_kbd_map_t;


typedef struct {
	adb_dev_t     dev;

	char          talking;

	unsigned      buf_i;
	unsigned      buf_j;
	unsigned char buf[ADB_KBD_BUF];

	char          keypad_motion_mode;

	adb_kbd_map_t *keymap;
} adb_kbd_t;


void adb_kbd_set_key (adb_kbd_t *kbd, unsigned event, pce_key_t key);

void adb_kbd_set_keypad_mode (adb_kbd_t *kbd, int motion);

int adb_kbd_get_keypad_mode (const adb_kbd_t *kbd);

void adb_kbd_init (adb_kbd_t *ak);

adb_kbd_t *adb_kbd_new (void);


#endif
