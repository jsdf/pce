/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/mmu.c                                          *
 * Created:       2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
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


int arm_ifetch (arm_t *c, uint32_t addr, uint32_t *val)
{
  addr &= ~0x03UL;

  if (c->get_uint32 != NULL) {
    *val = c->get_uint32 (c->mem_ext, addr);
  }
  else {
    *val = 0xffffffffUL;
  }

  return (0);
}

int arm_dload8 (arm_t *c, uint32_t addr, uint8_t *val)
{
  if (c->get_uint8 != NULL) {
    *val = c->get_uint8 (c->mem_ext, addr);
  }
  else {
    *val = 0xff;
  }

  return (0);
}

int arm_dload16 (arm_t *c, uint32_t addr, uint16_t *val)
{
  if (c->get_uint16 != NULL) {
    *val = c->get_uint16 (c->mem_ext, addr);
  }
  else {
    *val = 0xffffU;
  }

  return (0);
}

int arm_dload32 (arm_t *c, uint32_t addr, uint32_t *val)
{
  if (c->get_uint32 != NULL) {
    *val = c->get_uint32 (c->mem_ext, addr);
  }
  else {
    *val = 0xffffffffUL;
  }

  return (0);
}

int arm_dstore8 (arm_t *c, uint32_t addr, uint8_t val)
{
  if (c->set_uint8 != NULL) {
    c->set_uint8 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_dstore16 (arm_t *c, uint32_t addr, uint16_t val)
{
  if (c->set_uint16 != NULL) {
    c->set_uint16 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_dstore32 (arm_t *c, uint32_t addr, uint32_t val)
{
  if (c->set_uint32 != NULL) {
    c->set_uint32 (c->mem_ext, addr, val);
  }

  return (0);
}


int arm_get_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t *val)
{
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
  if (c->set_uint8 != NULL) {
    c->set_uint8 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_set_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t val)
{
  if (c->set_uint16 != NULL) {
    c->set_uint16 (c->mem_ext, addr, val);
  }

  return (0);
}

int arm_set_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t val)
{
  if (c->set_uint32 != NULL) {
    c->set_uint32 (c->mem_ext, addr, val);
  }

  return (0);
}
