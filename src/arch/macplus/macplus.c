/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/macplus.c                                   *
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


#include "main.h"


static
void mac_interrupt_check (macplus_t *sim)
{
	unsigned i;
	unsigned val;

	val = sim->intr >> 1;

	i = 0;
	while (val != 0) {
		i += 1;
		val = val >> 1;
	}

#ifdef DEBUG_INT
	mac_log_deb ("interrupt level %u\n", i);
#endif

	e68_interrupt (sim->cpu, i, 0, 1);
}

static
void mac_interrupt (macplus_t *sim, unsigned level, int val)
{
	if (val) {
		sim->intr |= (1U << level);
	}
	else {
		sim->intr &= ~(1U << level);
	}

	mac_interrupt_check (sim);
}

static
void mac_interrupt_via (void *ext, unsigned char val)
{
	mac_interrupt (ext, 1, val);
}

static
void mac_interrupt_scc (void *ext, unsigned char val)
{
	mac_interrupt (ext, 2, val);
}

static
void mac_interrupt_vbi (void *ext, unsigned char val)
{
	macplus_t *sim = ext;

	if (val) {
		e6522_set_ca1_inp (&sim->via, 0);
		e6522_set_ca1_inp (&sim->via, 1);
	}
}

void mac_interrupt_osi (void *ext, unsigned char val)
{
	macplus_t *sim = ext;

	if (val) {
		e6522_set_ca2_inp (&sim->via, 0);
		e6522_set_ca2_inp (&sim->via, 1);
	}
}

static
void mac_set_reset (void *ext, unsigned char val)
{
	if (val == 0) {
		return;
	}

	mac_reset (ext);
}

static
void mac_check_mouse (macplus_t *sim)
{
	if ((sim->mouse_delta_x <= -2) || (sim->mouse_delta_x >= 2)) {
		if (sim->dcd_a) {
			sim->via_port_b &= ~0x10;
		}
		else {
			sim->via_port_b |= 0x10;
		}

		if (sim->mouse_delta_x > 0) {
			sim->via_port_b ^= 0x10;
			sim->mouse_delta_x -= 2;
		}
		else {
			sim->mouse_delta_x += 2;
		}

		e6522_set_irb_inp (&sim->via, sim->via_port_b);
		e8530_set_dcd_a (&sim->scc, sim->dcd_a);

		sim->dcd_a = !sim->dcd_a;
	}

	if ((sim->mouse_delta_y <= -2) || (sim->mouse_delta_y >= 2)) {
		if (sim->dcd_b) {
			sim->via_port_b &= ~0x20;
		}
		else {
			sim->via_port_b |= 0x20;
		}

		if (sim->mouse_delta_y > 0) {
			sim->mouse_delta_y -= 2;
		}
		else {
			sim->via_port_b ^= 0x20;
			sim->mouse_delta_y += 2;
		}

		e6522_set_irb_inp (&sim->via, sim->via_port_b);
		e8530_set_dcd_b (&sim->scc, sim->dcd_b);

		sim->dcd_b = !sim->dcd_b;
	}
}

static
void mac_set_mouse (void *ext, int dx, int dy, unsigned but)
{
	macplus_t     *sim = ext;
	unsigned char old;

	if (sim->pause) {
		if (but) {
			sim->pause = 0;
		}

		return;
	}

	if ((sim->mouse_button ^ but) & ~but & 2) {
		trm_set_msg_trm (sim->trm, "term.release", "1");
	}

	old = sim->via_port_b;

	if (but & 1) {
		sim->via_port_b &= 0xf7;
	}
	else {
		sim->via_port_b |= 0x08;
	}

	if (sim->via_port_b != old) {
		e6522_set_irb_inp (&sim->via, sim->via_port_b);
	}

	sim->mouse_delta_x += dx;
	sim->mouse_delta_y += dy;
	sim->mouse_button = but;
}

static
void mac_set_rtc_data (void *ext, unsigned char val)
{
	macplus_t *sim = ext;

	if (val) {
		sim->via_port_b |= 0x01;
	}
	else {
		sim->via_port_b &= ~0x01;
	}

	e6522_set_irb_inp (&sim->via, sim->via_port_b);
}

