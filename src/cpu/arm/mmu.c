/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/mmu.c                                          *
 * Created:       2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-15 by Hampa Hug <hampa@hampa.ch>                   *
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

#include "arm.h"
#include "internal.h"


void arm_mmu_fault (arm_t *c, uint32_t addr, unsigned status, unsigned domn)
{
  arm_copr15_t *mmu;

  mmu = c->copr[15]->ext;

  mmu->reg[6] = addr;
  mmu->reg[5] = ((domn & 0x0f) << 4) | (status & 0x0f);

  arm_exception_data_abort (c);
}

void arm_mmu_translation_fault (arm_t *c, uint32_t addr, unsigned domn, int sect)
{
  arm_mmu_fault (c, addr, sect ? 0x05 : 0x07, domn);
}

void arm_mmu_domain_fault (arm_t *c, uint32_t addr, unsigned domn, int sect)
{
  arm_mmu_fault (c, addr, sect ? 0x09 : 0x0b, domn);
}

void arm_mmu_permission_fault (arm_t *c, uint32_t addr, unsigned domn, int sect)
{
  arm_mmu_fault (c, addr, sect ? 0x0d : 0x0f, domn);
}


/*!***************************************************************************
 * @short Check access permissions
 * @param cr    The control register (coprocessor 15 register 1)
 * @param perm  The page or section permission bits
 * @param priv  Check for privileged access if true
 * @param w     Check for write access if true, read access if false
 * @return Zero if access violation, non-zero if ok
 *****************************************************************************/
int arm_mmu_check_perm (uint32_t cr, unsigned perm, int priv, int w)
{
  if (perm == 0) {
    switch (cr & (ARM_C15_CR_S | ARM_C15_CR_R)) {
    case 0x00: /* no access */
      return (0);

    case ARM_C15_CR_S: /* priv: read only, user: no access */
      return (priv && (w == 0));

    case ARM_C15_CR_R: /* read only */
      return (w == 0);

    case ARM_C15_CR_S | ARM_C15_CR_R: /* undefined */
      return (0);
    }

    return (0);
  }

  if (priv) {
    return (1);
  }

  switch (perm) {
  case 0x01: /* no access */
    return (0);

  case 0x02: /* read only */
    return (w == 0);

  case 0x03: /* read/write */
    return (1);
  }

  return (0);
}

/*!***************************************************************************
 * @short Translate a virtual address
 * @param  c     The ARM context
 * @param  addr  Virtual address input, real address output
 * @retval domn  Returns the domain number
 * @retval perm  The page or section permission bits
 * @retval sect  The virtual address is in a section if true, in a page if false
 * @return Non-zero on translation fault
 *****************************************************************************/
int arm_translate (arm_t *c, uint32_t *addr, unsigned *domn, unsigned *perm, int *sect)
{
  arm_copr15_t *mmu;
  unsigned     ap;
  uint32_t     addr1, addr2;
  uint32_t     desc1, desc2;
  uint32_t     vaddr;

  mmu = c->copr[15]->ext;

  addr1 = (mmu->reg[2] & 0xffffc000UL) | ((*addr >> 18) & 0x00003ffcUL);
  desc1 = c->get_uint32 (c->mem_ext, addr1);

  addr2 = 0;

  switch (desc1 & 0x03) {
  case 0x00:
    /* section translation fault */
    *domn = 0;
    *sect = 1;
    return (1);

  case 0x01:
    /* coarse */
    addr2 = (desc1 & 0xfffffc00UL) | ((*addr >> 10) & 0x000003fcUL);
    *domn = arm_get_bits (desc1, 5, 4);
    break;

  case 0x02:
    /* section */
    *addr = (desc1 & 0xfff00000UL) | (*addr & 0x000fffffUL);
    *domn = arm_get_bits (desc1, 5, 4);
    *perm = arm_get_bits (desc1, 10, 2);
    *sect = 1;
    return (0);

  case 0x03:
    /* fine */
    addr2 = (desc1 & 0xfffff000UL) | ((*addr >> 8) & 0x00000ffcUL);
    *domn = arm_get_bits (desc1, 5, 4);
    break;
  }

  desc2 = c->get_uint32 (c->mem_ext, addr2);

  *sect = 0;

  switch (desc2 & 0x03) {
  case 0x00:
    /* page translation fault */
    *perm = 0;
    return (1);

  case 0x01:
    /* large page */
    ap = 4 + 2 * arm_get_bits (*addr, 14, 2);
    *addr = (desc2 & 0xffff0000UL) | (*addr & 0x0000ffffUL);
    *perm = arm_get_bits (desc2, ap, 2);
    return (0);

  case 0x02:
    /* small page */
    ap = 4 + 2 * arm_get_bits (*addr, 10, 2);
    *addr = (desc2 & 0xfffff000UL) | (*addr & 0x00000fffUL);
    *perm = arm_get_bits (desc2, ap, 2);
    return (0);

  case 0x03:
    if ((desc1 & 0x03) == 0x01) {
      /* xscale extended small page */
      *addr = (desc2 & 0xfffff000UL) | (*addr & 0x00000fffUL);
      *perm = arm_get_bits (desc2, 4, 2);
      return (0);
    }

    /* tiny page */
    vaddr = *addr;
    *perm = arm_get_bits (desc2, 4, 2);
    *addr = (desc2 & 0xfffffc00UL) | (*addr & 0x000003ffUL);
    return (0);
  }

  /* non-reachable */

  *perm = 0;

  return (1);
}

