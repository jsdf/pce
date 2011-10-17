/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/options.c                                        *
 * Created:     2009-10-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <drivers/options.h>


static
const char *drv_skip_option (const char *str)
{
	while (*str != 0) {
		if (*str == ':') {
			if (*(str + 1) == ':') {
				str += 2;
			}
			else {
				return (str + 1);
			}
		}
		else {
			str += 1;
		}
	}

	return (NULL);
}

static
const char *drv_skip_space (const char *str)
{
	while (1) {
		if (*str == ' ') {
			str += 1;
		}
		else if (*str == '\t') {
			str += 1;
		}
		else {
			return (str);
		}
	}
}

static
const char *drv_get_option_value (const char *str, const char *name)
{
	str = drv_skip_space (str);

	while (*name != 0) {
		if (*str != *name) {
			return (NULL);
		}

		str += 1;
		name += 1;
	}

	str = drv_skip_space (str);

	if (*str == '=') {
		str = drv_skip_space (str + 1);

		return (str);
	}

	return (NULL);
}

static
char *drv_option_dup (const char *str)
{
	unsigned i, j, n;
	char     *ret;

	n = 0;
	while (str[n] != 0) {
		if (str[n] == ':') {
			if (str[n + 1] == ':') {
				n += 2;
			}
			else {
				break;
			}
		}
		else {
			n += 1;
		}
	}

	while (n > 0) {
		if (str[n - 1] == ' ') {
			n -= 1;
		}
		else if (str[n - 1] == '\t') {
			n -= 1;
		}
		else {
			break;
		}
	}

	ret = malloc (n + 1);
	if (ret == NULL) {
		return (NULL);
	}

	i = 0;
	j = 0;

	while (i < n) {
		if ((str[i] == ':') && (str[i + 1] == ':')) {
			ret[j] = ':';
			i += 2;
		}
		else {
			ret[j] = str[i];
			i += 1;
		}

		j += 1;
	}

	ret[j] = 0;

	return (ret);
}

char *drv_get_option (const char *str, const char *name)
{
	const char *val;

	while (str != NULL) {
		val = drv_get_option_value (str, name);

		if (val != NULL) {
			return (drv_option_dup (val));
		}

		str = drv_skip_option (str);
	}

	return (NULL);
}

int drv_get_option_bool (const char *str, const char *name, int def)
{
	int  r;
	char *s;

	s = drv_get_option (str, name);

	if (s == NULL) {
		return (def);
	}

	r = def;

	if (strcmp (s, "1") == 0) {
		r = 1;
	}
	else if (strcmp (s, "true") == 0) {
		r = 1;
	}
	else if (strcmp (s, "yes") == 0) {
		r = 1;
	}
	else if (strcmp (s, "0") == 0) {
		r = 0;
	}
	else if (strcmp (s, "false") == 0) {
		r = 0;
	}
	else if (strcmp (s, "no") == 0) {
		r = 0;
	}

	free (s);

	return (r);
}

unsigned long drv_get_option_uint (const char *str, const char *name, unsigned long def)
{
	unsigned long val;
	char          *end;
	char          *s;

	s = drv_get_option (str, name);

	if (s == NULL) {
		return (def);
	}

	val = strtoul (s, &end, 0);

	if ((end != NULL) && (*end != 0)) {
		free (s);
		return (def);
	}

	free (s);

	return (val);
}

long drv_get_option_sint (const char *str, const char *name, long def)
{
	unsigned long val;
	char          *end;
	char          *s;

	s = drv_get_option (str, name);

	if (s == NULL) {
		return (def);
	}

	val = strtol (s, &end, 0);

	if ((end != NULL) && (*end != 0)) {
		free (s);
		return (def);
	}

	free (s);

	return (val);
}
