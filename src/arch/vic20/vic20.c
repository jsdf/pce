/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/vic20.c                                       *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#include "main.h"
#include "keybd.h"
#include "msg.h"
#include "setup.h"
#include "vic20.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>

#include <chipset/e6522.h>
#include <chipset/e6560.h>

#include <cpu/e6502/e6502.h>

#include <devices/cassette.h>

#include <lib/log.h>
#include <lib/sysdep.h>


unsigned char v20_get_uint8 (void *ext, unsigned long addr)
{
	vic20_t *sim = ext;

	if ((addr >= 0x9000) && (addr <= 0x900f)) {
		return (e6560_get_reg (&sim->video.vic, addr - 0x9000));
	}
	else if ((addr >= 0x9100) && (addr <= 0x93ff)) {
		if ((addr & 0x30) == 0x10) {
			return (e6522_get_uint8 (&sim->via1, addr & 0x0f));
		}
		else if ((addr & 0x30) == 0x20) {
			return (e6522_get_uint8 (&sim->via2, addr & 0x0f));
		}
	}
	else if ((addr >= 0x9400) && (addr <= 0x97ff)) {
		return (sim->video.colram[addr & 0x3ff]);
	}

	return (0xaa);
}

void v20_set_uint8 (void *ext, unsigned long addr, unsigned char val)
{
	vic20_t *sim = ext;

	if ((addr >= 0x9000) && (addr <= 0x900f)) {
		e6560_set_reg (&sim->video.vic, addr - 0x9000, val);
	}
	else if ((addr >= 0x9100) && (addr <= 0x93ff)) {
		if ((addr & 0x30) == 0x10) {
			e6522_set_uint8 (&sim->via1, addr & 0x0f, val);
		}
		else if ((addr & 0x30) == 0x20) {
			e6522_set_uint8 (&sim->via2, addr & 0x0f, val);
		}
	}
	else if ((addr >= 0x9400) && (addr <= 0x97ff)) {
		sim->video.colram[addr & 0x3ff] = val & 0x0f;
	}
}

static
unsigned char v20_get_uint8_debug (void *ext, unsigned long addr)
{
	vic20_t *sim = ext;

	return (mem_get_uint8 (sim->mem, addr));
}

static
void v20_set_uint8_debug (void *ext, unsigned long addr, unsigned char val)
{
	vic20_t *sim = ext;

	mem_set_uint8 (sim->mem, addr, val);
}

void v20_debug_mem (vic20_t *sim)
{
	e6502_set_mem_map_rd (sim->cpu, 0x0000, 0xffff, NULL);
	e6502_set_mem_map_wr (sim->cpu, 0x0000, 0xffff, NULL);

	e6502_set_mem_f (sim->cpu, sim, v20_get_uint8_debug, v20_set_uint8_debug);
}

static
void v20_set_bit (unsigned char *ptr, unsigned char bit, int val)
{
	if (val) {
		*ptr |= bit;
	}
	else {
		*ptr &= ~bit;
	}
}

void v20_set_via1_irq (vic20_t *sim, unsigned char val)
{
	sim->irq_via1 = (val != 0);

	if (val) {
		sim_log_deb ("6502: NMI\n");
	}

	e6502_set_nmi (sim->cpu, val);
}

void v20_set_via1_ca2 (vic20_t *sim, unsigned char val)
{
	cas_set_motor (&sim->cas, val == 0);
}

void v20_set_via1_ora (vic20_t *sim, unsigned char val)
{
	sim->ora1 = val;
}

void v20_set_via1_orb (vic20_t *sim, unsigned char val)
{
	sim->orb1 = val;
}

void v20_set_via2_irq (vic20_t *sim, unsigned char val)
{
	sim->irq_via2 = (val != 0);

	e6502_set_irq (sim->cpu, sim->irq_via2);
}

void v20_set_via2_ora (vic20_t *sim, unsigned char val)
{
	if (sim->ora2 == val) {
		return;
	}

	sim->ora2 = val;

	v20_update_keys (sim);
}

void v20_set_via2_orb (vic20_t *sim, unsigned char val)
{
	if (sim->orb2 == val) {
		return;
	}

	sim->orb2 = val;

	cas_set_out (&sim->cas, ~val & 0x08);

	v20_update_keys (sim);
}

void v20_set_restore (vic20_t *sim, unsigned char val)
{
	e6522_set_ca1_inp (&sim->via1, (val == 0));
}

void v20_update_keys (vic20_t *sim)
{
	unsigned char row, col;

	row = ~(sim->orb2 & sim->via2.ddrb);
	col = ~(sim->ora2 & sim->via2.ddrb);

	v20_keybd_matrix (sim, &row, &col);

	sim->irb2 = ~row;

	if (sim->joymat & 0x02) {
		sim->irb2 &= ~0x80;
	}

	sim->via2.ira = ~col;
	sim->via2.irb = sim->irb2;
}

