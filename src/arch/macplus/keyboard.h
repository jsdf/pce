/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/keyboard.h                                  *
 * Created:     2007-11-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_KEYBOARD_H
#define PCE_MACPLUS_KEYBOARD_H 1


#include <drivers/video/keys.h>


#define MAC_KBD_BUFSIZE 256


typedef struct {
	pce_key_t      pcekey;

	unsigned short down_cnt;
	unsigned char  down[8];

	unsigned short up_cnt;
	unsigned char  up[8];
} mac_kbd_map_t;


typedef struct {
	unsigned      buf_i;
	unsigned      buf_n;
	unsigned char buf[MAC_KBD_BUFSIZE];

	unsigned char data;
	unsigned long timeout;

	int           send_byte;

	unsigned      model;

	char          keypad_mode;

	mac_kbd_map_t *keymap;

	void          *set_data_ext;
	void          (*set_data) (void *ext, unsigned char val);

	void          *set_intr_ext;
	void          (*set_intr) (void *ext, unsigned level, int val);
} mac_kbd_t;


void mac_kbd_init (mac_kbd_t *kbd);
void mac_kbd_free (mac_kbd_t *kbd);

mac_kbd_t *mac_kbd_new (void);
void mac_kbd_del (mac_kbd_t *kbd);

void mac_kbd_set_data_fct (mac_kbd_t *kbd, void *ext, void *fct);
void mac_kbd_set_intr_fct (mac_kbd_t *kbd, void *ext, void *fct);

int mac_kbd_set_model (mac_kbd_t *kbd, unsigned model, int intl);

void mac_kbd_set_keypad_mode (mac_kbd_t *kbd, int motion);

void mac_kbd_set_key (mac_kbd_t *kbd, unsigned event, pce_key_t key);

void mac_kbd_set_uint8 (mac_kbd_t *kbd, unsigned char val);

void mac_kbd_set_data (mac_kbd_t *kbd, unsigned char val);

void mac_kbd_clock (mac_kbd_t *kbd, unsigned cnt);


#endif
