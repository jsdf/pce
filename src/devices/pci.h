/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/pci.h                                          *
 * Created:       2004-06-05 by Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_DEVICES_PCI_H
#define PCE_DEVICES_PCI_H 1


#include <stdio.h>

#include "memory.h"


typedef struct {
  unsigned char    config[256];

  mem_get_uint8_f  get_cfg8;
  mem_get_uint16_f get_cfg16;
  mem_get_uint32_f get_cfg32;

  mem_set_uint8_f  set_cfg8;
  mem_set_uint16_f set_cfg16;
  mem_set_uint32_f set_cfg32;

  void            *ext;
} pci_dev_t;


typedef struct {
  pci_dev_t     *dev[32];

  unsigned long cfg;
  unsigned long cfgdata;
} pci_bus_t;


void pci_dev_init (pci_dev_t *dev);
void pci_dev_free (pci_dev_t *dev);

void pci_init (pci_bus_t *pci);
void pci_free (pci_bus_t *pci);

void pci_set_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned char val);
void pci_set_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned short val);
void pci_set_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned long val);

unsigned char pci_get_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr);
unsigned short pci_get_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr);
unsigned long pci_get_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr);

void pci_set_config (pci_bus_t *pci, unsigned long val);
void pci_set_config_data (pci_bus_t *pci, unsigned long val);


#endif
