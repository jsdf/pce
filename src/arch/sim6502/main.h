/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim6502/main.h                                    *
 * Created:       2004-05-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-05-26 by Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_SIM6502_MAIN_H
#define PCE_SIM6502_MAIN_H 1


#include "sim6502.h"

#include <lib/cmd.h>
#include <lib/brkpt.h>


extern int       par_verbose;

extern sim6502_t *par_sim;

extern unsigned  par_sig_int;


#endif
