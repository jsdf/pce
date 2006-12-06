/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/pci.c                                          *
 * Created:       2004-06-05 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "pci.h"


void pci_dev_init (pci_dev_t *dev)
{
	unsigned i;

	for (i = 0; i < PCI_REG_MAX; i++) {
		dev->reg[i] = NULL;
	}

	for (i = 0; i < PCI_MEM_MAX; i++) {
		dev->mem[i] = NULL;
	}

	for (i = 0; i < 256; i++) {
		dev->config[i] = 0;
		dev->wrmask[i] = 0;
	}

	dev->config[0] = 0xff;
	dev->config[1] = 0xff;

	dev->cfg_ext = NULL;
	dev->get_cfg8 = NULL;
	dev->get_cfg16 = NULL;
	dev->get_cfg32 = NULL;
	dev->set_cfg8 = NULL;
	dev->get_cfg16 = NULL;
	dev->set_cfg32 = NULL;

	dev->ext = NULL;
}

void pci_dev_free (pci_dev_t *dev)
{
}

void pci_dev_set_irq_f (pci_dev_t *dev, unsigned idx, void *irq, void *ext)
{
	if (idx == 0) {
		dev->irqa = irq;
		dev->irqa_ext = ext;
	}
}

void pci_dev_set_irq_a (pci_dev_t *dev, unsigned char val)
{
	val = (val != 0);

	if (val != dev->irqa_val) {
		dev->irqa_val = val;

		if (dev->irqa != NULL) {
			dev->irqa (dev->irqa_ext, val);
		}
	}
}

void pci_dev_set_device_id (pci_dev_t *dev, unsigned vendor, unsigned id)
{
	buf_set_uint16_le (dev->config, 0x00, vendor);
	buf_set_uint16_le (dev->config, 0x02, id);
}

void pci_dev_set_cfg_mask (pci_dev_t *dev, unsigned i, unsigned n, int val)
{
	unsigned k, m;

	while (n > 0) {
		k = i >> 3;
		m = 1 << (i & 7);

		if (k > 255) {
			return;
		}

		if (val) {
			dev->wrmask[k] |= m;
		}
		else {
			dev->wrmask[k] &= ~m;
		}

		i += 1;
		n -= 1;
	}
}

unsigned char pci_dev_get_cfg8 (pci_dev_t *dev, unsigned long addr)
{
	if (addr <= 255) {
		return (dev->config[addr]);
	}

	return (0);
}

unsigned short pci_dev_get_cfg16 (pci_dev_t *dev, unsigned long addr)
{
	unsigned short val;

	val = pci_dev_get_cfg8 (dev, addr + 1);
	val = (val << 8) | pci_dev_get_cfg8 (dev, addr);

	return (val);
}

unsigned long pci_dev_get_cfg32 (pci_dev_t *dev, unsigned long addr)
{
	unsigned long val;

	val = pci_dev_get_cfg8 (dev, addr + 3);
	val = (val << 8) | pci_dev_get_cfg8 (dev, addr + 2);
	val = (val << 8) | pci_dev_get_cfg8 (dev, addr + 1);
	val = (val << 8) | pci_dev_get_cfg8 (dev, addr);

	return (val);
}

void pci_dev_set_cfg8 (pci_dev_t *dev, unsigned long addr, unsigned char val)
{
	if (addr <= 255) {
		val = (val & dev->wrmask[addr]) | (dev->config[addr] & ~dev->wrmask[addr]);
		dev->config[addr] = val;
	}
}

void pci_dev_set_cfg16 (pci_dev_t *dev, unsigned long addr, unsigned short val)
{
	pci_dev_set_cfg8 (dev, addr, val & 0xff);
	pci_dev_set_cfg8 (dev, addr + 1, (val >> 8) & 0xff);
}

void pci_dev_set_cfg32 (pci_dev_t *dev, unsigned long addr, unsigned long val)
{
	pci_dev_set_cfg8 (dev, addr, val & 0xff);
	pci_dev_set_cfg8 (dev, addr + 1, (val >> 8) & 0xff);
	pci_dev_set_cfg8 (dev, addr + 2, (val >> 16) & 0xff);
	pci_dev_set_cfg8 (dev, addr + 3, (val >> 24) & 0xff);
}


