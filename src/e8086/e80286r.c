/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/e8086/e80286r.c                                        *
 * Created:       2003-10-07 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-10-07 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e80286r.c,v 1.1 2003/10/07 16:05:09 hampa Exp $ */


#include "e8086.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>


/* OP 0F */
static
unsigned op_0f (e8086_t *c)
{
  e86_undefined (c);

  if (c->cpu & E86_CPU_INT6) {
    return (0);
  }

  return (1);
}

void e86_enable_286 (e8086_t *c)
{
  e86_enable_186 (c);

  c->cpu |= (E86_CPU_INT6 | E86_CPU_FLAGS286);

  c->op[0x0f] = &op_0f;
}
