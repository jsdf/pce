/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/pci-ata.c                                      *
 * Created:       2004-12-06 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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

#include "pci-ata.h"


void pci_ata_init (pci_ata_t *dev)
{
  pci_dev_init (&dev->pci);

  dev->pci.ext = dev;
  dev->pci.cfg_ext = dev;
  dev->pci.get_cfg8 = (mem_get_uint8_f) pci_ata_get_cfg8;
  dev->pci.set_cfg8 = (mem_set_uint8_f) pci_ata_set_cfg8;
  dev->pci.get_cfg16 = (mem_get_uint16_f) pci_ata_get_cfg16;
  dev->pci.set_cfg16 = (mem_set_uint16_f) pci_ata_set_cfg16;
  dev->pci.get_cfg32 = (mem_get_uint32_f) pci_ata_get_cfg32;
  dev->pci.set_cfg32 = (mem_set_uint32_f) pci_ata_set_cfg32;

  ata_init (&dev->ata[0], 0x1f0, 0x3f4);
  ata_init (&dev->ata[1], 0x170, 0x374);

  ata_set_irq_f (&dev->ata[0], pci_dev_set_irq_a, dev);
  ata_set_irq_f (&dev->ata[1], pci_dev_set_irq_a, dev);

  dev->pci.reg[0] = &dev->ata[0].reg_cmd;
  dev->pci.reg[1] = &dev->ata[0].reg_ctl;
  dev->pci.reg[2] = &dev->ata[1].reg_cmd;
  dev->pci.reg[3] = &dev->ata[1].reg_ctl;

  buf_set_uint16_le (dev->pci.config, 0x00, PCIID_INTEL);
  buf_set_uint16_le (dev->pci.config, 0x02, PCIID_INTEL_PIIX_IDE);
  buf_set_uint32_le (dev->pci.config, 0x04, 0x00000001);
  buf_set_uint32_le (dev->pci.config, 0x08, 0x01010f01);
  buf_set_uint32_le (dev->pci.config, 0x0c, 0x00000000);

  buf_set_uint32_le (dev->pci.config, 0x10, 0x000001f0 | 0x01);
  buf_set_uint32_le (dev->pci.config, 0x14, 0x000003f4 | 0x01);
  buf_set_uint32_le (dev->pci.config, 0x18, 0x00000170 | 0x01);
  buf_set_uint32_le (dev->pci.config, 0x1c, 0x00000374 | 0x01);

  buf_set_uint32_le (dev->pci.config, 0x3c, 0x00000104);

  pci_dev_set_cfg_mask (&dev->pci, 8 * 0x01, 32, 1);
  pci_dev_set_cfg_mask (&dev->pci, 8 * 0x04, 16, 1);
  pci_dev_set_cfg_mask (&dev->pci, 8 * 0x0c, 16, 1);
  pci_dev_set_cfg_mask (&dev->pci, 8 * 0x10, 4 * 32, 1);
  pci_dev_set_cfg_mask (&dev->pci, 8 * 0x3c, 16, 1);
}

void pci_ata_free (pci_ata_t *dev)
{
  ata_free (&dev->ata[1]);
  ata_free (&dev->ata[0]);

  pci_dev_free (&dev->pci);
}

void pci_ata_set_block (pci_ata_t *dev, disk_t *blk, unsigned i)
{
  unsigned chn, drv;

  if (i < 4) {
    chn = i / 2;
    drv = i % 2;

    ata_set_block (&dev->ata[chn], blk, drv);
  }
}


unsigned char pci_ata_get_cfg8 (pci_ata_t *dev, unsigned long addr)
{
  unsigned char val;

  val = pci_dev_get_cfg8 (&dev->pci, addr);

  return (val);
}

unsigned short pci_ata_get_cfg16 (pci_ata_t *dev, unsigned long addr)
{
  unsigned short val;

  val = pci_dev_get_cfg16 (&dev->pci, addr);

  return (val);
}

unsigned long pci_ata_get_cfg32 (pci_ata_t *dev, unsigned long addr)
{
  unsigned long val;

  val = pci_dev_get_cfg32 (&dev->pci, addr);

  return (val);
}

void pci_ata_set_cfg8 (pci_ata_t *dev, unsigned long addr, unsigned char val)
{
  pci_dev_set_cfg8 (&dev->pci, addr, val);
}

void pci_ata_set_cfg16 (pci_ata_t *dev, unsigned long addr, unsigned short val)
{
  pci_dev_set_cfg16 (&dev->pci, addr, val);
}

void pci_ata_set_cfg32 (pci_ata_t *dev, unsigned long addr, unsigned long val)
{
  if (addr == 0x10) {
    val = (val & ~0x07UL) | 1;
    pci_dev_set_cfg32 (&dev->pci, addr, val);
    if (dev->pci.reg[0] != NULL) {
      mem_blk_set_addr (dev->pci.reg[0], val & ~0x01);
    }
  }
  else if (addr == 0x14) {
    val = (val & ~0x03UL) | 1;
    pci_dev_set_cfg32 (&dev->pci, addr, val);
    if (dev->pci.reg[1] != NULL) {
      mem_blk_set_addr (dev->pci.reg[1], val & ~0x01);
    }
  }
  else if (addr == 0x18) {
    val = (val & ~0x07UL) | 1;
    pci_dev_set_cfg32 (&dev->pci, addr, val);
    if (dev->pci.reg[2] != NULL) {
      mem_blk_set_addr (dev->pci.reg[2], val & ~0x01);
    }
  }
  else if (addr == 0x1c) {
    val = (val & ~0x03UL) | 1;
    pci_dev_set_cfg32 (&dev->pci, addr, val);
    if (dev->pci.reg[3] != NULL) {
      mem_blk_set_addr (dev->pci.reg[3], val & ~0x01);
    }
  }
  else {
    pci_dev_set_cfg32 (&dev->pci, addr, val);
  }
}

void pci_ata_set_irq (pci_ata_t *dev, unsigned char val)
{
  pci_dev_set_irq_a (&dev->pci, val);
}
