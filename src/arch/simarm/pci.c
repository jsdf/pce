/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/pci.c                                      *
 * Created:       2004-11-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-22 by Hampa Hug <hampa@hampa.ch>                   *
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


/* #define DEBUG_PCI 1 */
/* #define DEBUG_PCI_IXP 1 */


unsigned char pci_ixp_get_cfg8 (pci_ixp_t *ixp, unsigned long addr)
{
#ifdef DEBUG_PCI_IXP
  pce_log (MSG_DEB, "pci-ixp: get cfg8 %08lX\n", addr);
#endif

  return (0);
}

unsigned short pci_ixp_get_cfg16 (pci_ixp_t *ixp, unsigned long addr)
{
#ifdef DEBUG_PCI_IXP
  pce_log (MSG_DEB, "pci-ixp: get cfg16 %08lX\n", addr);
#endif

  return (0);
}

unsigned long pci_ixp_get_cfg32 (pci_ixp_t *ixp, unsigned long addr)
{
  unsigned long val;

  val = pci_dev_get_cfg32 (&ixp->dev, addr);

#ifdef DEBUG_PCI_IXP
  pce_log (MSG_DEB, "pci-ixp: get cfg32 %08lX -> %08lX\n", addr, val);
#endif

  return (val);
}

void pci_ixp_set_cfg8 (pci_ixp_t *ixp, unsigned long addr, unsigned char val)
{
#ifdef DEBUG_PCI_IXP
  pce_log (MSG_DEB, "pci-ixp: set cfg32 %08lX <- %02X\n", addr, val);
#endif
}

void pci_ixp_set_cfg16 (pci_ixp_t *ixp, unsigned long addr, unsigned short val)
{
#ifdef DEBUG_PCI_IXP
  pce_log (MSG_DEB, "pci-ixp: set cfg32 %08lX <- %04X\n", addr, val);
#endif
}

void pci_ixp_set_cfg32 (pci_ixp_t *ixp, unsigned long addr, unsigned long val)
{
#ifdef DEBUG_PCI_IXP
  pce_log (MSG_DEB, "pci-ixp: set cfg32 %08lX <- %08lX)\n", addr, val);
#endif

  pci_dev_set_cfg32 (&ixp->dev, addr, val);
}


static
unsigned pci_ixp_get_device (unsigned long addr)
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
unsigned char pci_bus_get_io8 (pci_ixp_t *ixp, unsigned long addr)
{
  unsigned char val;

  addr ^= 0x03;

  val = mem_get_uint8 (&ixp->asio, addr);

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: get io8 %08lX -> %02X\n", addr, val);
#endif

  return (val);
}

static
unsigned short pci_bus_get_io16 (pci_ixp_t *ixp, unsigned long addr)
{
  unsigned short val;

  addr ^= 0x02;

  val = mem_get_uint16_le (&ixp->asio, addr);
  val = sarm_br16 (val);

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: get io16 %08lX -> %04X\n", addr, val);
#endif

  return (val);
}

static
unsigned long pci_bus_get_io32 (pci_ixp_t *ixp, unsigned long addr)
{
  unsigned long val;

  val = mem_get_uint32_le (&ixp->asio, addr);
  val = sarm_br32 (val);

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: get io32 %08lX -> %08lX\n", addr, val);
#endif

  return (val);
}

static
void pci_bus_set_io8 (pci_ixp_t *ixp, unsigned long addr, unsigned char val)
{
  addr ^= 0x03;

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: set io8 %08lX <- %02X\n", addr, val);
#endif

  mem_set_uint8 (&ixp->asio, addr, val);
}

static
void pci_bus_set_io16 (pci_ixp_t *ixp, unsigned long addr, unsigned short val)
{
  addr ^= 0x02;

  val = sarm_br16 (val);

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: set io16 %08lX <- %04X\n", addr, val);
#endif

  mem_set_uint16_le (&ixp->asio, addr, val);
}

static
void pci_bus_set_io32 (pci_ixp_t *ixp, unsigned long addr, unsigned long val)
{
  val = sarm_br32 (val);

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: set io32 %08lX <- %08lX\n", addr, val);
#endif

  mem_set_uint32_le (&ixp->asio, addr, val);
}


