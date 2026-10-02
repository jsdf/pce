/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/rc759.c                                       *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2025 Hampa Hug <hampa@hampa.ch>                     *
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
#include "fdc.h"
#include "msg.h"
#include "nvm.h"
#include "parport.h"
#include "ports.h"
#include "rc759.h"
#include "rtc.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <lib/brkpt.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/sysdep.h>

#include <chipset/80186/dma.h>
#include <chipset/80186/icu.h>
#include <chipset/80186/tcu.h>

#include <chipset/82xx/e8255.h>
#include <chipset/82xx/e8259.h>

#include <cpu/e8086/e8086.h>

#include <devices/memory.h>

#include <drivers/block/block.h>

#include <drivers/video/terminal.h>

#include <libini/libini.h>


#define PCE_IBMPC_SLEEP 25000


static char *par_intlog[256];


static
void rc759_set_timer0_out (rc759_t *sim, unsigned char val)
{
	sim->ppi_port_a |= 1;

	if (sim->ppi_port_c & 1) {
		if (val) {
			sim->ppi_port_a &= 0xfe;
		}
	}

	e8255_set_inp_a (&sim->ppi, sim->ppi_port_a);
}

static
void rc759_set_timer1_out (rc759_t *sim, unsigned char val)
{
	rc759_spk_set_out (&sim->spk, val);
}

static
void rc759_set_ppi_port_c (rc759_t *sim, unsigned char val)
{
	sim->ppi_port_c = val;

	rc759_kbd_set_enable (&sim->kbd, val & 0x80);
	e82730_set_graphic (&sim->crt, (val & 0x40) == 0);
}

static
void rc759_set_mouse (void *ext, int dx, int dy, unsigned button)
{
	rc759_t *sim = ext;

	chr_mouse_set (dx, dy, button);

	rc759_kbd_set_mouse (&sim->kbd, dx, dy, button);
}

/*
 * Patch the ROM for fast boot.
 */
static
void rc759_patch_fastboot (rc759_t *sim)
{
	unsigned i;

	static unsigned char check[] = { 0xbd, 0x00, 0x90, 0x73 };

	if (sim->fastboot == 0) {
		return;
	}

	for (i = 0; i < 4; i++) {
		if (mem_get_uint8 (sim->mem, 0xfff8c + i) != check[i]) {
			return;
		}
	}

	pce_log_tag (MSG_INF, "RC759:", "patching ROM for fastboot\n");

	mem_set_uint8_rw (sim->mem, 0xfff8d, 0x10);
}

/*
 * Fix the ROM checksum.
 */
static
void rc759_patch_checksum (rc759_t *sim)
{
	int      patch;
	unsigned i, v1, v2;

	patch = 0;

	v1 = 0;
	v2 = 0;

	for (i = 0; i < 0x4000; i++) {
		v1 += mem_get_uint8 (sim->mem, 0xf8000 + 2 * i);
		v2 += mem_get_uint8 (sim->mem, 0xf8000 + 2 * i + 1);
	}

	if (v1 & 0xff) {
		v1 = mem_get_uint8 (sim->mem, 0xf8000) - v1;
		mem_set_uint8_rw (sim->mem, 0xf8000, v1 & 0xff);
		patch = 1;
	}

	if (v2 & 0xff) {
		v2 = mem_get_uint8 (sim->mem, 0xf8001) - v2;
		mem_set_uint8_rw (sim->mem, 0xf8001, v2 & 0xff);
		patch = 1;
	}

	if (patch) {
		pce_log_tag (MSG_INF, "RC759:", "patching ROM checksum\n");
	}
}


