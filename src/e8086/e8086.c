/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     e8086.c                                                    *
 * Created:       1996-04-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-19 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8086.c,v 1.8 2003/04/20 00:22:51 hampa Exp $ */


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

  c->mem_get_uint8 = &e86_get_mem_uint8;
  c->mem_get_uint16 = &e86_get_mem_uint16;
  c->mem_set_uint8 = &e86_set_mem_uint8;
  c->mem_set_uint16 = &e86_set_mem_uint16;

  c->prt_get_uint8 = &e86_get_mem_uint8;
  c->prt_get_uint16 = &e86_get_mem_uint16;
  c->prt_set_uint8 = &e86_set_mem_uint8;
  c->prt_set_uint16 = &e86_set_mem_uint16;

  c->hook = NULL;
  c->opstat = NULL;

  c->mem = NULL;
  c->prt = NULL;
  c->ext = NULL;

  c->clocks = 0;
  c->instructions = 0;
  c->delay = 0;

  return (c);
}

void e86_del (e8086_t *c)
{
  free (c);
}

void e86_prt_state (e8086_t *c, FILE *fp)
{
  double      cpi, mips;
  static char ft[2] = { '-', '+' };

  cpi = (c->instructions > 0) ? ((double) c->clocks / (double) c->instructions) : 1.0;
  mips = (c->clocks > 0) ? (4.77 * (double) c->instructions / (double) c->clocks) : 0.0;
  fprintf (fp, "CLK=%016llX  OP=%016llX  DLY=%03lX  CPI=%.4f  MIPS=%.4f\n",
    c->clocks, c->instructions,
    c->delay,
    cpi, mips
  );

  fprintf (fp,
    "AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
    e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
    e86_get_sp (c), e86_get_bp (c), e86_get_si (c), e86_get_di (c)
  );

  fprintf (fp, "CS=%04X  DS=%04X  ES=%04X  SS=%04X  IP=%04X  F =%04X",
    e86_get_cs (c), e86_get_ds (c), e86_get_es (c), e86_get_ss (c),
    e86_get_ip (c), c->flg
  );

  fprintf (fp, "  C%c O%c S%c Z%c A%c P%c I%c D%c\n",
    ft[e86_get_cf (c)], ft[e86_get_of (c)], ft[e86_get_sf (c)],
    ft[e86_get_zf (c)], ft[e86_get_af (c)], ft[e86_get_pf (c)],
    ft[(c->flg & E86_FLG_I) != 0],
    ft[(c->flg & E86_FLG_D) != 0]
  );
}

void e86_log_op (e8086_t *c, const char *str, ...)
{
  va_list     va;
  static char ft[2] = { '-', '+' };

  fprintf (stderr, "E86: %04X:%04X  [%02X %02X %02X %02X %02X %02X] ",
    e86_get_cs (c), e86_get_ip (c),
    c->pq[0], c->pq[1], c->pq[2], c->pq[3], c->pq[4], c->pq[5]
  );

  if (str != NULL) {
    va_start (va, str);
    vfprintf (stderr, str, va);
    va_end (va);
  }

  fputs ("\n", stderr);

  fprintf (stderr,
    "AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
    e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
    e86_get_sp (c), e86_get_bp (c), e86_get_si (c), e86_get_di (c)
  );

  fprintf (stderr, "CS=%04X  DS=%04X  ES=%04X  SS=%04X  IP=%04X  F =%04X",
    e86_get_cs (c), e86_get_ds (c), e86_get_es (c), e86_get_ss (c),
    e86_get_ip (c), c->flg
  );

  fprintf (stderr, "  C%c O%c S%c Z%c A%c P%c I%c D%c\n",
    ft[e86_get_cf (c)], ft[e86_get_of (c)], ft[e86_get_sf (c)],
    ft[e86_get_zf (c)], ft[e86_get_af (c)], ft[e86_get_pf (c)],
    ft[(c->flg & E86_FLG_I) != 0],
    ft[(c->flg & E86_FLG_D) != 0]
  );

  fflush (stderr);
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

  c->sreg[E86_REG_CS] = 0xf000;
  c->ip = 0xfff0;
  c->flg = 0x0000;

  c->pq_cnt = 0;

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

void e86_execute (e8086_t *c)
{
  unsigned op;
  unsigned cnt;

  c->prefix = 0;

  do {
    e86_pq_fill (c);

    c->prefix &= ~E86_PREFIX_NEW;

    if (c->opstat != NULL) {
      c->opstat (c->ext, c->pq[0], c->pq[1]);
    }

    op = c->pq[0];

    cnt = e86_opcodes[op] (c);

    if (cnt > 0) {
      c->ip = (c->ip + cnt) & 0xffff;
      e86_pq_adjust (c, cnt);
    }
  } while (c->prefix & E86_PREFIX_NEW);

  c->instructions += 1;
}

void e86_clock (e8086_t *c)
{
  if (c->delay > 0) {
    c->delay--;
  }

  if (c->delay == 0) {
    e86_execute (c);
  }

  c->clocks += 1;
}

