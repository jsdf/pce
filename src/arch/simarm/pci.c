/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/pci.c                                      *
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


#include "main.h"


static unsigned pci_get_device (unsigned long addr)
{
  unsigned i;

  addr = (addr >> 16) & 0xffffU;

  i = 0;
  while ((addr != 0) && ((addr & 0x0001) == 0)) {
    if (addr & 0x01) {
      return (i);
    }

    i += 1;
    addr = addr >> 1;
  }

  return (i);
}

static
unsigned char pci_get_uint8 (ixp_pci_t *pci, unsigned long addr)
{
  pce_log (MSG_DEB, "PCI get8 %08lX\n", addr);
  return (0x55);
}

static
unsigned short pci_get_uint16 (ixp_pci_t *pci, unsigned long addr)
{
  pce_log (MSG_DEB, "PCI get16 %08lX\n", addr);
  return (0x5555);
}

static
unsigned long pci_get_uint32 (ixp_pci_t *pci, unsigned long addr)
{
  unsigned dev;

/*  pce_log (MSG_DEB, "PCI get32 %08lX\n", addr); */

  dev = pci_get_device (addr);

  return (pci_get_cfg32 (&pci->pci, dev, addr));
}

static
void pci_set_uint8 (ixp_pci_t *pci, unsigned long addr, unsigned char val)
{
  pce_log (MSG_DEB, "PCI set8 %08lX %02X\n", addr, val);
}

static
void pci_set_uint16 (ixp_pci_t *pci, unsigned long addr, unsigned short val)
{
  pce_log (MSG_DEB, "PCI set8 %08lX %04X\n", addr, val);
}

static
void pci_set_uint32 (ixp_pci_t *pci, unsigned long addr, unsigned long val)
{
  unsigned dev;

/*  pce_log (MSG_DEB, "PCI set8 %08lX %08lX\n", addr, val); */

  dev = pci_get_device (addr);

  pci_set_cfg32 (&pci->pci, dev, addr, val);
}


void ixppci_init (ixp_pci_t *pci, unsigned long base)
{
  mem_blk_init (&pci->mem, 0xda000000UL, 0x02000000UL, 0);
  pci->mem.ext = pci;
  pci->mem.get_uint8 = (mem_get_uint8_f) pci_get_uint8;
  pci->mem.set_uint8 = (mem_set_uint8_f) pci_set_uint8;
  pci->mem.get_uint16 = (mem_get_uint16_f) pci_get_uint16;
  pci->mem.set_uint16 = (mem_set_uint16_f) pci_set_uint16;
  pci->mem.get_uint32 = (mem_get_uint32_f) pci_get_uint32;
  pci->mem.set_uint32 = (mem_set_uint32_f) pci_set_uint32;

  pci_init (&pci->pci);
}

void ixppci_free (ixp_pci_t *pci)
{
}

ixp_pci_t *ixppci_new (unsigned long base)
{
  ixp_pci_t *pci;

  pci = malloc (sizeof (ixp_pci_t));
  if (pci == NULL) {
    return (NULL);
  }

  ixppci_init (pci, base);

  return (pci);
}

void ixppci_del (ixp_pci_t *pci)
{
  if (pci != NULL) {
    ixppci_free (pci);
    free (pci);
  }
}

mem_blk_t *ixppci_get_mem (ixp_pci_t *pci, unsigned i)
{
  if (i == 0) {
    return (&pci->mem);
  }

  return (NULL);
}
