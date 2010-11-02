/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/main.h                                        *
 * Created:     2001-05-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_IBMPC_MAIN_H
#define PCE_IBMPC_MAIN_H 1


#include <config.h>


#define PCE_IBMPC_CLK0 14318184
#define PCE_IBMPC_CLK1 (PCE_IBMPC_CLK0 / 3)
#define PCE_IBMPC_CLK2 (PCE_IBMPC_CLK0 / 12)

#define PCE_IBMPC_5150 0
#define PCE_IBMPC_5160 1


extern const char *par_terminal;
extern const char *par_video;


void pc_log_deb (const char *msg, ...);


#endif