static
void mac_set_via_port_a (void *ext, unsigned char val)
{
	macplus_t     *sim = ext;
	unsigned char old;

	if (sim->via_port_a == val) {
		return;
	}

#ifdef DEBUG_VIA
	mac_log_deb ("via: set port a: %02X\n", val);
#endif

	old = sim->via_port_a;
	sim->via_port_a = val;

	if ((old ^ val) & 0x10) {
		mac_set_overlay (sim, (val & 0x10) != 0);
	}

	if ((old ^ val) & 0x40) {
		unsigned char *vbuf;

		vbuf = mem_blk_get_data (sim->ram);

		if (val & 0x40) {
			mac_log_deb ("main video buffer\n");
			vbuf += sim->vbuf1;
		}
		else {
			mac_log_deb ("alternate video buffer\n");
			vbuf += sim->vbuf2;
		}

		mac_video_set_vbuf (sim->video, vbuf);
	}

	if ((old ^ val) & 0x08) {
		unsigned char *sbuf;

		sbuf = mem_blk_get_data (sim->ram);

		if (val & 0x08) {
			mac_log_deb ("main sound buffer\n");
			sbuf += sim->sbuf1;
		}
		else {
			mac_log_deb ("alternate sound buffer\n");
			sbuf += sim->sbuf2;
		}
	}

	if ((old ^ val) & 0x07) {
		/* sound volume */
		;
	}
}

static
void mac_set_via_port_b (void *ext, unsigned char val)
{
	macplus_t     *sim = ext;
	unsigned char old;

	if (sim->via_port_b == val) {
		return;
	}

#ifdef DEBUG_VIA
	mac_log_deb ("via: set port b: %02X\n", val);
#endif

	old = sim->via_port_b;
	sim->via_port_b = val;

	mac_rtc_set_uint8 (&sim->rtc, val);

	if ((old ^ val) & 0x80) {
		/* sound enable */
		;
	}
}

static
unsigned char mac_scc_get_uint8 (void *ext, unsigned long addr)
{
	unsigned char val;
	macplus_t     *sim = ext;

	val = 0xff;

	switch (addr) {
	case 0x1ffff8:
		val = e8530_get_ctl_b (&sim->scc);
		break;

	case 0x1ffffa:
		val = e8530_get_ctl_a (&sim->scc);
		break;

	case 0x1ffffc:
		val = e8530_get_data_b (&sim->scc);
		break;

	case 0x1ffffe:
		val = e8530_get_data_a (&sim->scc);
		break;
	}

#ifdef DEBUG_SCC
	mac_log_deb ("scc: get  8: %06lX -> %02X\n", addr, val);
#endif

	return (val);
}

static
void mac_scc_set_uint8 (void *ext, unsigned long addr, unsigned char val)
{
	macplus_t *sim = ext;

#ifdef DEBUG_SCC
	mac_log_deb ("scc: set  8: %06lX <- %02X\n", addr, val);
#endif

	switch (addr) {
	case 0x3ffff9:
		e8530_set_ctl_b (&sim->scc, val);
		break;

	case 0x3ffffb:
		e8530_set_ctl_a (&sim->scc, val);
		break;

	case 0x3ffffd:
		e8530_set_data_b (&sim->scc, val);
		break;

	case 0x3fffff:
		e8530_set_data_a (&sim->scc, val);
		break;
	}
}


static
void mac_setup_mem (macplus_t *sim, ini_sct_t *ini)
{
	sim->mem = mem_new();

	mem_set_fct (sim->mem, sim,
		mac_mem_get_uint8, mac_mem_get_uint16, mac_mem_get_uint32,
		mac_mem_set_uint8, mac_mem_set_uint16, mac_mem_set_uint32
	);

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	sim->ram = mem_get_blk (sim->mem, 0x00000000);
	sim->rom = mem_get_blk (sim->mem, 0x00400000);

	sim->ram_ovl = NULL;
	sim->rom_ovl = NULL;

	if (sim->ram == NULL) {
		pce_log (MSG_ERR, "*** RAM not found at 000000\n");
		return;
	}

	if (sim->rom == NULL) {
		pce_log (MSG_ERR, "*** ROM not found at 400000\n");
		return;
	}

	sim->ram_ovl = mem_blk_clone (sim->ram);
	mem_blk_set_addr (sim->ram_ovl, 0x00600000);

	if (mem_blk_get_size (sim->ram_ovl) > 0x00200000) {
		mem_blk_set_size (sim->ram_ovl, 0x00200000);
	}

	sim->rom_ovl = mem_blk_clone (sim->rom);
	mem_blk_set_addr (sim->rom_ovl, 0);

	mem_rmv_blk (sim->mem, sim->ram);
	mem_add_blk (sim->mem, sim->ram_ovl, 0);
	mem_add_blk (sim->mem, sim->rom_ovl, 0);

	sim->overlay = 1;
}