static
void rc759_setup_system (rc759_t *sim, ini_sct_t *ini)
{
	unsigned  model, speed;
	int       fastboot, rtc_enable, rtc_stop;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_uint16 (sct, "model", &model, 1);
	ini_get_uint16 (sct, "speed", &speed, 1);
	ini_get_bool (sct, "fastboot", &fastboot, 0);
	ini_get_bool (sct, "rtc", &rtc_enable, 1);
	ini_get_bool (sct, "rtc_stop", &rtc_stop, 0);

	if (model == 2) {
		sim->model = 2;
		sim->clock_freq = 8000000;
	}
	else {
		sim->model = 1;
		sim->clock_freq = 6000000;
	}

	if (rtc_stop) {
		rtc_enable = 0;
	}

	if (speed == 0) {
		sim->auto_speed = 1;
		sim->speed = 1;
	}
	else {
		sim->auto_speed = 0;
		sim->speed = speed;
	}

	sim->rtc_enable = (rtc_enable != 0);
	sim->rtc_stop = (rtc_stop != 0);

	sim->fastboot = (fastboot != 0);

	sim->current_int = 0;

	sim->brk = 0;
	sim->pause = 0;

	pce_log_tag (MSG_INF, "SYSTEM:",
		"model=rc759-%u %lu MHz speed=%u rtc=%d fastboot=%d\n",
		sim->model, sim->clock_freq / 1000000, speed,
		rtc_enable, fastboot
	);
}

static
void rc759_setup_mem (rc759_t *sim, ini_sct_t *ini)
{
	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);
}

static
void rc759_setup_ports (rc759_t *sim, ini_sct_t *ini)
{
	sim->iop = mem_new();

	mem_set_fct (sim->iop, sim,
		rc759_get_port8, rc759_get_port16, NULL,
		rc759_set_port8, rc759_set_port16, NULL
	);
}

static
void rc759_setup_cpu (rc759_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "CPU:", "model=80186\n");

	sim->cpu = e86_new();

	e86_set_80186 (sim->cpu);

	e86_set_mem (sim->cpu, sim->mem,
		(e86_get_uint8_f) mem_get_uint8,
		(e86_set_uint8_f) mem_set_uint8,
		(e86_get_uint16_f) mem_get_uint16_le,
		(e86_set_uint16_f) mem_set_uint16_le
	);

	e86_set_prt (sim->cpu, sim->iop,
		(e86_get_uint8_f) mem_get_uint8,
		(e86_set_uint8_f) mem_set_uint8,
		(e86_get_uint16_f) mem_get_uint16_le,
		(e86_set_uint16_f) mem_set_uint16_le
	);

	if (sim->ram != NULL) {
		e86_set_ram (sim->cpu, sim->ram->data, sim->ram->size);
	}
	else {
		e86_set_ram (sim->cpu, NULL, 0);
	}
}

static
void rc759_setup_icu (rc759_t *sim, ini_sct_t *ini)
{
	e80186_icu_init (&sim->icu);
	e80186_icu_set_intr_fct (&sim->icu, sim->cpu, e86_irq);
	e86_set_inta_fct (sim->cpu, &sim->icu, e80186_icu_inta);

	e8259_init (&sim->pic);
	e8259_set_int_fct (&sim->pic, &sim->icu, e80186_icu_set_irq_int0);
	e80186_icu_set_inta0_fct (&sim->icu, &sim->pic, e8259_inta);
}

static
void rc759_setup_tcu (rc759_t *sim, ini_sct_t *ini)
{
	e80186_tcu_init (&sim->tcu);

	e80186_tcu_set_input (&sim->tcu, 0, 1);
	e80186_tcu_set_input (&sim->tcu, 1, 1);
	e80186_tcu_set_input (&sim->tcu, 2, 1);

	e80186_tcu_set_int_fct (&sim->tcu, 0, &sim->icu, e80186_icu_set_irq_tmr0);
	e80186_tcu_set_int_fct (&sim->tcu, 1, &sim->icu, e80186_icu_set_irq_tmr1);
	e80186_tcu_set_int_fct (&sim->tcu, 2, &sim->icu, e80186_icu_set_irq_tmr2);

	e80186_tcu_set_out_fct (&sim->tcu, 0, sim, rc759_set_timer0_out);
	e80186_tcu_set_out_fct (&sim->tcu, 1, sim, rc759_set_timer1_out);
}

