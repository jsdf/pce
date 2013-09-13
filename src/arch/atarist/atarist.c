/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/atarist.c                                   *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "atarist.h"
#include "acsi.h"
#include "ikbd.h"
#include "mem.h"
#include "msg.h"
#include "psg.h"
#include "rp5c15.h"
#include "smf.h"
#include "video.h"

#include <string.h>

#include <cpu/e68000/e68000.h>

#include <chipset/e6850.h>
#include <chipset/e68901.h>
#include <chipset/e8530.h>
#include <chipset/wd179x.h>

#include <drivers/block/block.h>
#include <drivers/char/char.h>
#include <drivers/video/terminal.h>
#include <drivers/video/keys.h>

#include <lib/brkpt.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/sysdep.h>

#include <libini/libini.h>


/* The CPU is synchronized with real time ST_CPU_SYNC times per seconds */
#define ST_CPU_SYNC 250

#ifdef PCE_HOST_WINDOWS
#define ST_CPU_SLEEP 20000
#else
#define ST_CPU_SLEEP 10000
#endif

#define ST_VIDEO_HB 0x01
#define ST_VIDEO_VB 0x02


void st_interrupt (atari_st_t *sim, unsigned level, int val)
{
	unsigned i, v;
	unsigned mask;

	mask = 1U << level;

	if (val) {
		if (sim->int_mask & mask) {
			return;
		}

		sim->int_mask |= mask;
	}
	else {
		if (~sim->int_mask & mask) {
			return;
		}

		sim->int_mask &= ~mask;
	}

	i = 0;
	v = sim->int_mask >> 1;

	while (v != 0) {
		i += 1;
		v >>= 1;
	}

	sim->int_level = i;

	e68_interrupt (sim->cpu, sim->int_level);
}

static
void st_interrupt_mfp (void *ext, unsigned char val)
{
	st_interrupt (ext, 6, val);
}

static
unsigned st_inta (atari_st_t *sim, unsigned level)
{
	if (level == 6) {
		return (e68901_inta (&sim->mfp));
	}
	else if (level == 4) {
		st_interrupt (sim, 4, 0);
	}
	else if (level == 2) {
		st_interrupt (sim, 2, 0);
	}

	return (-1);
}

static
void st_set_hb (atari_st_t *sim, unsigned char val)
{
	int de;

	if (val) {
		sim->video_state |= ST_VIDEO_HB;
	}
	else {
		sim->video_state &= ~ST_VIDEO_HB;
	}

	de = (sim->video_state & (ST_VIDEO_HB | ST_VIDEO_VB)) == 0;

	e68901_set_tbi (&sim->mfp, !de);

	st_interrupt (sim, 2, val);
}

static
void st_set_vb (atari_st_t *sim, unsigned char val)
{
	if (val) {
		sim->video_state |= ST_VIDEO_VB;
	}
	else {
		sim->video_state &= ~ST_VIDEO_VB;
	}

	st_interrupt (sim, 4, val);
}

static
int st_usart_send (atari_st_t *sim, unsigned char val)
{
	if (sim->serport_drv != NULL) {
		chr_write (sim->serport_drv, &val, 1);
	}

	return (0);
}

static
int st_usart_recv (atari_st_t *sim, unsigned char *val)
{
	if (sim->serport_drv == NULL) {
		return (1);
	}

	if (sim->ser_buf_i >= sim->ser_buf_n) {
		sim->ser_buf_i = 0;
		sim->ser_buf_n = chr_read (sim->serport_drv, sim->ser_buf, sizeof (sim->ser_buf));
	}

	if (sim->ser_buf_i >= sim->ser_buf_n) {
		return (1);
	}

	*val = sim->ser_buf[sim->ser_buf_i++];

	return (0);
}

static
void st_midi_send (atari_st_t *sim, unsigned char val)
{
	if (sim->midi_drv != NULL) {
		chr_write (sim->midi_drv, &val, 1);
	}

	st_smf_set_uint8 (&sim->smf, val, sim->clk_cnt);
}

