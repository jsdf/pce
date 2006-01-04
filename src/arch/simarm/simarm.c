/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/simarm.c                                   *
 * Created:       2004-11-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-01-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                       *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/

/* $Id$ */


#include "main.h"


void sarm_break (simarm_t *sim, unsigned char val);


static
void sarm_setup_cpu (simarm_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *model;

	sct = ini_sct_find_sct (ini, "cpu");

	model = ini_get_str_def (sct, "model", "armv5");
	sim->bigendian = ini_get_lng_def (sct, "bigendian", 1) != 0;

	pce_log (MSG_INF, "CPU:        model=%s endian=%s\n",
		model, sim->bigendian ? "big" : "little"
	);

	sim->cpu = arm_new (sim->bigendian);
	if (sim->cpu == NULL) {
		return;
	}

	if (sim->bigendian) {
		arm_set_mem_fct (sim->cpu, sim->mem,
			&mem_get_uint8,
			&mem_get_uint16_be,
			&mem_get_uint32_be,
			&mem_set_uint8,
			&mem_set_uint16_be,
			&mem_set_uint32_be
		);
	}
	else {
		arm_set_mem_fct (sim->cpu, sim->mem,
			&mem_get_uint8,
			&mem_get_uint16_le,
			&mem_get_uint32_le,
			&mem_set_uint8,
			&mem_set_uint16_le,
			&mem_set_uint32_le
		);
	}

	if (sim->ram != NULL) {
		arm_set_ram (sim->cpu, mem_blk_get_data (sim->ram), mem_blk_get_size (sim->ram));
	}
}

static
void sarm_setup_intc (simarm_t *sim, ini_sct_t *ini)
{
	unsigned long base;
	ini_sct_t     *sct;

	sct = ini_sct_find_sct (ini, "intc");
	if (sct != NULL) {
		base = ini_get_lng_def (sct, "base", 0xd6000000UL);
	}
	else {
		base = 0xd6000000UL;
	}

	sim->intc = ict_new (base);
	if (sim->intc == NULL) {
		return;
	}

	ict_set_fiq_f (sim->intc, arm_set_fiq, sim->cpu);
	ict_set_irq_f (sim->intc, arm_set_irq, sim->cpu);

	mem_add_blk (sim->mem, ict_get_io (sim->intc, 0), 0);

	pce_log (MSG_INF, "INTC:       base=0x%08lx\n", base);
}

static
void sarm_setup_timer (simarm_t *sim, ini_sct_t *ini)
{
	unsigned long base;
	unsigned      scale;
	ini_sct_t     *sct;

	sct = ini_sct_find_sct (ini, "timer");
	if (sct != NULL) {
		base = ini_get_lng_def (sct, "base", 0xc0020000);
		scale = ini_get_lng_def (sct, "scale", 4);
	}
	else {
		base = 0xc0020000;
		scale = 4;
	}

	sim->timer = tmr_new (base);
	if (sim->timer == NULL) {
		return;
	}

	tmr_set_irq_f (sim->timer, 0, ict_set_irq4, sim->intc);
	tmr_set_irq_f (sim->timer, 1, ict_set_irq5, sim->intc);
	tmr_set_irq_f (sim->timer, 2, ict_set_irq6, sim->intc);
	tmr_set_irq_f (sim->timer, 3, ict_set_irq7, sim->intc);

	tmr_set_scale (sim->timer, scale);

	mem_add_blk (sim->mem, tmr_get_io (sim->timer, 0), 0);

	pce_log (MSG_INF, "TIMER:      base=0x%08lx ts=%u\n", base, scale);
}

static
void sarm_setup_serport (simarm_t *sim, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long base;
	unsigned      irq;
	unsigned      fifo;
	const char    *fname;
	const char    *chip;
	ini_sct_t     *sct;
	serport_t     *ser;

	sct = ini_sct_find_sct (ini, "serial");
	if (sct == NULL) {
		return;
	}

	i = 0;
	while (sct != NULL) {
		ini_get_uint32 (sct, "io", &base, 0xc0030000UL);
		ini_get_uint16 (sct, "irq", &irq, 2);
		ini_get_uint16 (sct, "fifo", &fifo, 16);
		ini_get_string (sct, "uart", &chip, "8250");
		ini_get_string (sct, "file", &fname, NULL);

		pce_log (MSG_INF, "UART%02u:     io=0x%08lx irq=%u uart=%s file=%s\n",
			i, base, irq, chip, (fname == NULL) ? "<none>" : fname
		);

		ser = ser_new (base, 2);

		if (ser == NULL) {
			pce_log (MSG_ERR, "*** serial port setup failed [%08lX/%u -> %s]\n",
				base, irq, (fname == NULL) ? "<none>" : fname
			);
		}
		else {
			if (fname != NULL) {
				if (ser_set_fname (ser, fname)) {
					pce_log (MSG_ERR, "*** can't open file (%s)\n", fname);
				}
			}
			else {
				ser_set_fp (ser, stdout, 0);
			}

			e8250_set_buf_size (ser_get_uart (ser), fifo, fifo);

			if (e8250_set_chip_str (ser_get_uart (ser), chip)) {
				pce_log (MSG_ERR, "*** unknown UART chip (%s)\n", chip);
			}

			e8250_set_irq_f (ser_get_uart (ser),
				ict_get_irq_f (sim->intc, irq), sim->intc
			);

			mem_add_blk (sim->mem, ser_get_reg (ser), 0);

			dev_lst_add (&sim->dev, ser_get_device (ser));

			i += 1;
		}

		sct = ini_sct_find_next (sct, "serial");
	}
}

