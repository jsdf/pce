/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     util.c                                                     *
 * Created:       2003-04-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: util.c,v 1.1 2003/04/17 14:15:08 hampa Exp $ */


#include "e8086.h"
#include "internal.h"

#include <stdio.h>
#include <stdarg.h>


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

void e86_set_flags_af (e8086_t *c, unsigned d)
{
  if (d & ~0x0f) {
    c->flg |= E86_FLG_A;
  }
  else {
    c->flg &= ~E86_FLG_A;
  }
}


/**************************************************************************
 * Utility functions
 **************************************************************************/

void e86_push (e8086_t *c, unsigned short val)
{
  e86_set_sp (c, e86_get_sp (c) - 2);

  e86_set_mem16 (c, e86_get_ss (c), e86_get_sp (c), val);
}

unsigned short e86_pop (e8086_t *c)
{
  unsigned short sp;

  sp = e86_get_sp (c);

  e86_set_sp (c, sp + 2);

  return (e86_get_mem16 (c, e86_get_ss (c), sp));
}

void e86_trap (e8086_t *c, unsigned n)
{
  unsigned short ofs;

  e86_push (c, c->flg);
  e86_push (c, c->sreg[E86_REG_CS]);
  e86_push (c, c->ip);

  ofs = (unsigned short) (n & 0xff) << 2;

  c->ip = e86_get_mem16 (c, 0, ofs);
  c->sreg[E86_REG_CS] = e86_get_mem16 (c, 0, ofs + 2);
  c->flg &= ~(E86_FLG_I | E86_FLG_T);

  e86_pq_init (c);
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
