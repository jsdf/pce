/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/console.c                                            *
 * Created:     2006-06-19 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include "console.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PCE_ENABLE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif


static FILE *pce_fp_inp = NULL;
static FILE *pce_fp_out = NULL;

static FILE *pce_redir_inp = NULL;
static FILE *pce_redir_out = NULL;


FILE *pce_get_redir_inp (void)
{
	return (pce_redir_inp);
}

FILE *pce_get_redir_out (void)
{
	return (pce_redir_out);
}

FILE *pce_get_fp_inp (void)
{
	return (pce_fp_inp);
}

FILE *pce_get_fp_out (void)
{
	return (pce_fp_out);
}

int pce_set_redir_inp (const char *fname)
{
	if (pce_redir_inp != NULL) {
		fclose (pce_redir_inp);
		pce_redir_inp = NULL;
	}

	if (fname == NULL) {
		return (0);
	}

	pce_redir_inp = fopen (fname, "r");

	if (pce_redir_inp == NULL) {
		return (1);
	}

	return (0);
}

int pce_set_redir_out (const char *fname, const char *mode)
{
	if (pce_redir_out != NULL) {
		fclose (pce_redir_out);
		pce_redir_out = NULL;
	}

	if (fname == NULL) {
		return (0);
	}

	pce_redir_out = fopen (fname, mode);

	if (pce_redir_out == NULL) {
		return (1);
	}

	return (0);
}

int pce_gets (const char *prompt, char *str, unsigned max)
{
	str[0] = 0;

	if (pce_redir_inp != NULL) {
		if (fgets (str, max, pce_redir_inp) == NULL) {
			str[0] = 0;
		}

		if (str[0] != 0) {
			pce_puts (str);
			return (0);
		}

		fclose (pce_redir_inp);
		pce_redir_inp = NULL;
	}

#ifdef PCE_ENABLE_READLINE
	{
		char *tmp;

		tmp = readline (prompt);

		if (tmp == NULL) {
			return (1);
		}
		else if (strlen (tmp) < max) {
			strcpy (str, tmp);
		}

		if (*str != 0) {
			add_history (str);
		}

		free (tmp);
	}
#else
	if (pce_fp_inp == NULL) {
		pce_fp_inp = stdin;
	}

	if (prompt != NULL) {
		pce_puts (prompt);
	}

	if (fgets (str, max, pce_fp_inp) == NULL) {
		str[0] = 0;
	}
#endif

	if (pce_redir_out != NULL) {
		fputs (str, pce_redir_out);
	}

	return (0);
}

void pce_puts (const char *str)
{
	if (pce_fp_out == NULL) {
		pce_fp_out = stdout;
	}

	fputs (str, pce_fp_out);
	fflush (pce_fp_out);

	if (pce_redir_out != NULL) {
		fputs (str, pce_redir_out);
		fflush (pce_redir_out);
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

	if (pce_redir_out != NULL) {
		vfprintf (pce_redir_out, msg, va);
		fflush (pce_redir_out);
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

	if (pce_redir_out != NULL) {
		vfprintf (pce_redir_out, msg, va);
		fflush (pce_redir_out);
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

#ifdef PCE_ENABLE_READLINE
	using_history();
#endif
}

void pce_console_done (void)
{
	pce_set_redir_out (NULL, NULL);
	pce_set_redir_inp (NULL);
}
