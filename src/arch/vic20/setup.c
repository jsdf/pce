/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/setup.c                                       *
 * Created:     2020-04-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2022 Hampa Hug <hampa@hampa.ch>                     *
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
#include "vic20.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>

#include <chipset/e6522.h>

#include <cpu/e6502/e6502.h>

#include <devices/cassette.h>
#include <devices/memory.h>

#include <drivers/pti/pti-io.h>
#include <drivers/sound/sound.h>
#include <drivers/video/terminal.h>

#include <lib/brkpt.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/load.h>
#include <lib/log.h>

#include <libini/libini.h>


#define V20_CLOCK_PAL  ((443361875 + 200) / 400)
#define V20_CLOCK_NTSC (14318181 / 14)


static
void v20_setup_vic20 (vic20_t *sim, ini_sct_t *ini)
{
	unsigned  speed;
	int       pal, aspeed;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_uint16 (sct, "speed", &speed, 1);
	ini_get_bool (sct, "pal", &pal, 0);
	ini_get_bool (sct, "auto_speed", &aspeed, 1);

	sim->pal = (pal != 0);
	sim->clock = sim->pal ? V20_CLOCK_PAL : V20_CLOCK_NTSC;

	sim->speed = speed;
	sim->speed_base = speed;
	sim->speed_auto = (aspeed != 0);
	sim->speed_tape = 0;

	sim->framedrop_base = 0;
	sim->framedrop_tape = 0;

	pce_log_tag (MSG_INF, "VIC20:",
		"cpu=6502 clock=%lu speed=%u autospeed=%d pal=%d\n",
		sim->clock, sim->speed, sim->speed_auto, sim->pal
	);

	if ((sim->cpu = e6502_new()) == NULL) {
		pce_log (MSG_ERR, "*** creating the cpu failed\n");
		return;
	}

	e6502_set_flags (sim->cpu, E6502_FLAG_UNDEF);

	if ((sim->mem = mem_new()) == NULL) {
		pce_log (MSG_ERR, "*** creating memory failed\n");
		return;
	}

	e6502_set_mem_f (sim->cpu, sim->mem, mem_get_uint8, mem_set_uint8);
}

static
void v20_setup_mem (vic20_t *sim, ini_sct_t *ini)
{
	mem_set_fct (sim->mem, sim,
		v20_get_uint8, NULL, NULL,
		v20_set_uint8, NULL, NULL
	);

	ini_get_ram (sim->mem, ini, NULL);
	ini_get_rom (sim->mem, ini);
}

static
void v20_setup_via (vic20_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "VIA:", "initialized\n");

	sim->ira1 = 0xfe;
	sim->irb2 = 0xff;

	e6522_init (&sim->via1, 0);
	e6522_set_irq_fct (&sim->via1, sim, v20_set_via1_irq);
	e6522_set_ora_fct (&sim->via1, sim, v20_set_via1_ora);
	e6522_set_orb_fct (&sim->via1, sim, v20_set_via1_orb);
	e6522_set_ca2_fct (&sim->via1, sim, v20_set_via1_ca2);

	e6522_init (&sim->via2, 0);
	e6522_set_irq_fct (&sim->via2, sim, v20_set_via2_irq);
	e6522_set_ora_fct (&sim->via2, sim, v20_set_via2_ora);
	e6522_set_orb_fct (&sim->via2, sim, v20_set_via2_orb);

	e6522_set_ca1_inp (&sim->via1, 1);
	e6522_set_ira_inp (&sim->via1, sim->ira1);
	e6522_set_irb_inp (&sim->via2, sim->irb2);
}

static
void v20_setup_video (vic20_t *sim, ini_sct_t *ini)
{
	int           pal, border;
	int           hue;
	unsigned      sat, brt, drop;
	unsigned long srate;
	const char    *snd;
	ini_sct_t     *sct;

	sct = ini_next_sct (ini, NULL, "video");

	ini_get_bool (sct, "pal", &pal, sim->pal);
	ini_get_bool (sct, "border", &border, 0);
	ini_get_sint16 (sct, "hue", &hue, 0);
	ini_get_uint16 (sct, "saturation", &sat, 50);
	ini_get_uint16 (sct, "brightness", &brt, 100);
	ini_get_uint16 (sct, "framedrop", &drop, 0);
	ini_get_string (sct, "sound", &snd, NULL);
	ini_get_uint32 (sct, "sample_rate", &srate, 44100);

	pce_log_tag (MSG_INF, "VIC:",
		"pal=%d drop=%u hue=%d sat=%u brt=%u srate=%lu\n",
		pal, drop, hue, sat, brt, srate
	);

	if (snd != NULL) {
		pce_log_tag (MSG_INF, "VIC:", "sound=%s\n", snd);
	}

	sim->framedrop_base = drop;

	v20_video_init (&sim->video);

	v20_video_set_clock (&sim->video, sim->clock);
	v20_video_set_srate (&sim->video, srate);

	v20_video_set_pal (&sim->video, pal, border);
	v20_video_set_memmap (&sim->video, sim->mem);

	v20_video_set_framedrop (&sim->video, drop);

	v20_video_set_hue (&sim->video, hue);
	v20_video_set_saturation (&sim->video, sat / 100.0);
	v20_video_set_brightness (&sim->video, brt / 100.0);

	if (v20_video_set_sound_driver (&sim->video, snd)) {
		pce_log (MSG_ERR, "*** sound driver failed\n");
	}
}

