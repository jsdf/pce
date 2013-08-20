/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/atarist.h                                   *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_ATARIST_H
#define PCE_ATARIST_ATARIST_H 1


#define PCE_ST_ST 1


#include "acsi.h"
#include "dma.h"
#include "fdc.h"
#include "ikbd.h"
#include "psg.h"
#include "rp5c15.h"
#include "smf.h"
#include "video.h"

#include <cpu/e68000/e68000.h>

#include <chipset/e6850.h>
#include <chipset/e68901.h>
#include <chipset/e8530.h>

#include <devices/memory.h>

#include <drivers/block/block.h>
#include <drivers/char/char.h>
#include <drivers/video/terminal.h>
#include <drivers/video/keys.h>

#include <lib/brkpt.h>

#include <libini/libini.h>


/*****************************************************************************
 * @short The Atari ST context struct
 *****************************************************************************/
struct atari_st_s {
	unsigned      model;

	e68000_t      *cpu;
	memory_t      *mem;
	mem_blk_t     *ram;
	bp_set_t      bps;
	e68901_t      mfp;
	e6850_t       acia0;
	e6850_t       acia1;
	st_kbd_t      kbd;
	st_smf_t      smf;
	rp5c15_t      rtc;
	st_psg_t      psg;
	st_fdc_t      fdc;
	st_acsi_t     acsi;
	st_dma_t      dma;
	st_video_t    *video;
	terminal_t    *trm;
	disks_t       *dsks;
	char_drv_t    *parport_drv;
	char_drv_t    *serport_drv;
	char_drv_t    *midi_drv;

	unsigned long rom_addr;

	unsigned char int_mask;
	unsigned char int_level;

	unsigned char video_state;

	char          mono;
	char          fastboot;
	char          reset;
	char          pause;
	unsigned      brk;

	unsigned char psg_port_a;
	unsigned char psg_port_b;

	unsigned      speed_factor;
	unsigned long speed_clock_extra;

	unsigned long sync_clk;
	unsigned long sync_us;
	long          sync_sleep;

	unsigned long clk_cnt;
	unsigned long clk_div[4];

	unsigned      ser_buf_i;
	unsigned      ser_buf_n;
	unsigned char ser_buf[128];
};


/*****************************************************************************
 * @short Initialize an Atari ST context
 * @param sim  The Atari ST context
 * @param ini  A libini section. Can be NULL.
 *****************************************************************************/
void st_init (atari_st_t *sim, ini_sct_t *ini);

/*****************************************************************************
 * @short Create and initialize a new Atari ST context
 * @param ini  A libini section. Can be NULL.
 *****************************************************************************/
atari_st_t *st_new (ini_sct_t *ini);

/*****************************************************************************
 * @short Free an Atari ST context
 * @param sim  The Atari ST context
 *****************************************************************************/
void st_free (atari_st_t *sim);

/*****************************************************************************
 * @short Delete an Atari ST context
 *****************************************************************************/
void st_del (atari_st_t *sim);

void st_interrupt (atari_st_t *sim, unsigned level, int val);

/*****************************************************************************
 * @short  Get the number of clock cycles
 * @return The number of clock cycles the simulation went through since the
 *         last initialization
 *****************************************************************************/
unsigned long long st_get_clkcnt (atari_st_t *sim);

void st_clock_discontinuity (atari_st_t *sim);

void st_set_pause (atari_st_t *sim, int pause);

void st_set_speed (atari_st_t *sim, unsigned factor);

int st_set_msg_trm (atari_st_t *sim, const char *msg, const char *val);

int st_set_cpu_model (atari_st_t *sim, const char *model);

/*****************************************************************************
 * @short Reset the simulation
 *****************************************************************************/
void st_reset (atari_st_t *sim);

/*****************************************************************************
 * @short Clock the simulator
 * @param n The number of clock cycles.
 *****************************************************************************/
void st_clock (atari_st_t *sim, unsigned n);


#endif