static
void st_set_port_a (atari_st_t *sim, unsigned char val)
{
	if (~val & 2) {
		wd179x_select_drive (&sim->fdc.wd179x, 0);
	}

	if (~val & 4) {
		wd179x_select_drive (&sim->fdc.wd179x, 1);
	}

	wd179x_select_head (&sim->fdc.wd179x, ~val & 1, 0);

	if ((sim->psg_port_a ^ val) & ~val & 0x20) {
		if (sim->parport_drv != NULL) {
			chr_write (sim->parport_drv, &sim->psg_port_b, 1);
		}
	}

	sim->psg_port_a = val;
}

static
void st_set_port_b (atari_st_t *sim, unsigned char val)
{
	sim->psg_port_b = val;
}

static
void st_setup_system (atari_st_t *sim, ini_sct_t *ini)
{
	int        mono, fastboot;
	const char *model, *parport, *serport;
	ini_sct_t  *sct;

	if ((sct = ini_next_sct (ini, NULL, "system")) == NULL) {
		sct = ini;
	}

	sim->parport_drv = NULL;
	sim->serport_drv = NULL;

	ini_get_string (sct, "model", &model, "st");
	ini_get_bool (sct, "mono", &mono, 1);
	ini_get_bool (sct, "fastboot", &fastboot, 0);
	ini_get_string (sct, "parport", &parport, NULL);
	ini_get_string (sct, "serport", &serport, NULL);

	pce_log_tag (MSG_INF, "SYSTEM:", "model=%s fastboot=%d\n", model, fastboot);

	if (strcmp (model, "st") == 0) {
		sim->model = PCE_ST_ST;
	}
	else {
		pce_log (MSG_ERR, "*** unknown model (%s)\n", model);
		sim->model = PCE_ST_ST;
	}

	sim->video_state = 0;

	sim->mono = (mono != 0);
	sim->fastboot = (fastboot != 0);

	if (parport != NULL) {
		pce_log_tag (MSG_INF, "PARPORT:", "driver=%s\n", parport);

		sim->parport_drv = chr_open (parport);

		if (sim->parport_drv == NULL) {
			pce_log (MSG_ERR, "*** can't open driver (%s)\n", parport);
		}

	}

	if (serport != NULL) {
		pce_log_tag (MSG_INF, "SERPORT:", "driver=%s\n", serport);

		sim->serport_drv = chr_open (serport);

		if (sim->serport_drv == NULL) {
			pce_log (MSG_ERR, "*** can't open driver (%s)\n", serport);
		}

	}
}

static
void st_setup_mem (atari_st_t *sim, ini_sct_t *ini)
{
	sim->mem = mem_new();

	mem_set_fct (sim->mem, sim,
		st_mem_get_uint8, st_mem_get_uint16, st_mem_get_uint32,
		st_mem_set_uint8, st_mem_set_uint16, st_mem_set_uint32
	);

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	sim->ram = mem_get_blk (sim->mem, 0);

	if (sim->ram == NULL) {
		pce_log (MSG_ERR, "*** RAM not found at address 0\n");
	}

	if (mem_get_blk (sim->mem, 0xfc0000) != NULL) {
		sim->rom_addr = 0xfc0000;
	}
	else if (mem_get_blk (sim->mem, 0xe00000) != NULL) {
		sim->rom_addr = 0xe00000;
	}
	else {
		pce_log (MSG_ERR, "*** can't determine ROM address\n");
	}
}

static
void st_setup_cpu (atari_st_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *model;
	unsigned   speed;

	sct = ini_next_sct (ini, NULL, "cpu");

	ini_get_string (sct, "model", &model, "68000");
	ini_get_uint16 (sct, "speed", &speed, 0);

	pce_log_tag (MSG_INF, "CPU:", "model=%s speed=%d\n", model, speed);

	if ((sim->cpu = e68_new()) == NULL) {
		return;
	}

	if (st_set_cpu_model (sim, model)) {
		pce_log (MSG_ERR, "*** unknown cpu model (%s)\n", model);
	}

	e68_set_mem_fct (sim->cpu, sim->mem,
		&mem_get_uint8,
		&mem_get_uint16_be,
		&mem_get_uint32_be,
		&mem_set_uint8,
		&mem_set_uint16_be,
		&mem_set_uint32_be
	);

	e68_set_inta_fct (sim->cpu, sim, st_inta);

	e68_set_flags (sim->cpu, E68_FLAG_NORESET, 1);

	sim->speed_factor = speed;
}

