/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/pci.c                                      *
 * Created:       2004-12-10 by Hampa Hug <hampa@hampa.ch>                   *
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


#include "main.h"


static
unsigned char pci_bus_get_ioa8 (pci_405_t *pci, unsigned long addr)
{
  return (mem_get_uint8 (&pci->asio, addr));
}

static
unsigned short pci_bus_get_ioa16 (pci_405_t *pci, unsigned long addr)
{
  unsigned short val;

  val = mem_get_uint16_le (&pci->asio, addr);
  val = s405_br16 (val);

  return (val);
}

static
unsigned long pci_bus_get_ioa32 (pci_405_t *pci, unsigned long addr)
{
  unsigned long val;

  val = mem_get_uint32_le (&pci->asio, addr);
  val = s405_br32 (val);

  return (val);
}

static
void pci_bus_set_ioa8 (pci_405_t *pci, unsigned long addr, unsigned char val)
{
  mem_set_uint8 (&pci->asio, addr, val);
}

static
void pci_bus_set_ioa16 (pci_405_t *pci, unsigned long addr, unsigned short val)
{
  mem_set_uint16_le (&pci->asio, addr, s405_br16 (val));
}

static
void pci_bus_set_ioa32 (pci_405_t *pci, unsigned long addr, unsigned long val)
{
  mem_set_uint32_le (&pci->asio, addr, s405_br32 (val));
}


static
unsigned char pci_bus_get_iob8 (pci_405_t *pci, unsigned long addr)
{
  return (mem_get_uint8 (&pci->asio, addr + 0x00800000UL));
}

static
unsigned short pci_bus_get_iob16 (pci_405_t *pci, unsigned long addr)
{
  unsigned short val;

  val = mem_get_uint16_le (&pci->asio, addr + 0x00800000UL);
  val = s405_br16 (val);

  return (val);
}

static
unsigned long pci_bus_get_iob32 (pci_405_t *pci, unsigned long addr)
{
  unsigned long val;

  val = mem_get_uint32_le (&pci->asio, addr + 0x00800000UL);
  val = s405_br32 (val);

  return (val);
}

static
void pci_bus_set_iob8 (pci_405_t *pci, unsigned long addr, unsigned char val)
{
  mem_set_uint8 (&pci->asio, addr + 0x00800000UL, val);
}

static
void pci_bus_set_iob16 (pci_405_t *pci, unsigned long addr, unsigned short val)
{
  mem_set_uint16_le (&pci->asio, addr + 0x00800000UL, s405_br16 (val));
}

static
void pci_bus_set_iob32 (pci_405_t *pci, unsigned long addr, unsigned long val)
{
  mem_set_uint32_le (&pci->asio, addr + 0x00800000UL, s405_br32 (val));
}


static
unsigned char pci_bus_get_cfg8 (pci_405_t *pci, unsigned long addr)
{
  if ((addr >= 4) && (addr <= 7)) {
    return (pci_get_config_data8 (&pci->bus, (addr - 4) & 0x03) & 0xff);
  }

  return (0x55);
}

static
unsigned short pci_bus_get_cfg16 (pci_405_t *pci, unsigned long addr)
{
  unsigned short val;

  if ((addr >= 4) && (addr <= 7)) {
    val = pci_get_config_data16 (&pci->bus, (addr - 4) & 0x03);
    val = s405_br16 (val);
    return (val);
  }

  return (0x5555);
}

static
unsigned long pci_bus_get_cfg32 (pci_405_t *pci, unsigned long addr)
{
  unsigned long val;

  switch (addr) {
  case 0x00:
    val = pci_get_config_addr (&pci->bus);
    break;

  case 0x04:
    val = pci_get_config_data32 (&pci->bus);
    break;

  default:
    val = 0;
    break;
  }

  return (s405_br32 (val));
}

static
void pci_bus_set_cfg8 (pci_405_t *pci, unsigned long addr, unsigned char val)
{
  if ((addr >= 4) && (addr <= 7)) {
    pci_set_config_data8 (&pci->bus, (addr - 4) & 0x03, val);
  }
}

static
void pci_bus_set_cfg16 (pci_405_t *pci, unsigned long addr, unsigned short val)
{
  val = s405_br16 (val);

  if ((addr >= 4) && (addr <= 7)) {
    pci_set_config_data16 (&pci->bus, (addr - 4) & 0x03, val);
  }
}

static
void pci_bus_set_cfg32 (pci_405_t *pci, unsigned long addr, unsigned long val)
{
  val = s405_br32 (val);

  switch (addr) {
  case 0x00:
    pci_set_config_addr (&pci->bus, val);
    break;

  case 0x04:
    pci_set_config_data32 (&pci->bus, val);
    break;

  default:
    pce_log (MSG_DEB, "pci-bus: set cfg 32 %08lX %08lX\n", addr, val);
    break;
  }
}


static
unsigned char pci_bus_get_csr8 (pci_405_t *pci, unsigned long addr)
{
  return (0);
}

static
unsigned short pci_bus_get_csr16 (pci_405_t *pci, unsigned long addr)
{
  return (0);
}

static
unsigned long pci_bus_get_csr32 (pci_405_t *pci, unsigned long addr)
{
  return (0);
}

static
void pci_bus_set_csr8 (pci_405_t *pci, unsigned long addr, unsigned char val)
{
}

static
void pci_bus_set_csr16 (pci_405_t *pci, unsigned long addr, unsigned short val)
{
}

static
void pci_bus_set_csr32 (pci_405_t *pci, unsigned long addr, unsigned long val)
{
}


