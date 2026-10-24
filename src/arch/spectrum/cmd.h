/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/cmd.h                                      *
 * Created:     2022-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef PCE_SPECTRUM_CMD_H
#define PCE_SPECTRUM_CMD_H 1


#include "spectrum.h"

#include <cpu/e8080/e8080.h>
#include <lib/cmd.h>
#include <lib/monitor.h>


void print_state_cpu (e8080_t *c);

void spec_run (spectrum_t *sim);

int spec_cmd (spectrum_t *sim, cmd_t *cmd);

void spec_cmd_init (spectrum_t *sim, monitor_t *mon);


#endif