static
void st_setup_midi (atari_st_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *raw, *smf;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "midi_raw", &raw, NULL);
	ini_get_string (sct, "midi_smf", &smf, NULL);

	st_smf_init (&sim->smf);

	sim->midi_drv = NULL;

	if (raw != NULL) {
		pce_log_tag (MSG_INF, "MIDI-RAW:", "driver=%s\n", raw);

		sim->midi_drv = chr_open (raw);

		if (sim->midi_drv == NULL) {
			pce_log (MSG_ERR, "*** can't open midi driver (%s)\n", raw);
		}
	}

	if (smf != NULL) {
		pce_log_tag (MSG_INF, "MIDI-SMF:", "file=%s\n", smf);

		if (st_smf_set_auto (&sim->smf, smf)) {
			pce_log (MSG_ERR, "*** can't open smf file (%s)\n", smf);
		}
	}
}

static
void st_setup_mfp (atari_st_t *sim, ini_sct_t *ini)
{
	mem_blk_t     *blk;
	unsigned long addr, size;

	addr = 0xfffa00;
	size = 0x40;

	pce_log_tag (MSG_INF, "MFP:", "addr=0x%06lx size=0x%lx\n", addr, size);

	e68901_init (&sim->mfp, 1);

	if ((blk = mem_blk_new (addr, size, 0)) == NULL) {
		return;
	}

	mem_blk_set_fct (blk, &sim->mfp,
		e68901_get_uint8, e68901_get_uint16, e68901_get_uint32,
		e68901_set_uint8, e68901_set_uint16, e68901_set_uint32
	);

	mem_add_blk (sim->mem, blk, 1);

	e68901_set_irq_fct (&sim->mfp, sim, st_interrupt_mfp);

	e68901_set_usart_timer (&sim->mfp, 3);
	e68901_set_send_fct (&sim->mfp, sim, st_usart_send);
	e68901_set_recv_fct (&sim->mfp, sim, st_usart_recv);

	e68901_set_clk_div (&sim->mfp, 13);

	if (sim->mono) {
		e68901_set_inp (&sim->mfp, 0x81);
	}
	else {
		e68901_set_inp (&sim->mfp, 0x01);
	}
}

static
void st_setup_acia (atari_st_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "ACIA0:", "addr=0x%06lx size=0x%lx\n", 0xfffc00, 4);

	e6850_init (&sim->acia0);
	e6850_set_irq_fct (&sim->acia0, &sim->mfp, e68901_set_inp_4);
	e6850_set_send_fct (&sim->acia0, &sim->kbd, st_kbd_set_uint8);
	e6850_set_recv_fct (&sim->acia0, &sim->kbd, st_kbd_get_uint8);

	pce_log_tag (MSG_INF, "ACIA1:", "addr=0x%06lx size=0x%lx\n", 0xfffc04, 4);

	e6850_init (&sim->acia1);
	e6850_set_irq_fct (&sim->acia1, &sim->mfp, e68901_set_inp_4);
	e6850_set_send_fct (&sim->acia1, sim, st_midi_send);
}

static
void st_setup_kbd (atari_st_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "IKBD:", "initialized\n");

	st_kbd_init (&sim->kbd);
}

static
void st_setup_rtc (atari_st_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "RTC:", "initialized\n");

	rp5c15_init (&sim->rtc);
}

static
void st_setup_psg (atari_st_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "PSG:", "initialized\n");

	st_psg_init (&sim->psg);
	st_psg_set_port_a_fct (&sim->psg, sim, st_set_port_a);
	st_psg_set_port_b_fct (&sim->psg, sim, st_set_port_b);
}

