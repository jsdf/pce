/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/pci.h                                      *
 * Created:       2004-11-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-10 by Hampa Hug <hampa@hampa.ch>                   *
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

  uint32_t  pci_control;
  uint32_t  pci_addr_ext;
  uint32_t  xscale_int_enable;
} pci_ixp_t;


void pci_ixp_init (pci_ixp_t *ixp);
void pci_ixp_free (pci_ixp_t *ixp);

pci_ixp_t *pci_ixp_new (void);
void pci_ixp_del (pci_ixp_t *ixp);

mem_blk_t *pci_ixp_get_mem_io (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_cfg (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_special (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_pcicfg (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_csr (pci_ixp_t *ixp);
mem_blk_t *pci_ixp_get_mem_mem (pci_ixp_t *ixp);


#endif
