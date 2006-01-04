/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/sim405.c                                   *
 * Created:       2004-06-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-01-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1999-2006 Hampa Hug <hampa@hampa.ch>                   *
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


static unsigned long s405_get_dcr (void *ext, unsigned long dcrn);
static void s405_set_dcr (void *ext, unsigned long dcrn, unsigned long val);

void s405_break (sim405_t *sim, unsigned char val);


static
void s405_setup_nvram (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *fname;
	unsigned long base, size;

	sim->nvr = NULL;

	sct = ini_sct_find_sct (ini, "nvram");
	if (sct == NULL) {
		return;
	}

	fname = ini_get_str (sct, "file");
	base = ini_get_lng_def (sct, "base", 0);
	size = ini_get_lng_def (sct, "size", 65536L);

	pce_log (MSG_INF, "NVRAM:\tbase=0x%08lx size=%lu file=%s\n",
		base, size, (fname == NULL) ? "<>" : fname
	);

	sim->nvr = nvr_new (base, size);
	if (sim->nvr == NULL) {
		pce_log (MSG_ERR, "*** creating nvram failed\n");
		return;
	}

	nvr_set_endian (sim->nvr, 1);

	mem_add_blk (sim->mem, nvr_get_mem (sim->nvr), 0);

	if (fname != NULL) {
		if (nvr_set_fname (sim->nvr, fname)) {
			pce_log (MSG_ERR, "*** loading nvram failed (%s)\n", fname);
		}
	}
}

static
void s405_setup_ppc (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *model;
	unsigned long uicinv;
	unsigned      timer_scale;

	sct = ini_sct_find_sct (ini, "powerpc");

	model = ini_get_str_def (sct, "model", "ppc405");
	uicinv = ini_get_lng_def (sct, "uic_invert", 0x0000007f);
	timer_scale = ini_get_lng_def (sct, "timer_scale", 8);

	pce_log (MSG_INF, "CPU:\tmodel=%s uicinv=%08lX ts=%u\n",
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
	p405uic_set_nint_f (&sim->uic, (p405uic_int_f) &p405_interrupt, sim->ppc);
}

static
void s405_setup_serport (sim405_t *sim, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long base;
	unsigned      irq;
	const char    *fname;
	const char    *chip;
	ini_sct_t     *sct;

	static unsigned long defbase[4] = { 0xef600300UL, 0xef600400UL };
	static unsigned      defirq[4] = { 0, 1 };


	sim->serport[0] = NULL;
	sim->serport[1] = NULL;

	sct = ini_sct_find_sct (ini, "serial");
	if (sct == NULL) {
		return;
	}

	i = 0;
	while ((i < 2) && (sct != NULL)) {
		base = ini_get_lng_def (sct, "io", defbase[i]);
		irq = ini_get_lng_def (sct, "irq", defirq[i]);
		chip = ini_get_str_def (sct, "uart", "8250");
		fname = ini_get_str (sct, "file");

		pce_log (MSG_INF, "UART%u:\tio=0x%08lx irq=%u uart=%s file=%s\n",
			i, base, irq, chip, (fname == NULL) ? "<none>" : fname
		);

		sim->serport[i] = ser_new (base, 0);
		if (sim->serport[i] == NULL) {
			pce_log (MSG_ERR, "*** serial port setup failed [%08lX/%u -> %s]\n",
				base, irq, (fname == NULL) ? "<none>" : fname
			);
		}
		else {
			if (fname != NULL) {
				if (ser_set_fname (sim->serport[i], fname)) {
					pce_log (MSG_ERR, "*** can't open file (%s)\n", fname);
				}
			}
			else {
				ser_set_fp (sim->serport[i], stdout, 0);
			}

			if (e8250_set_chip_str (&sim->serport[i]->uart, chip)) {
				pce_log (MSG_ERR, "*** unknown UART chip (%s)\n", chip);
			}

			e8250_set_irq_f (&sim->serport[i]->uart,
				p405uic_get_irq_f (&sim->uic, irq), &sim->uic
			);

			mem_add_blk (sim->mem, ser_get_reg (sim->serport[i]), 0);

			i += 1;
		}

		sct = ini_sct_find_next (sct, "serial");
	}
}

static
void s405_setup_slip (sim405_t *sim, ini_sct_t *ini)
{
	ini_sct_t  *sct;
	unsigned   ser;
	const char *name;

	sct = ini_sct_find_sct (ini, "slip");
	if (sct == NULL) {
		return;
	}

	ser = ini_get_lng_def (sct, "serial", 0);
	name = ini_get_str_def (sct, "interface", "tun0");

	pce_log (MSG_INF, "SLIP:\tserport=%u interface=%s\n", ser, name);

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

	slip_set_serport (sim->slip, sim->serport[ser]);

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

	pci_set_irq_f (&sim->pci->bus, p405uic_get_irq_f (&sim->uic, 31), &sim->uic);

	pce_log (MSG_INF, "PCI:\tirq=%u\n", 31);
}

static
void s405_setup_ata (sim405_t *sim, ini_sct_t *ini)
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

	pce_log (MSG_INF, "PCI-ATA:\tpcidev=%u irq=%u\n", pcidev, pciirq);

	pci_set_device (&sim->pci->bus, &sim->pciata.pci, pcidev);
	pci_dev_set_irq_f (&sim->pciata.pci, 0,
		p405uic_get_irq_f (&sim->uic, pciirq), &sim->uic
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
			pce_log (MSG_INF, "PCI-ATA:\tchn=%u dev=%u dsk=%u chs=%lu/%lu/%lu\n",
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
void s405_load_mem (sim405_t *sim, ini_sct_t *ini)
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

sim405_t *s405_new (ini_sct_t *ini)
{
	unsigned i;
	sim405_t *sim;

	sim = (sim405_t *) malloc (sizeof (sim405_t));
	if (sim == NULL) {
		return (NULL);
	}

	sim->brk = 0;
	sim->clk_cnt = 0;

	for (i = 0; i < 4; i++) {
		sim->clk_div[i] = 0;
	}

	sim->brkpt = NULL;

	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);

	s405_setup_nvram (sim, ini);
	s405_setup_ppc (sim, ini);
	s405_setup_serport (sim, ini);
	s405_setup_slip (sim, ini);
	s405_setup_disks (sim, ini);
	s405_setup_pci (sim, ini);
	s405_setup_ata (sim, ini);

	sim->cpc0_cr1 = 0x00000000UL;
	sim->cpc0_psr = 0x00000400UL;

	s405_load_mem (sim, ini);

	return (sim);
}

void s405_del (sim405_t *sim)
{
	if (sim == NULL) {
		return;
	}

	pci_ata_free (&sim->pciata);
	s405_pci_del (sim->pci);

	dsks_del (sim->dsks);

	slip_del (sim->slip);

	ser_del (sim->serport[1]);
	ser_del (sim->serport[0]);

	p405uic_free (&sim->uic);
	p405_del (sim->ppc);

	mem_del (sim->mem);

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

		default:
			pce_log (MSG_DEB, "get dcr %03lx\n", dcrn);
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

		default:
			pce_log (MSG_DEB, "set dcr %03lx <- %08lx\n", dcrn, val);
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
	ser_receive (sim->serport[1], val);
}

void s405_reset (sim405_t *sim)
{
	p405_reset (sim->ppc);
}

void s405_clock (sim405_t *sim, unsigned n)
{
	if (sim->clk_div[0] >= 4096) {
		if (sim->serport[0] != NULL) {
			ser_clock (sim->serport[0], 4096);
		}

		if (sim->serport[1] != NULL) {
			ser_clock (sim->serport[1], 4096);
		}

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
