/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/main.h                                      *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_MAIN_H
#define PCE_MACPLUS_MAIN_H 1


#include <config.h>


//#define DEBUG_INT 1
//#define DEBUG_IWM 1
//#define DEBUG_KBD 1
//#define DEBUG_MEM 1
//#define DEBUG_RTC 1
//#define DEBUG_SCC 1
//#define DEBUG_SERIAL 1
//#define DEBUG_SCSI 1
//#define DEBUG_SONY 1
//#define DEBUG_VIA 1


#define MAC_CPU_CLOCK 7833600


struct macplus_s;
typedef struct macplus_s macplus_t;


extern int        par_verbose;

extern const char *par_terminal;

extern unsigned   par_disk_delay_valid;
extern unsigned   par_disk_delay[];

extern macplus_t  *par_sim;

extern unsigned   par_sig_int;


void mac_log_deb (const char *msg, ...);


#endif