static
void mac_setup_cpu (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *model;
	unsigned   speed;

	sct = ini_next_sct (ini, NULL, "cpu");

	ini_get_string (sct, "model", &model, "68000");
	ini_get_uint16 (sct, "speed", &speed, 0);

	pce_log_tag (MSG_INF, "CPU:", "model=%s speed=%d\n", model, speed);

	sim->cpu = e68_new();
	if (sim->cpu == NULL) {
		return;
	}

	if (mac_set_cpu_model (sim, model)) {
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

	e68_set_reset_fct (sim->cpu, sim, mac_set_reset);

	e68_set_hook_fct (sim->cpu, sim, mac_hook);

	e68_set_address_check (sim->cpu, 0);

	sim->speed_factor = speed;
	sim->rt_clk = 0;
	sim->rt_us = 0;
	pce_get_interval_us (&sim->rt_us);
}

static
void mac_setup_via (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *blk;
	unsigned long addr, size;

	sct = ini_next_sct (ini, NULL, "via");

	ini_get_uint32 (sct, "address", &addr, 0x00efe000);
	ini_get_uint32 (sct, "size", &size, 16 * 512);

	pce_log_tag (MSG_INF, "VIA:", "addr=0x%06lx size=0x%lx\n", addr, size);

	sim->via_port_a = 0xd8;
	sim->via_port_b = 0x88;

	e6522_init (&sim->via, 9);

	e6522_set_irq_fct (&sim->via, sim, mac_interrupt_via);

	e6522_set_ira_inp (&sim->via, sim->via_port_a);
	e6522_set_irb_inp (&sim->via, sim->via_port_b);

	e6522_set_ora_fct (&sim->via, sim, mac_set_via_port_a);
	e6522_set_orb_fct (&sim->via, sim, mac_set_via_port_b);

	blk = mem_blk_new (addr, size, 0);
	if (blk == NULL) {
		return;
	}

	mem_blk_set_fct (blk, &sim->via,
		e6522_get_uint8, e6522_get_uint16, e6522_get_uint32,
		e6522_set_uint8, e6522_set_uint16, e6522_set_uint32
	);

	mem_add_blk (sim->mem, blk, 1);
}

static
void mac_setup_scc (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *blk;
	unsigned long addr, size;

	sct = ini_next_sct (ini, NULL, "scc");

	ini_get_uint32 (sct, "address", &addr, 0x800000);
	ini_get_uint32 (sct, "size", &size, 0x400000);

	pce_log_tag (MSG_INF, "SCC:", "addr=0x%06lx size=0x%lx\n", addr, size);

	e8530_init (&sim->scc);
	e8530_set_irq_fct (&sim->scc, sim, mac_interrupt_scc);
	e8530_set_clock (&sim->scc, 3672000, 3672000, 3672000);

	blk = mem_blk_new (addr, size, 0);
	if (blk == NULL) {
		return;
	}

	mem_blk_set_fct (blk, sim,
		mac_scc_get_uint8, NULL, NULL,
		mac_scc_set_uint8, NULL, NULL
	);

	mem_add_blk (sim->mem, blk, 1);
}

static
void mac_setup_serial (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	unsigned   port;
	const char *driver;

	mac_ser_init (&sim->ser[0]);
	mac_ser_set_scc (&sim->ser[0], &sim->scc, 0);

	mac_ser_init (&sim->ser[1]);
	mac_ser_set_scc (&sim->ser[1], &sim->scc, 1);

	sct = ini_next_sct (ini, NULL, "serial");

	while (sct != NULL) {
		ini_get_uint16 (sct, "port", &port, 0);

		ini_get_string (sct, "driver", &driver, NULL);

		pce_log_tag (MSG_INF, "SERIAL:", "port=%u driver=%s\n",
			port,
			(driver != NULL) ? driver : "<none>"
		);

		sct = ini_next_sct (ini, sct, "serial");

		if (port > 1) {
			pce_log (MSG_ERR, "*** bad port number (%u)\n", port);
			continue;
		}

		if (driver != NULL) {
			if (mac_ser_set_driver (&sim->ser[port], driver)) {
				pce_log (MSG_ERR, "*** can't open driver (%s)\n", driver);
			}
		}
	}
}

static
void mac_setup_rtc (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *fname;
	int        realtime;

	sct = ini_next_sct (ini, NULL, "rtc");

	ini_get_string (sct, "file", &fname, "pram.dat");
	ini_get_bool (sct, "realtime", &realtime, 1);

	pce_log_tag (MSG_INF, "RTC:", "file=%s realtime=%d\n", fname, realtime);

	sim->rtc_fname = strdup (fname);

	mac_rtc_init (&sim->rtc);

	mac_rtc_set_data_fct (&sim->rtc, sim, mac_set_rtc_data);
	mac_rtc_set_osi_fct (&sim->rtc, sim, mac_interrupt_osi);

	mac_rtc_set_realtime (&sim->rtc, realtime);

	if (mac_rtc_load_file (&sim->rtc, fname)) {
		pce_log (MSG_ERR, "*** reading rtc file failed\n");
	}

	mac_rtc_set_current_time (&sim->rtc);
}

static
void mac_setup_kbd (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;
	unsigned  model;
	unsigned  intl;

	sct = ini_next_sct (ini, NULL, "keyboard");

	ini_get_uint16 (sct, "model", &model, 1);
	ini_get_uint16 (sct, "intl", &intl, 0);

	pce_log_tag (MSG_INF, "KEYBOARD:", "model=%u international=%u\n",
		model, intl
	);

	mac_kbd_init (&sim->kbd);
	mac_kbd_set_model (&sim->kbd, model, intl);
	mac_kbd_set_data_fct (&sim->kbd, &sim->via, e6522_set_shift_inp);
	mac_kbd_set_intr_fct (&sim->kbd, sim, mac_interrupt);

	e6522_set_shift_out_fct (&sim->via, &sim->kbd, mac_kbd_set_uint8);
}

static
void mac_setup_disks (macplus_t *sim, ini_sct_t *ini)
{
	sim->dsks = ini_get_disks (ini);
}

static
void mac_setup_scsi (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct, *sctdev;
	mem_blk_t     *blk;
	unsigned long addr, size;
	unsigned      id, drive;
	const char    *vendor, *product;

	sct = ini_next_sct (ini, NULL, "scsi");

	if (sct == NULL) {
		return;
	}

	ini_get_uint32 (sct, "address", &addr, 0x580000);
	ini_get_uint32 (sct, "size", &size, 0x1000);

	pce_log_tag (MSG_INF, "SCSI:", "addr=0x%06lx size=0x%lx\n", addr, size);

	mac_scsi_init (&sim->scsi);

	mac_scsi_set_disks (&sim->scsi, sim->dsks);

	blk = mem_blk_new (addr, size, 0);
	if (blk == NULL) {
		return;
	}

	mem_blk_set_fct (blk, sim,
		mac_scsi_get_uint8, mac_scsi_get_uint16, NULL,
		mac_scsi_set_uint8, mac_scsi_set_uint16, NULL
	);

	mem_add_blk (sim->mem, blk, 1);

	sctdev = ini_next_sct (sct, NULL, "device");
	while (sctdev != NULL) {
		ini_get_uint16 (sctdev, "id", &id, 0);
		ini_get_uint16 (sctdev, "drive", &drive, 0);
		ini_get_string (sctdev, "vendor", &vendor, "PCE");
		ini_get_string (sctdev, "product", &product, "PCEDISK");

		pce_log_tag (MSG_INF,
			"SCSI:", "id=%u drive=%u vendor=\"%s\" product=\"%s\"\n",
			id, drive, vendor, product
		);

		mac_scsi_set_drive (&sim->scsi, id, drive);
		mac_scsi_set_drive_vendor (&sim->scsi, id, vendor);
		mac_scsi_set_drive_product (&sim->scsi, id, product);

		sctdev = ini_next_sct (sct, sctdev, "device");
	}
}

static
void mac_setup_sony (macplus_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "sony");

	mac_sony_init (&sim->sony);

	sim->sony.delay = par_disk_delay;

	if (sim->sony.delay == 0) {
		sim->sony.delay = 1;
	}

	pce_log_tag (MSG_INF, "SONY:", "delay=%lu\n", sim->sony.delay);
}

