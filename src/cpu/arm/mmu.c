/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/mmu.c                                          *
 * Created:       2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-11 by Hampa Hug <hampa@hampa.ch>                   *
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


int arm_translate (arm_t *c, uint32_t *addr, unsigned *domn, unsigned *perm)
{
  arm_copr15_t *mmu;
  unsigned     ap;
  uint32_t     addr1, addr2;
  uint32_t     desc1, desc2;

  mmu = c->copr[15]->ext;

  addr1 = mmu->reg[2] | ((*addr >> 18) & 0x0000fffcUL);
  desc1 = c->get_uint32 (c->mem_ext, addr1);

  addr2 = 0;

  switch (desc1 & 0x03) {
  case 0x00:
    /* section translation fault */
    fprintf (stderr, "level 1 translation fault (base=%08lX addr=%08lX desc=%08lX\n",
      (unsigned long) mmu->reg[2],
      (unsigned long) *addr,
      (unsigned long) desc1
    );
    return (1);

  case 0x01:
    /* coarse */
    *domn = arm_get_bits (desc1, 5, 4);
    addr2 = (desc1 & 0xfffffc00UL) | ((*addr >> 10) & 0x03fc);
    break;

  case 0x02:
    /* section */
    *domn = arm_get_bits (desc1, 5, 4);
    *perm = arm_get_bits (desc1, 10, 2);
    *addr = (desc1 & 0xfff00000UL) | (*addr & 0x000fffffUL);
    return (0);

  case 0x03:
    /* fine */
    *domn = arm_get_bits (desc1, 5, 4);
    addr2 = (desc1 & 0xfffffc00UL) | ((*addr >> 8) & 0x00000ffcUL);
    break;
  }

  desc2 = c->get_uint32 (c->mem_ext, addr2);

  switch (desc2 & 0x03) {
  case 0x00:
    /* page translation fault */
    return (1);

  case 0x01:
    /* large page */
    ap = 4 + 2 * arm_get_bits (*addr, 14, 2);
    *perm = arm_get_bits (desc2, ap, 2);
    *addr = (desc2 & 0xffff0000UL) | (*addr & 0x0000ffffUL);
    return (0);

  case 0x02:
    /* small page */
    ap = 4 + 2 * arm_get_bits (*addr, 10, 2);
    *perm = arm_get_bits (desc2, ap, 2);
    *addr = (desc2 & 0xfffff000UL) | (*addr & 0x00000fffUL);
    return (0);

  case 0x03:
    /* tiny page */
    *perm = arm_get_bits (desc2, 4, 2);
    *addr = (desc2 & 0xfffffc00UL) | (*addr & 0x000003ffUL);
    return (0);
  }

  return (1);
}

int arm_translate_read (arm_t *c, uint32_t *addr)
{
  arm_copr15_t *mmu;
  unsigned     domn, perm;
  uint32_t     tmp;

  if (c->copr[15] == NULL) {
    return (0);
  }

  mmu = c->copr[15]->ext;

  if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
    return (0);
  }

  tmp = *addr;

  if (arm_translate (c, addr, &domn, &perm)) {
    /* exception */
    return (1);
  }

  /* check data access */

  return (0);
}

int arm_translate_write (arm_t *c, uint32_t *addr)
{
  arm_copr15_t *mmu;
  unsigned     domn, perm;
  uint32_t     tmp;

  if (c->copr[15] == NULL) {
    return (0);
  }

  mmu = c->copr[15]->ext;

  if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
    return (0);
  }

  tmp = *addr;

  if (arm_translate (c, addr, &domn, &perm)) {
    /* exception */
    return (1);
  }

  /* check data access */

  return (0);
}

int arm_translate_exec (arm_t *c, uint32_t *addr)
{
  arm_copr15_t *mmu;
  unsigned     domn, perm;
  uint32_t     tmp;

  if (c->copr[15] == NULL) {
    return (0);
  }

  mmu = c->copr[15]->ext;

  if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
    return (0);
  }

  tmp = *addr;

  if (arm_translate (c, addr, &domn, &perm)) {
    /* exception */
    return (1);
  }

  /* check read access */

  return (0);
}

int arm_translate_extern (arm_t *c, uint32_t *addr, unsigned xlat)
{
  arm_copr15_t *mmu;
  unsigned     domn, perm;

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

  if (arm_translate (c, addr, &domn, &perm)) {
    return (1);
  }

  return (0);
}

int arm_ifetch (arm_t *c, uint32_t addr, uint32_t *val)
{
  addr &= ~0x03UL;

  if (arm_translate_exec (c, &addr)) {
    return (1);
  }

  *val = c->get_uint32 (c->mem_ext, addr);

  return (0);
}

int arm_dload8 (arm_t *c, uint32_t addr, uint8_t *val)
{
  if (arm_translate_read (c, &addr)) {
    return (1);
  }

  *val = c->get_uint8 (c->mem_ext, addr);

  return (0);
}

int arm_dload16 (arm_t *c, uint32_t addr, uint16_t *val)
{
  if (arm_translate_read (c, &addr)) {
    return (1);
  }

  *val = c->get_uint16 (c->mem_ext, addr);

  return (0);
}

int arm_dload32 (arm_t *c, uint32_t addr, uint32_t *val)
{
  if (arm_translate_read (c, &addr)) {
    return (1);
  }

  *val = c->get_uint32 (c->mem_ext, addr);

  return (0);
}

int arm_dstore8 (arm_t *c, uint32_t addr, uint8_t val)
{
  if (arm_translate_write (c, &addr)) {
    return (1);
  }

  c->set_uint8 (c->mem_ext, addr, val);

  return (0);
}

int arm_dstore16 (arm_t *c, uint32_t addr, uint16_t val)
{
  if (arm_translate_write (c, &addr)) {
    return (1);
  }

  c->set_uint16 (c->mem_ext, addr, val);

  return (0);
}

int arm_dstore32 (arm_t *c, uint32_t addr, uint32_t val)
{
  if (arm_translate_write (c, &addr)) {
    return (1);
  }

  c->set_uint32 (c->mem_ext, addr, val);

  return (0);
}


int arm_get_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t *val)
{
  if (arm_translate_extern (c, &addr, xlat)) {
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
  if (arm_translate_extern (c, &addr, xlat)) {
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
  if (arm_translate_extern (c, &addr, xlat)) {
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
  if (arm_translate_extern (c, &addr, xlat)) {
    return (1);
  }

  if (c->set_uint8 != NULL) {
    c->set_uint8 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_set_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t val)
{
  if (arm_translate_extern (c, &addr, xlat)) {
    return (1);
  }

  if (c->set_uint16 != NULL) {
    c->set_uint16 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_set_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t val)
{
  if (arm_translate_extern (c, &addr, xlat)) {
    return (1);
  }

  if (c->set_uint32 != NULL) {
    c->set_uint32 (c->mem_ext, addr, val);
  }

  return (0);
}