static
unsigned char pci_bus_get_cfg8 (pci_ixp_t *ixp, unsigned long addr)
{
#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: get cfg8 %08lX\n", addr);
#endif

  return (0x55);
}

static
unsigned short pci_bus_get_cfg16 (pci_ixp_t *ixp, unsigned long addr)
{
#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: get cfg16 %08lX\n", addr);
#endif

  return (0x5555);
}

static
unsigned long pci_bus_get_cfg32 (pci_ixp_t *ixp, unsigned long addr)
{
  unsigned      dev;
  unsigned long val;

  dev = pci_ixp_get_device (addr);
  val = pci_get_cfg32 (&ixp->bus, dev, addr & 0xffffU);

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: get cfg32 %08lX -> %02X\n", addr, val);
#endif

  return (val);
}

static
void pci_bus_set_cfg8 (pci_ixp_t *ixp, unsigned long addr, unsigned char val)
{
#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: set cfg8 %08lX <- %02X\n", addr, val);
#endif
}

static
void pci_bus_set_cfg16 (pci_ixp_t *ixp, unsigned long addr, unsigned short val)
{
#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: set cfg16 %08lX <- %04X\n", addr, val);
#endif
}

static
void pci_bus_set_cfg32 (pci_ixp_t *ixp, unsigned long addr, unsigned long val)
{
  unsigned dev;

#ifdef DEBUG_PCI
  pce_log (MSG_DEB, "pci-bus: set cfg32 %08lX <- %08lX\n", addr, val);
#endif

  dev = pci_ixp_get_device (addr);

  pci_set_cfg32 (&ixp->bus, dev, addr & 0xffffU, val);
}


static
unsigned char pci_bus_get_csr8 (pci_ixp_t *ixp, unsigned long addr)
{
  pce_log (MSG_DEB, "pci-bus: get csr 8 %08lX\n", addr);
  return (0);
}

static
unsigned short pci_bus_get_csr16 (pci_ixp_t *ixp, unsigned long addr)
{
  pce_log (MSG_DEB, "pci-bus: get csr 16 %08lX\n", addr);
  return (0);
}

static
unsigned long pci_bus_get_csr32 (pci_ixp_t *ixp, unsigned long addr)
{
  switch (addr) {
  case 0x13c:
    return (ixp->pci_control);

  case 0x140:
    return (ixp->pci_addr_ext);

  case 0x15c:
    return (ixp->xscale_int_enable);
  }

  pce_log (MSG_DEB, "pci-bus: get csr 32 %08lX\n", addr);

  return (0);
}

static
void pci_bus_set_csr8 (pci_ixp_t *ixp, unsigned long addr, unsigned char val)
{
  pce_log (MSG_DEB, "pci-bus: set csr 8 %08lX <- %02X\n", addr, val);
}

static
void pci_bus_set_csr16 (pci_ixp_t *ixp, unsigned long addr, unsigned short val)
{
  pce_log (MSG_DEB, "pci-bus: set csr 16 %08lX <- %04X\n", addr, val);
}

static
void pci_bus_set_csr32 (pci_ixp_t *ixp, unsigned long addr, unsigned long val)
{
  switch (addr) {
  case 0x13c:
    ixp->pci_control = val;
    break;

  case 0x140:
    ixp->pci_addr_ext = val;
    break;

  case 0x15c:
    ixp->xscale_int_enable = val;
    break;

  default:
    pce_log (MSG_DEB, "pci-bus: set csr 32 %08lX <- %08lX\n", addr, val);
    break;
  }
}


