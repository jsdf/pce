/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/main.h                                     *
 * Created:       2004-11-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-26 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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

/* $Id$ */


#ifndef PCE_SIMARM_MAIN_H
#define PCE_SIMARM_MAIN_H 1


#include <lib/cmd.h>

#include <cpu/arm/arm.h>

#include <devices/memory.h>
#include <devices/pci.h>
#include <devices/slip.h>

#include "timer.h"
#include "intc.h"
#include "pci.h"
#include "simarm.h"
#include "sercons.h"
#include "cmd_arm.h"


#define sarm_br16(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#define sarm_br32(x) ((((x) & 0xff) << 24) | ((((x) >> 8) & 0xff) << 16) \
 | ((((x) >> 16) & 0xff) << 8) | (((x) >> 24) & 0xff))


extern int      par_verbose;

extern unsigned par_xlat;

extern simarm_t *par_sim;

extern unsigned par_sig_int;


void prt_sep (FILE *fp, const char *str, ...);

void prt_state (simarm_t *sim, FILE *fp, const char *str);

void pce_start (void);
void pce_stop (void);
void pce_run (void);


#endif