static
void st_setup_fdc (atari_st_t *sim, ini_sct_t *ini)
{
	const char *fname0, *fname1;
	ini_sct_t  *sct;

	sct = ini_next_sct (ini, NULL, "fdc");

	ini_get_string (sct, "file0", &fname0, NULL);
	ini_get_string (sct, "file1", &fname1, NULL);

	pce_log_tag (MSG_INF, "FDC:",
		"file0=%s file1=%s\n",
		(fname0 != NULL) ? fname0 : "<none>",
		(fname1 != NULL) ? fname1 : "<none>"
	);

	st_fdc_init (&sim->fdc);

	wd179x_set_irq_fct (&sim->fdc.wd179x, &sim->mfp, e68901_set_inp_5);
	wd179x_set_drq_fct (&sim->fdc.wd179x, &sim->dma, st_dma_set_dreq);

	wd179x_set_input_clock (&sim->fdc.wd179x, ST_CPU_CLOCK);
	wd179x_set_bit_clock (&sim->fdc.wd179x, 1000000);
	wd179x_set_auto_motor (&sim->fdc.wd179x, 1);

	st_fdc_set_disks (&sim->fdc, sim->dsks);

	st_fdc_set_fname (&sim->fdc, 0, fname0);
	st_fdc_set_fname (&sim->fdc, 1, fname1);

	st_fdc_set_disk_id (&sim->fdc, 0, 0);
	st_fdc_set_disk_id (&sim->fdc, 1, 1);
}

static
void st_setup_acsi (atari_st_t *sim, ini_sct_t *ini)
{
	unsigned i;

	pce_log_tag (MSG_INF, "ACSI:", "initialized\n");

	st_acsi_init (&sim->acsi);
	st_acsi_set_drq_fct (&sim->acsi, &sim->dma, st_dma_set_dreq);
	st_acsi_set_irq_fct (&sim->acsi, &sim->mfp, e68901_set_inp_5);
	st_acsi_set_disks (&sim->acsi, sim->dsks);

	for (i = 0; i < 8; i++) {
		st_acsi_set_disk_id (&sim->acsi, i, 128 + i);
	}
}

static
void st_setup_dma (atari_st_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "DMA:", "initialized\n");

	st_dma_init (&sim->dma);
	st_dma_set_memory (&sim->dma, sim->mem);
	st_dma_set_fdc (&sim->dma, &sim->fdc.wd179x);
	st_dma_set_acsi (&sim->dma, &sim->acsi);
}

static
void st_setup_disks (atari_st_t *sim, ini_sct_t *ini)
{
	sim->dsks = ini_get_disks (ini);
}

static
void st_setup_terminal (atari_st_t *sim, ini_sct_t *ini)
{
	sim->trm = ini_get_terminal (ini, par_terminal);

	if (sim->trm == NULL) {
		return;
	}

	trm_set_msg_fct (sim->trm, sim, st_set_msg);
	trm_set_key_fct (sim->trm, &sim->kbd, st_kbd_set_key);
	trm_set_mouse_fct (sim->trm, &sim->kbd, st_kbd_set_mouse);
}

static
void st_setup_video (atari_st_t *sim, ini_sct_t *ini)
{
	unsigned  skip;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_uint16 (sct, "frame_skip", &skip, 0);

	pce_log_tag (MSG_INF, "VIDEO:", "mono=%d frame_skip=%u\n",
		sim->mono, skip
	);

	if ((sim->video = st_video_new (0xff8200, sim->mono)) == NULL) {
		return;
	}

	st_video_set_memory (sim->video, sim->mem);
	st_video_set_hb_fct (sim->video, sim, st_set_hb);
	st_video_set_vb_fct (sim->video, sim, st_set_vb);
	st_video_set_frame_skip (sim->video, skip);

	if (sim->trm != NULL) {
		st_video_set_terminal (sim->video, sim->trm);
	}

	mem_add_blk (sim->mem, &sim->video->reg, 0);
}