void pci_ixp_init (pci_ixp_t *ixp)
{
  mem_init (&ixp->asio);

  mem_blk_init (&ixp->pci_io, 0xd8000000UL, 0x02000000UL, 0);
  ixp->pci_io.ext = ixp;
  ixp->pci_io.get_uint8 = (mem_get_uint8_f) pci_bus_get_io8;
  ixp->pci_io.set_uint8 = (mem_set_uint8_f) pci_bus_set_io8;
  ixp->pci_io.get_uint16 = (mem_get_uint16_f) pci_bus_get_io16;
  ixp->pci_io.set_uint16 = (mem_set_uint16_f) pci_bus_set_io16;
  ixp->pci_io.get_uint32 = (mem_get_uint32_f) pci_bus_get_io32;
  ixp->pci_io.set_uint32 = (mem_set_uint32_f) pci_bus_set_io32;

  mem_blk_init (&ixp->pci_cfg, 0xda000000UL, 0x02000000UL, 0);
  ixp->pci_cfg.ext = ixp;
  ixp->pci_cfg.get_uint8 = (mem_get_uint8_f) pci_bus_get_cfg8;
  ixp->pci_cfg.set_uint8 = (mem_set_uint8_f) pci_bus_set_cfg8;
  ixp->pci_cfg.get_uint16 = (mem_get_uint16_f) pci_bus_get_cfg16;
  ixp->pci_cfg.set_uint16 = (mem_set_uint16_f) pci_bus_set_cfg16;
  ixp->pci_cfg.get_uint32 = (mem_get_uint32_f) pci_bus_get_cfg32;
  ixp->pci_cfg.set_uint32 = (mem_set_uint32_f) pci_bus_set_cfg32;

  mem_blk_init (&ixp->pci_special, 0xdc000000UL, 0x02000000UL, 0);
  ixp->pci_special.ext = ixp;
  ixp->pci_special.get_uint8 = (mem_get_uint8_f) pci_bus_get_io8;
  ixp->pci_special.set_uint8 = (mem_set_uint8_f) pci_bus_set_io8;
  ixp->pci_special.get_uint16 = (mem_get_uint16_f) pci_bus_get_io16;
  ixp->pci_special.set_uint16 = (mem_set_uint16_f) pci_bus_set_io16;
  ixp->pci_special.get_uint32 = (mem_get_uint32_f) pci_bus_get_io32;
  ixp->pci_special.set_uint32 = (mem_set_uint32_f) pci_bus_set_io32;

  mem_blk_init (&ixp->pci_pcicfg, 0xde000000UL, 0x01000000UL, 0);
  ixp->pci_pcicfg.ext = ixp;
  ixp->pci_pcicfg.get_uint8 = (mem_get_uint8_f) pci_ixp_get_cfg8;
  ixp->pci_pcicfg.set_uint8 = (mem_set_uint8_f) pci_ixp_set_cfg8;
  ixp->pci_pcicfg.get_uint16 = (mem_get_uint16_f) pci_ixp_get_cfg16;
  ixp->pci_pcicfg.set_uint16 = (mem_set_uint16_f) pci_ixp_set_cfg16;
  ixp->pci_pcicfg.get_uint32 = (mem_get_uint32_f) pci_ixp_get_cfg32;
  ixp->pci_pcicfg.set_uint32 = (mem_set_uint32_f) pci_ixp_set_cfg32;

  mem_blk_init (&ixp->pci_csr, 0xdf000000UL, 0x01000000UL, 0);
  ixp->pci_csr.ext = ixp;
  ixp->pci_csr.get_uint8 = (mem_get_uint8_f) pci_bus_get_csr8;
  ixp->pci_csr.set_uint8 = (mem_set_uint8_f) pci_bus_set_csr8;
  ixp->pci_csr.get_uint16 = (mem_get_uint16_f) pci_bus_get_csr16;
  ixp->pci_csr.set_uint16 = (mem_set_uint16_f) pci_bus_set_csr16;
  ixp->pci_csr.get_uint32 = (mem_get_uint32_f) pci_bus_get_csr32;
  ixp->pci_csr.set_uint32 = (mem_set_uint32_f) pci_bus_set_csr32;

  mem_blk_init (&ixp->pci_mem, 0xe0000000UL, 0x20000000UL, 0);
  ixp->pci_mem.ext = ixp;
  ixp->pci_mem.get_uint8 = (mem_get_uint8_f) pci_bus_get_io8;
  ixp->pci_mem.set_uint8 = (mem_set_uint8_f) pci_bus_set_io8;
  ixp->pci_mem.get_uint16 = (mem_get_uint16_f) pci_bus_get_io16;
  ixp->pci_mem.set_uint16 = (mem_set_uint16_f) pci_bus_set_io16;
  ixp->pci_mem.get_uint32 = (mem_get_uint32_f) pci_bus_get_io32;
  ixp->pci_mem.set_uint32 = (mem_set_uint32_f) pci_bus_set_io32;

  pci_init (&ixp->bus);

  pci_dev_init (&ixp->dev);

  ixp->dev.ext = ixp;
  ixp->dev.cfg_ext = ixp;
  ixp->dev.get_cfg8 = (mem_get_uint8_f) pci_ixp_get_cfg8;
  ixp->dev.set_cfg8 = (mem_set_uint8_f) pci_ixp_set_cfg8;
  ixp->dev.get_cfg16 = (mem_get_uint16_f) pci_ixp_get_cfg16;
  ixp->dev.set_cfg16 = (mem_set_uint16_f) pci_ixp_set_cfg16;
  ixp->dev.get_cfg32 = (mem_get_uint32_f) pci_ixp_get_cfg32;
  ixp->dev.set_cfg32 = (mem_set_uint32_f) pci_ixp_set_cfg32;

  buf_set_uint16_le (ixp->dev.config, 0x00, PCIID_INTEL);
  buf_set_uint16_le (ixp->dev.config, 0x02, PCIID_INTEL_IXP2400);
  buf_set_uint32_le (ixp->dev.config, 0x04, 0x00000001);
  buf_set_uint32_le (ixp->dev.config, 0x08, 0x0b400101);
  buf_set_uint32_le (ixp->dev.config, 0x0c, 0x00000000);
  buf_set_uint32_le (ixp->dev.config, 0x3c, 0x04010100);

  pci_dev_set_cfg_mask (&ixp->dev, 8 * 0x01, 32, 1);
  pci_dev_set_cfg_mask (&ixp->dev, 8 * 0x04, 16, 1);
  pci_dev_set_cfg_mask (&ixp->dev, 8 * 0x0c, 16, 1);
  pci_dev_set_cfg_mask (&ixp->dev, 8 * 0x3c, 32, 1);

  pci_set_device (&ixp->bus, &ixp->dev, 0);
}

