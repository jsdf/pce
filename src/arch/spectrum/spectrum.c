/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/spectrum.c                                 *
 * Created:     2021-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "keyboard.h"
#include "msg.h"
#include "snapshot.h"
#include "spectrum.h"
#include "video.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <cpu/e8080/e8080.h>

#include <devices/cassette.h>
#include <devices/memory.h>

#include <drivers/block/block.h>
#include <drivers/char/char.h>
#include <drivers/pti/pti-io.h>
#include <drivers/video/keys.h>
#include <drivers/video/terminal.h>

#include <libini/libini.h>

#include <lib/brkpt.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <lib/path.h>
#include <lib/string.h>
#include <lib/sysdep.h>


#define SPEC_CPU_SYNC 25


extern const char *par_snapshot;


void spec_set_port_fe (spectrum_t *sim, unsigned char val)
{
	if ((sim->port_fe ^ val) & 0x08) {
		cas_set_out (&sim->cas, ~val & 0x08);
	}

	if ((sim->port_fe ^ val) & 0x10) {
		spk_set_input (&sim->spk, (val & 0x10) ? 1 : 0);
	}

	sim->port_fe = val;

	spec_video_set_bg (&sim->video, val & 7);
}

static
void spec_set_port8 (spectrum_t *sim, unsigned long addr, unsigned char val)
{
	if (addr == 0xfe) {
		spec_set_port_fe (sim, val);
	}
	else if (~addr & 4) {
		sim_log_deb ("OUT %04lX, %02X\n", addr, val);
		; /* zx printer */
	}
	else if (~addr & 1) {
		sim_log_deb ("OUT %04lX, %02X\n", addr, val);
		spec_set_port_fe (sim, val);
	}
#if 0
	else {
		sim_log_deb ("OUT %04lX, %02X\n", addr, val);
	}
#endif
}

static
unsigned char spec_get_port8 (spectrum_t *sim, unsigned long addr)
{
	unsigned      addr8;
	unsigned char val;

	addr8 = addr & 0xff;

	if (~addr & 1) {
		val = ~spec_kbd_get_keys (sim, ~(addr >> 8) & 0xff);
		val &= ~0x40;
		val |= sim->cas_inp ? 0x40 : 0x00;

		if ((addr & 0xff) != 0xfe) {
			sim_log_deb ("IN %04lX -> %02X\n", addr, val);
		}
	}
	else if (~addr & 4) {
		val = 0xff; /* zx printer */
	}
	else if (addr8 == 0x1f) {
		val = sim->joystate;
	}
	else if ((addr8 == 0xff) || (addr8 == 0x9f)) {
		val = spec_video_get_data (&sim->video);
	}
	else {
		val = 0xff;
#ifdef DEBUG_PORTS
		sim_log_deb ("IN %04lX -> %02X\n", addr, val);
#endif
	}

	return (val);
}

void spec_cas_set_inp (spectrum_t *sim, unsigned char val)
{
	sim->cas_inp = (val != 0);
}

unsigned long spec_get_clock (spectrum_t *sim)
{
	return (sim->clk_cnt);
}

static
void spec_magic (spectrum_t *sim, pce_key_t key)
{
	switch (key) {
	case PCE_KEY_F1:
		spec_set_speed (sim, 1);
		break;

	case PCE_KEY_F2:
		spec_set_speed (sim, 4);
		break;

	case PCE_KEY_F3:
		spec_set_speed (sim, 8);
		break;

	case PCE_KEY_F4:
		spec_set_speed (sim, 0);
		break;

	case PCE_KEY_F5:
		spec_snapshot (sim);
		break;

	case PCE_KEY_F8:
		spec_stop (sim);
		break;

	case PCE_KEY_F9:
		spec_set_msg (sim, "emu.cas.play", "");
		break;

	case PCE_KEY_F10:
		spec_set_msg (sim, "emu.cas.record", "");
		break;

	case PCE_KEY_F11:
		spec_set_msg (sim, "emu.cas.load", "0");
		break;

	case PCE_KEY_F12:
		spec_set_msg (sim, "emu.cas.stop", "");
		break;

	case PCE_KEY_KP_5:
	case PCE_KEY_ESC:
		spec_kbd_set_keypad_mode (sim, (sim->keypad_mode + 1) % 3);
		break;

	case PCE_KEY_K:
		spec_kbd_set_kempston (sim);
		break;

	default:
		sim_log_deb ("unknown magic key (%u)\n", key);
		break;
	}
}