static
void mac_setup_terminal (macplus_t *sim, ini_sct_t *ini)
{
	sim->trm = ini_get_terminal (ini, par_terminal);

	if (sim->trm == NULL) {
		return;
	}

	trm_set_msg_fct (sim->trm, sim, mac_set_msg);
	trm_set_key_fct (sim->trm, &sim->kbd, mac_kbd_set_key);
	trm_set_mouse_fct (sim->trm, sim, mac_set_mouse);
}

static
void mac_setup_video (macplus_t *sim, ini_sct_t *ini)
{
	unsigned long addr;
	ini_sct_t     *sct;
	int           realtime;

	if (sim->ram == NULL) {
		return;
	}

	sct = ini_next_sct (ini, NULL, "video");

	addr = mem_blk_get_size (sim->ram);
	addr = (addr < 0x5900) ? 0 : (addr - 0x5900);

	if (sct != NULL) {
		ini_get_uint32 (sct, "address", &addr, addr);
		ini_get_bool (sct, "realtime", &realtime, 1);
	}

	pce_log_tag (MSG_INF, "VIDEO:", "addr=0x%06lX realtime=%d\n",
		addr, realtime
	);

	sim->vbuf1 = addr;
	sim->vbuf2 = addr - 0x8000;

	sim->video = mac_video_new();
	if (sim->video == NULL) {
		return;
	}

	mac_video_set_vbi_fct (sim->video, sim, mac_interrupt_vbi);

	mac_video_set_vbuf (sim->video, mem_blk_get_data (sim->ram) + sim->vbuf1);

	mac_video_set_realtime (sim->video, realtime);

	if (sim->trm != NULL) {
		mac_video_set_terminal (sim->video, sim->trm);

		trm_open (sim->trm, 512, 342);
	}
}

