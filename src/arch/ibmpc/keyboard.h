/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/keyboard.h                                    *
 * Created:     2007-11-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_IBMPC_KEYBOARD_H
#define PCE_IBMPC_KEYBOARD_H 1


#define PC_KBD_BUF 256


typedef struct {
	/* transmission delay in 1.19 MHz clocks */
	unsigned long delay;

	/* the shift register */
	unsigned char key;
	unsigned char key_valid;

	unsigned char enable;

	unsigned char clk;

	unsigned      key_i;
	unsigned      key_j;
	unsigned char key_buf[PC_KBD_BUF];

	void          *irq_ext;
	void          (*irq) (void *ext, unsigned char val);
} pc_kbd_t;


/*!***************************************************************************
 * @short Initialize the keyboard structure
 *****************************************************************************/
void pc_kbd_init (pc_kbd_t *kbd);

/*!***************************************************************************
 * @short Set the interrupt request function
 *****************************************************************************/
void pc_kbd_set_irq_fct (pc_kbd_t *kbd, void *ext, void *fct);

/*!***************************************************************************
 * @short Reset the keyboard
 *****************************************************************************/
void pc_kbd_reset (pc_kbd_t *kbd);

/*!***************************************************************************
 * @short Set the clock output on the PC side
 *****************************************************************************/
void pc_kbd_set_clk (pc_kbd_t *kbd, unsigned char val);

void pc_kbd_set_enable (pc_kbd_t *kbd, unsigned char val);

/*!***************************************************************************
 * @short Send a key event to the PC
 *****************************************************************************/
void pc_kbd_set_key (pc_kbd_t *kbd, unsigned event, unsigned key);

/*!***************************************************************************
 * @short Send a key code to the PC (deprecated)
 *****************************************************************************/
void pc_kbd_set_keycode (pc_kbd_t *kbd, unsigned char val);

/*!***************************************************************************
 * @short Read the shift register
 *****************************************************************************/
unsigned char pc_kbd_get_key (pc_kbd_t *kbd);

void pc_kbd_clock (pc_kbd_t *kbd, unsigned long cnt);


#endif