void st_init (atari_st_t *sim, ini_sct_t *ini)
{
	unsigned i;

	sim->trm = NULL;
	sim->video = NULL;

	sim->reset = 0;

	sim->int_mask = 0;
	sim->int_level = 0;

	sim->pause = 0;
	sim->brk = 0;

	sim->speed_factor = 1;
	sim->speed_clock_extra = 0;

	sim->clk_cnt = 0;

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	sim->ser_buf_i = 0;
	sim->ser_buf_n = 0;

	bps_init (&sim->bps);

	st_setup_system (sim, ini);
	st_setup_mem (sim, ini);
	st_setup_cpu (sim, ini);
	st_setup_midi (sim, ini);
	st_setup_mfp (sim, ini);
	st_setup_acia (sim, ini);
	st_setup_kbd (sim, ini);
	st_setup_rtc (sim, ini);
	st_setup_psg (sim, ini);
	st_setup_disks (sim, ini);
	st_setup_fdc (sim, ini);
	st_setup_acsi (sim, ini);
	st_setup_dma (sim, ini);
	st_setup_terminal (sim, ini);
	st_setup_video (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

	if (sim->trm != NULL) {
		trm_set_msg_trm (sim->trm, "term.title", "pce-atarist");
	}

	st_clock_discontinuity (sim);
}

atari_st_t *st_new (ini_sct_t *ini)
{
	atari_st_t *sim;

	sim = malloc (sizeof (atari_st_t));

	if (sim == NULL) {
		return (NULL);
	}

	st_init (sim, ini);

	return (sim);
}

void st_free (atari_st_t *sim)
{
	if (sim == NULL) {
		return;
	}

	chr_close (sim->serport_drv);
	chr_close (sim->parport_drv);
	chr_close (sim->midi_drv);
	st_video_del (sim->video);
	trm_del (sim->trm);
	st_acsi_free (&sim->acsi);
	st_fdc_free (&sim->fdc);
	dsks_del (sim->dsks);
	st_psg_free (&sim->psg);
	rp5c15_free (&sim->rtc);
	e6850_free (&sim->acia1);
	e6850_free (&sim->acia0);
	e68901_free (&sim->mfp);
	st_smf_free (&sim->smf);
	e68_del (sim->cpu);
	mem_del (sim->mem);

	bps_free (&sim->bps);
}

void st_del (atari_st_t *sim)
{
	if (sim != NULL) {
		st_free (sim);
		free (sim);
	}
}


unsigned long long st_get_clkcnt (atari_st_t *sim)
{
	return (sim->clk_cnt);
}

void st_clock_discontinuity (atari_st_t *sim)
{
	sim->sync_clk = 0;
	sim->sync_us = 0;
	pce_get_interval_us (&sim->sync_us);

	sim->speed_clock_extra = 0;
}

void st_set_pause (atari_st_t *sim, int pause)
{
	sim->pause = (pause != 0);

	if (sim->pause == 0) {
		st_clock_discontinuity (sim);
	}
}

void st_set_speed (atari_st_t *sim, unsigned factor)
{
	st_log_deb ("speed = %u\n", factor);

	sim->speed_factor = factor;
	sim->speed_clock_extra = 0;

	st_clock_discontinuity (sim);
}

int st_set_msg_trm (atari_st_t *sim, const char *msg, const char *val)
{
	if (sim->trm == NULL) {
		return (1);
	}

	return (trm_set_msg_trm (sim->trm, msg, val));
}

int st_set_cpu_model (atari_st_t *sim, const char *model)
{
	if (strcmp (model, "68000") == 0) {
		e68_set_68000 (sim->cpu);
	}
	else if (strcmp (model, "68010") == 0) {
		e68_set_68010 (sim->cpu);
	}
	else if (strcmp (model, "68020") == 0) {
		e68_set_68020 (sim->cpu);
	}
	else {
		return (1);
	}

	return (0);
}

void st_reset (atari_st_t *sim)
{
	if (sim->reset) {
		return;
	}

	sim->reset = 1;

	st_log_deb ("st: reset\n");

	sim->int_mask = 0;
	sim->int_level = 0;

	e68901_reset (&sim->mfp);
	e6850_reset (&sim->acia0);
	e6850_reset (&sim->acia1);
	st_acsi_reset (&sim->acsi);
	st_fdc_reset (&sim->fdc);
	st_dma_reset (&sim->dma);
	st_kbd_reset (&sim->kbd);
	st_video_reset (sim->video);

	mem_set_uint32_be (sim->mem, 0, mem_get_uint32_be (sim->mem, sim->rom_addr));
	mem_set_uint32_be (sim->mem, 4, mem_get_uint32_be (sim->mem, sim->rom_addr + 4));

	e68_reset (sim->cpu);

	if (sim->fastboot) {
		mem_set_uint32_be (sim->mem, 0x0420, 0x752019f3);
		mem_set_uint32_be (sim->mem, 0x043a, 0x237698aa);
		mem_set_uint32_be (sim->mem, 0x051a, 0x5555aaaa);

		mem_set_uint8 (sim->mem, 0x0424, 5);
		mem_set_uint32_be (sim->mem, 0x042e, mem_blk_get_size (sim->ram));
		mem_set_uint32_be (sim->mem, 0x04ba, 16000);
	}

	st_clock_discontinuity (sim);

	sim->reset = 0;
}

static
void st_realtime_sync (atari_st_t *sim, unsigned long n)
{
	unsigned long us1, us2;

	sim->sync_clk += n;

	if (sim->sync_clk >= (ST_CPU_CLOCK / ST_CPU_SYNC)) {
		sim->sync_clk -= (ST_CPU_CLOCK / ST_CPU_SYNC);

		us1 = pce_get_interval_us (&sim->sync_us);
		us2 = (1000000 / ST_CPU_SYNC);

		if (us1 < us2) {
			sim->sync_sleep += us2 - us1;

			if (sim->sync_sleep > 0) {
				sim->speed_clock_extra += 1;
			}
		}
		else {
			sim->sync_sleep -= us1 - us2;

			if (sim->sync_sleep < 0) {
				if (sim->speed_clock_extra > 0) {
					sim->speed_clock_extra -= 1;
				}
			}
		}

		if (sim->sync_sleep >= ST_CPU_SLEEP) {
			pce_usleep (sim->sync_sleep);
		}

		if (sim->sync_sleep < -1000000) {
			st_log_deb ("system too slow, skipping 1 second\n");
			sim->sync_sleep += 1000000;
		}
	}
}

void st_clock (atari_st_t *sim, unsigned n)
{
	unsigned long clk, cpuclk;

	if (n == 0) {
		n = 16;
	}

	if (sim->speed_factor == 0) {
		cpuclk = n + sim->speed_clock_extra;
	}
	else {
		cpuclk = sim->speed_factor * n;
	}

	sim->clk_cnt += n;

	e68_clock (sim->cpu, cpuclk);

	st_video_clock (sim->video, n);

	sim->clk_div[0] += n;

	if (sim->clk_div[0] < 16) {
		return;
	}

	clk = sim->clk_div[0] & ~15UL;

	sim->clk_div[1] += clk;
	sim->clk_div[0] &= 15;

	st_fdc_clock (&sim->fdc, clk);

	e6850_clock (&sim->acia0, clk >> 4);
	e6850_clock (&sim->acia1, clk >> 4);

	e68901_clock (&sim->mfp, clk << 2);

	if (sim->clk_div[1] < 256) {
		return;
	}

	if (sim->ser_buf_i < sim->ser_buf_n) {
		if (e68901_receive (&sim->mfp, sim->ser_buf[sim->ser_buf_i]) == 0) {
			sim->ser_buf_i += 1;
		}
	}

	sim->clk_div[2] += sim->clk_div[1];
	sim->clk_div[1] = 0;

	if (sim->clk_div[2] < 8192) {
		return;
	}

	if (sim->ser_buf_i >= sim->ser_buf_n) {
		if (sim->serport_drv != NULL) {
			sim->ser_buf_i = 0;
			sim->ser_buf_n = chr_read (sim->serport_drv, sim->ser_buf, sizeof (sim->ser_buf));
		}
	}

	st_acsi_clock (&sim->acsi);

	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	if (sim->kbd.idle) {
		unsigned char val;

		if (st_kbd_buf_get (&sim->kbd, &val) == 0) {
			e6850_receive (&sim->acia0, val);
		}
	}

	st_fdc_clock_media_change (&sim->fdc, sim->clk_div[2]);

	st_realtime_sync (sim, sim->clk_div[2]);

	sim->clk_div[2] -= 8192;
}
