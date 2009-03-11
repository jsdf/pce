/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim405/sim405.c                                     *
 * Created:     2004-06-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1999-2009 Hampa Hug <hampa@hampa.ch>                     *
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


static unsigned long s405_get_dcr (void *ext, unsigned long dcrn);
static void s405_set_dcr (void *ext, unsigned long dcrn, unsigned long val);

void s405_break (sim405_t *sim, unsigned char val);


static
void s405_setup_ppc (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *model;
	unsigned long uicinv;
	unsigned      timer_scale;

	sct = ini_next_sct (ini, NULL, "powerpc");

	ini_get_string (sct, "model", &model, "ppc405");
	ini_get_uint32 (sct, "uic_invert", &uicinv, 0x0000007f);
	ini_get_uint16 (sct, "timer_scale", &timer_scale, 8);

	pce_log_tag (MSG_INF, "CPU:", "model=%s uicinv=%08lX ts=%u\n",
		model, uicinv, timer_scale
	);

	sim->ppc = p405_new ();

	p405_set_mem_fct (sim->ppc, sim->mem,
		&mem_get_uint8,
		&mem_get_uint16_be,
		&mem_get_uint32_be,
		&mem_set_uint8,
		&mem_set_uint16_be,
		&mem_set_uint32_be
	);

	if (sim->ram != NULL) {
		p405_set_ram (sim->ppc, mem_blk_get_data (sim->ram), mem_blk_get_size (sim->ram));
	}

	p405_set_dcr_fct (sim->ppc, sim, &s405_get_dcr, &s405_set_dcr);

	p405_set_timer_scale (sim->ppc, timer_scale);

	p405uic_init (&sim->uic);
	p405uic_set_invert (&sim->uic, uicinv);
	p405uic_set_nint_fct (&sim->uic, sim->ppc, p405_interrupt);
}

static
void s405_setup_serport (sim405_t *sim, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long addr;
	unsigned      irq;
	unsigned      multichar;
	const char    *driver;
	const char    *chip;
	ini_sct_t     *sct;

	static unsigned long defbase[2] = { 0xef600300, 0xef600400 };
	static unsigned      defirq[2] = { 0, 1 };


	sim->serport[0] = NULL;
	sim->serport[1] = NULL;

	i = 0;
	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "serial")) != NULL) {
		if (i >= 2) {
			break;
		}

		ini_get_uint32 (sct, "address", &addr, defbase[i]);
		ini_get_uint16 (sct, "irq", &irq, defirq[i]);
		ini_get_string (sct, "uart", &chip, "8250");
		ini_get_uint16 (sct, "multichar", &multichar, 1);
		ini_get_string (sct, "driver", &driver, NULL);

		pce_log_tag (MSG_INF, "UART:",
			"n=%u addr=0x%08lx irq=%u uart=%s multi=%u driver=%s\n",
			i, addr, irq, chip, multichar,
			(driver == NULL) ? "<none>" : driver
		);

		sim->serport[i] = ser_new (addr, 0);

		if (sim->serport[i] == NULL) {
			pce_log (MSG_ERR,
				"*** serial port setup failed [%08lX/%u -> %s]\n",
				addr, irq, (driver == NULL) ? "<none>" : driver
			);
		}
		else {
			e8250_t *uart;

			if (driver != NULL) {
				if (ser_set_driver (sim->serport[i], driver)) {
					pce_log (MSG_ERR, "*** can't open driver (%s)\n", driver);
				}
			}

			uart = &sim->serport[i]->uart;

			e8250_set_buf_size (uart, 256, 256);
			e8250_set_multichar (uart, multichar, multichar);

			if (e8250_set_chip_str (uart, chip)) {
				pce_log (MSG_ERR, "*** unknown UART chip (%s)\n", chip);
			}

			e8250_set_irq_fct (uart,
				&sim->uic, p405uic_get_irq_fct (&sim->uic, irq)
			);

			mem_add_blk (sim->mem, ser_get_reg (sim->serport[i]), 0);

			i += 1;
		}
	}
}

static
void s405_setup_sercons (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	unsigned   ser;

	sct = ini_next_sct (ini, NULL, "sercons");

	if (sct == NULL) {
		ser = 0;
	}
	else {
		ini_get_uint16 (sct, "serial", &ser, 0);
	}

	pce_log_tag (MSG_INF, "CONSOLE:", "serport=%u\n", ser);

	if (ser >= 2) {
		return;
	}

	sim->sercons = ser;
}