static
void v20_setup_terminal (vic20_t *sim, ini_sct_t *ini)
{
	sim->trm = ini_get_terminal (ini, par_terminal);

	if (sim->trm == NULL) {
		return;
	}

	trm_set_msg_fct (sim->trm, sim, v20_set_msg);
	trm_set_key_fct (sim->trm, sim, v20_keybd_set_key);

	v20_video_set_term (&sim->video, sim->trm);
}

static
void v20_setup_keybd (vic20_t *sim, ini_sct_t *ini)
{
	unsigned  i;
	int       joy;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "keyboard");

	ini_get_bool (sct, "keypad_joystick", &joy, 0);

	sim->keypad_joystick = (joy != 0);

	pce_log_tag (MSG_INF, "KEYBOARD:", "joystick=%d\n",
		sim->keypad_joystick
	);

	sim->joymat = 0;

	for (i = 0; i < 8; i++) {
		sim->keymat[i] = 0;
	}
}

static
void v20_setup_datasette (vic20_t *sim, ini_sct_t *ini)
{
	const char    *read_name, *write_name;
	unsigned long delay;
	ini_sct_t     *sct;

	cas_init (&sim->cas);

	if ((sct = ini_next_sct (ini, NULL, "datasette")) == NULL) {
		return;
	}

	ini_get_string (sct, "file", &write_name, NULL);
	ini_get_string (sct, "write", &write_name, write_name);
	ini_get_string (sct, "read", &read_name, write_name);
	ini_get_uint32 (sct, "motor_delay", &delay, 50);

	pce_log_tag (MSG_INF, "CASSETTE:", "read=%s write=%s motor_delay=%lu\n",
		(read_name != NULL) ? read_name : "<none>",
		(write_name != NULL) ? write_name : "<none>",
		delay
	);

	delay = (unsigned long) (((double) delay * sim->clock) / 1000.0);

	cas_set_inp_fct (&sim->cas, sim, v20_cas_set_inp);
	cas_set_play_fct (&sim->cas, sim, v20_cas_set_play);
	cas_set_run_fct (&sim->cas, sim, v20_set_speed_tape);

	cas_set_clock (&sim->cas, sim->clock);
	cas_set_motor_delay (&sim->cas, delay);

	pti_set_default_clock (sim->clock);

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
void v20_set_memmap (vic20_t *sim)
{
	unsigned  i;
	mem_blk_t *blk;

	e6502_set_mem_map_rd (sim->cpu, 0x0000, 0xffff, NULL);
	e6502_set_mem_map_wr (sim->cpu, 0x0000, 0xffff, NULL);

	for (i = 0; i < sim->mem->cnt; i++) {
		blk = sim->mem->lst[i].blk;

		if (blk->data == NULL) {
			continue;
		}

		e6502_set_mem_map_rd (sim->cpu, blk->addr1, blk->addr2, blk->data);

		if (blk->readonly) {
			continue;
		}

		e6502_set_mem_map_wr (sim->cpu, blk->addr1, blk->addr2, blk->data);
	}
}

vic20_t *v20_new (ini_sct_t *ini)
{
	vic20_t *sim;

	if ((sim = malloc (sizeof (vic20_t))) == NULL) {
		return (NULL);
	}

	memset (sim, 0, sizeof (vic20_t));

	sim->cfg = ini;

	sim->brk = 0;
	sim->irq_via1 = 0;
	sim->irq_via2 = 0;
	sim->clk_div = 0;

	bps_init (&sim->bps);

	v20_setup_vic20 (sim, ini);
	v20_setup_mem (sim, ini);
	v20_setup_via (sim, ini);
	v20_setup_video (sim, ini);
	v20_setup_keybd (sim, ini);
	v20_setup_datasette (sim, ini);
	v20_setup_terminal (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

	v20_set_memmap (sim);

	v20_set_msg (sim, "term.title", "VIC-20");

	/* v20_debug_mem (sim); */

	return (sim);
}

void v20_del (vic20_t *sim)
{
	if (sim == NULL) {
		return;
	}

	cas_free (&sim->cas);

	v20_video_free (&sim->video);

	trm_del (sim->trm);

	e6522_free (&sim->via2);
	e6522_free (&sim->via1);

	e6502_del (sim->cpu);

	mem_del (sim->mem);

	bps_free (&sim->bps);

	free (sim);
}