static
void spec_set_key (spectrum_t *sim, unsigned event, pce_key_t key)
{
	switch (key) {
	case PCE_KEY_F1:
	case PCE_KEY_F2:
	case PCE_KEY_F3:
	case PCE_KEY_F4:
	case PCE_KEY_F5:
	case PCE_KEY_F8:
	case PCE_KEY_F9:
	case PCE_KEY_F10:
	case PCE_KEY_F11:
	case PCE_KEY_F12:
		if (event == PCE_KEY_EVENT_DOWN) {
			spec_magic (sim, key);
		}
		return;

	default:
		break;
	}

	switch (event) {
	case PCE_KEY_EVENT_MAGIC:
		spec_magic (sim, key);
		break;

	case PCE_KEY_EVENT_DOWN:
		spec_kbd_set_key (sim, 1, key);
		break;

	case PCE_KEY_EVENT_UP:
		spec_kbd_set_key (sim, 0, key);
		break;
	}
}

static
void spec_set_mouse (spectrum_t *sim, int dx, int dy, unsigned button)
{
}

static
void spec_setup_system (spectrum_t *sim, ini_sct_t *ini)
{
	unsigned   keypad;
	const char *model;
	ini_sct_t  *sct;

	sim->brk = 0;
	sim->clk_cnt = 0;
	sim->clk_div = 0;

	sim->model = SPEC_MODEL_16;
	sim->port_fe = 0x00;
	sim->keypad_mode = 0;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "model", &model, "48K");
	ini_get_uint16 (sct, "keypad", &keypad, 0);
	ini_get_uint16 (sct, "video_w", &sim->video_w, 256);
	ini_get_uint16 (sct, "video_h", &sim->video_h, 192);

	pce_log_tag (MSG_INF,
		"SYSTEM:",
		"model=%s keypad=%u\n",
		model, keypad
	);

	if (strcmp (model, "16K") == 0) {
		sim->model = SPEC_MODEL_16;
	}
	else if (strcmp (model, "48K") == 0) {
		sim->model = SPEC_MODEL_48;
	}
	else {
		pce_log (MSG_ERR, "*** unknown machine model (%s)\n", model);
	}

	sim->keypad_mode = keypad;
}

static
void spec_setup_mem (spectrum_t *sim, ini_sct_t *ini)
{
	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, NULL);
	ini_get_rom (sim->mem, ini);
}

static
void spec_setup_cpu (spectrum_t *sim, ini_sct_t *ini)
{
	unsigned      speed;
	unsigned long clock;
	ini_sct_t     *sct;

	sct = ini_next_sct (ini, NULL, "system");

	sim->speed = 0;

	if (ini_get_uint32 (sct, "clock", &clock, 0)) {
		ini_get_uint16 (sct, "speed", &speed, 1);

		clock = SPEC_CLOCK * (unsigned long) speed;

		sim->speed = speed;
	}

	pce_log_tag (MSG_INF, "CPU:", "Z80 clock=%lu\n", clock);

	if ((sim->cpu = e8080_new()) == NULL) {
		pce_log (MSG_ERR, "*** failed to create the CPU\n");
		return;
	}

	e8080_set_mem_fct (sim->cpu, sim->mem, &mem_get_uint8, &mem_set_uint8);
	e8080_set_port_fct (sim->cpu, sim, spec_get_port8, spec_set_port8);

	e8080_set_z80 (sim->cpu);

	sim->clock_base = clock;
	sim->clock_freq = clock;

	{
		void *ptr;

		if ((ptr = mem_get_ptr (sim->mem, 0, 16384)) != NULL) {
			e8080_set_mem_map_rd (sim->cpu, 0, 16383, ptr);
		}

		if ((ptr = mem_get_ptr (sim->mem, 32768, 32768)) != NULL) {
			e8080_set_mem_map_rd (sim->cpu, 32768, 32767, ptr);
			e8080_set_mem_map_wr (sim->cpu, 32768, 32767, ptr);
		}
	}
}

