/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e6502/e6502.c                                      *
 * Created:       2004-05-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-05-31 by Hampa Hug <hampa@hampa.ch>                   *
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


#include "e6502.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


static unsigned char e6502_get_mem_uint8 (void *mem, unsigned long addr);
static unsigned short e6502_get_mem_uint16 (void *mem, unsigned long addr);
static void e6502_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val);
static void e6502_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val);


void e6502_init (e6502_t *c)
{
  unsigned i;

  c->cpu = 0;

  c->ea = 0;
  c->ea_page = 0;

  c->rst_val = 1;
  c->irq_val = 0;
  c->nmi_val = 0;
  c->nmi_pnd = 0;

  c->mem = NULL;
  c->mem_get_uint8 = &e6502_get_mem_uint8;
  c->mem_get_uint16 = &e6502_get_mem_uint16;
  c->mem_set_uint8 = &e6502_set_mem_uint8;
  c->mem_set_uint16 = &e6502_set_mem_uint16;

  c->ram = NULL;
  c->ram_lo = 0xffffU;
  c->ram_hi = 0;

  c->op_ext = NULL;
  c->op_hook = NULL;
  c->op_stat = NULL;
  c->op_undef = NULL;

  for (i = 0; i < 256; i++) {
    c->op[i] = e6502_opcodes[i];
  }

  c->delay = 0;
  c->clkcnt = 0;
  c->inscnt = 0;
}

e6502_t *e6502_new (void)
{
  e6502_t *c;

  c = (e6502_t *) malloc (sizeof (e6502_t));
  if (c == NULL) {
    return (NULL);
  }

  e6502_init (c);

  return (c);
}

void e6502_free (e6502_t *c)
{
}

void e6502_del (e6502_t *c)
{
  if (c != NULL) {
    e6502_free (c);
    free (c);
  }
}

void e6502_set_ram (e6502_t *c, unsigned char *ram, unsigned short lo, unsigned short hi)
{
  c->ram = ram;
  c->ram_lo = lo;
  c->ram_hi = hi;
}

void e6502_set_mem_f (e6502_t *c, void *mem,
  void *get8, void *set8, void *get16, void *set16)
{
  c->mem = mem;
  c->mem_get_uint8 = (e6502_get_uint8_f) get8;
  c->mem_set_uint8 = (e6502_set_uint8_f) set8;
  c->mem_get_uint16 = (e6502_get_uint16_f) get16;
  c->mem_set_uint16 = (e6502_set_uint16_f) set16;
}

static
unsigned char e6502_get_mem_uint8 (void *mem, unsigned long addr)
{
  return (0xaa);
}

static
unsigned short e6502_get_mem_uint16 (void *mem, unsigned long addr)
{
  return (0xaaaa);
}

static
void e6502_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val)
{
}

static
void e6502_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val)
{
}

unsigned long long e6502_get_clock (e6502_t *c)
{
  return (c->clkcnt);
}

unsigned long long e6502_get_opcnt (e6502_t *c)
{
  return (c->inscnt);
}

unsigned long e6502_get_delay (e6502_t *c)
{
  return (c->delay);
}

void e6502_set_reset (e6502_t *c, unsigned char val)
{
  if ((c->rst_val != 0) && (val == 0)) {
    e6502_reset (c);
  }

  c->rst_val = (val != 0);
}

void e6502_set_irq (e6502_t *c, unsigned char val)
{
  c->irq_val = (val != 0);
}

void e6502_set_nmi (e6502_t *c, unsigned char val)
{
  if ((c->nmi_val == 0) && (val != 0)) {
    c->nmi_pnd = 1;
  }

  c->nmi_val = (val != 0);
}


void e6502_undefined (e6502_t *c)
{
  if (c->op_undef != NULL) {
    c->op_undef (c->op_ext, c->inst[0]);
  }
}

void e6502_reset (e6502_t *c)
{
  c->delay = 7;
  c->rst_val = 0;
  c->nmi_pnd = 0;

  e6502_set_a (c, 0x00);
  e6502_set_x (c, 0x00);
  e6502_set_y (c, 0x00);
  e6502_set_s (c, 0x00);
  e6502_set_p (c, E6502_FLG_R | E6502_FLG_I);
  e6502_set_pc (c, e6502_get_mem16 (c, 0xfffcU));
}

void e6502_execute (e6502_t *c)
{
  if (c->nmi_pnd) {
    c->nmi_pnd = 0;
    e6502_trap (c, 0xfffaU);
    return;
  }

  if ((c->irq_val != 0) && (e6502_get_if (c) == 0)) {
    e6502_trap (c, 0xfffeU);
    return;
  }

  c->inst[0] = e6502_get_mem8 (c, c->pc);

  if (c->op_stat != NULL) {
    c->op_stat (c->op_ext, c->inst[0]);
  }

  if (c->op_stat != NULL) {
    c->op_stat (c->op_ext, c->inst[0]);
  }

  c->op[c->inst[0]] (c);

  c->inscnt += 1;
}

void e6502_clock (e6502_t *c, unsigned n)
{
  if (c->rst_val) {
    return;
  }

  while (n >= c->delay) {
    n -= c->delay;
    c->clkcnt += c->delay;
    c->delay = 0;
    e6502_execute (c);
  }

  c->delay -= n;
  c->clkcnt += n;
}
