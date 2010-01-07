/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/monitor.c                                            *
 * Created:     2006-12-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "monitor.h"
#include "cmd.h"
#include "console.h"


void mon_init (monitor_t *mon)
{
	mon->cmdext = NULL;
	mon->docmd = NULL;

	mon->msgext = NULL;
	mon->setmsg = NULL;

	mon->terminate = 0;
	mon->prompt = NULL;
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

void mon_set_msg_fct (monitor_t *mon, void *fct, void *ext)
{
	mon->msgext = ext;
	mon->setmsg = fct;
}

void mon_set_terminate (monitor_t *mon, int val)
{
	mon->terminate = (val != 0);
}

void mon_set_prompt (monitor_t *mon, const char *str)
{
	mon->prompt = str;
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
void mon_cmd_m (monitor_t *mon, cmd_t *cmd)
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
			pce_puts ("error\n");
		}
	}
	else {
		pce_puts ("monitor: no set message function\n");
	}
}

static
void mon_cmd_redir_inp (monitor_t *mon, cmd_t *cmd)
{
	int  close;
	char fname[256];

	close = 0;

	if (!cmd_match_str (cmd, fname, 256)) {
		close = 1;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (close) {
		pce_set_redir_inp (NULL);
	}
	else {
		if (pce_set_redir_inp (fname)) {
			pce_puts ("error setting redirection\n");
		}
		else {
			pce_printf ("redirecting from \"%s\"\n", fname);
		}
	}
}

static
void mon_cmd_redir_out (monitor_t *mon, cmd_t *cmd)
{
	int        close;
	const char *mode;
	char       fname[256];

	close = 0;
	mode = "w";

	if (cmd_match (cmd, ">")) {
		mode = "a";
	}

	if (!cmd_match_str (cmd, fname, 256)) {
		close = 1;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (close) {
		pce_set_redir_out (NULL, NULL);
	}
	else {
		if (pce_set_redir_out (fname, mode)) {
			pce_puts ("error setting redirection\n");
		}
		else {
			pce_printf ("redirecting to \"%s\"\n", fname);
		}
	}
}

static
void mon_cmd_v (cmd_t *cmd)
{
	unsigned long val;

	if (cmd_match_eol (cmd)) {
		cmd_list_syms (cmd);
		return;
	}

	while (cmd_match_uint32 (cmd, &val)) {
		pce_printf ("%lX\n", val);
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
		if (mon->setmsg != NULL) {
			mon->setmsg (mon->msgext, "term.release", "1");
			mon->setmsg (mon->msgext, "term.fullscreen", "0");
		}

		cmd_get (&cmd, mon->prompt);

		if (mon->docmd != NULL) {
			r = mon->docmd (mon->cmdext, &cmd);
		}
		else {
			r = 1;
		}

		if (r != 0) {
			if (cmd_match (&cmd, "m")) {
				mon_cmd_m (mon, &cmd);
			}
			else if (cmd_match (&cmd, "q")) {
				break;
			}
			else if (cmd_match (&cmd, "v")) {
				mon_cmd_v (&cmd);
			}
			else if (cmd_match (&cmd, "<")) {
				mon_cmd_redir_inp (mon, &cmd);
			}
			else if (cmd_match (&cmd, ">")) {
				mon_cmd_redir_out (mon, &cmd);
			}
			else if (!cmd_match_eol (&cmd)) {
				cmd_error (&cmd, "unknown command");
			}
		}
	};

	return (0);
}
