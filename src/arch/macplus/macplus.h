/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/macplus.h                                   *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_MACPLUS_H
#define PCE_MACPLUS_MACPLUS_H 1


#include "adb.h"
#include "adb_keyboard.h"
#include "adb_mouse.h"
#include "iwm.h"
#include "keyboard.h"
#include "rtc.h"
#include "scsi.h"
#include "serial.h"
#include "sony.h"
#include "sound.h"
#include "video.h"

#include <chipset/e6522.h>
#include <chipset/e8530.h>

#include <cpu/e68000/e68000.h>

#include <devices/memory.h>
#include <devices/nvram.h>

#include <drivers/block/block.h>

#include <drivers/video/terminal.h>

#include <lib/brkpt.h>


#define PCE_MAC_MACPLUS 1
#define PCE_MAC_MACSE   2


/*****************************************************************************
 * @short The macplus context struct
 *****************************************************************************/
struct macplus_s {
	unsigned           model;

	e68000_t           *cpu;

	memory_t           *mem;

	mem_blk_t          *ram;
	mem_blk_t          *rom;

	mem_blk_t          *ram_ovl;
	mem_blk_t          *rom_ovl;

	bp_set_t           bps;

	e6522_t            via;
	e8530_t            scc;
	mac_rtc_t          rtc;
	mac_kbd_t          *kbd;
	mac_adb_t          *adb;
	adb_kbd_t          *adb_kbd;
	adb_mouse_t        *adb_mouse;
	mac_scsi_t         scsi;
	mac_sony_t         sony;
	mac_sound_t        sound;
	mac_video_t        *video;
	terminal_t         *trm;
	disks_t            *dsks;

	mac_ser_t          ser[2];

	unsigned char      via_port_a;
	unsigned char      via_port_b;

	int                overlay;
	int                reset;

	unsigned long      vbuf1;
	unsigned long      vbuf2;

	unsigned long      sbuf1;
	unsigned long      sbuf2;

	unsigned char      intr;

	long               mouse_delta_x;
	long               mouse_delta_y;
	unsigned           mouse_button;

	unsigned char      iwm_lines;
	unsigned char      iwm_status;
	unsigned char      iwm_mode;

	unsigned char      dcd_a;
	unsigned char      dcd_b;

	char               *rtc_fname;

	char               pause;
	unsigned           brk;

	unsigned           speed_factor;
	unsigned long      speed_clock_extra;

	unsigned long      sync_clk;
	unsigned long      sync_us;
	long               sync_sleep;

	unsigned long long clk_cnt;
	unsigned long      clk_div[4];
};


void mac_init (macplus_t *sim, ini_sct_t *ini);

/*****************************************************************************
 * @short Create a new macplus context
 * @param ini A libini macplus section. Can be NULL.
 *****************************************************************************/
macplus_t *mac_new (ini_sct_t *ini);

void mac_free (macplus_t *sim);

/*****************************************************************************
 * @short Delete a macplus context
 *****************************************************************************/
void mac_del (macplus_t *sim);

void mac_interrupt (macplus_t *sim, unsigned level, int val);

/*****************************************************************************
 * @short  Get the number of clock cycles
 * @return The number of clock cycles the simulation went through since the
 *         last initialization
 *****************************************************************************/
unsigned long long mac_get_clkcnt (macplus_t *sim);

void mac_clock_discontinuity (macplus_t *sim);

void mac_set_pause (macplus_t *sim, int pause);

void mac_set_speed (macplus_t *sim, unsigned factor);

int mac_set_msg_trm (macplus_t *sim, const char *msg, const char *val);

int mac_set_cpu_model (macplus_t *sim, const char *model);

/*****************************************************************************
 * @short Reset the simulator
 *****************************************************************************/
void mac_reset (macplus_t *sim);

/*****************************************************************************
 * @short Clock the simulator
 * @param n The number of clock cycles.
 *****************************************************************************/
void mac_clock (macplus_t *sim, unsigned n);


#endif
