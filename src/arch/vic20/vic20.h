/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/vic20.h                                       *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_VIC20_VIC20_H
#define PCE_VIC20_VIC20_H 1


#include <config.h>

#include "video.h"

#include <chipset/e6522.h>
#include <chipset/e6560.h>

#include <cpu/e6502/e6502.h>

#include <devices/cassette.h>
#include <devices/memory.h>

#include <drivers/video/terminal.h>

#include <lib/brkpt.h>

#include <libini/libini.h>


/*****************************************************************************
 * @short The vic20 context struct
 *****************************************************************************/
typedef struct vic20_s {
	ini_sct_t     *cfg;
	e6502_t       *cpu;
	memory_t      *mem;
	terminal_t    *trm;
	bp_set_t      bps;
	vic20_video_t video;
	e6522_t       via1;
	e6522_t       via2;
	cassette_t    cas;

	unsigned      brk;
	char          pal;
	char          border;
	char          trace;

	unsigned char irq_via1;
	unsigned char irq_via2;

	unsigned char ora1;
	unsigned char orb1;
	unsigned char ora2;
	unsigned char orb2;
	unsigned char ira1;
	unsigned char irb2;

	char          keypad_joystick;
	unsigned char joymat;
	unsigned char keymat[8];

	unsigned      speed;
	unsigned      speed_base;
	char          speed_auto;
	char          speed_tape;

	unsigned      framedrop_base;
	unsigned      framedrop_tape;

	unsigned long clock;
	unsigned long sync_clk;
	unsigned long sync_us;
	long          sync_sleep;

	unsigned      clk_div;
} vic20_t;


unsigned char v20_get_uint8 (void *ext, unsigned long addr);
void v20_set_uint8 (void *ext, unsigned long addr, unsigned char val);

void v20_debug_mem (vic20_t *sim);

void v20_set_via1_irq (vic20_t *sim, unsigned char val);
void v20_set_via1_ca2 (vic20_t *sim, unsigned char val);
void v20_set_via1_ora (vic20_t *sim, unsigned char val);
void v20_set_via1_orb (vic20_t *sim, unsigned char val);
void v20_set_via2_irq (vic20_t *sim, unsigned char val);
void v20_set_via2_ora (vic20_t *sim, unsigned char val);
void v20_set_via2_orb (vic20_t *sim, unsigned char val);

void v20_set_restore (vic20_t *sim, unsigned char val);

void v20_update_keys (vic20_t *sim);
void v20_update_joystick (vic20_t *sim);

void v20_set_joystick (vic20_t *sim, unsigned char dir, int fire);

void v20_cas_set_play (vic20_t *sim, unsigned char val);
void v20_cas_set_inp (vic20_t *sim, unsigned char val);

void v20_reset (vic20_t *sim);

void v20_set_keypad_mode (vic20_t *sim, int mode);

void v20_stop (vic20_t *sim);

void v20_set_speed_tape (vic20_t *sim, unsigned char val);
void v20_set_speed (vic20_t *sim, unsigned speed);

void v20_clock_resync (vic20_t *sim);

void v20_clock (vic20_t *sim);


#endif
