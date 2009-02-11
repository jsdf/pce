/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/macplus.h                                   *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_MACPLUS_H
#define PCE_MACPLUS_MACPLUS_H 1


/*****************************************************************************
 * @short The macplus context struct
 *****************************************************************************/
struct macplus_s {
	e68000_t           *cpu;

	memory_t           *mem;

	mem_blk_t          *ram;
	mem_blk_t          *rom;

	mem_blk_t          *ram_ovl;
	mem_blk_t          *rom_ovl;

	bp_set_t           bps;

	e6522_t            via;
	e8530_t            scc;
	mac_rtc_t          rtc;
	mac_kbd_t          kbd;
	mac_scsi_t         scsi;
	mac_sony_t         sony;
	mac_video_t        *video;
	terminal_t         *trm;
	disks_t            *dsks;

	mac_ser_t          ser[2];

	unsigned char      via_port_a;
	unsigned char      via_port_b;

	int                overlay;
	int                reset;

	unsigned long      vbuf1;
	unsigned long      vbuf2;

	unsigned long      sbuf1;
	unsigned long      sbuf2;

	unsigned char      intr;

	long               mouse_delta_x;
	long               mouse_delta_y;
	unsigned           mouse_button;

	unsigned char      iwm_lines;
	unsigned char      iwm_status;
	unsigned char      iwm_mode;

	unsigned char      dcd_a;
	unsigned char      dcd_b;

	char               *rtc_fname;

	char               pause;
	unsigned           brk;

	unsigned           speed_factor;
	unsigned long      rt_clk;
	unsigned long      rt_us;

	unsigned long      via_clk_div;

	unsigned long long clk_cnt;
	unsigned long      clk_div[4];
};


void mac_init (macplus_t *sim, ini_sct_t *ini);

/*****************************************************************************
 * @short Create a new macplus context
 * @param ini A libini macplus section. Can be NULL.
 *****************************************************************************/
macplus_t *mac_new (ini_sct_t *ini);

void mac_free (macplus_t *sim);

/*****************************************************************************
 * @short Delete a macplus context
 *****************************************************************************/
void mac_del (macplus_t *sim);

/*****************************************************************************
 * @short  Get the number of clock cycles
 * @return The number of clock cycles the simulation went through since the
 *         last initialization
 *****************************************************************************/
unsigned long long mac_get_clkcnt (macplus_t *sim);

void mac_set_speed_accurate (macplus_t *sim);

void mac_set_speed (macplus_t *sim, unsigned factor);

int mac_set_msg_trm (macplus_t *sim, const char *msg, const char *val);

/*****************************************************************************
 * @short Reset the simulator
 *****************************************************************************/
void mac_reset (macplus_t *sim);

/*****************************************************************************
 * @short Clock the simulator
 * @param n The number of clock cycles.
 *****************************************************************************/
void mac_clock (macplus_t *sim, unsigned n);


#endif
