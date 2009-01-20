/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sims32/sims32.h                                     *
 * Created:     2004-09-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_SIMS32_SIMS32_H
#define PCE_SIMS32_SIMS32_H 1


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif


#define PCE_BRK_STOP  1
#define PCE_BRK_ABORT 2
#define PCE_BRK_SNAP  3


typedef void (*seta_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*seta_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef unsigned char (*geta_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*geta_uint16_f) (void *ext, unsigned long addr);

typedef void (*set_uint8_f) (void *ext, unsigned char val);
typedef void (*set_uint16_f) (void *ext, unsigned short val);
typedef unsigned char (*get_uint8_f) (void *ext);
typedef unsigned short (*get_uint16_f) (void *ext);


struct sims32_t;


extern char          *par_terminal;
extern char          *par_video;
extern char          *par_cpu;


void pce_dump_hex (FILE *fp, void *buf, unsigned long n,
	unsigned long addr, unsigned cols, char *prefix, int ascii
);


#include <devices/memory.h>
#include <devices/nvram.h>
#include <devices/serport.h>

#include <cpu/sparc32/sparc32.h>

#include <libini/libini.h>

#include <lib/log.h>
#include <lib/brkpt.h>
#include <lib/load.h>


/*****************************************************************************
 * @short The sims32 context struct
 *****************************************************************************/
typedef struct sims32_s {
	sparc32_t          *cpu;

	memory_t           *mem;
	mem_blk_t          *ram;

	nvram_t            *nvr;

	serport_t          *serport[2];

	bp_set_t           bps;

	unsigned long long clk_cnt;
	unsigned long      clk_div[4];

	unsigned           brk;
} sims32_t;


/*****************************************************************************
 * @short Create a new sims32 context
 * @param ini A libini sims32 section. Can be NULL.
 *****************************************************************************/
sims32_t *ss32_new (ini_sct_t *ini);

/*****************************************************************************
 * @short Delete a sims32 context
 *****************************************************************************/
void ss32_del (sims32_t *sim);

/*****************************************************************************
 * @short  Get the number of clock cycles
 * @return The number of clock cycles the simulation went through since the
 *         last initialization
 *****************************************************************************/
unsigned long long ss32_get_clkcnt (sims32_t *sim);

/*****************************************************************************
 * @short Reset the simulator
 *****************************************************************************/
void ss32_reset (sims32_t *sim);

/*****************************************************************************
 * @short Clock the simulator
 * @param n The number of clock cycles. Must not be 0.
 *****************************************************************************/
void ss32_clock (sims32_t *sim, unsigned n);

/*****************************************************************************
 * @short Interrupt the emulator
 * @param val The type of break (see PCE_BRK_* constants)
 *
 * This is a hack
 *****************************************************************************/
void ss32_break (sims32_t *sim, unsigned char val);

/*****************************************************************************
 * Don't use.
 *****************************************************************************/
void ss32_set_keycode (sims32_t *sim, unsigned char val);

void ss32_set_msg (sims32_t *sim, const char *msg, const char *val);


#endif
