/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/main.h                                       *
 * Created:     2004-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#ifndef PCE_SIMARM_MAIN_H
#define PCE_SIMARM_MAIN_H 1


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <lib/log.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/msg.h>
#include <lib/monitor.h>
#include <lib/iniata.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/sysdep.h>

#include <cpu/arm/arm.h>

#include <devices/memory.h>
#include <devices/pci.h>

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


void prt_state (simarm_t *sim, FILE *fp, const char *str);


#endif