static
void rc759_setup_dma (rc759_t *sim, ini_sct_t *ini)
{
	e80186_dma_init (&sim->dma);
	e80186_dma_set_getmem_fct (&sim->dma, sim->mem, mem_get_uint8, mem_get_uint16_le);
	e80186_dma_set_setmem_fct (&sim->dma, sim->mem, mem_set_uint8, mem_set_uint16_le);
	e80186_dma_set_getio_fct (&sim->dma, sim->iop, mem_get_uint8, mem_get_uint16_le);
	e80186_dma_set_setio_fct (&sim->dma, sim->iop, mem_set_uint8, mem_set_uint16_le);
	e80186_dma_set_int_fct (&sim->dma, 0, &sim->icu, e80186_icu_set_irq_dma0);
	e80186_dma_set_int_fct (&sim->dma, 1, &sim->icu, e80186_icu_set_irq_dma1);
}

static
void rc759_setup_ppi (rc759_t *sim, ini_sct_t *ini)
{
	e8255_init (&sim->ppi);

	sim->ppi_port_a = 0x02;
	sim->ppi_port_b = 0x87;

	if (sim->ram != NULL) {
		if (sim->model == 2) {
			if (sim->ram->size >= (832 * 1024)) {
				sim->ppi_port_a |= 0x00;
			}
			else if (sim->ram->size >= (640 * 1024)) {
				sim->ppi_port_a |= 0x10;
			}
			else if (sim->ram->size >= (512 * 1024)) {
				sim->ppi_port_a |= 0x30;
			}
		}
		else {
			if (sim->ram->size >= (768 * 1024)) {
				sim->ppi_port_a |= 0x20;
			}
			else if (sim->ram->size >= (640 * 1024)) {
				sim->ppi_port_a |= 0x00;
			}
			else if (sim->ram->size >= (384 * 1024)) {
				sim->ppi_port_a |= 0x10;
			}
			else if (sim->ram->size >= (256 * 1024)) {
				sim->ppi_port_a |= 0x30;
			}
		}
	}

	e8255_set_inp_a (&sim->ppi, sim->ppi_port_a);
	e8255_set_inp_b (&sim->ppi, sim->ppi_port_b);

	sim->ppi.port[2].write_ext = sim;
	sim->ppi.port[2].write = (void *) rc759_set_ppi_port_c;
}

static
void rc759_setup_kbd (rc759_t *sim, ini_sct_t *ini)
{
	rc759_kbd_init (&sim->kbd);

	rc759_kbd_set_irq_fct (&sim->kbd, &sim->pic, e8259_set_irq1);
}

static
void rc759_setup_nvm (rc759_t *sim, ini_sct_t *ini)
{
	const char *nvm;
	int        sanitize;
	ini_sct_t  *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "nvm", &nvm, "nvm.dat");
	ini_get_bool (sct, "sanitize_nvm", &sanitize, 0);

	pce_log_tag (MSG_INF, "NVM:",
		"file=%s sanitize=%d\n", nvm, sanitize
	);

	rc759_nvm_init (&sim->nvm);
	rc759_nvm_set_fname (&sim->nvm, nvm);

	if (rc759_nvm_load (&sim->nvm)) {
		pce_log (MSG_ERR, "*** error loading the NVM (%s)\n",
			(nvm != NULL) ? nvm : "<none>"
		);
	}

	if (sanitize) {
		rc759_nvm_sanitize (&sim->nvm);
		rc759_nvm_fix_checksum (&sim->nvm);
	}
}

static
void rc759_setup_rtc (rc759_t *sim, ini_sct_t *ini)
{
	rc759_rtc_init (&sim->rtc);
	rc759_rtc_set_irq_fct (&sim->rtc, &sim->pic, e8259_set_irq3);
	rc759_rtc_set_input_clock (&sim->rtc, sim->clock_freq);

	if (sim->rtc_enable) {
		rc759_rtc_set_time_now (&sim->rtc);
	}
	else {
		rc759_rtc_set_time (&sim->rtc, 0, 0, 0, 0);
	}
}

