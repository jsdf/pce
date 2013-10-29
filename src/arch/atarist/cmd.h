/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/cmd.h                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_CMD_H
#define PCE_ATARIST_CMD_H 1


#include "atarist.h"

#include <lib/cmd.h>
#include <lib/monitor.h>


void st_print_state_cpu (atari_st_t *sim);

void st_run (atari_st_t *sim);

int st_cmd (atari_st_t *sim, cmd_t *cmd);

void st_cmd_init (atari_st_t *sim, monitor_t *mon);

// emscripten specific run loop
void st_run_emscripten (atari_st_t *sim);
void st_run_emscripten_step ();

#endif
