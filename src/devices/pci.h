/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/pci.h                                            *
 * Created:     2004-06-05 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_PCI_H
#define PCE_DEVICES_PCI_H 1


#include <stdio.h>

#include <devices/memory.h>


#define PCI_REG_MAX 8
#define PCI_MEM_MAX 8

#define PCIID_VENDOR_INTEL 0x8086
#define PCIID_VENDOR_OPTI  0x1045
#define PCIID_VENDOR_NS    0x100b
#define PCIID_VENDOR_VIA   0x1106


struct pci_bus_t;


typedef struct {
	unsigned         index;

	struct pci_bus_t *pci;

	mem_blk_t        *reg[PCI_REG_MAX];
	mem_blk_t        *mem[PCI_MEM_MAX];

	unsigned char    config[256];
	unsigned char    wrmask[256];

	void             *cfg_ext;

	mem_get_uint8_f  get_cfg8;
	mem_get_uint16_f get_cfg16;
	mem_get_uint32_f get_cfg32;

	mem_set_uint8_f  set_cfg8;
	mem_set_uint16_f set_cfg16;
	mem_set_uint32_f set_cfg32;

	void             (*intr[4]) (void *ext, unsigned char val);
	void             *intr_ext[4];
	unsigned char    intr_val[4];

	void             *ext;
} pci_dev_t;


typedef struct pci_bus_t {
	pci_dev_t     *dev[32];

	memory_t      *asio;
	memory_t      *asmem;

	unsigned long cfgaddr;
	unsigned long cfgdata;

	void          (*irq[32][4]) (void *ext, unsigned char val);
	void          *irq_ext[32][4];
	unsigned char irq_val[32][4];
} pci_bus_t;


void pci_dev_init (pci_dev_t *dev);
void pci_dev_free (pci_dev_t *dev);

void pci_dev_set_intr_fct (pci_dev_t *dev, unsigned intr, void *ext, void *fct);
void pci_dev_set_intr (pci_dev_t *dev, unsigned intr, unsigned char val);
void pci_dev_set_inta (pci_dev_t *dev, unsigned char val);
void pci_dev_set_intb (pci_dev_t *dev, unsigned char val);
void pci_dev_set_intc (pci_dev_t *dev, unsigned char val);
void pci_dev_set_intd (pci_dev_t *dev, unsigned char val);

void pci_dev_set_device_id (pci_dev_t *dev, unsigned vendor, unsigned id);

void pci_dev_set_cfg_mask (pci_dev_t *dev, unsigned i, unsigned n, int val);

unsigned char pci_dev_get_cfg8 (pci_dev_t *dev, unsigned long addr);
unsigned short pci_dev_get_cfg16 (pci_dev_t *dev, unsigned long addr);
unsigned long pci_dev_get_cfg32 (pci_dev_t *dev, unsigned long addr);

void pci_dev_set_cfg8 (pci_dev_t *dev, unsigned long addr, unsigned char val);
void pci_dev_set_cfg16 (pci_dev_t *dev, unsigned long addr, unsigned short val);
void pci_dev_set_cfg32 (pci_dev_t *dev, unsigned long addr, unsigned long val);


void pci_init (pci_bus_t *pci);
void pci_free (pci_bus_t *pci);

void pci_set_device (pci_bus_t *pci, pci_dev_t *dev, unsigned i);

void pci_set_asio (pci_bus_t *pci, memory_t *as);
void pci_set_asmem (pci_bus_t *pci, memory_t *as);

void pci_set_irq_fct (pci_bus_t *pci, unsigned dev, unsigned intr, void *ext, void *fct);

void pci_set_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned char val);
void pci_set_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned short val);
void pci_set_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned long val);

unsigned char pci_get_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr);
unsigned short pci_get_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr);
unsigned long pci_get_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr);

void pci_set_intr (pci_bus_t *pci, unsigned dev, unsigned intr, unsigned char val);
void pci_set_inta (pci_bus_t *pci, unsigned dev, unsigned char val);
void pci_set_intb (pci_bus_t *pci, unsigned dev, unsigned char val);
void pci_set_intc (pci_bus_t *pci, unsigned dev, unsigned char val);
void pci_set_intd (pci_bus_t *pci, unsigned dev, unsigned char val);

unsigned long pci_get_config_addr (pci_bus_t *pci);
unsigned char pci_get_config_data8 (pci_bus_t *pci, unsigned addr);
unsigned short pci_get_config_data16 (pci_bus_t *pci, unsigned addr);
unsigned long pci_get_config_data32 (pci_bus_t *pci);

void pci_set_config_addr (pci_bus_t *pci, unsigned long val);
void pci_set_config_data8 (pci_bus_t *pci, unsigned addr, unsigned char val);
void pci_set_config_data16 (pci_bus_t *pci, unsigned addr, unsigned short val);
void pci_set_config_data32 (pci_bus_t *pci, unsigned long val);


#endif
