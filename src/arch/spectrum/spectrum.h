/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/spectrum.h                                 *
 * Created:     2021-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_SPECTRUM_SPECTRUM_H
#define PCE_SPECTRUM_SPECTRUM_H 1


#include "video.h"

#include <cpu/e8080/e8080.h>

#include <devices/cassette.h>
#include <devices/memory.h>
#include <devices/speaker.h>

#include <drivers/block/block.h>
#include <drivers/char/char.h>
#include <drivers/video/terminal.h>

#include <libini/libini.h>

#include <lib/brkpt.h>


#define PCE_BRK_STOP  1
#define PCE_BRK_ABORT 2
#define PCE_BRK_SNAP  3

#define SPEC_CLOCK 3500000

#define SPEC_MODEL_16 1
#define SPEC_MODEL_48 2


/*****************************************************************************
 * @short The ZX Spectrum context struct
 *****************************************************************************/
typedef struct spectrum_s {
	unsigned       model;

	e8080_t        *cpu;
	memory_t       *mem;
	terminal_t     *term;
	bp_set_t       bps;
	spec_video_t   video;
	cassette_t     cas;
	speaker_t      spk;

	unsigned char  port_fe;
	unsigned char  cas_inp;

	char           kbd_alt;
	unsigned char  joystate;
	unsigned char  keypad_mode;

	unsigned       video_w;
	unsigned       video_h;

	unsigned       snap_count;

	unsigned char  keys[8];

	unsigned long  clock_base;
	unsigned long  clock_freq;

	unsigned long  clk_cnt;
	unsigned long  clk_div;

	unsigned long  sync_clk;
	unsigned long  sync_us;
	long           sync_sleep;

	unsigned       speed;

	unsigned       brk;
} spectrum_t;


void spec_set_port_fe (spectrum_t *sim, unsigned char val);

spectrum_t *spec_new (ini_sct_t *ini);

void spec_del (spectrum_t *sim);

void spec_stop (spectrum_t *sim);

void spec_reset (spectrum_t *sim);

void spec_idle (spectrum_t *sim);

void spec_clock_discontinuity (spectrum_t *sim);

void spec_set_clock (spectrum_t *sim, unsigned long clock);
void spec_set_speed (spectrum_t *sim, unsigned speed);

void spec_clock (spectrum_t *sim);


#endif