void pci_init (pci_bus_t *pci)
{
	unsigned i;

	for (i = 0; i < 32; i++) {
		pci->dev[i] = NULL;
	}

	pci->asio = NULL;
	pci->asmem = NULL;

	pci->intr = 0;

	pci->irq = NULL;
	pci->irq_ext = NULL;
	pci->irq_val = 0;

	pci->cfgaddr = 0;
	pci->cfgdata = 0;
}

void pci_free (pci_bus_t *pci)
{
}

void pci_set_device (pci_bus_t *pci, pci_dev_t *dev, unsigned i)
{
	unsigned  j;
	pci_dev_t *old;

	old = pci->dev[i & 0x1f];

	if (old != NULL) {
		if (pci->asio != NULL) {
			for (j = 0; j < PCI_REG_MAX; j++) {
				if (dev->reg[j] != NULL) {
					mem_rmv_blk (pci->asio, dev->reg[j]);
				}
			}
		}

		if (pci->asmem != NULL) {
			for (j = 0; j < PCI_MEM_MAX; j++) {
				if (dev->mem[j] != NULL) {
					mem_rmv_blk (pci->asmem, dev->mem[j]);
				}
			}
		}
	}

	pci->dev[i & 0x1f] = dev;

	if (dev != NULL) {
		if (pci->asio != NULL) {
			for (j = 0; j < PCI_REG_MAX; j++) {
				if (dev->reg[j] != NULL) {
					mem_add_blk (pci->asio, dev->reg[j], 0);
				}
			}
		}

		if (pci->asmem != NULL) {
			for (j = 0; j < PCI_MEM_MAX; j++) {
				if (dev->mem[j] != NULL) {
					mem_add_blk (pci->asmem, dev->mem[j], 0);
				}
			}
		}
	}
}

void pci_set_asio (pci_bus_t *pci, memory_t *as)
{
	pci->asio = as;
}

void pci_set_asmem (pci_bus_t *pci, memory_t *as)
{
	pci->asmem = as;
}

void pci_set_irq_f (pci_bus_t *pci, void *irq, void *ext)
{
	pci->irq = irq;
	pci->irq_ext = ext;
}

void pci_set_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned char val)
{
	pci_dev_t *pdev = pci->dev[dev & 0x1f];

	if ((pdev != NULL) && (pdev->set_cfg8 != NULL)) {
		pdev->set_cfg8 (pdev->cfg_ext, addr, val);
	}
}

void pci_set_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned short val)
{
	pci_dev_t *pdev = pci->dev[dev & 0x1f];

	if ((pdev != NULL) && (pdev->set_cfg16 != NULL)) {
		pdev->set_cfg16 (pdev->cfg_ext, addr, val);
	}
}

void pci_set_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned long val)
{
	pci_dev_t *pdev = pci->dev[dev & 0x1f];

	if ((pdev != NULL) && (pdev->set_cfg32 != NULL)) {
		pdev->set_cfg32 (pdev->cfg_ext, addr, val);
	}
}

unsigned char pci_get_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr)
{
	pci_dev_t *pdev = pci->dev[dev & 0x1f];

	if ((pdev != NULL) && (pdev->get_cfg8 != NULL)) {
		return (pdev->get_cfg8 (pdev->cfg_ext, addr));
	}

	return (0xff);
}

unsigned short pci_get_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr)
{
	pci_dev_t *pdev = pci->dev[dev & 0x1f];

	if ((pdev != NULL) && (pdev->get_cfg16 != NULL)) {
		return (pdev->get_cfg16 (pdev->cfg_ext, addr));
	}

	return (0xffffU);
}

unsigned long pci_get_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr)
{
	pci_dev_t *pdev = pci->dev[dev & 0x1f];

	if ((pdev != NULL) && (pdev->get_cfg32 != NULL)) {
		return (pdev->get_cfg32 (pdev->cfg_ext, addr));
	}

	return (0xffffffffUL);
}

