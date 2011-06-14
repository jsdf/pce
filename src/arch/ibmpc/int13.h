/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/int13.h                                       *
 * Created:     2003-04-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_INT13_H
#define PCE_INT13_H 1


#include "ibmpc.h"

#include <cpu/e8086/e8086.h>
#include <drivers/block/block.h>


void dsk_int_13_check (ibmpc_t *pc);

void dsk_int13 (disks_t *dsks, e8086_t *cpu);


#endif
