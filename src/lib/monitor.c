/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/monitor.c                                          *
 * Created:       2006-12-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2006 Hampa Hug <hampa@hampa.ch>                        *
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "monitor.h"
#include "cmd.h"


void mon_init (monitor_t *mon)
{
	mon->cmdext = NULL;
	mon->docmd = NULL;

	mon->msgext = NULL;
	mon->setmsg = NULL;
	mon->getmsg = NULL;

	mon->terminate = 0;
}

void mon_free (monitor_t *mon)
{
}

monitor_t *mon_new (void)
{
	monitor_t *mon;

	mon = malloc (sizeof (monitor_t));
	if (mon == NULL) {
		return (NULL);
	}

	mon_init (mon);

	return (mon);
}

void mon_del (monitor_t *mon)
{
	if (mon != NULL) {
		mon_free (mon);
		free (mon);
	}
}

void mon_set_cmd_fct (monitor_t *mon, void *fct, void *ext)
{
	mon->cmdext = ext;
	mon->docmd = fct;
}

void mon_set_msg_fct (monitor_t *mon, void *set, void *get, void *ext)
{
	mon->msgext = ext;
	mon->setmsg = set;
	mon->getmsg = get;
}

void mon_set_terminate (monitor_t *mon, int val)
{
	mon->terminate = (val != 0);
}

#if 0
static
int mon_set_msg (monitor_t *mon, const char *msg, const char *val)
{
	if (mon->setmsg != NULL) {
		return (mon->setmsg (mon->msgext, msg, val));
	}

	return (1);
}

static
int mon_get_msg (monitor_t *mon, const char *msg, char *val, unsigned max)
{
	if (mon->getmsg != NULL) {
		return (mon->getmsg (mon->msgext, msg, val, max));
	}

	return (1);
}
#endif

static
void mon_prt_prompt (monitor_t *mon)
{
	/* reset terminal colors */
	fputs ("\x1b[0;37;40m", stdout);

	fputs ("-", stdout);
	fflush (stdout);
}

static
void mon_do_ms (monitor_t *mon, cmd_t *cmd)
{
	char msg[256];
	char val[256];

	if (!cmd_match_str (cmd, msg, 256)) {
		strcpy (msg, "");
	}

	if (!cmd_match_str (cmd, val, 256)) {
		strcpy (val, "");
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (mon->setmsg != NULL) {
		if (mon->setmsg (mon->msgext, msg, val)) {
			printf ("error\n");
		}
	}
	else {
		printf ("monitor: no set message function\n");
	}
}

static
void mon_do_mg (monitor_t *mon, cmd_t *cmd)
{
	char msg[256];
	char val[256];

	if (!cmd_match_str (cmd, msg, 256)) {
		strcpy (msg, "");
	}

	if (!cmd_match_str (cmd, val, 256)) {
		strcpy (val, "");
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (mon->getmsg != NULL) {
		if (mon->getmsg (mon->msgext, msg, val, 256)) {
			printf ("error\n");
		}

		printf ("%s\n", val);
	}
	else {
		printf ("monitor: no get message function\n");
	}
}

static
void mon_do_m (monitor_t *mon, cmd_t *cmd)
{
	if (cmd_match (cmd, "s")) {
		mon_do_ms (mon, cmd);
	}
	else if (cmd_match (cmd, "g")) {
		mon_do_mg (mon, cmd);
	}
	else {
		mon_do_ms (mon, cmd);
	}
}

static
void mon_do_v (cmd_t *cmd)
{
	unsigned long val;

	while (cmd_match_uint32 (cmd, &val)) {
		printf ("%lX\n", val);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}


int mon_run (monitor_t *mon)
{
	int   r;
	cmd_t cmd;

	while (mon->terminate == 0) {
		mon_prt_prompt (mon);

		cmd_get (&cmd);

		if (cmd_match_eol (&cmd)) {
			r = 1;
		}
		else if (mon->docmd != NULL) {
			r = mon->docmd (mon->cmdext, &cmd);
		}
		else {
			r = 1;
		}

		if (r != 0) {
			if (cmd_match (&cmd, "m")) {
				mon_do_m (mon, &cmd);
			}
			else if (cmd_match (&cmd, "q")) {
				break;
			}
			else if (cmd_match (&cmd, "v")) {
				mon_do_v (&cmd);
			}
		}
	};

	return (0);
}
