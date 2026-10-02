/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/keyboard.h                                 *
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


#ifndef PCE_SPECTRUM_KEYBOARD_H
#define PCE_SPECTRUM_KEYBOARD_H 1


#include "spectrum.h"

#include <drivers/video/keys.h>


void spec_kbd_reset (spectrum_t *sim);

void spec_kbd_set_cursor (spectrum_t *sim);
void spec_kbd_set_joy_1 (spectrum_t *sim);
void spec_kbd_set_joy_2 (spectrum_t *sim);
void spec_kbd_set_kempston (spectrum_t *sim);
void spec_kbd_set_keypad_mode (spectrum_t *sim, unsigned mode);

unsigned spec_kbd_get_keys (spectrum_t *sim, unsigned val);

void spec_kbd_set_key (spectrum_t *sim, int press, pce_key_t key);


#endif
