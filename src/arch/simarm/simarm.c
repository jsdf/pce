/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/simarm.c                                     *
 * Created:     2004-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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
	unsigned      scale;
	ini_sct_t     *sct;

	sct = ini_next_sct (ini, NULL, "timer");

	ini_get_uint32 (sct, "address", &addr, 0xc0020000);
	ini_get_uint16 (sct, "scale", &scale, 4);

	sim->timer = tmr_new (addr);
	if (sim->timer == NULL) {
		return;
	}

	tmr_set_irq_f (sim->timer, 0, ict_set_irq4, sim->intc);
	tmr_set_irq_f (sim->timer, 1, ict_set_irq5, sim->intc);
	tmr_set_irq_f (sim->timer, 2, ict_set_irq6, sim->intc);
	tmr_set_irq_f (sim->timer, 3, ict_set_irq7, sim->intc);

	tmr_set_scale (sim->timer, scale);

	mem_add_blk (sim->mem, tmr_get_io (sim->timer, 0), 0);

	pce_log_tag (MSG_INF, "TIMER:", "addr=0x%08lx ts=%u\n", addr, scale);
}

static
void sarm_setup_serport (simarm_t *sim, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long addr;
	unsigned      irq;
	unsigned      fifo;
	unsigned      rate_cnt;
	unsigned long rate_clk;
	const char    *fname;
	const char    *chip;
	ini_sct_t     *sct;
	serport_t     *ser;

	i = 0;
	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "serial")) != NULL) {
		if (ini_get_uint32 (sct, "address", &addr, 0xc0030000)) {
			ini_get_uint32 (sct, "io", &addr, 0xc0030000);
		}
		ini_get_uint16 (sct, "irq", &irq, 2);
		ini_get_uint16 (sct, "fifo", &fifo, 16);
		ini_get_uint16 (sct, "rate_chars", &rate_cnt, 16);
		ini_get_uint32 (sct, "rate_clocks", &rate_clk, 16384);
		ini_get_string (sct, "uart", &chip, "8250");
		ini_get_string (sct, "file", &fname, NULL);

		pce_log_tag (MSG_INF, "UART:",
			"n=%u addr=0x%08lx irq=%u uart=%s rate=%u/%lu file=%s\n",
			i, addr, irq, chip, rate_cnt, rate_clk,
			(fname == NULL) ? "<none>" : fname
		);

		ser = ser_new (addr, 2);

		if (ser == NULL) {
			pce_log (MSG_ERR,
				"*** serial port setup failed [%08lX/%u -> %s]\n",
				addr, irq, (fname == NULL) ? "<none>" : fname
			);
		}
		else {
			if (fname != NULL) {
				if (ser_set_fname (ser, fname)) {
					pce_log (MSG_ERR,
						"*** can't open file (%s)\n",
						fname
					);
				}
			}
			else {
				ser_set_fp (ser, stdout, 0);
			}

			e8250_set_buf_size (ser_get_uart (ser), fifo, fifo);
			e8250_set_rate (ser_get_uart (ser), rate_cnt, rate_clk);

			if (e8250_set_chip_str (ser_get_uart (ser), chip)) {
				pce_log (MSG_ERR,
					"*** unknown UART chip (%s)\n", chip
				);
			}

			e8250_set_irq_fct (ser_get_uart (ser),
				sim->intc, ict_get_irq_f (sim->intc, irq)
			);

			mem_add_blk (sim->mem, ser_get_reg (ser), 0);

			dev_lst_add (&sim->dev, ser_get_device (ser));

			i += 1;
		}
	}
}

static
void sarm_setup_console (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  seridx;
	ini_sct_t *sct;

	/* default */
	sim->console = dev_lst_get_ext (&sim->dev, "uart", 0);

	sct = ini_next_sct (ini, NULL, "console");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "serial", &seridx, 0);

	pce_log_tag (MSG_INF, "CONSOLE:", "serport=%u\n", seridx);

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
	e8250_t    *uart;

	sct = ini_next_sct (ini, NULL, "slip");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "serial", &seridx, 0);
	ini_get_string (sct, "interface", &name, "tun0");

	pce_log_tag (MSG_INF, "SLIP:", "serport=%u interface=%s\n",
		seridx, name
	);

	ser = dev_lst_get (&sim->dev, "uart", seridx);
	if (ser == NULL) {
		pce_log (MSG_ERR, "*** no serial port (%u)\n", seridx);
		return;
	}

	sim->slip = slip_new();
	if (sim->slip == NULL) {
		return;
	}

	uart = ser_get_uart (ser->ext);

	e8250_set_send_fct (uart, sim->slip, slip_uart_check_out);
	e8250_set_recv_fct (uart, sim->slip, slip_uart_check_inp);
	e8250_set_setup_fct (uart, NULL, NULL);

	slip_set_get_uint8_fct (sim->slip, uart, e8250_send);
	slip_set_set_uint8_fct (sim->slip, uart, e8250_receive);

	if (slip_set_tun (sim->slip, name)) {
		pce_log (MSG_ERR, "*** creating tun interface failed (%s)\n",
			name
		);
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

	pce_log_tag (MSG_INF, "PCI:", "initialized\n");
}

static
void sarm_setup_ata (simarm_t *sim, ini_sct_t *ini)
{
	unsigned  pcidev, pciirq;
	ini_sct_t *sct;

	pci_ata_init (&sim->pciata);

	sct = ini_next_sct (ini, NULL, "pci_ata");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "pci_device", &pcidev, 1);
	ini_get_uint16 (sct, "pci_irq", &pciirq, 31);

	pce_log_tag (MSG_INF, "PCI-ATA:", "pcidev=%u irq=%u\n",
		pcidev, pciirq
	);

	pci_ixp_add_device (sim->pci, &sim->pciata.pci);
	pci_set_device (&sim->pci->bus, &sim->pciata.pci, pcidev);
	pci_dev_set_intr_fct (&sim->pciata.pci, 0,
		sim->intc, ict_get_irq_f (sim->intc, pciirq)
	);

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

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	sim->cfg = ini;

	bps_init (&sim->bps);

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

	slip_del (sim->slip);

	tmr_del (sim->timer);
	ict_del (sim->intc);

	arm_del (sim->cpu);

	mem_del (sim->mem);

	dev_lst_free (&sim->dev);

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
	if (sim->clk_div[0] >= 256) {
		tmr_clock (sim->timer, 256);

		sim->clk_div[1] += sim->clk_div[0];
		sim->clk_div[0] &= 255;

		if (sim->clk_div[1] >= 4096) {
			dev_lst_clock (&sim->dev, 4096);

			if (sim->slip != NULL) {
				slip_clock (sim->slip, 4096);
			}

			sim->clk_div[2] += sim->clk_div[1];
			sim->clk_div[1] &= 4095;

			if (sim->clk_div[2] >= 16384) {
				scon_check (sim);

				sim->clk_div[2] &= 16383;
			}
		}
	}

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
