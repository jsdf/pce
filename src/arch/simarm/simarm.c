/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/simarm.c                                     *
 * Created:     2004-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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


#include "main.h"
#include "timer.h"


void sarm_break (simarm_t *sim, unsigned char val);


static
void sarm_setup_cpu (simarm_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *model;
	unsigned long id;

	sct = ini_next_sct (ini, NULL, "cpu");

	ini_get_string (sct, "model", &model, "armv5");
	ini_get_bool (sct, "bigendian", &sim->bigendian, 1);

	if (strcmp (model, "xscale") == 0) {
		id = 0x69052000;
	}
	else if (strcmp (model, "ixp2400") == 0) {
		id = 0x69054190;
	}
	else {
		id = 0x69054190;
	}

	ini_get_uint32 (sct, "id", &id, id);

	pce_log_tag (MSG_INF, "CPU:", "model=%s id=0x%08lx endian=%s\n",
		model, id, sim->bigendian ? "big" : "little"
	);

	sim->cpu = arm_new();
	if (sim->cpu == NULL) {
		return;
	}

	arm_set_flags (sim->cpu, ARM_FLAG_XSCALE, 1);
	arm_set_flags (sim->cpu, ARM_FLAG_BIGENDIAN, sim->bigendian);

	arm_set_id (sim->cpu, id);

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
	unsigned long addr;
	ini_sct_t     *sct;

	sct = ini_next_sct (ini, NULL, "intc");

	ini_get_uint32 (sct, "address", &addr, 0xd6000000);

	sim->intc = ict_new (addr);
	if (sim->intc == NULL) {
		return;
	}

	ict_set_fiq_f (sim->intc, arm_set_fiq, sim->cpu);
	ict_set_irq_f (sim->intc, arm_set_irq, sim->cpu);

	mem_add_blk (sim->mem, ict_get_io (sim->intc, 0), 0);

	pce_log_tag (MSG_INF, "INTC:", "addr=0x%08lx\n", addr);
}

static
void sarm_setup_timer (simarm_t *sim, ini_sct_t *ini)
{
	unsigned long addr;
	ini_sct_t     *sct;

	sct = ini_next_sct (ini, NULL, "timer");

	ini_get_uint32 (sct, "address", &addr, 0xc0020000);

	sim->timer = tmr_new (addr);
	if (sim->timer == NULL) {
		return;
	}

	tmr_set_irq_f (sim->timer, 0, ict_set_irq4, sim->intc);
	tmr_set_irq_f (sim->timer, 1, ict_set_irq5, sim->intc);
	tmr_set_irq_f (sim->timer, 2, ict_set_irq6, sim->intc);
	tmr_set_irq_f (sim->timer, 3, ict_set_irq7, sim->intc);

	mem_add_blk (sim->mem, tmr_get_io (sim->timer, 0), 0);

	sim->rclk_interval = 0;

	pce_log_tag (MSG_INF, "TIMER:", "addr=0x%08lx\n", addr);
}

static
void sarm_setup_serport (simarm_t *sim, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long addr;
	unsigned      irq;
	unsigned      multichar;
	const char    *driver;
	const char    *chip;
	ini_sct_t     *sct;
	serport_t     *ser;
	e8250_t       *uart;

	sim->serport[0] = NULL;
	sim->serport[1] = NULL;

	i = 0;
	sct = NULL;

	while ((i < 2) && (sct = ini_next_sct (ini, sct, "serial")) != NULL) {
		ini_get_uint32 (sct, "address", &addr, 0xc0030000);
		ini_get_uint16 (sct, "irq", &irq, 2);
		ini_get_uint16 (sct, "multichar", &multichar, 1);
		ini_get_string (sct, "uart", &chip, "8250");
		ini_get_string (sct, "driver", &driver, NULL);

		pce_log_tag (MSG_INF, "UART:",
			"n=%u addr=0x%08lx irq=%u uart=%s multi=%u driver=%s\n",
			i, addr, irq, chip, multichar,
			(driver == NULL) ? "<none>" : driver
		);

		ser = ser_new (addr, 2);

		if (ser == NULL) {
			pce_log (MSG_ERR,
				"*** serial port setup failed [%08lX/%u -> %s]\n",
				addr, irq, (driver == NULL) ? "<none>" : driver
			);
		}
		else {
			sim->serport[i] = ser;

			uart = ser_get_uart (ser);

			if (driver != NULL) {
				if (ser_set_driver (ser, driver)) {
					pce_log (MSG_ERR,
						"*** can't open driver (%s)\n",
						driver
					);
				}
			}

			e8250_set_buf_size (uart, 256, 256);
			e8250_set_multichar (uart, multichar, multichar);

			if (e8250_set_chip_str (uart, chip)) {
				pce_log (MSG_ERR,
					"*** unknown UART chip (%s)\n", chip
				);
			}

			e8250_set_irq_fct (uart,
				sim->intc, ict_get_irq_f (sim->intc, irq)
			);

			mem_add_blk (sim->mem, ser_get_reg (ser), 0);

			i += 1;
		}
	}
}