static
void spec_setup_terminal (spectrum_t *sim, ini_sct_t *ini)
{
	sim->term = ini_get_terminal (ini, par_terminal);

	if (sim->term == NULL) {
		return;
	}

	trm_set_msg_fct (sim->term, sim, spec_set_msg);
	trm_set_key_fct (sim->term, sim, spec_set_key);
	trm_set_mouse_fct (sim->term, sim, spec_set_mouse);
}

static
void spec_setup_video (spectrum_t *sim, ini_sct_t *ini)
{
	const unsigned char *vram;

	pce_log_tag (MSG_INF, "VIDEO:", "w=%u h=%u\n",
		sim->video_w, sim->video_h
	);

	spec_video_init (&sim->video);

	vram = mem_get_ptr (sim->mem, 0x4000, 6912);

	spec_video_set_vram (&sim->video, vram);
	spec_video_set_size (&sim->video, sim->video_w, sim->video_h);
	spec_video_set_irq_fct (&sim->video, sim->cpu, e8080_set_int);

	if (sim->term != NULL) {
		spec_video_set_terminal (&sim->video, sim->term, 1);
	}
}

static
void spec_setup_cassette (spectrum_t *sim, ini_sct_t *ini)
{
	const char *read_name, *write_name;
	ini_sct_t  *sct;

	cas_init (&sim->cas);

	if ((sct = ini_next_sct (ini, NULL, "cassette")) == NULL) {
		return;
	}

	ini_get_string (sct, "file", &write_name, NULL);
	ini_get_string (sct, "write", &write_name, write_name);
	ini_get_string (sct, "read", &read_name, write_name);

	pce_log_tag (MSG_INF, "CASSETTE:", "read=%s write=%s\n",
		(read_name != NULL) ? read_name : "<none>",
		(write_name != NULL) ? write_name : "<none>"
	);

	cas_set_clock (&sim->cas, sim->clock_base);
	cas_set_auto_motor (&sim->cas, 1);
	cas_set_inp_fct (&sim->cas, sim, spec_cas_set_inp);

	pti_set_default_clock (sim->clock_base);

	sim->cas_inp = 1;

	if (cas_set_read_name (&sim->cas, read_name)) {
		pce_log (MSG_ERR, "*** opening read file failed (%s)\n",
			read_name
		);
	}

	if (cas_set_write_name (&sim->cas, write_name, 1)) {
		pce_log (MSG_ERR, "*** opening write file failed (%s)\n",
			write_name
		);
	}
}

static
void spec_setup_speaker (spectrum_t *sim, ini_sct_t *ini)
{
	const char    *driver;
	unsigned      volume;
	unsigned long srate;
	unsigned long lowpass;
	ini_sct_t     *sct;

	spk_init (&sim->spk, 0);
	spk_set_clock_fct (&sim->spk, sim, spec_get_clock);
	spk_set_input_clock (&sim->spk, sim->clock_base);

	if ((sct = ini_next_sct (ini, NULL, "speaker")) == NULL) {
		return;
	}

	ini_get_string (sct, "driver", &driver, NULL);
	ini_get_uint16 (sct, "volume", &volume, 500);
	ini_get_uint32 (sct, "sample_rate", &srate, 44100);
	ini_get_uint32 (sct, "lowpass", &lowpass, 0);

	pce_log_tag (MSG_INF, "SPEAKER:", "volume=%u srate=%lu lowpass=%lu driver=%s\n",
		volume, srate, lowpass,
		(driver != NULL) ? driver : "<none>"
	);

	if (driver != NULL) {
		if (spk_set_driver (&sim->spk, driver, srate)) {
			pce_log (MSG_ERR,
				"*** setting sound driver failed (%s)\n",
				driver
			);
		}
	}

	spk_set_lowpass (&sim->spk, lowpass);
	spk_set_volume (&sim->spk, volume);
}

static
int file_exists (const char *fname)
{
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	fclose (fp);

	return (1);
}

static
void spec_setup_snap (spectrum_t *sim, ini_sct_t *ini)
{
	const char *snap;
	ini_sct_t  *sct;

	if ((sct = ini_next_sct (ini, NULL, "system")) == NULL) {
		return;
	}

	if (ini_get_string (sct, "snapshot", &snap, NULL)) {
		return;
	}

	if (par_snapshot != NULL) {
		snap = par_snapshot;
	}

	if (file_exists (snap) == 0) {
		return;
	}

	if (spec_snap_load (sim, snap)) {
		return;
	}
}