void mac_init (macplus_t *sim, ini_sct_t *ini)
{
	sim->trm = NULL;
	sim->video = NULL;

	sim->reset = 0;

	sim->dcd_a = 0;
	sim->dcd_b = 0;

	sim->mouse_delta_x = 0;
	sim->mouse_delta_y = 0;
	sim->mouse_button = 0;

	sim->intr = 0;

	sim->pause = 0;
	sim->brk = 0;

	sim->speed_factor = 0;
	sim->via_clk_div = 10;

	sim->clk_cnt = 0;
	sim->clk_div[0] = 0;
	sim->clk_div[1] = 0;

	bps_init (&sim->bps);

	mac_setup_mem (sim, ini);
	mac_setup_cpu (sim, ini);
	mac_setup_via (sim, ini);
	mac_setup_scc (sim, ini);
	mac_setup_serial (sim, ini);
	mac_setup_rtc (sim, ini);
	mac_setup_kbd (sim, ini);
	mac_setup_disks (sim, ini);
	mac_iwm_init (sim);
	mac_setup_scsi (sim, ini);
	mac_setup_sony (sim, ini);
	mac_setup_terminal (sim, ini);
	mac_setup_video (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

	trm_set_msg_trm (sim->trm, "term.title", "pce-macplus");
}

macplus_t *mac_new (ini_sct_t *ini)
{
	macplus_t *sim;

	sim = malloc (sizeof (macplus_t));
	if (sim == NULL) {
		return (NULL);
	}

	mac_init (sim, ini);

	return (sim);
}

void mac_free (macplus_t *sim)
{
	if (sim == NULL) {
		return;
	}

	if (mac_rtc_save_file (&sim->rtc, sim->rtc_fname)) {
		pce_log (MSG_ERR, "*** writing rtc file failed (%s)\n",
			sim->rtc_fname
		);
	}

	free (sim->rtc_fname);

	mac_video_del (sim->video);
	trm_del (sim->trm);
	mac_sony_free (&sim->sony);
	mac_scsi_free (&sim->scsi);
	mac_iwm_free (sim);
	dsks_del (sim->dsks);
	mac_kbd_free (&sim->kbd);
	mac_rtc_free (&sim->rtc);
	mac_ser_free (&sim->ser[1]);
	mac_ser_free (&sim->ser[0]);
	e8530_free (&sim->scc);
	e6522_free (&sim->via);
	e68_del (sim->cpu);
	mem_del (sim->mem);

	mem_blk_del (sim->ram_ovl);
	mem_blk_del (sim->rom_ovl);

	bps_free (&sim->bps);
}

void mac_del (macplus_t *sim)
{
	if (sim != NULL) {
		mac_free (sim);
		free (sim);
	}
}


unsigned long long mac_get_clkcnt (macplus_t *sim)
{
	return (sim->clk_cnt);
}

void mac_set_speed_accurate (macplus_t *sim)
{
	mac_log_deb ("speed = accurate\n");

	mac_rtc_set_realtime (&sim->rtc, 0);
	mac_video_set_realtime (sim->video, 0);
	sim->via_clk_div = 10;
	sim->speed_factor = 0;
}

void mac_set_speed (macplus_t *sim, unsigned factor)
{
	mac_log_deb ("speed = %u\n", factor);

	if (factor == 0) {
		mac_rtc_set_realtime (&sim->rtc, 1);
		mac_video_set_realtime (sim->video, 1);
		sim->via_clk_div = 10;
		sim->via_clk_div = 40;
	}
	else if (factor == 1) {
		mac_rtc_set_realtime (&sim->rtc, 0);
		mac_video_set_realtime (sim->video, 0);
		sim->via_clk_div = 10;
	}
	else {
		mac_rtc_set_realtime (&sim->rtc, 1);
		mac_video_set_realtime (sim->video, 1);
		sim->via_clk_div = 10 * factor;
	}

	sim->speed_factor = factor;
}

int mac_set_msg_trm (macplus_t *sim, const char *msg, const char *val)
{
	if (sim->trm == NULL) {
		return (1);
	}

	return (trm_set_msg_trm (sim->trm, msg, val));
}

int mac_set_cpu_model (macplus_t *sim, const char *model)
{
	if (strcmp (model, "68000") == 0) {
		e68_set_68000 (sim->cpu);
	}
	else if (strcmp (model, "68010") == 0) {
		e68_set_68010 (sim->cpu);
	}
	else {
		return (1);
	}

	return (0);
}

void mac_reset (macplus_t *sim)
{
	if (sim->reset) {
		return;
	}

	sim->reset = 1;

	mac_log_deb ("mac: reset\n");

	sim->dcd_a = 0;
	sim->dcd_b = 0;

	sim->mouse_delta_x = 0;
	sim->mouse_delta_y = 0;
	sim->mouse_button = 0;

	sim->intr = 0;

	sim->via_port_a = 0xd8;
	sim->via_port_b = 0x88;

	mac_set_overlay (sim, 1);

	mac_sony_reset (sim);
	mac_scsi_reset (&sim->scsi);
	e6522_reset (&sim->via);
	e8530_reset (&sim->scc);
	e68_reset (sim->cpu);

	sim->reset = 0;
}

static
void mac_realtime_sync (macplus_t *sim, unsigned long n)
{
	unsigned long fct;
	unsigned long us1, us2;

	sim->rt_clk += n;

	fct = sim->speed_factor * (MAC_CPU_CLOCK / 100);

	if (sim->rt_clk >= fct) {
		sim->rt_clk -= fct;

		us1 = pce_get_interval_us (&sim->rt_us);
		us2 = (1000000 / 100);

		if (us1 < us2) {
			pce_usleep (us2 - us1);

			pce_get_interval_us (&sim->rt_us);
		}
	}
}

void mac_clock (macplus_t *sim, unsigned n)
{
	if (n == 0) {
		n = sim->cpu->delay;
		if (n == 0) {
			n = 1;
		}
	}

	e68_clock (sim->cpu, n);

	sim->clk_cnt += n;

	sim->clk_div[0] += n;

	while (sim->clk_div[0] >= sim->via_clk_div) {
		e6522_clock (&sim->via, 1);
		sim->clk_div[0] -= sim->via_clk_div;
	}

	sim->clk_div[1] += n;

	if (sim->clk_div[1] < 4096) {
		return;
	}

	mac_check_mouse (sim);
	mac_kbd_clock (&sim->kbd, sim->clk_div[1]);

	sim->clk_div[2] += sim->clk_div[1];
	sim->clk_div[1] = 0;

	if (sim->clk_div[2] < 4 * 4096) {
		return;
	}

	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	mac_video_clock (sim->video, sim->clk_div[2]);
	mac_rtc_clock (&sim->rtc, sim->clk_div[2]);
	mac_sony_clock (sim, sim->clk_div[2]);

	mac_ser_clock (&sim->ser[0], sim->clk_div[2]);
	mac_ser_clock (&sim->ser[1], sim->clk_div[2]);

	if (sim->speed_factor > 0) {
		mac_realtime_sync (sim, sim->clk_div[2]);
	}

	sim->clk_div[2] = 0;
}
