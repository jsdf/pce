/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/main.h                                      *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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

/* The CPU is synchronized with real time MAC_CPU_SYNC times per seconds */
#define MAC_CPU_SYNC 250


#define MAC_HOOK_NOP    0
#define MAC_HOOK_STOP   1
#define MAC_HOOK_EXIT   2
#define MAC_HOOK_INSERT 3
#define MAC_HOOK_MARK   4

#define MAC_HOOK_SONY        16
#define MAC_HOOK_SONY_OPEN   (MAC_HOOK_SONY + 0)
#define MAC_HOOK_SONY_PRIME  (MAC_HOOK_SONY + 1)
#define MAC_HOOK_SONY_CTRL   (MAC_HOOK_SONY + 2)
#define MAC_HOOK_SONY_STATUS (MAC_HOOK_SONY + 3)
#define MAC_HOOK_SONY_CLOSE  (MAC_HOOK_SONY + 4)


struct macplus_s;
typedef struct macplus_s macplus_t;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif


#include <terminal/terminal.h>
#include <terminal/keys.h>

#include <libini/libini.h>

#include <lib/brkpt.h>
#include <lib/cmd.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/monitor.h>
#include <lib/msg.h>
#include <lib/path.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/sysdep.h>

#include <cpu/e68000/e68000.h>

#include <chipset/e6522.h>
#include <chipset/e8530.h>

#include <devices/memory.h>
#include <devices/nvram.h>
#include <devices/block/block.h>

#include "hook.h"
#include "iwm.h"
#include "keyboard.h"
#include "mem.h"
#include "rtc.h"
#include "scsi.h"
#include "serial.h"
#include "sony.h"
#include "traps.h"
#include "video.h"
#include "macplus.h"
#include "cmd_68k.h"
#include "msg.h"


extern int        par_verbose;

extern const char *par_terminal;

extern unsigned   par_disk_delay_valid;
extern unsigned   par_disk_delay[SONY_DRIVES];

extern monitor_t  par_mon;

extern macplus_t  *par_sim;

extern unsigned   par_sig_int;


void mac_log_deb (const char *msg, ...);


#endif