static
void rc759_setup_fdc (rc759_t *sim, ini_sct_t *ini)
{
	unsigned  id0, id1;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "fdc");

	ini_get_uint16 (sct, "id0", &id0, 0);
	ini_get_uint16 (sct, "id1", &id1, 1);

	pce_log_tag (MSG_INF, "FDC:", "drive0=%u drive1=%u\n", id0, id1);

	rc759_fdc_init (&sim->fdc);

	wd179x_set_irq_fct (&sim->fdc.wd179x, &sim->pic, e8259_set_irq0);
	wd179x_set_drq_fct (&sim->fdc.wd179x, &sim->dma, e80186_dma_set_dreq0);

	wd179x_set_input_clock (&sim->fdc.wd179x, sim->clock_freq);
	wd179x_set_bit_clock (&sim->fdc.wd179x, 2000000);

	wd179x_set_drive_mask (&sim->fdc.wd179x, 0, 1);
	wd179x_set_drive_mask (&sim->fdc.wd179x, 1, 1);

	rc759_fdc_set_disks (&sim->fdc, sim->dsks);

	rc759_fdc_set_disk_id (&sim->fdc, 0, id0);
	rc759_fdc_set_disk_id (&sim->fdc, 1, id1);

	rc759_fdc_load (&sim->fdc, 0);
	rc759_fdc_load (&sim->fdc, 1);
}

static
void rc759_setup_speaker (rc759_t *sim, ini_sct_t *ini)
{
	const char    *driver;
	unsigned      volume;
	unsigned long srate;
	unsigned long lowpass;
	ini_sct_t     *sct;

	rc759_spk_init (&sim->spk);
	rc759_spk_set_clk_fct (&sim->spk, sim, rc759_get_clock);
	rc759_spk_set_input_clock (&sim->spk, sim->clock_freq);

	sct = ini_next_sct (ini, NULL, "speaker");

	if (sct == NULL) {
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
		if (rc759_spk_set_driver (&sim->spk, driver, srate)) {
			pce_log (MSG_ERR,
				"*** setting sound driver failed (%s)\n",
				driver
			);
		}
	}

	rc759_spk_set_lowpass (&sim->spk, lowpass);
	rc759_spk_set_volume (&sim->spk, volume);
}

static
void rc759_setup_terminal (rc759_t *sim, ini_sct_t *ini)
{
	sim->trm = ini_get_terminal (ini, par_terminal);

	if (sim->trm == NULL) {
		return;
	}

	trm_set_key_fct (sim->trm, &sim->kbd, rc759_kbd_set_key);
	trm_set_mouse_fct (sim->trm, sim, rc759_set_mouse);
	trm_set_msg_fct (sim->trm, sim, rc759_set_msg);
}