void v20_update_joystick (vic20_t *sim)
{
	v20_set_bit (&sim->ira1, 0x20, ~sim->joymat & 0x10); /* fire */
	v20_set_bit (&sim->ira1, 0x10, ~sim->joymat & 0x01); /* left */
	v20_set_bit (&sim->irb2, 0x80, ~sim->joymat & 0x02); /* right */
	v20_set_bit (&sim->ira1, 0x04, ~sim->joymat & 0x04); /* up */
	v20_set_bit (&sim->ira1, 0x08, ~sim->joymat & 0x08); /* down */

	e6522_set_ira_inp (&sim->via1, sim->ira1);
	e6522_set_irb_inp (&sim->via2, sim->irb2);
}

void v20_cas_set_play (vic20_t *sim, unsigned char val)
{
	if (val) {
		sim->ira1 &= ~0x40;
	}
	else {
		sim->ira1 |= 0x40;
	}

	e6522_set_ira_inp (&sim->via1, sim->ira1);
}

void v20_cas_set_inp (vic20_t *sim, unsigned char val)
{
	e6522_set_ca1_inp (&sim->via2, val != 0);
}

void v20_reset (vic20_t *sim)
{
	sim_log_deb ("vic20: reset\n");

	e6502_reset (sim->cpu);
	e6522_reset (&sim->via1);
	e6522_reset (&sim->via2);

	v20_keybd_reset (sim);
	v20_video_reset (&sim->video);

	v20_clock_resync (sim);
}

void v20_set_keypad_mode (vic20_t *sim, int mode)
{
	if (mode < 0) {
		mode = (sim->keypad_joystick == 0);
	}
	else {
		mode = (mode != 0);
	}

	if (sim->keypad_joystick != mode) {
		sim_log_deb ("keypad mode: %s\n", mode ? "joystick" : "keypad");
		sim->keypad_joystick = (mode != 0);
	}
}

void v20_stop (vic20_t *sim)
{
	sim->brk = PCE_BRK_STOP;
}

void v20_set_speed_tape (vic20_t *sim, unsigned char val)
{
	if (val) {
		sim->speed_tape = 1;
		sim->framedrop_tape = 4;
	}
	else {
		sim->speed_tape = 0;
		sim->framedrop_tape = 0;
	}

	v20_set_speed (sim, sim->speed_base);
}

void v20_set_speed (vic20_t *sim, unsigned speed)
{
	int      report;
	unsigned drop;

	report = (sim->speed_base != speed);

	drop = sim->framedrop_base;

	if (sim->framedrop_tape > drop) {
		drop = sim->framedrop_tape;
	}

	v20_video_set_framedrop (&sim->video, drop);

	sim->speed_base = speed;

	if (sim->speed_auto && sim->speed_tape) {
		speed = 0;
	}
	else {
		speed = sim->speed_base;
	}

	if (sim->speed == speed) {
		return;
	}

	sim->speed = speed;

	if (report) {
		pce_log (MSG_INF, "set speed to %u\n", sim->speed);
	}

	v20_clock_resync (sim);
}

void v20_clock_resync (vic20_t *sim)
{
	sim->sync_clk = 0;
	sim->sync_us = 0;
	sim->sync_sleep = 0;
	pce_get_interval_us (&sim->sync_us);
}

static
void v20_clock_sync (vic20_t *sim, unsigned long n)
{
	unsigned long fct;
	unsigned long us1, us2, sl;

	if (sim->speed == 0) {
		return;
	}

	sim->sync_clk += n;

	fct = (sim->speed * sim->clock) / V20_CPU_SYNC;

	if (sim->sync_clk < fct) {
		return;
	}

	sim->sync_clk -= fct;

	us1 = pce_get_interval_us (&sim->sync_us);
	us2 = (1000000 / V20_CPU_SYNC);

	if (us1 < us2) {
		sim->sync_sleep += us2 - us1;
	}
	else {
		sim->sync_sleep -= us1 - us2;
	}

	if (sim->sync_sleep >= (1000000 / V20_CPU_SYNC)) {
		pce_usleep (1000000 / V20_CPU_SYNC);
		sl = pce_get_interval_us (&sim->sync_us);
		sim->sync_sleep -= sl;
	}

	if (sim->sync_sleep < -1000000) {
		pce_log (MSG_INF, "system too slow, skipping 1 second\n");
		sim->sync_sleep += 1000000;
	}
}

void v20_clock (vic20_t *sim)
{
	e6502_clock (sim->cpu, 1);
	e6560_clock (&sim->video.vic);
	e6522_clock (&sim->via1, 1);
	e6522_clock (&sim->via2, 1);
	cas_clock (&sim->cas);

	sim->clk_div += 1;

	if (sim->clk_div < 4096) {
		return;
	}

	sim->clk_div -= 4096;

	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	v20_clock_sync (sim, 4096);
}
