/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/pci.h                                        *
 * Created:     2004-11-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_SIMARM_PCI_H
#define PCE_SIMARM_PCI_H 1


#define PCIID_INTEL_IXP2400 0x9001
#define PCIID_INTEL_IXP2800 0x9004


typedef struct {
	memory_t  asio;

	mem_blk_t pci_cfg;
	mem_blk_t pci_io;
	mem_blk_t pci_mem;
	mem_blk_t pci_special;
	mem_blk_t pci_pcicfg;
	mem_blk_t pci_csr;

	pci_bus_t bus;

	pci_dev_t dev;

	int       bigendian;

	uint32_t  pci_control;
	uint32_t  pci_addr_ext;
	uint32_t  xscale_int_status;
	uint32_t  xscale_int_enable;

	void      *set_irq_ext;
	void      (*set_irq) (void *ext, unsigned char val);
} pci_ixp_t;


void pci_ixp_set_endian (pci_ixp_t *ixp, int big);

void pci_ixp_set_irq_fct (pci_ixp_t *ixp, void *ext, void *fct);

void pci_ixp_add_device (pci_ixp_t *ixp, pci_dev_t *dev);

void pci_ixp_set_int_a (pci_ixp_t *ixp, unsigned char val);
void pci_ixp_set_int_b (pci_ixp_t *ixp, unsigned char val);

mem_blk_t *pci_ixp_get_mem_io (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_cfg (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_special (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_pcicfg (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_csr (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_mem (pci_ixp_t *ixp);

void pci_ixp_init (pci_ixp_t *ixp);
void pci_ixp_free (pci_ixp_t *ixp);

pci_ixp_t *pci_ixp_new (void);
void pci_ixp_del (pci_ixp_t *ixp);


#endif
