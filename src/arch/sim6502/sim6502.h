/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim6502/sim6502.h                                   *
 * Created:     2004-05-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_SIM6502_SIM6502_H
#define PCE_SIM6502_SIM6502_H 1


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


struct sim6502_s;


#include <devices/memory.h>
#include <cpu/e6502/e6502.h>

#include <libini/libini.h>

#include <lib/brkpt.h>
#include <lib/log.h>
#include <lib/ihex.h>
#include <lib/load.h>

#include "console.h"


/*****************************************************************************
 * @short The sim6502 context struct
 *****************************************************************************/
typedef struct sim6502_s {
	e6502_t            *cpu;

	memory_t           *mem;
	mem_blk_t          *ram;

	console_t          console;

	bp_set_t           bps;

	unsigned long long clk_cnt;

	unsigned long      clk_div;

	unsigned           brk;
} sim6502_t;


sim6502_t *s6502_new (ini_sct_t *ini);

void s6502_del (sim6502_t *sim);

unsigned long long s6502_get_clkcnt (sim6502_t *sim);

void s6502_break (sim6502_t *sim, unsigned char val);

void s6502_reset (sim6502_t *sim);

void s6502_clock (sim6502_t *sim, unsigned n);

void s6502_set_msg (sim6502_t *sim, const char *msg, const char *val);


#endif
