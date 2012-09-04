/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/log.c                                                *
 * Created:     2003-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2012 Hampa Hug <hampa@hampa.ch>                     *
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

#include "log.h"


typedef struct {
	FILE     *fp;
	int      close;
	unsigned level;
} pce_log_t;


static unsigned  par_log_cnt = 0;
static pce_log_t par_log[PCE_LOG_MAX];


void pce_log_init (void)
{
	par_log_cnt = 0;
}

void pce_log_done (void)
{
	unsigned i;

	for (i = 0; i < par_log_cnt; i++) {
		if (par_log[i].close) {
			fclose (par_log[i].fp);
		}
	}

	par_log_cnt = 0;
}

int pce_log_add_fp (FILE *fp, int close, unsigned level)
{
	if (par_log_cnt >= PCE_LOG_MAX) {
		return (1);
	}

	par_log[par_log_cnt].fp = fp;
	par_log[par_log_cnt].close = (close != 0);
	par_log[par_log_cnt].level = level;

	par_log_cnt += 1;

	return (0);
}

int pce_log_add_fname (const char *fname, unsigned level)
{
	FILE *fp;

	fp = fopen (fname, "a");

	if (fp == NULL) {
		return (1);
	}

	if (pce_log_add_fp (fp, 1, level)) {
		fclose (fp);
		return (1);
	}

	return (0);
}

void pce_log_rmv_fp (FILE *fp)
{
	unsigned i, j;

	i = 0;
	while (i < par_log_cnt) {
		if (par_log[i].fp == fp) {
			if (par_log[i].close) {
				fclose (par_log[i].fp);
			}

			for (j = i + 1; j < par_log_cnt; j++) {
				par_log[j - 1] = par_log[j];
			}

			par_log_cnt -= 1;
		}
		else {
			i += 1;
		}
	}
}

void pce_log_set_level (FILE *fp, unsigned level)
{
	unsigned i;

	for (i = 0; i < par_log_cnt; i++) {
		if (par_log[i].fp == fp) {
			par_log[i].level = level;
			return;
		}
	}
}

unsigned pce_log_get_level (FILE *fp)
{
	unsigned i;

	for (i = 0; i < par_log_cnt; i++) {
		if (par_log[i].fp == fp) {
			return (par_log[i].level);
		}
	}

	return (MSG_DEB);
}

void pce_log (unsigned level, const char *msg, ...)
{
	unsigned i;
	va_list va;

	for (i = 0; i < par_log_cnt; i++) {
		if (level <= par_log[i].level) {
			va_start (va, msg);
			vfprintf (par_log[i].fp, msg, va);
			va_end (va);
			fflush (par_log[i].fp);
		}
	}
}

void pce_log_va (unsigned level, const char *msg, va_list va)
{
	unsigned i;

	for (i = 0; i < par_log_cnt; i++) {
		if (level <= par_log[i].level) {
			vfprintf (par_log[i].fp, msg, va);
			fflush (par_log[i].fp);
		}
	}
}

void pce_log_deb (const char *msg, ...)
{
	va_list va;

	va_start (va, msg);
	pce_log_va (MSG_DEB, msg, va);
	va_end (va);
}

void pce_log_tag (unsigned level, const char *tag, const char *msg, ...)
{
	unsigned i;
	va_list va;

	if (tag == NULL) {
		tag = "";
	}

	for (i = 0; i < par_log_cnt; i++) {
		if (level <= par_log[i].level) {
			va_start (va, msg);
			fprintf (par_log[i].fp, "%-9s ", tag);
			vfprintf (par_log[i].fp, msg, va);
			va_end (va);
			fflush (par_log[i].fp);
		}
	}
}