static
void sarm_setup_console (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  seridx;
	ini_sct_t *sct;

	/* default */
	sim->console = dev_lst_get_ext (&sim->dev, "uart", 0);

	sct = ini_sct_find_sct (ini, "console");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "serial", &seridx, 0);

	pce_log (MSG_INF, "CONSOLE:    serport=%u\n", seridx);

	sim->console = dev_lst_get_ext (&sim->dev, "uart", seridx);

	if (sim->console == NULL) {
		pce_log (MSG_ERR, "*** no serial port (%u)\n", seridx);
	}
}

static
void sarm_setup_slip (simarm_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	unsigned   seridx;
	const char *name;
	device_t   *ser;

	sct = ini_sct_find_sct (ini, "slip");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "serial", &seridx, 0);
	ini_get_string (sct, "interface", &name, "tun0");

	pce_log (MSG_INF, "SLIP:       serport=%u interface=%s\n", seridx, name);

	ser = dev_lst_get (&sim->dev, "uart", seridx);
	if (ser == NULL) {
		pce_log (MSG_ERR, "*** no serial port (%u)\n", seridx);
		return;
	}

	sim->slip = slip_new();
	if (sim->slip == NULL) {
		return;
	}

	slip_set_serport (sim->slip, ser->ext);

	if (slip_set_tun (sim->slip, name)) {
		pce_log (MSG_ERR, "*** creating tun interface failed (%s)\n", name);
	}
}

static
void sarm_setup_disks (simarm_t *sim, ini_sct_t *ini)
{
	sim->dsks = ini_get_disks (ini);
}