static
void pci_set_irq (pci_bus_t *pci, unsigned char val)
{
	val = (val != 0);

	if (val != pci->irq_val) {
		pci->irq_val = val;

		if (pci->irq != NULL) {
			pci->irq (pci->irq_ext, val);
		}
	}
}

void pci_set_intr (pci_bus_t *pci, unsigned intr, unsigned char val)
{
	unsigned msk;

	msk = 1 << (intr & 0x03);

	if (val) {
		pci->intr |= msk;
	}
	else {
		pci->intr &= ~msk;
	}

	pci_set_irq (pci, pci->intr != 0);
}

void pci_set_intr_a (pci_bus_t *pci, unsigned char val)
{
	pci_set_intr (pci, 0, val);
}

void pci_set_intr_b (pci_bus_t *pci, unsigned char val)
{
	pci_set_intr (pci, 1, val);
}


unsigned long pci_get_config_addr (pci_bus_t *pci)
{
	return (pci->cfgaddr);
}

unsigned char pci_get_config_data8 (pci_bus_t *pci, unsigned addr)
{
	unsigned  dev;
	pci_dev_t *pdev;

	addr &= 0x03;

	dev = (pci->cfgaddr >> 11) & 0x1f;
	pdev = pci->dev[dev];

	if ((pdev != NULL) && (pdev->get_cfg8 != NULL)) {
		return (pdev->get_cfg8 (pdev->cfg_ext, (pci->cfgaddr & 0x03fcU) + addr));
	}

	return (0);
}

unsigned short pci_get_config_data16 (pci_bus_t *pci, unsigned addr)
{
	unsigned  dev;
	pci_dev_t *pdev;

	addr &= 0x03;

	dev = (pci->cfgaddr >> 11) & 0x1f;
	pdev = pci->dev[dev];

	if ((pdev != NULL) && (pdev->get_cfg16 != NULL)) {
		return (pdev->get_cfg16 (pdev->cfg_ext, (pci->cfgaddr & 0x03fcU) + addr));
	}

	return (0);
}

unsigned long pci_get_config_data32 (pci_bus_t *pci)
{
	unsigned  dev;
	pci_dev_t *pdev;

	dev = (pci->cfgaddr >> 11) & 0x1f;
	pdev = pci->dev[dev];

	if ((pdev != NULL) && (pdev->get_cfg32 != NULL)) {
		return (pdev->get_cfg32 (pdev->cfg_ext, pci->cfgaddr & 0x03fcU));
	}

	return (0);
}

void pci_set_config_addr (pci_bus_t *pci, unsigned long val)
{
	pci->cfgaddr = val;
}

void pci_set_config_data8 (pci_bus_t *pci, unsigned addr, unsigned char val)
{
	unsigned  dev;
	pci_dev_t *pdev;

	addr &= 0x03;

	dev = (pci->cfgaddr >> 11) & 0x1f;
	pdev = pci->dev[dev];

	if ((pdev != NULL) && (pdev->set_cfg8 != NULL)) {
		pdev->set_cfg8 (pdev->cfg_ext, (pci->cfgaddr & 0x03fcU) + addr, val);
	}
}

void pci_set_config_data16 (pci_bus_t *pci, unsigned addr, unsigned short val)
{
	unsigned  dev;
	pci_dev_t *pdev;

	addr &= 0x03;

	dev = (pci->cfgaddr >> 11) & 0x1f;
	pdev = pci->dev[dev];

	if ((pdev != NULL) && (pdev->set_cfg16 != NULL)) {
		pdev->set_cfg16 (pdev->cfg_ext, (pci->cfgaddr & 0x03fcU) + addr, val);
	}
}

void pci_set_config_data32 (pci_bus_t *pci, unsigned long val)
{
	unsigned  dev;
	pci_dev_t *pdev;

	dev = (pci->cfgaddr >> 11) & 0x1f;
	pdev = pci->dev[dev];

	if ((pdev != NULL) && (pdev->set_cfg32 != NULL)) {
		pdev->set_cfg32 (pdev->cfg_ext, pci->cfgaddr & 0x03fcU, val);
	}
}