static
void rc759_setup_video (rc759_t *sim, ini_sct_t *ini)
{
	unsigned  min_h;
	int       mono, hires;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "video");

	if (sct == NULL) {
		sct = ini;
	}

	ini_get_uint16 (sct, "min_h", &min_h, 0);
	ini_get_bool (sct, "mono", &mono, 0);
	ini_get_bool (sct, "hires", &hires, 0);

	if (par_video != NULL) {
		if (strcmp (par_video, "mono") == 0) {
			mono = 1;
		}
		else if (strcmp (par_video, "color") == 0) {
			mono = 0;
		}
		else {
			pce_log (MSG_ERR,
				"*** unknown video type (%s)\n",
				par_video
			);
		}
	}

	e82730_init (&sim->crt);

	e82730_set_getmem_fct (&sim->crt, sim->mem, mem_get_uint8, mem_get_uint16_le);
	e82730_set_setmem_fct (&sim->crt, sim->mem, mem_set_uint8, mem_set_uint16_le);
	e82730_set_sint_fct (&sim->crt, &sim->pic, e8259_set_irq4);
	e82730_set_terminal (&sim->crt, sim->trm);
	e82730_set_monochrome (&sim->crt, mono);
	e82730_set_min_h (&sim->crt, min_h);

	pce_log_tag (MSG_INF, "VIDEO:",
		"monochrome=%d 22KHz=%d min_h=%u\n",
		mono, hires, min_h
	);

	if (hires) {
		sim->ppi_port_b |= 0x40;
		e82730_set_clock (&sim->crt, 1250000, sim->clock_freq);
	}
	else {
		sim->ppi_port_b &= ~0x40;
		e82730_set_clock (&sim->crt, 750000, sim->clock_freq);
	}

	if (mono) {
		sim->ppi_port_b |= 0x20;
	}
	else {
		sim->ppi_port_b &= ~0x20;
	}

	e8255_set_inp_b (&sim->ppi, sim->ppi_port_b);

	if (sim->trm != NULL) {
		if (hires) {
			trm_open (sim->trm, 720, 341);
		}
		else {
			trm_open (sim->trm, 560, 260);
		}

		trm_set_msg_trm (sim->trm, "term.title", "pce-rc759");
	}
}

static
void rc759_setup_disks (rc759_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;
	disk_t    *dsk;

	sim->dsks = dsks_new();

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "disk")) != NULL) {
		if (ini_get_disk (sct, &dsk)) {
			pce_log (MSG_ERR, "*** loading drive failed\n");
			continue;
		}

		if (dsk == NULL) {
			continue;
		}

		dsks_add_disk (sim->dsks, dsk);
	}
}

static
void rc759_setup_parport (rc759_t *sim, ini_sct_t *ini)
{
	const char *driver1, *driver2;
	ini_sct_t  *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "parport", &driver1, NULL);
	ini_get_string (sct, "parport1", &driver1, driver1);
	ini_get_string (sct, "parport2", &driver2, NULL);

	pce_log_tag (MSG_INF,
		"PARPORT1:", "type=local driver=%s\n",
		(driver1 == NULL) ? "<none>" : driver1
	);

	rc759_par_init (&sim->par[0]);
	rc759_par_set_irq_fct (&sim->par[0], &sim->pic, e8259_set_irq6);

	if (driver1 != NULL) {
		if (rc759_par_set_driver (&sim->par[0], driver1)) {
			pce_log (MSG_ERR, "*** can't open driver (%s)\n",
				driver1
			);
		}
	}

	pce_log_tag (MSG_INF,
		"PARPORT2:", "type=remote driver=%s\n",
		(driver2 == NULL) ? "<none>" : driver2
	);

	rc759_par_init (&sim->par[1]);
	rc759_par_set_irq_fct (&sim->par[1], &sim->pic, e8259_set_irq2);

	if (driver2 != NULL) {
		if (rc759_par_set_driver (&sim->par[1], driver2)) {
			pce_log (MSG_ERR, "*** can't open driver (%s)\n",
				driver2
			);
		}
	}
}