void s405_pci_init (pci_405_t *pci)
{
  mem_init (&pci->asio);

  mem_blk_init (&pci->pci_ioa, 0xe8000000UL, 0x00010000UL, 0);
  pci->pci_ioa.ext = pci;
  pci->pci_ioa.get_uint8 = (mem_get_uint8_f) pci_bus_get_ioa8;
  pci->pci_ioa.set_uint8 = (mem_set_uint8_f) pci_bus_set_ioa8;
  pci->pci_ioa.get_uint16 = (mem_get_uint16_f) pci_bus_get_ioa16;
  pci->pci_ioa.set_uint16 = (mem_set_uint16_f) pci_bus_set_ioa16;
  pci->pci_ioa.get_uint32 = (mem_get_uint32_f) pci_bus_get_ioa32;
  pci->pci_ioa.set_uint32 = (mem_set_uint32_f) pci_bus_set_ioa32;

  mem_blk_init (&pci->pci_iob, 0xe8800000UL, 0x03800000UL, 0);
  pci->pci_iob.ext = pci;
  pci->pci_iob.get_uint8 = (mem_get_uint8_f) pci_bus_get_iob8;
  pci->pci_iob.set_uint8 = (mem_set_uint8_f) pci_bus_set_iob8;
  pci->pci_iob.get_uint16 = (mem_get_uint16_f) pci_bus_get_iob16;
  pci->pci_iob.set_uint16 = (mem_set_uint16_f) pci_bus_set_iob16;
  pci->pci_iob.get_uint32 = (mem_get_uint32_f) pci_bus_get_iob32;
  pci->pci_iob.set_uint32 = (mem_set_uint32_f) pci_bus_set_iob32;

  mem_blk_init (&pci->pci_cfg, 0xeec00000UL, 0x00100000UL, 0);
  pci->pci_cfg.ext = pci;
  pci->pci_cfg.get_uint8 = (mem_get_uint8_f) pci_bus_get_cfg8;
  pci->pci_cfg.set_uint8 = (mem_set_uint8_f) pci_bus_set_cfg8;
  pci->pci_cfg.get_uint16 = (mem_get_uint16_f) pci_bus_get_cfg16;
  pci->pci_cfg.set_uint16 = (mem_set_uint16_f) pci_bus_set_cfg16;
  pci->pci_cfg.get_uint32 = (mem_get_uint32_f) pci_bus_get_cfg32;
  pci->pci_cfg.set_uint32 = (mem_set_uint32_f) pci_bus_set_cfg32;

  mem_blk_init (&pci->pci_special, 0xeed00000UL, 0x00100000UL, 0);
  pci->pci_special.ext = pci;
  pci->pci_special.get_uint8 = (mem_get_uint8_f) pci_bus_get_ioa8;
  pci->pci_special.set_uint8 = (mem_set_uint8_f) pci_bus_set_ioa8;
  pci->pci_special.get_uint16 = (mem_get_uint16_f) pci_bus_get_ioa16;
  pci->pci_special.set_uint16 = (mem_set_uint16_f) pci_bus_set_ioa16;
  pci->pci_special.get_uint32 = (mem_get_uint32_f) pci_bus_get_ioa32;
  pci->pci_special.set_uint32 = (mem_set_uint32_f) pci_bus_set_ioa32;

  mem_blk_init (&pci->pci_csr, 0xef400000UL, 0x00100000UL, 0);
  pci->pci_csr.ext = pci;
  pci->pci_csr.get_uint8 = (mem_get_uint8_f) pci_bus_get_csr8;
  pci->pci_csr.set_uint8 = (mem_set_uint8_f) pci_bus_set_csr8;
  pci->pci_csr.get_uint16 = (mem_get_uint16_f) pci_bus_get_csr16;
  pci->pci_csr.set_uint16 = (mem_set_uint16_f) pci_bus_set_csr16;
  pci->pci_csr.get_uint32 = (mem_get_uint32_f) pci_bus_get_csr32;
  pci->pci_csr.set_uint32 = (mem_set_uint32_f) pci_bus_set_csr32;

  pci_init (&pci->bus);
  pci_set_asio (&pci->bus, &pci->asio);
}

void s405_pci_free (pci_405_t *pci)
{
  mem_blk_free (&pci->pci_csr);
  mem_blk_free (&pci->pci_special);
  mem_blk_free (&pci->pci_cfg);
  mem_blk_free (&pci->pci_iob);
  mem_blk_free (&pci->pci_ioa);
  mem_free (&pci->asio);

  pci_free (&pci->bus);
}

pci_405_t *s405_pci_new (void)
{
  pci_405_t *pci;

  pci = malloc (sizeof (pci_405_t));
  if (pci == NULL) {
    return (NULL);
  }

  s405_pci_init (pci);

  return (pci);
}

void s405_pci_del (pci_405_t *pci)
{
  if (pci != NULL) {
    s405_pci_free (pci);
    free (pci);
  }
}

mem_blk_t *s405_pci_get_mem_ioa (pci_405_t *pci)
{
  return (&pci->pci_ioa);
}

mem_blk_t *s405_pci_get_mem_iob (pci_405_t *pci)
{
  return (&pci->pci_iob);
}

mem_blk_t *s405_pci_get_mem_cfg (pci_405_t *pci)
{
  return (&pci->pci_cfg);
}

mem_blk_t *s405_pci_get_mem_special (pci_405_t *pci)
{
  return (&pci->pci_special);
}

mem_blk_t *s405_pci_get_mem_csr (pci_405_t *pci)
{
  return (&pci->pci_csr);
}
