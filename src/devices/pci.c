/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/pci.c                                          *
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


#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "pci.h"


void pci_dev_init (pci_dev_t *dev)
{
  unsigned i;

  for (i = 0; i < 256; i++) {
    dev->config[i] = 0;
  }

  dev->config[0] = 0xff;
  dev->config[1] = 0xff;

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


void pci_init (pci_bus_t *pci)
{
  unsigned i;

  for (i = 0; i < 32; i++) {
    pci->dev[i] = NULL;
  }
}

void pci_free (pci_bus_t *pci)
{
}

void pci_set_device (pci_bus_t *pci, pci_dev_t *dev, unsigned i)
{
  pci->dev[i & 0x1f] = dev;
}


void pci_set_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned char val)
{
  pci_dev_t *pdev = pci->dev[dev & 0x1f];

  if ((pdev != NULL) && (pdev->set_cfg8 != NULL)) {
    pdev->set_cfg8 (pdev, addr, val);
  }
}

void pci_set_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned short val)
{
  pci_dev_t *pdev = pci->dev[dev & 0x1f];

  if ((pdev != NULL) && (pdev->set_cfg16 != NULL)) {
    pdev->set_cfg16 (pdev, addr, val);
  }
}

void pci_set_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr, unsigned long val)
{
  pci_dev_t *pdev = pci->dev[dev & 0x1f];

  if ((pdev != NULL) && (pdev->set_cfg32 != NULL)) {
    pdev->set_cfg32 (pdev, addr, val);
  }
}

unsigned char pci_get_cfg8 (pci_bus_t *pci, unsigned dev, unsigned long addr)
{
  pci_dev_t *pdev = pci->dev[dev & 0x1f];

  if ((pdev != NULL) && (pdev->get_cfg8 != NULL)) {
    return (pdev->get_cfg8 (pdev, addr));
  }

  return (0xff);
}

unsigned short pci_get_cfg16 (pci_bus_t *pci, unsigned dev, unsigned long addr)
{
  pci_dev_t *pdev = pci->dev[dev & 0x1f];

  if ((pdev != NULL) && (pdev->get_cfg16 != NULL)) {
    return (pdev->get_cfg16 (pdev, addr));
  }

  return (0xffffU);
}

unsigned long pci_get_cfg32 (pci_bus_t *pci, unsigned dev, unsigned long addr)
{
  pci_dev_t *pdev = pci->dev[dev & 0x1f];

  if ((pdev != NULL) && (pdev->get_cfg32 != NULL)) {
    return (pdev->get_cfg32 (pdev, addr));
  }

  return (0xffffffffUL);
}


void pci_set_config (pci_bus_t *pci, unsigned long val)
{
  pci->cfg = val;
}

void pci_set_config_data (pci_bus_t *pci, unsigned long val)
{
}
