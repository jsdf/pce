/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/monitor.h                                            *
 * Created:     2006-12-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_MONITOR_H
#define PCE_LIB_MONITOR_H 1


#include <lib/cmd.h>


typedef struct {
	void *cmdext;
	int  (*docmd) (void *ext, cmd_t *cmd);

	void *msgext;
	int  (*setmsg) (void *ext, const char *msg, const char *val);

	char       terminate;

	const char *prompt;
} monitor_t;


void mon_init (monitor_t *mon);
void mon_free (monitor_t *mon);

monitor_t *mon_new (void);
void mon_del (monitor_t *mon);

void mon_set_cmd_fct (monitor_t *mon, void *fct, void *ext);
void mon_set_msg_fct (monitor_t *mon, void *fct, void *ext);

void mon_set_terminate (monitor_t *mon, int val);

void mon_set_prompt (monitor_t *mon, const char *str);

int mon_run (monitor_t *mon);


#endif
