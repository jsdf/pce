/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/e8086/e80186.c                                         *
 * Created:       2003-08-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-29 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e80186.c,v 1.2 2003/08/29 12:42:18 hampa Exp $ */


#include "e8086.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


/* OP 60: PUSHA */
static
unsigned op_60 (e8086_t *c)
{
  unsigned short tmp;

  tmp = e86_get_sp (c);

  e86_push (c, e86_get_ax (c));
  e86_push (c, e86_get_cx (c));
  e86_push (c, e86_get_dx (c));
  e86_push (c, e86_get_bx (c));
  e86_push (c, tmp);
  e86_push (c, e86_get_bp (c));
  e86_push (c, e86_get_si (c));
  e86_push (c, e86_get_di (c));

  e86_set_clk (c, 19);

  return (1);
}

/* OP 61: POPA */
static
unsigned op_61 (e8086_t *c)
{
  e86_set_di (c, e86_pop (c));
  e86_set_si (c, e86_pop (c));
  e86_set_bp (c, e86_pop (c));
  e86_pop (c);
  e86_set_bx (c, e86_pop (c));
  e86_set_dx (c, e86_pop (c));
  e86_set_cx (c, e86_pop (c));
  e86_set_ax (c, e86_pop (c));

  e86_set_clk (c, 19);

  return (1);
}

/* OP 62: BOUND */
static
unsigned op_62 (e8086_t *c)
{
  long           s1, lo, hi;
  unsigned short seg, ofs;

  e86_get_ea_ptr (c, c->pq + 1);

  s1 = e86_get_reg16 (c, (c->pq[1] >> 3) & 7);

  if (c->ea.is_mem == 0) {
    fprintf (stderr, "BOUND EA is register\n");
    return (c->ea.cnt + 1);
  }

  seg = c->ea.seg;
  ofs = c->ea.ofs;

  lo = e86_get_mem16 (c, seg, ofs);
  hi = e86_get_mem16 (c, seg, ofs + 2);

  s1 = (s1 > 0x7fff) ? (s1 - 0x10000) : s1;
  lo = (lo > 0x7fff) ? (lo - 0x10000) : lo;
  hi = (hi > 0x7fff) ? (hi - 0x10000) : hi;

  if ((s1 < lo) || (s1 > (hi + 2))) {
    e86_trap (c, 5);
    return (0);
  }

  e86_set_clk (c, 19);

  return (c->ea.cnt + 1);
}

/* OP C8: ENTER (80186) */
static
unsigned op_c8 (e8086_t *c)
{
  unsigned char  level;
  unsigned short tmp;

  e86_push (c, e86_get_bp (c));

  tmp = e86_get_sp (c);

  level = c->pq[3] & 0x1f;

  if (level > 0) {
    unsigned       i;
    unsigned short sp, bp;

    sp = e86_get_sp (c);
    bp = e86_get_bp (c);

    for (i = 0; i < level; i++) {
      bp -= 2;
      e86_push (c, e86_get_mem16 (c, sp, bp));
    }

    e86_push (c, tmp);
  }

  e86_set_bp (c, tmp);
  e86_set_sp (c, tmp - e86_mk_uint16 (c->pq[1], c->pq[2]));

  switch (level) {
    case 0:
      e86_set_clk (c, 11);
      break;

    case 1:
      e86_set_clk (c, 15);
      break;

    default:
      e86_set_clk (c, 12 + 4 * (level - 1));
      break;
  }

  return (4);
}

/* OP C9: LEAVE (80186) */
static
unsigned op_c9 (e8086_t *c)
{
  fprintf (stderr, "LEAVE\n"); fflush (stderr);
  sleep (1);

  e86_set_sp (c, e86_get_bp (c));
  e86_set_bp (c, e86_pop (c));
  e86_set_clk (c, 5);

  return (1);
}

void e186_enable (void)
{
  e86_opcodes[0x60] = &op_60;
  e86_opcodes[0x61] = &op_61;
  e86_opcodes[0x62] = &op_62;
  e86_opcodes[0xc8] = &op_c8;
  e86_opcodes[0xc9] = &op_c9;
}
