/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim405/sim405.h                                     *
 * Created:     2004-06-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2018 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#ifndef PCE_SIM405_SIM405_H
#define PCE_SIM405_SIM405_H 1


#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "pci.h"

#include <chipset/clock/ds1743.h>
#include <chipset/ppc405/uic.h>

#include <cpu/ppc405/ppc405.h>

#include <devices/device.h>
#include <devices/ata.h>
#include <devices/clock/ds1743.h>
#include <devices/memory.h>
#include <devices/nvram.h>
#include <devices/pci.h>
#include <devices/pci-ata.h>
#include <devices/serport.h>
#include <devices/slip.h>

#include <lib/brkpt.h>
#include <lib/log.h>
#include <lib/inidsk.h>
#include <lib/load.h>

#include <libini/libini.h>


#define PCE_BRK_STOP  1
#define PCE_BRK_ABORT 2
#define PCE_BRK_SNAP  3


struct sim405_t;


void pce_dump_hex (FILE *fp, void *buf, unsigned long n,
	unsigned long addr, unsigned cols, char *prefix, int ascii
);


#define SIM405_DCRN_OCM0_ISARC  0x18
#define SIM405_DCRN_OCM0_ISCNTL 0x19
#define SIM405_DCRN_OCM0_DSARC  0x1a
#define SIM405_DCRN_OCM0_DSCNTL 0x1b

#define SIM405_DCRN_CPC0_CR0 0xb1
#define SIM405_DCRN_CPC0_CR1 0xb2
#define SIM405_DCRN_CPC0_PSR 0xb4
#define SIM405_DCRN_CPC0_PSR_PAE 0x00000400UL

#define SIM405_DCRN_UIC0_SR  0xc0
#define SIM405_DCRN_UIC0_ER  0xc2
#define SIM405_DCRN_UIC0_CR  0xc3
#define SIM405_DCRN_UIC0_PR  0xc4
#define SIM405_DCRN_UIC0_TR  0xc5
#define SIM405_DCRN_UIC0_MSR 0xc6
#define SIM405_DCRN_UIC0_VR  0xc7
#define SIM405_DCRN_UIC0_VCR 0xc8

#define SIM405_DCRN_MAL0_CFG 0x180
#define SIM405_DCRN_MAL0_ESR 0x181


/*****************************************************************************
 * @short The sim405 context struct
 *****************************************************************************/
typedef struct sim405_s {
	p405_t             *ppc;
	p405_uic_t         uic;

	memory_t           *mem;
	mem_blk_t          *ram;

	dev_list_t         devlst;

	disks_t            *dsks;

	pci_405_t          *pci;
	pci_ata_t          pciata;

	serport_t          *serport[2];
	unsigned           sercons;

	slip_t             *slip;

	bp_set_t           bps;

	/* OCM DCRs */
	uint32_t           ocm0_iscntl;
	uint32_t           ocm0_isarc;
	uint32_t           ocm0_dscntl;
	uint32_t           ocm0_dsarc;

	uint32_t           cpc0_cr0;
	uint32_t           cpc0_cr1;
	uint32_t           cpc0_psr;

	unsigned long long clk_cnt;
	unsigned long      clk_div[4];

	clock_t            real_clk;

	char               sync_time_base;

	unsigned long      sync_clock_sim;
	unsigned long      sync_clock_real;
	unsigned long      sync_interval;

	unsigned long      serial_clock;
	unsigned long      serial_clock_count;

	unsigned           hook_idx;
	unsigned           hook_cnt;
	unsigned char      hook_buf[256];

	FILE               *hook_read;
	FILE               *hook_write;

	unsigned           brk;
} sim405_t;


extern sim405_t *par_sim;


/*****************************************************************************
 * @short Create a new sim405 context
 * @param ini A libini sim405 section. Can be NULL.
 *****************************************************************************/
sim405_t *s405_new (ini_sct_t *ini);

/*****************************************************************************
 * @short Delete a sim405 context
 *****************************************************************************/
void s405_del (sim405_t *sim);

/*****************************************************************************
 * @short  Get the number of clock cycles
 * @return The number of clock cycles the SIM405GS3 went through since the last
 *         initialization
 *****************************************************************************/
unsigned long long s405_get_clkcnt (sim405_t *sim);

/*****************************************************************************
 * @short Reset the simulator
 *****************************************************************************/
void s405_reset (sim405_t *sim);

void s405_clock_discontinuity (sim405_t *sim);

/*****************************************************************************
 * @short Clock the simulator
 * @param n The number of clock cycles. Must not be 0.
 *****************************************************************************/
void s405_clock (sim405_t *sim, unsigned n);

/*****************************************************************************
 * @short Interrupt the emulator
 * @param val The type of break (see PCE_BRK_* constants)
 *
 * This is a hack
 *****************************************************************************/
void s405_break (sim405_t *sim, unsigned char val);

/*****************************************************************************
 * Don't use.
 *****************************************************************************/
void s405_set_keycode (sim405_t *sim, unsigned char val);


#endif
