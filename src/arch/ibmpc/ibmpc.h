/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/ibmpc.h                                     *
 * Created:       2001-05-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-07-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2006 Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_IBMPC_H
#define PCE_IBMPC_H 1


#define PCE_CPU_8086  0
#define PCE_CPU_8088  1
#define PCE_CPU_V30   2
#define PCE_CPU_V20   3
#define PCE_CPU_80186 4
#define PCE_CPU_80188 5
#define PCE_CPU_80286 6


typedef struct ibmpc_t {
	e8086_t            *cpu;
	video_t            *video;
	disks_t            *dsk;
	mouse_t            *mse;

	memory_t           *mem;
	mem_blk_t          *ram;

	memory_t           *prt;

	nvram_t            *nvr;

	e8237_t            dma;
	e8253_t            pit;
	e8255_t            ppi;
	e8259_t            pic;

	unsigned char      ppi_port_a[2];
	unsigned char      ppi_port_b;
	unsigned char      ppi_port_c[2];

	unsigned           key_i;
	unsigned           key_j;
	unsigned char      key_buf[256];

	ems_t              *ems;
	xms_t              *xms;

	terminal_t         *trm;

	serport_t          *serport[4];
	parport_t          *parport[4];

	ini_sct_t          *cfg;

	breakpoint_t       *brkpt;

	unsigned           cpu_model;

	unsigned           bootdrive;

	/* allow int 1a to get real time */
	char               support_rtc;

	unsigned           fd_cnt;
	unsigned           hd_cnt;

	char               pit_real;
	unsigned long      pit_clk;
	unsigned long      pit_clkdiv;

	unsigned long long clk_cnt;
	unsigned long      clk_div[4];

	unsigned           brk;
	char               pause;
} ibmpc_t;


ibmpc_t *pc_new (ini_sct_t *ini);

void pc_del (ibmpc_t *pc);

void pc_clock (ibmpc_t *pc);

void pc_screenshot (ibmpc_t *pc, const char *fname);

int pc_set_cpu_model (ibmpc_t *pc, unsigned model);

void pc_set_bootdrive (ibmpc_t *pc, unsigned drv);
unsigned pc_get_bootdrive (ibmpc_t *pc);

void pc_break (ibmpc_t *pc, unsigned char val);

void pc_set_keycode (ibmpc_t *pc, unsigned char val);


#endif
