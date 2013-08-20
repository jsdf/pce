/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/main.h                                      *
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


#ifndef PCE_ATARIST_MAIN_H
#define PCE_ATARIST_MAIN_H 1


#include <config.h>


#define ST_CPU_CLOCK 8000000


struct atari_st_s;
typedef struct atari_st_s atari_st_t;


extern int        par_verbose;

extern const char *par_terminal;

extern atari_st_t *par_sim;


void sim_stop (void);

void st_log_deb (const char *msg, ...);


#endif
