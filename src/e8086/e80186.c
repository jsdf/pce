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

/* $Id: e80186.c,v 1.1 2003/08/29 09:45:42 hampa Exp $ */


#include "e8086.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


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
  e86_set_sp (c, e86_get_bp (c));
  e86_set_bp (c, e86_pop (c));
  e86_set_clk (c, 5);

  return (1);
}

void e186_enable (void)
{
  e86_opcodes[0xc8] = &op_c8;
  e86_opcodes[0xc9] = &op_c9;
}