rc759_t *rc759_new (ini_sct_t *ini)
{
	rc759_t *sim;

	sim = malloc (sizeof (rc759_t));

	if (sim == NULL) {
		return (NULL);
	}

	memset (sim, 0, sizeof (rc759_t));

	sim->cfg = ini;

	sim->disk_id = 0;

	bps_init (&sim->bps);
	rc759_setup_system (sim, ini);
	rc759_setup_mem (sim, ini);
	rc759_setup_ports (sim, ini);
	rc759_setup_cpu (sim, ini);
	rc759_setup_icu (sim, ini);
	rc759_setup_tcu (sim, ini);
	rc759_setup_dma (sim, ini);
	rc759_setup_ppi (sim, ini);
	rc759_setup_kbd (sim, ini);
	rc759_setup_nvm (sim, ini);
	rc759_setup_rtc (sim, ini);
	rc759_setup_speaker (sim, ini);
	rc759_setup_terminal (sim, ini);
	rc759_setup_video (sim, ini);
	rc759_setup_disks (sim, ini);
	rc759_setup_fdc (sim, ini);
	rc759_setup_parport (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

	mem_move_to_front (sim->mem, 0xf8000);

	rc759_patch_fastboot (sim);
	rc759_patch_checksum (sim);

	rc759_clock_reset (sim);

	return (sim);
}

void rc759_del (rc759_t *sim)
{
	if (sim == NULL) {
		return;
	}

	bps_free (&sim->bps);
	rc759_par_free (&sim->par[1]);
	rc759_par_free (&sim->par[0]);
	rc759_fdc_free (&sim->fdc);
	dsks_del (sim->dsks);
	trm_del (sim->trm);
	rc759_rtc_free (&sim->rtc);
	rc759_nvm_free (&sim->nvm);
	rc759_spk_free (&sim->spk);
	e8255_free (&sim->ppi);
	e80186_dma_free (&sim->dma);
	e80186_tcu_free (&sim->tcu);
	e8259_free (&sim->pic);
	e80186_icu_free (&sim->icu);
	e86_del (sim->cpu);
	mem_del (sim->mem);
	mem_del (sim->iop);
	ini_sct_del (sim->cfg);

	free (sim);
}

int rc759_set_parport_driver (rc759_t *sim, unsigned port, const char *driver)
{
	if (port > 1) {
		return (1);
	}

	if (rc759_par_set_driver (&sim->par[port], driver)) {
		return (1);
	}

	return (0);
}

int rc759_set_parport_file (rc759_t *sim, unsigned port, const char *fname)
{
	int  r;
	char *driver;

	if (port > 1) {
		return (1);
	}

	driver = str_cat_alloc ("stdio:file=", fname);

	r = rc759_set_parport_driver (sim, port, driver);

	free (driver);

	return (r);
}

const char *rc759_intlog_get (rc759_t *sim, unsigned n)
{
	return (par_intlog[n & 0xff]);
}

void rc759_intlog_set (rc759_t *sim, unsigned n, const char *expr)
{
	char **str;

	str = &par_intlog[n & 0xff];

	free (*str);

	if ((expr == NULL) || (*expr == 0)) {
		*str = NULL;
		return;
	}

	*str = str_copy_alloc (expr);
}

int rc759_intlog_check (rc759_t *sim, unsigned n)
{
	unsigned long val;
	const char    *str;
	cmd_t         cmd;

	str = par_intlog[n & 0xff];

	if (str == NULL) {
		return (0);
	}

	cmd_set_str (&cmd, str);

	if (cmd_match_uint32 (&cmd, &val)) {
		if (val) {
			return (1);
		}
	}

	return (0);
}

void rc759_reset (rc759_t *sim)
{
	pce_log_tag (MSG_INF, "RC759:", "reset\n");

	e86_reset (sim->cpu);
	e82730_reset (&sim->crt);
	e8259_reset (&sim->pic);
	e80186_tcu_reset (&sim->tcu);
	e80186_dma_reset (&sim->dma);
	e80186_icu_reset (&sim->icu);
	rc759_kbd_reset (&sim->kbd);
	rc759_rtc_reset (&sim->rtc);
	rc759_fdc_reset (&sim->fdc);
	rc759_par_reset (&sim->par[0]);
	rc759_par_reset (&sim->par[1]);

	if (sim->rtc_enable) {
		rc759_rtc_set_time_now (&sim->rtc);
	}
}

unsigned long rc759_get_clock (rc759_t *sim)
{
	return (sim->clock_cnt);
}

void rc759_set_speed (rc759_t *sim, unsigned speed)
{
	sim->auto_speed = (speed == 0);

	if (sim->auto_speed) {
		pce_log (MSG_INF, "setting CPU speed to auto\n");
		return;
	}

	sim->speed = speed;

	pce_log (MSG_INF, "setting CPU speed to %lu MHz\n",
		(sim->speed * sim->clock_freq) / 1000000
	);
}


void rc759_clock_reset (rc759_t *sim)
{
	sim->sync_vclock = 0;
	sim->sync_rclock = 0;
	sim->sync_vclock_last = sim->clock_cnt;
	pce_get_interval_us (&sim->sync_rclock_last);

	sim->clock_cnt = 0;
	sim->clock_rem8 = 0;
	sim->clock_rem1024 = 0;
	sim->clock_rem65536 = 0;
}

void rc759_clock_discontinuity (rc759_t *sim)
{
	sim->sync_vclock = 0;
	sim->sync_rclock = 0;
	sim->sync_vclock_last = sim->clock_cnt;
	pce_get_interval_us (&sim->sync_rclock_last);
}

/*
 * Synchronize the system clock with real time
 */
static
void rc759_clock_delay (rc759_t *sim)
{
	unsigned long vclk, dvclk;
	unsigned long rclk, drclk;
	unsigned long us;

	dvclk = sim->clock_cnt - sim->sync_vclock_last;
	sim->sync_vclock_last = sim->clock_cnt;

	drclk = pce_get_interval_us (&sim->sync_rclock_last);
	drclk = (sim->clock_freq * (unsigned long long) drclk) / 1000000;

	vclk = sim->sync_vclock + dvclk;
	rclk = sim->sync_rclock + drclk;

	if (vclk < rclk) {
		sim->sync_vclock = 0;
		sim->sync_rclock = rclk - vclk;

		if (sim->auto_speed) {
			if (dvclk < drclk) {
				if (sim->speed > 1) {
					sim->speed -= 1;
				}
			}
		}

		if (sim->sync_rclock > sim->clock_freq) {
			sim->sync_rclock -= sim->clock_freq;

			pce_log (MSG_INF,
				"host system too slow, skipping 1 second.\n"
			);
		}
	}
	else {
		sim->sync_vclock = vclk - rclk;
		sim->sync_rclock = 0;

		if (sim->auto_speed) {
			if (drclk < dvclk) {
				sim->speed += 1;
			}
		}
		else {
			us = (1000000ULL * sim->sync_vclock) / sim->clock_freq;

			if (us > PCE_IBMPC_SLEEP) {
				pce_usleep (us);
			}
		}
	}
}

void rc759_clock (rc759_t *sim, unsigned cnt)
{
	unsigned long sysclk, cpuclk;

	sysclk = 1;
	cpuclk = sim->speed;

	e86_clock (sim->cpu, cpuclk);

	e80186_tcu_clock (&sim->tcu, sysclk);
	e80186_dma_clock (&sim->dma, cpuclk);
	rc759_fdc_clock (&sim->fdc.wd179x, cpuclk);

	sim->clock_cnt += sysclk;
	sim->clock_rem8 += sysclk;

	if (sim->clock_rem8 < 8) {
		return;
	}

	sysclk = sim->clock_rem8;
	sim->clock_rem8 &= 7;
	sysclk -= sim->clock_rem8;

	e82730_clock (&sim->crt, sysclk);

	if (sim->rtc_stop == 0) {
		rc759_rtc_clock (&sim->rtc, sysclk);
	}

	sim->clock_rem1024 += sysclk;

	if (sim->clock_rem1024 < 1024) {
		return;
	}

	sysclk = sim->clock_rem1024;
	sim->clock_rem1024 &= 1023;
	sysclk -= sim->clock_rem1024;

	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	rc759_kbd_clock (&sim->kbd, sysclk);
	rc759_spk_clock (&sim->spk, sysclk);

	sim->clock_rem65536 += sysclk;

	if (sim->clock_rem65536 < 65536) {
		return;
	}

	sysclk = sim->clock_rem65536;
	sim->clock_rem65536 &= 65535;
	sysclk -= sim->clock_rem65536;

	rc759_clock_delay (sim);
}
