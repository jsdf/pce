/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/sim405.h                                   *
 * Created:       2004-06-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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

/* $Id$ */


#ifndef PCE_SIM405_SIM405_H
#define PCE_SIM405_SIM405_H 1


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


struct sim405_t;


extern char          *par_terminal;
extern char          *par_video;
extern char          *par_cpu;


void pce_dump_hex (FILE *fp, void *buf, unsigned long n,
  unsigned long addr, unsigned cols, char *prefix, int ascii
);


#include <devices/memory.h>
#include <devices/nvram.h>
#include <devices/serport.h>
#include <devices/ata.h>
#include <devices/pci-ata.h>

#include <chipset/ppc405/uic.h>

#include <cpu/ppc405/ppc405.h>

#include <libini/libini.h>

#include <lib/log.h>
#include <lib/brkpt.h>
#include <lib/inidsk.h>
#include <lib/load.h>


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


/*****************************************************************************
 * @short The sim405 context struct
 *****************************************************************************/
typedef struct sim405_s {
  p405_t             *ppc;
  p405_uic_t         uic;

  memory_t           *mem;
  mem_blk_t          *ram;

  memory_t           *cab;

  nvram_t            *nvr;

  disks_t            *dsks;

  pci_405_t          *pci;
  pci_ata_t          pciata;

  serport_t          *serport[2];

  breakpoint_t       *brkpt;

  uint32_t           cpc0_cr1;
  uint32_t           cpc0_psr;

  unsigned long long clk_cnt;
  unsigned long      clk_div[4];

  unsigned           brk;
} sim405_t;


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

void s405_set_msg (sim405_t *sim, const char *msg, const char *val);


#endif