static
void s405_setup_slip (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	unsigned   ser;
	const char *name;
	e8250_t    *uart;

	sct = ini_next_sct (ini, NULL, "slip");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "serial", &ser, 0);
	ini_get_string (sct, "interface", &name, "tun0");

	pce_log_tag (MSG_INF, "SLIP:", "serport=%u interface=%s\n", ser, name);

	if (ser >= 2) {
		return;
	}

	if (sim->serport[ser] == NULL) {
		return;
	}

	sim->slip = slip_new();
	if (sim->slip == NULL) {
		return;
	}

	uart = ser_get_uart (sim->serport[ser]);

	e8250_set_send_fct (uart, sim->slip, slip_uart_check_out);
	e8250_set_recv_fct (uart, sim->slip, slip_uart_check_inp);
	e8250_set_setup_fct (uart, NULL, NULL);

	slip_set_get_uint8_fct (sim->slip, uart, e8250_send);
	slip_set_set_uint8_fct (sim->slip, uart, e8250_receive);

	if (slip_set_tun (sim->slip, name)) {
		pce_log (MSG_ERR, "*** creating tun interface failed (%s)\n", name);
	}
}

static
void s405_setup_disks (sim405_t *sim, ini_sct_t *ini)
{
	sim->dsks = ini_get_disks (ini);
}

static
void s405_setup_pci (sim405_t *sim, ini_sct_t *ini)
{
	sim->pci = s405_pci_new();

	mem_add_blk (sim->mem, s405_pci_get_mem_ioa (sim->pci), 0);
	mem_add_blk (sim->mem, s405_pci_get_mem_iob (sim->pci), 0);
	mem_add_blk (sim->mem, s405_pci_get_mem_cfg (sim->pci), 0);
	mem_add_blk (sim->mem, s405_pci_get_mem_special (sim->pci), 0);
	mem_add_blk (sim->mem, s405_pci_get_mem_csr (sim->pci), 0);

	pce_log_tag (MSG_INF, "PCI:", "initialized\n");
}

static
void s405_setup_ata (sim405_t *sim, ini_sct_t *ini)
{
	unsigned       pcidev, pciirq;
	unsigned  long vendor_id, device_id;
	ini_sct_t      *sct;

	pci_ata_init (&sim->pciata);

	sct = ini_next_sct (ini, NULL, "pci_ata");
	if (sct == NULL) {
		return;
	}

	ini_get_uint16 (sct, "pci_device", &pcidev, 1);
	ini_get_uint16 (sct, "pci_irq", &pciirq, 31);

	ini_get_uint32 (sct, "vendor_id", &vendor_id, PCIID_VENDOR_VIA);
	ini_get_uint32 (sct, "device_id", &device_id, PCIID_VIA_82C561);

	pce_log_tag (MSG_INF, "PCI-ATA:",
		"pcidev=%u irq=%u vendor=0x%04lx id=0x%04lx\n",
		pcidev, pciirq, vendor_id, device_id
	);

	pci_dev_set_device_id (&sim->pciata.pci, vendor_id, device_id);

	pci_set_device (&sim->pci->bus, &sim->pciata.pci, pcidev);
	pci_dev_set_intr_fct (&sim->pciata.pci, 0,
		&sim->uic, p405uic_get_irq_fct (&sim->uic, pciirq)
	);

	ini_get_pci_ata (&sim->pciata, sim->dsks, sct);
}

static
void s405_setup_ds1743 (sim405_t *sim, ini_sct_t *sct)
{
	unsigned long addr, size;
	const char    *fname;
	dev_ds1743_t  *rtc;

	ini_get_uint32 (sct, "address", &addr, 0xf0000000);
	ini_get_uint32 (sct, "size", &size, 8192);
	ini_get_string (sct, "file", &fname, NULL);

	pce_log_tag (MSG_INF, "DS1743:", "addr=0x%08lx size=%lu file=%s\n",
		addr, size, (fname != NULL) ? fname : "<none>"
	);

	rtc = dev_ds1743_new (addr, size);
	if (rtc == NULL) {
		return;
	}

	if (fname != NULL) {
		if (dev_ds1743_set_fname (rtc, fname)) {
			pce_log (MSG_ERR, "*** opening file failed (%s)\n",
				fname
			);
		}
	}

	mem_add_blk (sim->mem, &rtc->blk, 0);

	dev_lst_add (&sim->devlst, &rtc->dev);
}

