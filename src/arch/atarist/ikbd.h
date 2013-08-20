/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/ikbd.h                                      *
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


#ifndef PCE_ATARIST_IKBD_H
#define PCE_ATARIST_IKBD_H 1


#include <drivers/video/keys.h>


typedef struct {
	char          idle;

	unsigned      cmd_cnt;
	unsigned char cmd[16];

	char          paused;
	char          disabled;
	char          abs_pos;
	char          y0_at_top;
	char          joy_report;

	int           mouse_dx;
	int           mouse_dy;
	unsigned      mouse_but[2];

	unsigned      joy[2];
	unsigned char keypad_joy;

	unsigned      cur_x;
	unsigned      cur_y;
	unsigned      button_delta;
	unsigned      max_x;
	unsigned      max_y;
	unsigned      scale_x;
	unsigned      scale_y;

	unsigned      buf_hd;
	unsigned      buf_tl;
	unsigned char buf[64];
} st_kbd_t;


void st_kbd_init (st_kbd_t *kbd);

int st_kbd_buf_put (st_kbd_t *kbd, unsigned char val);
int st_kbd_buf_get (st_kbd_t *kbd, unsigned char *val);

void st_kbd_set_mouse (st_kbd_t *kbd, int dx, int dy, unsigned but);
void st_kbd_set_key (st_kbd_t *kbd, unsigned event, pce_key_t key);

void st_kbd_set_uint8 (st_kbd_t *kbd, unsigned char val);
int st_kbd_get_uint8 (st_kbd_t *kbd, unsigned char *val);

void st_kbd_reset (st_kbd_t *kbd);


#endif
