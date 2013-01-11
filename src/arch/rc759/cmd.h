/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/cmd.h                                         *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_RC759_CMD_H
#define PCE_RC759_CMD_H 1


#include "rc759.h"

#include <cpu/e8086/e8086.h>

#include <lib/cmd.h>
#include <lib/monitor.h>


void print_state_cpu (e8086_t *c);

void rc759_run (rc759_t *sim);

int rc759_cmd (rc759_t *sim, cmd_t *cmd);

void rc759_cmd_init (rc759_t *sim, monitor_t *mon);


#endif
