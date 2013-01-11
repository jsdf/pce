/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/cmd_68k.h                                   *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_CMD_68K_H
#define PCE_MACPLUS_CMD_68K_H 1


#include "macplus.h"

#include <lib/cmd.h>
#include <lib/monitor.h>


void mac_prt_state (macplus_t *sim, const char *str);

void mac_run (macplus_t *sim);

int mac_cmd (macplus_t *sim, cmd_t *cmd);

void mac_cmd_init (macplus_t *sim, monitor_t *mon);


#endif
