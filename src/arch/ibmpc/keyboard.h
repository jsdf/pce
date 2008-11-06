/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/keyboard.h                                    *
 * Created:     2007-11-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2008 Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id$ */


#ifndef PCE_IBMPC_KEYBOARD_H
#define PCE_IBMPC_KEYBOARD_H 1


/*!***************************************************************************
 * @short Send a key event to the PC
 *****************************************************************************/
void pc_set_key (ibmpc_t *pc, unsigned event, unsigned key);

/*!***************************************************************************
 * @short Send a key code to the PC
 *
 * This function is deprecated.
 *****************************************************************************/
void pc_set_keycode (ibmpc_t *pc, unsigned char val);

/*!***************************************************************************
 * @short Clear the emulator keyboard buffer
 *****************************************************************************/
void pc_kbd_clear (ibmpc_t *pc);

void pc_kbd_init (ibmpc_t *pc);

void pc_kbd_reset (ibmpc_t *pc);

void pc_kbd_clock (ibmpc_t *pc, unsigned long cnt);


#endif
