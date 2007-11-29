/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/console.c                                          *
 * Created:       2006-06-19 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2006-2007 Hampa Hug <hampa@hampa.ch>                   *
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


#include <config.h>

#include "console.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


static FILE *pce_fp_inp = NULL;
static FILE *pce_fp_out = NULL;
static FILE *pce_fp_redir = NULL;


FILE *pce_get_redirection (void)
{
	return (pce_fp_redir);
}

FILE *pce_get_fp_out (void)
{
	return (pce_fp_out);
}

FILE *pce_get_fp_inp (void)
{
	return (pce_fp_inp);
}

int pce_set_redirection (const char *fname, const char *mode)
{
	if (pce_fp_redir != NULL) {
		fclose (pce_fp_redir);
		pce_fp_redir = NULL;
	}

	if (fname == NULL) {
		return (0);
	}

	pce_fp_redir = fopen (fname, mode);

	if (pce_fp_redir == NULL) {
		return (1);
	}

	return (0);
}

void pce_gets (char *str, unsigned max)
{
	str[0] = 0;

	if (pce_fp_inp == NULL) {
		pce_fp_inp = stdin;
	}

	fgets (str, max, pce_fp_inp);

	if (pce_fp_redir != NULL) {
		fputs (str, pce_fp_redir);
	}
}

void pce_puts (const char *str)
{
	if (pce_fp_out == NULL) {
		pce_fp_out = stdout;
	}

	fputs (str, pce_fp_out);
	fflush (pce_fp_out);

	if (pce_fp_redir != NULL) {
		fputs (str, pce_fp_redir);
		fflush (pce_fp_redir);
	}
}

void pce_printf (const char *msg, ...)
{
	va_list va;

	if (pce_fp_out == NULL) {
		pce_fp_out = stdout;
	}

	va_start (va, msg);

	vfprintf (pce_fp_out, msg, va);
	fflush (pce_fp_out);

	if (pce_fp_redir != NULL) {
		vfprintf (pce_fp_redir, msg, va);
		fflush (pce_fp_redir);
	}

	va_end (va);
}

void pce_vprintf (const char *msg, va_list va)
{
	if (pce_fp_out == NULL) {
		pce_fp_out = stdout;
	}

	vfprintf (pce_fp_out, msg, va);
	fflush (pce_fp_out);

	if (pce_fp_redir != NULL) {
		vfprintf (pce_fp_redir, msg, va);
		fflush (pce_fp_redir);
	}
}

void pce_prt_sep (const char *str)
{
	unsigned i, n;

	n = strlen (str);

	pce_puts ("-");
	pce_puts (str);

	for (i = n + 1; i < 78; i++) {
		pce_puts ("-");
	}

	pce_puts ("\n");
}

void pce_console_init (FILE *inp, FILE *out)
{
	pce_fp_inp = inp;
	pce_fp_out = out;
}

void pce_console_done (void)
{
	pce_set_redirection (NULL, NULL);
}