static
void sarm_setup_pci (simarm_t *sim, ini_sct_t *ini)
{
	sim->pci = pci_ixp_new();

	pci_ixp_set_endian (sim->pci, sim->bigendian);

	mem_add_blk (sim->mem, pci_ixp_get_mem_io (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_cfg (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_special (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_pcicfg (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_csr (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_mem (sim->pci), 0);

	pci_set_irq_f (&sim->pci->bus, NULL, NULL);

	pce_log (MSG_INF, "PCI:        initialized\n");
}

static
void sarm_setup_ata (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  pcidev, pciirq;
	unsigned  chn, dev, drv;
	ini_sct_t *sct;
	disk_t    *dsk;

	pci_ata_init (&sim->pciata);

	sct = ini_sct_find_sct (ini, "pci_ata");
	if (sct == NULL) {
		return;
	}

	pcidev = ini_get_lng_def (sct, "pci_device", 1);
	pciirq = ini_get_lng_def (sct, "pci_irq", 31);

	pce_log (MSG_INF, "PCI-ATA:    pcidev=%u irq=%u\n", pcidev, pciirq);

	pci_ixp_add_device (sim->pci, &sim->pciata.pci);
	pci_set_device (&sim->pci->bus, &sim->pciata.pci, pcidev);
//  pci_dev_set_irq_f (&sim->pciata.pci, 0, pci_set_intr_a, &sim->pci->bus);
	pci_dev_set_irq_f (&sim->pciata.pci, 0,
		ict_get_irq_f (sim->intc, pciirq), sim->intc
	);

	sct = ini_sct_find_sct (sct, "device");

	while (sct != NULL) {
		chn = ini_get_lng_def (sct, "channel", 0);
		dev = ini_get_lng_def (sct, "device", 0);
		drv = ini_get_lng_def (sct, "drive", 0);

		dsk = dsks_get_disk (sim->dsks, drv);
		if (dsk == NULL) {
			pce_log (MSG_ERR, "*** no such drive (%u)\n", drv);
		}
		else {
			pce_log (MSG_INF, "PCI-ATA:    chn=%u dev=%u dsk=%u chs=%lu/%lu/%lu\n",
				chn, dev, drv,
				(unsigned long) dsk->visible_c,
				(unsigned long) dsk->visible_h,
				(unsigned long) dsk->visible_s
			);

			pci_ata_set_block (&sim->pciata, dsk, 2 * chn + dev);
		}

		sct = ini_sct_find_next (sct, "device");
	}
}

static
void sarm_load_mem (simarm_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *fmt;
	const char    *fname;
	unsigned long addr;

	sct = ini_sct_find_sct (ini, "load");

	while (sct != NULL) {
		fmt = ini_get_str_def (sct, "format", "binary");
		fname = ini_get_str (sct, "file");
		addr = ini_get_lng_def (sct, "base", 0);

		if (fname != NULL) {
			pce_log (MSG_INF, "Load:\tformat=%s file=%s\n",
				fmt, (fname != NULL) ? fname : "<none>"
			);

			if (pce_load_mem (sim->mem, fname, fmt, addr)) {
				pce_log (MSG_ERR, "*** loading failed (%s)\n", fname);
			}
		}

		sct = ini_sct_find_next (sct, "load");
	}
}

simarm_t *sarm_new (ini_sct_t *ini)
{
	unsigned i;
	simarm_t *sim;

	sim = (simarm_t *) malloc (sizeof (simarm_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->ram = NULL;
	sim->brk = 0;
	sim->clk_cnt = 0;

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	sim->cfg = ini;

	sim->brkpt = NULL;

	dev_lst_init (&sim->dev);

	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	sarm_setup_cpu (sim, ini);
	sarm_setup_intc (sim, ini);
	sarm_setup_timer (sim, ini);
	sarm_setup_serport (sim, ini);
	sarm_setup_console (sim, ini);
	sarm_setup_slip (sim, ini);
	sarm_setup_disks (sim, ini);
	sarm_setup_pci (sim, ini);
	sarm_setup_ata (sim, ini);

	sarm_load_mem (sim, ini);

	return (sim);
}

void sarm_del (simarm_t *sim)
{
	if (sim == NULL) {
		return;
	}

	pci_ata_free (&sim->pciata);
	pci_ixp_del (sim->pci);

	dsks_del (sim->dsks);

	slip_del (sim->slip);

	tmr_del (sim->timer);
	ict_del (sim->intc);

	arm_del (sim->cpu);

	mem_del (sim->mem);

	dev_lst_free (&sim->dev);

	free (sim);
}

unsigned long long sarm_get_clkcnt (simarm_t *sim)
{
	return (sim->clk_cnt);
}

void sarm_break (simarm_t *sim, unsigned char val)
{
	if ((val == PCE_BRK_STOP) || (val == PCE_BRK_ABORT)) {
		sim->brk = val;
	}
}

void sarm_set_keycode (simarm_t *sim, unsigned char val)
{
	if (sim->console != NULL) {
		ser_receive (sim->console, val);
	}
}

void sarm_reset (simarm_t *sim)
{
	arm_reset (sim->cpu);
}

void sarm_clock (simarm_t *sim, unsigned n)
{
	if (sim->clk_div[0] >= 4096) {
		if (sim->slip != NULL) {
			slip_clock (sim->slip, 4096);
		}

		sim->clk_div[1] += sim->clk_div[0];
		sim->clk_div[0] &= 4095;

		if (sim->clk_div[1] >= 16384) {
			scon_check (sim);

			sim->clk_div[0] &= 16383;
		}
	}

	dev_lst_clock (&sim->dev, n);

	tmr_clock (sim->timer, n);

	arm_clock (sim->cpu, n);

	sim->clk_cnt += n;
	sim->clk_div[0] += n;
}

int sarm_set_msg (simarm_t *sim, const char *msg, const char *val)
{
	/* a hack, for debugging only */
	if (sim == NULL) {
		sim = par_sim;
	}

	if (msg == NULL) {
		msg = "";
	}

	if (val == NULL) {
		val = "";
	}

	if (msg_is_message ("emu.stop", msg)) {
		sim->brk = PCE_BRK_STOP;
		return (0);
	}
	else if (strcmp (msg, "emu.exit") == 0) {
		sim->brk = PCE_BRK_ABORT;
		return (0);
	}

	pce_log (MSG_DEB, "msg (\"%s\", \"%s\")\n", msg, val);

	if (msg_is_message ("disk.commit", msg)) {
		if (strcmp (val, "") == 0) {
			if (dsks_commit (sim->dsks)) {
				pce_log (MSG_ERR, "commit failed for at least one disk\n");
				return (1);
			}
		}
		else {
			unsigned d;

			d = strtoul (val, NULL, 0);

			if (dsks_set_msg (sim->dsks, d, "commit", NULL)) {
				pce_log (MSG_ERR, "commit failed (%s)\n", val);
				return (1);
			}
		}

		return (0);
	}
	else if (msg_is_message ("emu.timer_scale", msg)) {
		unsigned long v;

		v = strtoul (val, NULL, 0);

		tmr_set_scale (sim->timer, v);

		return (0);
	}
	else if (msg_is_message ("emu.config.save", msg)) {
		if (ini_write (sim->cfg, val)) {
			return (1);
		}

		return (0);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
