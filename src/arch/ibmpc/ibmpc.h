/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/ibmpc.h                                       *
 * Created:     2001-05-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_IBMPC_H
#define PCE_IBMPC_H 1


#define PCE_IBMPC_CLK0 14318184
#define PCE_IBMPC_CLK1 (PCE_IBMPC_CLK0 / 3)
#define PCE_IBMPC_CLK2 (PCE_IBMPC_CLK0 / 12)


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
	pc_kbd_t           kbd;
	pc_cassette_t      *cas;

	unsigned char      ppi_port_a[2];
	unsigned char      ppi_port_b;
	unsigned char      ppi_port_c[2];

	ems_t              *ems;
	xms_t              *xms;

	terminal_t         *trm;

	serport_t          *serport[4];
	parport_t          *parport[4];

	ini_sct_t          *cfg;

	bp_set_t           bps;

	unsigned           bootdrive;

	/* allow int 1a to get real time */
	int                support_rtc;

	unsigned           fd_cnt;
	unsigned           hd_cnt;

	unsigned long      sync_clock2_sim;
	unsigned long      sync_clock2_real;
	unsigned long      sync_interval;

	/* cpu speed factor */
	unsigned           speed_current;
	unsigned           speed_default;
	unsigned           speed_saved;

	unsigned long      clk_div[4];

	unsigned long      clock2;

	unsigned           brk;
	char               pause;
} ibmpc_t;


ibmpc_t *pc_new (ini_sct_t *ini);

void pc_del (ibmpc_t *pc);

void pc_log_deb (ibmpc_t *pc, const char *msg, ...);

int pc_set_serport_driver (ibmpc_t *pc, unsigned port, const char *driver);
int pc_set_serport_file (ibmpc_t *pc, unsigned port, const char *fname);

int pc_set_parport_driver (ibmpc_t *pc, unsigned port, const char *driver);
int pc_set_parport_file (ibmpc_t *pc, unsigned port, const char *fname);

unsigned pc_get_pcex_seg (ibmpc_t *pc);

/*!***************************************************************************
 * @short Reset the PC
 *****************************************************************************/
void pc_reset (ibmpc_t *pc);

unsigned long pc_get_clock2 (ibmpc_t *pc);

/*!***************************************************************************
 * @short Reset the clock counters
 *****************************************************************************/
void pc_clock_reset (ibmpc_t *pc);

/*!***************************************************************************
 * @short Synchronize the clocks after a discontinuity
 *****************************************************************************/
void pc_clock_discontinuity (ibmpc_t *pc);

/*!***************************************************************************
 * @short Clock the pc
 *****************************************************************************/
void pc_clock (ibmpc_t *pc);

/*!***************************************************************************
 * @short Set the specific CPU model to be emulated
 *****************************************************************************/
int pc_set_cpu_model (ibmpc_t *pc, const char *str);

/*!***************************************************************************
 * @short Set the emulated cpu clock frequency as a multiple of 4.77 MHz
 *****************************************************************************/
void pc_set_speed (ibmpc_t *pc, unsigned factor);

void pc_set_bootdrive (ibmpc_t *pc, unsigned drv);
unsigned pc_get_bootdrive (ibmpc_t *pc);


#endif
