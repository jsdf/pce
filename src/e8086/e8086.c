/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/e8086/e8086.c                                          *
 * Created:       1996-04-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-27 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: e8086.c,v 1.16 2003/04/26 23:34:02 hampa Exp $ */


#include "e8086.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


static unsigned char e86_get_mem_uint8 (void *mem, unsigned long addr);
static unsigned short e86_get_mem_uint16 (void *mem, unsigned long addr);
static void e86_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val);
static void e86_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val);


e8086_t *e86_new (void)
{
  e8086_t *c;

  c = (e8086_t *) malloc (sizeof (e8086_t));
  if (c == NULL) {
    return (NULL);
  }

  c->mem = NULL;
  c->mem_get_uint8 = &e86_get_mem_uint8;
  c->mem_get_uint16 = &e86_get_mem_uint16;
  c->mem_set_uint8 = &e86_set_mem_uint8;
  c->mem_set_uint16 = &e86_set_mem_uint16;

  c->prt = NULL;
  c->prt_get_uint8 = &e86_get_mem_uint8;
  c->prt_get_uint16 = &e86_get_mem_uint16;
  c->prt_set_uint8 = &e86_set_mem_uint8;
  c->prt_set_uint16 = &e86_set_mem_uint16;

  c->ram = NULL;
  c->ram_cnt = 0;

  c->inta_ext = NULL;
  c->inta = NULL;

  c->op_ext = NULL;
  c->op_hook = NULL;
  c->op_stat = NULL;
  c->op_undef = NULL;

  c->irq = 0;

  c->last_interrupt = 0;

  c->clocks = 0;
  c->instructions = 0;
  c->delay = 0;

  return (c);
}

void e86_del (e8086_t *c)
{
  free (c);
}

void e86_set_ram (e8086_t *c, unsigned char *ram, unsigned long cnt)
{
  c->ram = ram;
  c->ram_cnt = cnt;
}

void e86_set_mem (e8086_t *c, void *mem,
  e86_get_uint8_f get8, e86_set_uint8_f set8,
  e86_get_uint16_f get16, e86_set_uint16_f set16)
{
  c->mem = mem;
  c->mem_get_uint8 = get8;
  c->mem_set_uint8 = set8;
  c->mem_get_uint16 = get16;
  c->mem_set_uint16 = set16;
}

void e86_set_prt (e8086_t *c, void *prt,
  e86_get_uint8_f get8, e86_set_uint8_f set8,
  e86_get_uint16_f get16, e86_set_uint16_f set16)
{
  c->prt = prt;
  c->prt_get_uint8 = get8;
  c->prt_set_uint8 = set8;
  c->prt_get_uint16 = get16;
  c->prt_set_uint16 = set16;
}

void e86_reset (e8086_t *c)
{
  unsigned i;

  c->instructions = 0;

  for (i = 0; i < 8; i++) {
    c->dreg[i] = 0;
  }

  for (i = 0; i < 4; i++) {
    c->sreg[i] = 0;
  }

  e86_set_cs (c, 0xf000);
  e86_set_ip (c, 0xfff0);
  c->flg = 0x0000;

  c->pq_cnt = 0;

  c->irq = 0;

  c->prefix = 0;
}

static
unsigned char e86_get_mem_uint8 (void *mem, unsigned long addr)
{
  return (0xaa);
}

static
unsigned short e86_get_mem_uint16 (void *mem, unsigned long addr)
{
  return (0xaaaa);
}

static
void e86_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val)
{
}

static
void e86_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val)
{
}

void e86_irq (e8086_t *cpu, unsigned val)
{
  cpu->irq = 1;
}

int e86_interrupt (e8086_t *cpu, unsigned n)
{
  if (e86_get_if (cpu)) {
    e86_trap (cpu, n);
    return (0);
  }

  return (1);
}

unsigned long long e86_get_clock (e8086_t *c)
{
  return (c->clocks);
}

unsigned long long e86_get_opcnt (e8086_t *c)
{
  return (c->instructions);
}

unsigned long e86_get_delay (e8086_t *c)
{
  return (c->delay);
}

unsigned e86_get_last_int (e8086_t *c)
{
  return (c->last_interrupt);
}

void e86_execute (e8086_t *c)
{
  unsigned cnt, op;
  int      tf;

  c->prefix = 0;

  tf = e86_get_tf (c);

  do {
    e86_pq_fill (c);

    c->prefix &= ~E86_PREFIX_NEW;

    if (c->op_stat != NULL) {
      c->op_stat (c->op_ext, c->pq[0], c->pq[1]);
    }

    op = c->pq[0];

    cnt = e86_opcodes[op] (c);

    if (cnt > 0) {
      c->ip = (c->ip + cnt) & 0xffff;
      e86_pq_adjust (c, cnt);
    }
  } while (c->prefix & E86_PREFIX_NEW);

  c->instructions += 1;

  tf &= e86_get_tf (c);

  if (tf) {
    e86_trap (c, 1);
  }
  else if (c->irq && e86_get_if (c)) {
    c->irq = 0;

    if (c->inta != NULL) {
      unsigned char irq;

      irq = c->inta (c->inta_ext);

      e86_trap (c, irq);
    }
  }
}

void e86_clock (e8086_t *c, unsigned n)
{
  while (n >= c->delay) {
    n -= c->delay;
    c->clocks += c->delay;
    c->delay = 0;
    e86_execute (c);
  }

  c->delay -= n;
  c->clocks += n;
}