int arm_translate_check (arm_t *c, uint32_t *addr, int p, int w, int x)
{
  arm_copr15_t *mmu;
  unsigned     domn, perm;
  int          sect;
  uint32_t     vaddr;

  if (c->copr[15] == NULL) {
    return (0);
  }

  mmu = c->copr[15]->ext;

  if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
    return (0);
  }

  vaddr = *addr;

  if (arm_translate (c, addr, &domn, &perm, &sect)) {
    if (x) {
      arm_exception_prefetch_abort (c);
    }
    else {
      arm_mmu_translation_fault (c, vaddr, domn, sect);
    }
    return (1);
  }

  /* check domain */
  switch ((mmu->reg[3] >> (2 * domn)) & 0x03) {
  case 0x00: /* no access */
    if (x) {
      arm_exception_prefetch_abort (c);
    }
    else {
      arm_mmu_domain_fault (c, vaddr, domn, sect);
    }
    return (1);

  case 0x01: /* client */
    if (arm_mmu_check_perm (mmu->reg[1], perm, p, w) == 0) {
      if (x) {
        arm_exception_prefetch_abort (c);
      }
      else {
        arm_mmu_permission_fault (c, vaddr, domn, sect);
      }
      return (1);
    }
    return (0);

  case 0x02: /* undefined */
    return (0);

  case 0x03: /* manager */
    return (0);
  }

  return (0);
}

int arm_translate_extern (arm_t *c, uint32_t *addr, unsigned xlat,
  unsigned *domn, unsigned *perm)
{
  arm_copr15_t *mmu;
  unsigned     domn1, perm1;
  int          sect;

  if (domn == NULL) {
    domn = &domn1;
  }

  if (perm == NULL) {
    perm = &perm1;
  }

  *domn = 0;
  *perm = 0;

  if (xlat == ARM_XLAT_REAL) {
    return (0);
  }

  if (c->copr[15] == NULL) {
    return (0);
  }

  mmu = c->copr[15]->ext;

  if ((xlat == ARM_XLAT_CPU) && ((mmu->reg[1] & ARM_C15_CR_M) == 0)) {
    return (0);
  }

  if (arm_translate (c, addr, domn, perm, &sect)) {
    return (1);
  }

  return (0);
}

int arm_ifetch (arm_t *c, uint32_t addr, uint32_t *val)
{
  addr &= ~0x03UL;

  if (arm_translate_check (c, &addr, arm_is_privileged (c), 0, 1)) {
    return (1);
  }

  *val = c->get_uint32 (c->mem_ext, addr);

  return (0);
}

int arm_dload8 (arm_t *c, uint32_t addr, uint8_t *val)
{
  if (arm_translate_check (c, &addr, arm_is_privileged (c), 0, 0)) {
    return (1);
  }

  *val = c->get_uint8 (c->mem_ext, addr);

  return (0);
}

