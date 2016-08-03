/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/cpm80.h                                       *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2016 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_CPM80_CPM80_H
#define PCE_CPM80_CPM80_H 1


#include <cpu/e8080/e8080.h>
#include <devices/memory.h>
#include <drivers/block/block.h>
#include <drivers/char/char.h>
#include <libini/libini.h>
#include <lib/brkpt.h>


#define PCE_BRK_STOP  1
#define PCE_BRK_ABORT 2
#define PCE_BRK_SNAP  3

#define CPM80_CPU_SYNC  100
#define CPM80_DRIVE_MAX 16

#define CPM80_MODEL_PLAIN 0
#define CPM80_MODEL_CPM   1


/*****************************************************************************
 * @short The cpm80 context struct
 *****************************************************************************/
typedef struct cpm80_s {
	unsigned       model;

	e8080_t        *cpu;

	memory_t       *mem;
	mem_blk_t      *ram;

	bp_set_t       bps;

	unsigned long  clk_cnt;
	unsigned long  clk_div;

	unsigned long  clock;
	unsigned long  sync_clk;
	unsigned long  sync_us;
	long           sync_sleep;

	unsigned       brk;

	char_drv_t     *con;
	char_drv_t     *aux;
	char_drv_t     *lst;

	unsigned char  con_buf;
	unsigned char  con_buf_cnt;

	disks_t        *dsks;

	unsigned       addr_ccp;
	unsigned       addr_bdos;
	unsigned       addr_bios;

	unsigned       bios_disk_cnt;
	unsigned char  bios_disk_type[CPM80_DRIVE_MAX];

	unsigned short bios_dma;
	unsigned char  bios_dsk;
	unsigned char  bios_trk;
	unsigned char  bios_sec;
} cpm80_t;


cpm80_t *c80_new (ini_sct_t *ini);

void c80_del (cpm80_t *sim);

void c80_stop (cpm80_t *sim);

void c80_reset (cpm80_t *sim);

int c80_set_cpu_model (cpm80_t *sim, const char *str);

void c80_idle (cpm80_t *sim);

void c80_clock_discontinuity (cpm80_t *sim);

void c80_set_clock (cpm80_t *sim, unsigned long clock);
void c80_set_speed (cpm80_t *sim, unsigned speed);

void c80_clock (cpm80_t *sim, unsigned n);


#endif
