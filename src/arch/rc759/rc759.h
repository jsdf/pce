/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/rc759.h                                       *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_RC759_RC759_H
#define PCE_RC759_RC759_H 1


#include "fdc.h"
#include "keyboard.h"
#include "nvm.h"
#include "parport.h"
#include "rtc.h"
#include "speaker.h"
#include "video.h"

#include <chipset/80186/dma.h>
#include <chipset/80186/icu.h>
#include <chipset/80186/tcu.h>

#include <chipset/82xx/e8250.h>
#include <chipset/82xx/e8255.h>
#include <chipset/82xx/e8259.h>

#include <cpu/e8086/e8086.h>

#include <devices/memory.h>

#include <drivers/block/block.h>

#include <drivers/video/terminal.h>

#include <lib/brkpt.h>

#include <libini/libini.h>


#define RC759_FLAG_MEM2 1


typedef struct rc759_t {
	unsigned           flags;

	memory_t           *mem;
	mem_blk_t          *ram;
	memory_t           *iop;

	e8086_t            *cpu;
	e8259_t            pic;
	e80186_tcu_t       tcu;
	e80186_icu_t       icu;
	e80186_dma_t       dma;
	e8255_t            ppi;
	rc759_kbd_t        kbd;
	rc759_nvm_t        nvm;
	rc759_rtc_t        rtc;
	rc759_fdc_t        fdc;
	rc759_speaker_t    spk;
	e82730_t           crt;
	terminal_t         *trm;
	disks_t            *dsks;
	rc759_parport_t    par[2];

	ini_sct_t          *cfg;

	bp_set_t           bps;

	unsigned char      ppi_port_a;
	unsigned char      ppi_port_b;
	unsigned char      ppi_port_c;

	unsigned           current_int;

	unsigned long      cpu_clock_frq;
	unsigned long      cpu_clock_cnt;
	unsigned long      cpu_clock_rem8;
	unsigned long      cpu_clock_rem1024;
	unsigned long      cpu_clock_rem32768;

	unsigned long      sync_clock_sim;
	unsigned long      sync_clock_real;
	unsigned long      sync_interval;

	unsigned           brk;
	char               pause;
} rc759_t;


extern rc759_t *par_sim;


rc759_t *rc759_new (ini_sct_t *ini);

void rc759_del (rc759_t *sim);

int rc759_set_parport_driver (rc759_t *sim, unsigned port, const char *driver);
int rc759_set_parport_file (rc759_t *sim, unsigned port, const char *fname);

const char *rc759_intlog_get (rc759_t *sim, unsigned n);
void rc759_intlog_set (rc759_t *sim, unsigned n, const char *expr);
int rc759_intlog_check (rc759_t *sim, unsigned n);

/*!***************************************************************************
 * @short Reset the simulation
 *****************************************************************************/
void rc759_reset (rc759_t *sim);

void rc759_set_cpu_clock (rc759_t *sim, unsigned long clk);

/*!***************************************************************************
 * @short Set the emulated cpu clock frequency as a multiple of 4.77 MHz
 *****************************************************************************/
void rc759_set_speed (rc759_t *sim, unsigned factor);

/*!***************************************************************************
 * @short Get the CPU clock
 *****************************************************************************/
unsigned long rc759_get_cpu_clock (rc759_t *sim);

/*!***************************************************************************
 * @short Reset the clock counters
 *****************************************************************************/
void rc759_clock_reset (rc759_t *sim);

/*!***************************************************************************
 * @short Synchronize the clocks after a discontinuity
 *****************************************************************************/
void rc759_clock_discontinuity (rc759_t *sim);

/*!***************************************************************************
 * @short Clock the simulation
 *****************************************************************************/
void rc759_clock (rc759_t *sim, unsigned cnt);


#endif
