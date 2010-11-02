/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/cmd.h                                         *
 * Created:     2010-09-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_IBMPC_CMD_H
#define PCE_IBMPC_CMD_H 1


#include "ibmpc.h"

#include <cpu/e8086/e8086.h>

#include <lib/cmd.h>


void prt_state_cpu (e8086_t *c);

void pc_run (ibmpc_t *pc);

int pc_cmd (ibmpc_t *pc, cmd_t *cmd);

void pc_cmd_init (ibmpc_t *pc);


#endif
