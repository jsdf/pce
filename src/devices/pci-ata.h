/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/pci-ata.h                                        *
 * Created:     2004-12-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_PCI_ATA_H
#define PCE_DEVICES_PCI_ATA_H 1


#include <stdio.h>

#include <devices/memory.h>
#include <devices/pci.h>
#include <devices/ata.h>


/* these are IDE chipsets */
#define PCIID_INTEL_PIIX_IDE  0x1239
#define PCIID_INTEL_PIIX3_IDE 0x7010
#define PCIID_INTEL_PIIX4_IDE 0x7111
#define PCIID_OPTI_82C558     0xc558
#define PCIID_NS_87410        0xd001
#define PCIID_VIA_82C561      0x0561


typedef struct {
	pci_dev_t pci;

	ata_chn_t ata[2];
} pci_ata_t;


void pci_ata_init (pci_ata_t *dev);
void pci_ata_free (pci_ata_t *dev);

void pci_ata_set_block (pci_ata_t *dev, disk_t *blk, unsigned i);

ata_chn_t *pci_ata_get_chn (pci_ata_t *dev, unsigned i);

unsigned char pci_ata_get_cfg8 (pci_ata_t *dev, unsigned long addr);
unsigned short pci_ata_get_cfg16 (pci_ata_t *dev, unsigned long addr);
unsigned long pci_ata_get_cfg32 (pci_ata_t *dev, unsigned long addr);

void pci_ata_set_cfg8 (pci_ata_t *dev, unsigned long addr, unsigned char val);
void pci_ata_set_cfg16 (pci_ata_t *dev, unsigned long addr, unsigned short val);
void pci_ata_set_cfg32 (pci_ata_t *dev, unsigned long addr, unsigned long val);

void pci_ata_set_irq (pci_ata_t *dev, unsigned char val);


#endif
