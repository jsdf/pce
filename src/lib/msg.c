/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/msg.c                                                *
 * Created:     2005-12-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2009 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <lib/msg.h>
#include <lib/string.h>


int msg_is_message (const char *msg, const char *val)
{
	while (*msg != 0) {
		if (strcmp (msg, val) == 0) {
			return (1);
		}

		while ((*msg != 0) && (*msg != '.')) {
			msg += 1;
		}

		if (*msg == '.') {
			msg += 1;
		}
	}

	return (0);
}

int msg_is_prefix (const char *pre, const char *val)
{
	while ((*pre != 0) && (*pre == *val)) {
		pre += 1;
		val += 1;
	}

	if ((*pre != 0) || (*val != '.')) {
		return (0);
	}

	return (1);
}

int msg_get_ulng (const char *str, unsigned long *val)
{
	unsigned long tmp;
	char          *end;

	tmp = strtoul (str, &end, 0);

	if ((end == str) || (*end != 0)) {
		return (1);
	}

	*val = tmp;

	return (0);
}

int msg_get_slng (const char *str, long *val)
{
	unsigned long tmp;
	char          *end;

	tmp = strtol (str, &end, 0);

	if ((end == str) || (*end != 0)) {
		return (1);
	}

	*val = tmp;

	return (0);
}

int msg_get_uint (const char *str, unsigned *val)
{
	unsigned long tmp;

	if (msg_get_ulng (str, &tmp)) {
		return (1);
	}

	*val = tmp;

	if (*val != tmp) {
		return (1);
	}

	return (0);
}

int msg_get_sint (const char *str, int *val)
{
	long tmp;

	if (msg_get_slng (str, &tmp)) {
		return (1);
	}

	*val = tmp;

	if (*val != tmp) {
		return (1);
	}

	return (0);
}

int msg_get_bool (const char *str, int *val)
{
	unsigned long tmp;

	if ((str == NULL) || (*str == 0)) {
		*val = 0;
		return (0);
	}

	if (strcmp (str, "true") == 0) {
		*val = 1;
		return (0);
	}
	else if (strcmp (str, "false") == 0) {
		*val = 0;
		return (0);
	}

	if (msg_get_ulng (str, &tmp)) {
		return (1);
	}

	*val = (tmp != 0);

	return (0);
}

int msg_get_prefix_ulng (const char **str, unsigned long *val, const char *sep, const char *trim)
{
	int        r;
	char       *pre1, *pre2;
	const char *tmp;

	tmp = *str;

	pre1 = str_extract_alloc (tmp, sep, &tmp);
	pre2 = str_trim (pre1, " \t", " \t");

	r = msg_get_ulng (pre2, val);

	free (pre1);

	if (r) {
		return (1);
	}

	*str = str_ltrim (tmp, trim);

	return (0);
}

int msg_get_prefix_uint (const char **str, unsigned *val, const char *sep, const char *trim)
{
	unsigned long tmp;

	if (msg_get_prefix_ulng (str, &tmp, sep, trim)) {
		return (1);
	}

	*val = tmp;

	return (0);
}

int msg_get_prefix_slng (const char **str, long *val, const char *sep, const char *trim)
{
	int        r;
	char       *pre1, *pre2;
	const char *tmp;

	tmp = *str;

	pre1 = str_extract_alloc (tmp, sep, &tmp);
	pre2 = str_trim (pre1, " \t", " \t");

	r = msg_get_slng (pre2, val);

	free (pre1);

	if (r) {
		return (1);
	}

	*str = str_ltrim (tmp, trim);

	return (0);
}

int msg_get_prefix_sint (const char **str, int *val, const char *sep, const char *trim)
{
	long tmp;

	if (msg_get_prefix_slng (str, &tmp, sep, trim)) {
		return (1);
	}

	*val = tmp;

	return (0);
}

int msg_get_prefix_bool (const char **str, int *val, const char *sep, const char *trim)
{
	int        r;
	char       *pre1, *pre2;
	const char *tmp;

	tmp = *str;

	pre1 = str_extract_alloc (tmp, sep, &tmp);
	pre2 = str_trim (pre1, " \t", " \t");

	r = msg_get_bool (pre2, val);

	free (pre1);

	if (r) {
		return (1);
	}

	*str = str_ltrim (tmp, trim);

	return (0);
}
