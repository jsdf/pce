/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/pci.h                                      *
 * Created:       2004-11-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-16 by Hampa Hug <hampa@hampa.ch>                   *
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


typedef struct {
  mem_blk_t mem;

  pci_bus_t pci;
} ixp_pci_t;


void ixppci_init (ixp_pci_t *pci, unsigned long base);
void ixppci_free (ixp_pci_t *pci);

ixp_pci_t *ixppci_new (unsigned long base);
void ixppci_del (ixp_pci_t *pci);

mem_blk_t *ixppci_get_mem (ixp_pci_t *pci, unsigned i);


#endif