void pci_ixp_free (pci_ixp_t *ixp)
{
  mem_blk_free (&ixp->pci_mem);
  mem_blk_free (&ixp->pci_csr);
  mem_blk_free (&ixp->pci_pcicfg);
  mem_blk_free (&ixp->pci_special);
  mem_blk_free (&ixp->pci_cfg);
  mem_blk_free (&ixp->pci_io);
  mem_free (&ixp->asio);

  pci_dev_free (&ixp->dev);

  pci_free (&ixp->bus);
}

pci_ixp_t *pci_ixp_new (void)
{
  pci_ixp_t *ixp;

  ixp = malloc (sizeof (pci_ixp_t));
  if (ixp == NULL) {
    return (NULL);
  }

  pci_ixp_init (ixp);

  return (ixp);
}

void pci_ixp_del (pci_ixp_t *ixp)
{
  if (ixp != NULL) {
    pci_ixp_free (ixp);
    free (ixp);
  }
}

void pci_ixp_add_device (pci_ixp_t *ixp, pci_dev_t *dev)
{
  unsigned i;

  for (i = 0; i < PCI_REG_MAX; i++) {
    if (dev->reg[i] != NULL) {
      mem_add_blk (&ixp->asio, dev->reg[i], 0);
    }
  }
}

mem_blk_t *pci_ixp_get_mem_io (pci_ixp_t *ixp)
{
  return (&ixp->pci_io);
}

mem_blk_t *pci_ixp_get_mem_cfg (pci_ixp_t *ixp)
{
  return (&ixp->pci_cfg);
}

mem_blk_t *pci_ixp_get_mem_special (pci_ixp_t *ixp)
{
  return (&ixp->pci_special);
}

mem_blk_t *pci_ixp_get_mem_pcicfg (pci_ixp_t *ixp)
{
  return (&ixp->pci_pcicfg);
}

mem_blk_t *pci_ixp_get_mem_csr (pci_ixp_t *ixp)
{
  return (&ixp->pci_csr);
}

mem_blk_t *pci_ixp_get_mem_mem (pci_ixp_t *ixp)
{
  return (&ixp->pci_mem);
}