static
void s405_setup_devices (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	const char *type;

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "device")) != NULL) {
		ini_get_string (sct, "type", &type, NULL);

		if (type != NULL) {
			if (strcmp (type, "ds1743") == 0) {
				s405_setup_ds1743 (sim, sct);
			}
			else {
				pce_log (MSG_INF, "*** unknown device '%s'\n",
					type
				);
			}
		}
	}
}

sim405_t *s405_new (ini_sct_t *ini)
{
	unsigned i;
	sim405_t *sim;

	sim = malloc (sizeof (sim405_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->brk = 0;
	sim->clk_cnt = 0;
	sim->real_clk = clock();

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	bps_init (&sim->bps);

	dev_lst_init (&sim->devlst);

	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	s405_setup_ppc (sim, ini);
	s405_setup_serport (sim, ini);
	s405_setup_sercons (sim, ini);
	s405_setup_slip (sim, ini);
	s405_setup_disks (sim, ini);
	s405_setup_pci (sim, ini);
	s405_setup_ata (sim, ini);
	s405_setup_devices (sim, ini);

	sim->ocm0_iscntl = 0;
	sim->ocm0_isarc = 0;
	sim->ocm0_dscntl = 0;
	sim->ocm0_dsarc = 0;
	sim->cpc0_cr1 = 0x00000000;
	sim->cpc0_psr = 0x00000400;

	pce_load_mem_ini (sim->mem, ini);

	return (sim);
}

void s405_del (sim405_t *sim)
{
	if (sim == NULL) {
		return;
	}

	dev_lst_free (&sim->devlst);

	pci_ata_free (&sim->pciata);
	s405_pci_del (sim->pci);

	dsks_del (sim->dsks);

	slip_del (sim->slip);

	ser_del (sim->serport[1]);
	ser_del (sim->serport[0]);

	p405uic_free (&sim->uic);
	p405_del (sim->ppc);

	mem_del (sim->mem);

	bps_free (&sim->bps);

	free (sim);
}

unsigned long long s405_get_clkcnt (sim405_t *sim)
{
	return (sim->clk_cnt);
}

static
unsigned long s405_get_dcr (void *ext, unsigned long dcrn)
{
	sim405_t *sim;

	sim = (sim405_t *) ext;

	switch (dcrn) {
	case 0:
		s405_break (sim, PCE_BRK_STOP);
		break;

	case SIM405_DCRN_OCM0_ISARC: /* 0x18 */
		return (sim->ocm0_isarc);

	case SIM405_DCRN_OCM0_ISCNTL: /* 0x19 */
		return (sim->ocm0_iscntl);

	case SIM405_DCRN_OCM0_DSARC: /* 0x1a */
		return (sim->ocm0_dsarc);

	case SIM405_DCRN_OCM0_DSCNTL: /* 0x1b */
		return (sim->ocm0_dscntl);

	case SIM405_DCRN_CPC0_CR1: /* 0xb2 */
		return (sim->cpc0_cr1);

	case SIM405_DCRN_CPC0_PSR: /* 0xb4 */
		return (sim->cpc0_psr);

	case SIM405_DCRN_UIC0_SR:
		return (p405uic_get_sr (&sim->uic));

	case SIM405_DCRN_UIC0_ER:
		return (p405uic_get_er (&sim->uic));

	case SIM405_DCRN_UIC0_CR:
		return (p405uic_get_cr (&sim->uic));

	case SIM405_DCRN_UIC0_PR:
		return (p405uic_get_er (&sim->uic));

	case SIM405_DCRN_UIC0_TR:
		return (p405uic_get_er (&sim->uic));

	case SIM405_DCRN_UIC0_MSR:
		return (p405uic_get_msr (&sim->uic));

	case SIM405_DCRN_UIC0_VR:
		return (p405uic_get_vr (&sim->uic));

	case SIM405_DCRN_MAL0_CFG: /* 0x180 */
		return (0);

	default:
		pce_log (MSG_DEB, "%08lX: get dcr %03lx\n",
			(unsigned long) p405_get_pc (sim->ppc), dcrn
		);
		break;
	}

	return (0);
}

static
void s405_set_dcr (void *ext, unsigned long dcrn, unsigned long val)
{
	sim405_t *sim;

	sim = (sim405_t *) ext;

	switch (dcrn) {
	case SIM405_DCRN_OCM0_ISARC: /* 0x18 */
		sim->ocm0_isarc = val;
		break;

	case SIM405_DCRN_OCM0_ISCNTL: /* 0x19 */
		sim->ocm0_iscntl = val;
		if (val & 0x80000000) {
			pce_log (MSG_DEB,
				"%08lX: instruction side ocm0 enabled\n",
				(unsigned long) p405_get_pc (sim->ppc)
			);
		}
		break;

	case SIM405_DCRN_OCM0_DSARC: /* 0x1a */
		sim->ocm0_dsarc = val;
		break;

	case SIM405_DCRN_OCM0_DSCNTL: /* 0x1b */
		sim->ocm0_dscntl = val;
		if (val & 0x80000000) {
			pce_log (MSG_DEB,
				"%08lX: data side ocm0 enabled\n",
				(unsigned long) p405_get_pc (sim->ppc)
			);
		}
		break;

	case SIM405_DCRN_CPC0_CR1: /* 0xb2 */
		sim->cpc0_cr1 = val;
		break;

	case SIM405_DCRN_CPC0_PSR: /* 0xb4 */
		sim->cpc0_psr = val;
		break;

	case SIM405_DCRN_UIC0_SR:
		p405uic_set_sr (&sim->uic, val);
		break;

	case SIM405_DCRN_UIC0_ER:
		p405uic_set_er (&sim->uic, val);
		break;

	case SIM405_DCRN_UIC0_CR:
		p405uic_set_cr (&sim->uic, val);
		break;

	case SIM405_DCRN_UIC0_PR:
		p405uic_set_pr (&sim->uic, val);
		break;

	case SIM405_DCRN_UIC0_TR:
		p405uic_set_tr (&sim->uic, val);
		break;

	case SIM405_DCRN_UIC0_VCR:
		p405uic_set_vcr (&sim->uic, val);
		break;

	case SIM405_DCRN_MAL0_CFG: /* 0x180 */
		break;

	default:
		pce_log (MSG_DEB, "%08lX: set dcr %03lx <- %08lx\n",
			(unsigned long) p405_get_pc (sim->ppc), dcrn, val
		);
		break;
	}
}

void s405_break (sim405_t *sim, unsigned char val)
{
	if ((val == PCE_BRK_STOP) || (val == PCE_BRK_ABORT)) {
		sim->brk = val;
	}
}

void s405_set_keycode (sim405_t *sim, unsigned char val)
{
	if (sim->serport[sim->sercons] != NULL) {
		ser_receive (sim->serport[sim->sercons], val);
	}
}

void s405_reset (sim405_t *sim)
{
	p405_reset (sim->ppc);
}

void s405_clock (sim405_t *sim, unsigned n)
{
	unsigned long clk;

	if (sim->clk_div[0] >= 256) {
		clk = sim->clk_div[0] & ~255UL;
		sim->clk_div[1] += clk;
		sim->clk_div[0] &= 255;

		if (sim->serport[0] != NULL) {
			e8250_clock (&sim->serport[0]->uart, clk / 4);
		}

		if (sim->serport[1] != NULL) {
			e8250_clock (&sim->serport[1]->uart, clk / 4);
		}


		if (sim->clk_div[1] >= 4096) {
			clk = sim->clk_div[1] & ~4095UL;
			sim->clk_div[2] += clk;
			sim->clk_div[1] &= 4095UL;

			if (sim->serport[0] != NULL) {
				ser_clock (sim->serport[0], clk);
			}

			if (sim->serport[1] != NULL) {
				ser_clock (sim->serport[1], clk);
			}

			if (sim->slip != NULL) {
				slip_clock (sim->slip, clk);
			}

			if (sim->clk_div[2] >= 16384) {
				scon_check (sim);

				sim->clk_div[2] &= 16383;
			}
		}
	}

	p405_clock (sim->ppc, n);

	sim->clk_cnt += n;
	sim->clk_div[0] += n;
}

int s405_set_msg (sim405_t *sim, const char *msg, const char *val)
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

	if (msg_is_message ("emu.break", msg)) {
		if (strcmp (val, "stop") == 0) {
			sim->brk = PCE_BRK_STOP;
			return (0);
		}
		else if (strcmp (val, "abort") == 0) {
			sim->brk = PCE_BRK_ABORT;
			return (0);
		}
		else if (strcmp (val, "") == 0) {
			sim->brk = PCE_BRK_ABORT;
			return (0);
		}
	}
	else if (msg_is_message ("emu.stop", msg)) {
		sim->brk = PCE_BRK_STOP;
		return (0);
	}
	else if (msg_is_message ("emu.exit", msg)) {
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

		p405_set_timer_scale (sim->ppc, v);

		return (0);
	}

	pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

	return (1);
}
