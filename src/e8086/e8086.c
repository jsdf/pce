/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     e8086.c                                                    *
 * Created:       1996-04-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8086.c,v 1.1 2003/04/15 04:03:58 hampa Exp $ */


#include <pce.h>

#include "e8086.h"
#include "internal.h"


/*************************************************************************
 * Prefetch queue functions
 *************************************************************************/

void e86_pq_init (e8086_t *c)
{
  c->pq_cnt = 0;
}

void e86_pq_fill (e8086_t *c)
{
  unsigned i, j;

  j = 0;

  for (i = c->pq_cnt; i < E86_PQ_SIZE; i++) {
    c->pq[i] = e86_get_mem8 (c, c->sreg[E86_REG_CS], c->ip + i);
  }
}

void e86_pq_adjust (e8086_t *c, unsigned cnt)
{
  unsigned i;

  if (cnt >= c->pq_cnt) {
    c->pq_cnt = 0;
    return;
  }

  for (i = cnt; i < c->pq_cnt; i++) {
    c->pq[i - cnt] = c->pq[i];
  }

  c->pq_cnt -= cnt;
}


/*************************************************************************
 * Flags functions
 *************************************************************************/

void e86_set_flags8 (e8086_t *c, unsigned short mask, unsigned short dst, unsigned short src)
{
  unsigned short set;

  set = 0;

  if (mask & E86_FLG_C) {
    if (dst & 0xff00) {
      set |= E86_FLG_C;
    }
  }

  if (mask & E86_FLG_Z) {
    if ((dst & 0x00ff) == 0) {
      set |= E86_FLG_Z;
    }
  }

  if (mask & E86_FLG_S) {
    if (dst & 0x0080) {
      set |= E86_FLG_S;
    }
  }

  if (mask & E86_FLG_O) {
    if (((src ^ dst) & 0x80) && ((dst & 0xff00) == 0)) {
      set |= E86_FLG_O;
    }
  }

  if (mask & E86_FLG_P) {
    unsigned i, n;

    i = dst & 0xff;
    n = 0;
    while (i != 0) {
      n += 1;
      i &= (i - 1);
    }

    if ((n & 1) == 0) {
      set |= E86_FLG_P;
    }
  }

  c->flg &= ~mask;
  c->flg |= set;
}

void e86_set_flags16 (e8086_t *c, unsigned short mask, unsigned long dst, unsigned long src)
{
  unsigned short set;

  set = 0;

  if (mask & E86_FLG_C) {
    if (dst & 0xffff0000UL) {
      set |= E86_FLG_C;
    }
  }

  if (mask & E86_FLG_Z) {
    if ((dst & 0x0000ffffUL) == 0) {
      set |= E86_FLG_Z;
    }
  }

  if (mask & E86_FLG_S) {
    if (dst & 0x00008000UL) {
      set |= E86_FLG_S;
    }
  }

  if (mask & E86_FLG_O) {
    if (((src ^ dst) & 0x8000UL) && ((dst & 0xffff0000UL) == 0)) {
      set |= E86_FLG_O;
    }
  }

  if (mask & E86_FLG_P) {
    unsigned i, n;

    i = dst & 0xffff;
    n = 0;
    while (i != 0) {
      n += 1;
      i &= (i - 1);
    }

    if ((n & 1) == 0) {
      set |= E86_FLG_P;
    }
  }

  c->flg &= ~mask;
  c->flg |= set;
}

void e86_set_flags_af (e8086_t *c, unsigned d)
{
  if (d & ~0x0f) {
    c->flg |= E86_FLG_A;
  }
  else {
    c->flg &= ~E86_FLG_A;
  }
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

void e86_prt_state (e8086_t *c, FILE *fp)
{
  static char  ft[2] = { '-', '+' };

  fprintf (fp, "-8086-state------------------------------------------------------------------\n");
  fprintf (fp, "clk=%lu  op=%lu  delay=%lu\n", c->clocks, c->instructions, c->delay);

  fprintf (fp,
    "AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
    c->dreg[E86_REG_AX],
    c->dreg[E86_REG_BX],
    c->dreg[E86_REG_CX],
    c->dreg[E86_REG_DX],
    c->dreg[E86_REG_SP],
    c->dreg[E86_REG_BP],
    c->dreg[E86_REG_SI],
    c->dreg[E86_REG_DI]
  );

  fprintf (fp, "CS=%04X  DS=%04X  ES=%04X  SS=%04X  IP=%04X  F =%04X",
    c->sreg[E86_REG_CS],
    c->sreg[E86_REG_DS],
    c->sreg[E86_REG_ES],
    c->sreg[E86_REG_SS],
    c->ip,
    c->flg
  );

  fprintf (fp, "  C%c O%c S%c Z%c A%c P%c I%c D%c\n",
    ft[(c->flg & E86_FLG_C) != 0],
    ft[(c->flg & E86_FLG_O) != 0],
    ft[(c->flg & E86_FLG_S) != 0],
    ft[(c->flg & E86_FLG_Z) != 0],
    ft[(c->flg & E86_FLG_A) != 0],
    ft[(c->flg & E86_FLG_P) != 0],
    ft[(c->flg & E86_FLG_I) != 0],
    ft[(c->flg & E86_FLG_D) != 0]
  );
}

void e86_execute (e8086_t *c)
{
  unsigned op;
  unsigned cnt;

  c->prefix = 0;

  do {
    e86_pq_fill (c);

    op = c->pq[0];

    c->prefix &= ~E86_PREFIX_NEW;

    cnt = e86_opcodes[op] (c);

    if (cnt == 0) {
      e86_pq_init (c);
    }
    else {
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
  c->hook_ext = NULL;

  c->clocks = 0;
  c->instructions = 0;
  c->delay = 0;

  return (c);
}

void e86_del (e8086_t *c)
{
  free (c);
}
