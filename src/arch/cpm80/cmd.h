/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/cmd.h                                         *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2016 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_CPM80_CMD_H
#define PCE_CPM80_CMD_H 1


#include "cpm80.h"

#include <cpu/e8080/e8080.h>
#include <lib/cmd.h>
#include <lib/monitor.h>


void print_state_cpu (e8080_t *c);

void c80_run (cpm80_t *sim);

int c80_cmd (cpm80_t *sim, cmd_t *cmd);

void c80_cmd_init (cpm80_t *sim, monitor_t *mon);


#endif