static
void sarm_setup_console (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  ser;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "console");

	if (sct == NULL) {
		ser = 0;
	}
	else {
		ini_get_uint16 (sct, "serial", &ser, 0);
	}

	pce_log_tag (MSG_INF, "CONSOLE:", "serport=%u\n", ser);

	if ((ser >= 2) || (sim->serport[ser] == NULL)) {
		pce_log (MSG_ERR, "*** no serial port (%u)\n", ser);
		return;
	}

	sim->sercons = ser;
}

static
void sarm_setup_disks (simarm_t *sim, ini_sct_t *ini)
{
	sim->dsks = ini_get_disks (ini);
}

static
void sarm_setup_pci (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  irq;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "pci");

	ini_get_uint16 (sct, "irq", &irq, 15);

	pce_log_tag (MSG_INF, "PCI:", "irq=%u\n", irq);

	sim->pci = pci_ixp_new();

	pci_ixp_set_endian (sim->pci, sim->bigendian);

	pci_ixp_set_irq_fct (sim->pci, sim->intc, ict_get_irq_f (sim->intc, irq));

	mem_add_blk (sim->mem, pci_ixp_get_mem_io (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_cfg (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_special (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_pcicfg (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_csr (sim->pci), 0);
	mem_add_blk (sim->mem, pci_ixp_get_mem_mem (sim->pci), 0);
}

static
void sarm_setup_ata (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  dev, irq;
	ini_sct_t *sct;

	pci_ata_init (&sim->pciata);

	sct = ini_next_sct (ini, NULL, "pci_ata");

	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "pci_device", &dev, 1);
	ini_get_uint16 (sct, "pci_irq", &irq, 255);

	pce_log_tag (MSG_INF, "PCI-ATA:", "device=%u\n", dev);

	if (irq < 32) {
		pce_log_tag (MSG_INF, "PCI-ATA:", "irq=%u\n", irq);
	}

	pci_ixp_add_device (sim->pci, &sim->pciata.pci);
	pci_set_device (&sim->pci->bus, &sim->pciata.pci, dev);

	/*
	 * If an irq is specified, the ATA PCI interrupt is connected
	 * directly to the interrupt controller instead of PCI INT A.
	 * This is a hack and should not be used.
	 */
	if (irq < 32) {
		pci_dev_set_intr_fct (&sim->pciata.pci, 0,
			sim->intc, ict_get_irq_f (sim->intc, irq)
		);
	}
	else {
		pci_dev_set_intr_fct (&sim->pciata.pci, 0,
			sim->pci, pci_ixp_set_int_a
		);
	}

	ini_get_pci_ata (&sim->pciata, sim->dsks, sct);
}

simarm_t *sarm_new (ini_sct_t *ini)
{
	unsigned i;
	simarm_t *sim;

	sim = malloc (sizeof (simarm_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->ram = NULL;
	sim->brk = 0;
	sim->clk_cnt = 0;

	sim->sercons = 0;

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	sim->cfg = ini;

	bps_init (&sim->bps);

	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	sarm_setup_cpu (sim, ini);
	sarm_setup_intc (sim, ini);
	sarm_setup_timer (sim, ini);
	sarm_setup_serport (sim, ini);
	sarm_setup_console (sim, ini);
	sarm_setup_disks (sim, ini);
	sarm_setup_pci (sim, ini);
	sarm_setup_ata (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

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

	ser_del (sim->serport[1]);
	ser_del (sim->serport[0]);

	tmr_del (sim->timer);
	ict_del (sim->intc);

	arm_del (sim->cpu);

	mem_del (sim->mem);

	bps_free (&sim->bps);

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
	if (sim->serport[sim->sercons] != NULL) {
		ser_receive (sim->serport[sim->sercons], val);
	}
}

void sarm_clock_discontinuity (simarm_t *sim)
{
	pce_get_interval_us (&sim->rclk_interval);
}

void sarm_reset (simarm_t *sim)
{
	arm_reset (sim->cpu);
}

void sarm_clock (simarm_t *sim, unsigned n)
{
	unsigned long clk, rclk;

	arm_clock (sim->cpu, n);

	sim->clk_cnt += n;
	sim->clk_div[0] += n;

	if (sim->clk_div[0] < 256) {
		return;
	}

	clk = sim->clk_div[0] & ~255UL;
	sim->clk_div[1] += clk;
	sim->clk_div[0] &= 255;

	if (sim->serport[0] != NULL) {
		e8250_clock (&sim->serport[0]->uart, clk / 4);
	}

	if (sim->serport[1] != NULL) {
		e8250_clock (&sim->serport[1]->uart, clk / 4);
	}

	if (sim->clk_div[1] < 4096) {
		return;
	}

	clk = sim->clk_div[1] & ~4095UL;
	sim->clk_div[2] += clk;
	sim->clk_div[1] &= 4095;

	rclk = pce_get_interval_us (&sim->rclk_interval);

	tmr_clock (sim->timer, 50 * rclk);

	if (sim->serport[0] != NULL) {
		ser_clock (sim->serport[0], clk);
	}

	if (sim->serport[1] != NULL) {
		ser_clock (sim->serport[1], clk);
	}

	if (sim->clk_div[2] < 16384) {
		return;
	}

	sim->clk_div[2] &= 16383;

	scon_check (sim);
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

	if (msg_is_prefix ("term", msg)) {
		return (1);
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
	else if (msg_is_message ("emu.config.save", msg)) {
		if (ini_write (val, sim->cfg)) {
			return (1);
		}

		return (0);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
