/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/keyboard.h                                    *
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


#ifndef PCE_RC759_KEYBOARD_H
#define PCE_RC759_KEYBOARD_H 1


#define RC759_KBD_BUF 256


typedef struct {
	unsigned char enable;

	unsigned long delay;

	unsigned char key;
	unsigned char key_valid;

	unsigned      key_i;
	unsigned      key_j;
	unsigned char key_buf[RC759_KBD_BUF];

	int           mouse_dx;
	int           mouse_dy;
	unsigned      mouse_but[2];

	void          *irq_ext;
	void          (*irq) (void *ext, unsigned char val);
} rc759_kbd_t;


void rc759_kbd_init (rc759_kbd_t *kbd);

void rc759_kbd_set_irq_fct (rc759_kbd_t *kbd, void *ext, void *fct);

void rc759_kbd_reset (rc759_kbd_t *kbd);

void rc759_kbd_set_enable (rc759_kbd_t *kbd, unsigned char val);

void rc759_kbd_set_mouse (rc759_kbd_t *kbd, int dx, int dy, unsigned but);

void rc759_kbd_set_key (rc759_kbd_t *kbd, unsigned event, unsigned key);

void rc759_kbd_set_keycode (rc759_kbd_t *kbd, unsigned char val);

unsigned char rc759_kbd_get_key (rc759_kbd_t *kbd);

void rc759_kbd_clock (rc759_kbd_t *kbd, unsigned cnt);


#endif
