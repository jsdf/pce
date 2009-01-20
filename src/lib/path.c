/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/path.c                                               *
 * Created:     2008-11-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2009 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libini/libini.h>

#include <lib/path.h>


static unsigned par_path_cnt = 0;
static char     **par_path_dir = NULL;


static
char *pce_path_strdup (const char *str)
{
	unsigned n;
	char     *ret;

	n = strlen (str);

	ret = malloc (n + 1);
	if (ret == NULL) {
		return (NULL);
	}

	memcpy (ret, str, n);
	ret[n] = 0;

	return (ret);
}

static
char *pce_path_cat (const char *str1, const char *str2)
{
	unsigned n1, n2;
	char     *ret;

	if (str1 == NULL) {
		return (pce_path_strdup (str2));
	}

	n1 = strlen (str1);
	n2 = strlen (str2);

	ret = malloc (n1 + n2 + 2);
	if (ret == NULL) {
		return (NULL);
	}

	memcpy (ret, str1, n1);
	ret[n1] = PCE_DIR_SEP;
	memcpy (ret + n1 + 1, str2, n2);

	ret[n1 + n2 + 1] = 0;

	return (ret);
}

static
int pce_path_is_absolute (const char *str)
{
	if (*str == PCE_DIR_SEP) {
		return (1);
	}

	return (0);
}

void pce_path_clear (void)
{
	unsigned i;

	for (i = 0; i < par_path_cnt; i++) {
		free (par_path_dir[i]);
	}

	free (par_path_dir);

	par_path_cnt = 0;
	par_path_dir = NULL;
}

int pce_path_add (const char *dir, int atend)
{
	unsigned i, n;
	char     **tmp;
	char     *str;

	n = strlen (dir);

	while ((n > 0) && (dir[n - 1] == PCE_DIR_SEP)) {
		n -= 1;
	}

	if ((n == 0) || ((n == 1) && (dir[0] == '.'))) {
		str = NULL;
	}
	else {
		str = malloc (n + 1);
		if (str == NULL) {
			return (1);
		}

		memcpy (str, dir, n);
		str[n] = 0;
	}

	tmp = realloc (par_path_dir, (par_path_cnt + 1) * sizeof (char *));
	if (tmp == NULL) {
		free (str);
		return (1);
	}

	if (atend) {
		tmp[par_path_cnt] = str;
	}
	else {
		for (i = par_path_cnt; i > 0; i--) {
			tmp[i] = tmp[i - 1];
		}

		tmp[0] = str;
	}

	par_path_cnt += 1;
	par_path_dir = tmp;

	return (0);
}

int pce_path_set (const char *dir)
{
	int atend;

	atend = 1;

	if (*dir == '+') {
		dir += 1;
	}
	else if (*dir == '-') {
		dir += 1;
		atend = 0;
	}
	else if (*dir == '=') {
		dir += 1;
		pce_path_clear();
	}

	return (pce_path_add (dir, atend));
}

char *pce_path_get (const char *fname)
{
	unsigned i;
	char     *str, *tmp;
	FILE     *fp;

	if (fname == NULL) {
		return (NULL);
	}

	str = pce_path_strdup (fname);
	if (str == NULL) {
		return (NULL);
	}

	if (pce_path_is_absolute (str)) {
		return (str);
	}

	for (i = 0; i < par_path_cnt; i++) {
		tmp = pce_path_cat (par_path_dir[i], str);
		if (tmp == NULL) {
			free (str);
			return (NULL);
		}

		fp = fopen (tmp, "r");

		if (fp != NULL) {
			fclose (fp);
			free (str);
			return (tmp);
		}

		free (tmp);
	}

	return (str);
}

FILE *pce_fopen_inp (const char *fname, const char *mode, char **path)
{
	unsigned i;
	char     *str;
	FILE     *fp;

	if (pce_path_is_absolute (fname)) {
		if (path != NULL) {
			*path = pce_path_strdup (fname);
		}

		return (fopen (fname, mode));
	}

	for (i = 0; i < par_path_cnt; i++) {
		str = pce_path_cat (par_path_dir[i], fname);
		if (str == NULL) {
			return (NULL);
		}

		fp = fopen (str, mode);

		if (fp != NULL) {
			if (path != NULL) {
				*path = str;
			}
			else {
				free (str);
			}

			return (fp);
		}

		free (str);
	}

	if (path != NULL) {
		*path = pce_path_strdup (fname);
	}

	return (fopen (fname, mode));
}

FILE *pce_fopen_out (const char *fname, const char *mode)
{
	return (fopen (fname, mode));
}

int pce_path_ini (ini_sct_t *sct)
{
	const char *str;
	ini_val_t  *val;

	val = NULL;
	while (1) {
		val = ini_next_val (sct, val, "path");

		if (val == NULL) {
			break;
		}

		str = ini_val_get_str (val);
		if (str == NULL) {
			return (1);
		}

		if (pce_path_set (str)) {
			return (1);
		}
	}

	return (0);
}
