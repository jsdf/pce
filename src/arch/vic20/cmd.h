/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/cmd.h                                         *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_VIC20_CMD_H
#define PCE_VIC20_CMD_H 1


#include "vic20.h"

#include <cpu/e6502/e6502.h>

#include <lib/cmd.h>
#include <lib/monitor.h>


void v20_print_state_cpu (e6502_t *c);

void v20_run (vic20_t *sim);

int v20_cmd (vic20_t *sim, cmd_t *cmd);

void v20_cmd_init (vic20_t *sim, monitor_t *mon);


#endif