spectrum_t *spec_new (ini_sct_t *ini)
{
	spectrum_t *sim;

	if ((sim = malloc (sizeof (spectrum_t))) == NULL) {
		return (NULL);
	}

	memset (sim, 0, sizeof (spectrum_t));

	bps_init (&sim->bps);

	spec_setup_system (sim, ini);
	spec_setup_mem (sim, ini);
	spec_setup_cpu (sim, ini);
	spec_setup_terminal (sim, ini);
	spec_setup_video (sim, ini);
	spec_setup_cassette (sim, ini);
	spec_setup_speaker (sim, ini);
	pce_load_mem_ini (sim->mem, ini);
	spec_kbd_set_keypad_mode (sim, sim->keypad_mode);

	spec_reset (sim);
	spec_setup_snap (sim, ini);

	return (sim);
}

void spec_del (spectrum_t *sim)
{
	if (sim == NULL) {
		return;
	}

	spk_free (&sim->spk);
	cas_free (&sim->cas);
	spec_video_free (&sim->video);
	trm_del (sim->term);
	e8080_del (sim->cpu);
	mem_del (sim->mem);
	bps_free (&sim->bps);

	free (sim);
}

void spec_stop (spectrum_t *sim)
{
	spec_set_msg (sim, "emu.stop", NULL);
}

void spec_reset (spectrum_t *sim)
{
	sim_log_deb ("system reset\n");

	spec_kbd_reset (sim);
	spec_video_reset (&sim->video);
	e8080_reset (sim->cpu);
}

void spec_idle (spectrum_t *sim)
{
	pce_usleep (10000);

	spec_clock_discontinuity (sim);
}

void spec_clock_discontinuity (spectrum_t *sim)
{
	sim->sync_clk = 0;
	sim->sync_us = 0;
	sim->sync_sleep = 0;

	pce_get_interval_us (&sim->sync_us);
}

void spec_set_clock (spectrum_t *sim, unsigned long clock)
{
	unsigned drop;

	sim->clock_freq = clock;

	if (clock == 0) {
		spec_video_set_frame_drop (&sim->video, 16);
	}
	else {
		drop = clock / sim->clock_base;

		if (drop > 0) {
			drop -= 1;
		}

		spec_video_set_frame_drop (&sim->video, drop);
	}

	spec_clock_discontinuity (sim);
}

void spec_set_speed (spectrum_t *sim, unsigned speed)
{
	if (sim->speed == speed) {
		return;
	}

	sim->speed = speed;

	if (speed == 0) {
		sim_log_deb ("set speed to max\n");
	}
	else {
		sim_log_deb ("set speed to %u\n", speed);
	}

	spec_set_clock (sim, speed * sim->clock_base);
}

static
void spec_realtime_sync (spectrum_t *sim, unsigned long n)
{
	unsigned long fct;
	unsigned long us1, us2, sl;

	if (sim->clock_freq == 0) {
		return;
	}

	sim->sync_clk += n;

	fct = sim->clock_freq / SPEC_CPU_SYNC;

	if (sim->sync_clk < fct) {
		return;
	}

	sim->sync_clk -= fct;

	us1 = pce_get_interval_us (&sim->sync_us);
	us2 = (1000000 / SPEC_CPU_SYNC);

	if (us1 < us2) {
		sim->sync_sleep += us2 - us1;
	}
	else {
		sim->sync_sleep -= us1 - us2;
	}

	if (sim->sync_sleep >= (1000000 / SPEC_CPU_SYNC)) {
		pce_usleep (1000000 / SPEC_CPU_SYNC);
		sl = pce_get_interval_us (&sim->sync_us);
		sim->sync_sleep -= sl;
	}

	if (sim->sync_sleep < -1000000) {
		sim_log_deb ("system too slow, skipping 1 second\n");
		sim->sync_sleep += 1000000;
	}
}

void spec_clock (spectrum_t *sim)
{
	sim->clk_cnt += 1;
	sim->clk_div += 1;

	spec_video_clock (&sim->video);
	cas_clock (&sim->cas);

	if (sim->clk_div >= 16384) {
		sim->clk_div -= 16384;

		spk_check (&sim->spk);

		spec_realtime_sync (sim, 16384);

		if (sim->term != NULL) {
			trm_check (sim->term);
		}
	}

	e8080_clock (sim->cpu, 1);
}