int arm_dload16 (arm_t *c, uint32_t addr, uint16_t *val)
{
  if (arm_translate_check (c, &addr, arm_is_privileged (c), 0, 0)) {
    return (1);
  }

  *val = c->get_uint16 (c->mem_ext, addr);

  return (0);
}

int arm_dload32 (arm_t *c, uint32_t addr, uint32_t *val)
{
  if (arm_translate_check (c, &addr, arm_is_privileged (c), 0, 0)) {
    return (1);
  }

  *val = c->get_uint32 (c->mem_ext, addr);

  return (0);
}

int arm_dstore8 (arm_t *c, uint32_t addr, uint8_t val)
{
  if (arm_translate_check (c, &addr, arm_is_privileged (c), 1, 0)) {
    return (1);
  }

  c->set_uint8 (c->mem_ext, addr, val);

  return (0);
}

int arm_dstore16 (arm_t *c, uint32_t addr, uint16_t val)
{
  if (arm_translate_check (c, &addr, arm_is_privileged (c), 1, 0)) {
    return (1);
  }

  c->set_uint16 (c->mem_ext, addr, val);

  return (0);
}

int arm_dstore32 (arm_t *c, uint32_t addr, uint32_t val)
{
  if (arm_translate_check (c, &addr, arm_is_privileged (c), 1, 0)) {
    return (1);
  }

  c->set_uint32 (c->mem_ext, addr, val);

  return (0);
}

int arm_dload8_t (arm_t *c, uint32_t addr, uint8_t *val)
{
  if (arm_translate_check (c, &addr, 0, 0, 0)) {
    return (1);
  }

  *val = c->get_uint8 (c->mem_ext, addr);

  return (0);
}

int arm_dload16_t (arm_t *c, uint32_t addr, uint16_t *val)
{
  if (arm_translate_check (c, &addr, 0, 0, 0)) {
    return (1);
  }

  *val = c->get_uint16 (c->mem_ext, addr);

  return (0);
}

int arm_dload32_t (arm_t *c, uint32_t addr, uint32_t *val)
{
  if (arm_translate_check (c, &addr, 0, 0, 0)) {
    return (1);
  }

  *val = c->get_uint32 (c->mem_ext, addr);

  return (0);
}

int arm_dstore8_t (arm_t *c, uint32_t addr, uint8_t val)
{
  if (arm_translate_check (c, &addr, 0, 1, 0)) {
    return (1);
  }

  c->set_uint8 (c->mem_ext, addr, val);

  return (0);
}

int arm_dstore16_t (arm_t *c, uint32_t addr, uint16_t val)
{
  if (arm_translate_check (c, &addr, 0, 1, 0)) {
    return (1);
  }

  c->set_uint16 (c->mem_ext, addr, val);

  return (0);
}

int arm_dstore32_t (arm_t *c, uint32_t addr, uint32_t val)
{
  if (arm_translate_check (c, &addr, 0, 1, 0)) {
    return (1);
  }

  c->set_uint32 (c->mem_ext, addr, val);

  return (0);
}


int arm_get_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t *val)
{
  if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
    return (1);
  }

  if (c->get_uint8 != NULL) {
    *val = c->get_uint8 (c->mem_ext, addr);
  }
  else {
    *val = 0xff;
  }

  return (0);
}

int arm_get_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t *val)
{
  if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
    return (1);
  }

  if (c->get_uint16 != NULL) {
    *val = c->get_uint16 (c->mem_ext, addr);
  }
  else {
    *val = 0xffffU;
  }

  return (0);
}

int arm_get_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t *val)
{
  if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
    return (1);
  }

  if (c->get_uint32 != NULL) {
    *val = c->get_uint32 (c->mem_ext, addr);
  }
  else {
    *val = 0xffffffffUL;
  }

  return (0);
}

int arm_set_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t val)
{
  if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
    return (1);
  }

  if (c->set_uint8 != NULL) {
    c->set_uint8 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_set_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t val)
{
  if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
    return (1);
  }

  if (c->set_uint16 != NULL) {
    c->set_uint16 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_set_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t val)
{
  if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
    return (1);
  }

  if (c->set_uint32 != NULL) {
    c->set_uint32 (c->mem_ext, addr, val);
  }

  return (0);
}
