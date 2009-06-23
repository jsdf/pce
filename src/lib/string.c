/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/string.c                                             *
 * Created:     2009-06-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009 Hampa Hug <hampa@hampa.ch>                          *
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
#include <time.h>

#include <lib/string.h>


char *str_cat_alloc (const char *str1, const char *str2)
{
	char          *ret;
	unsigned long n1, n2;

	n1 = (str1 == NULL) ? 0 : strlen (str1);
	n2 = (str2 == NULL) ? 0 : strlen (str2);

	ret = malloc (n1 + n2 + 1);

	if (ret == NULL) {
		return (NULL);
	}

	if (n1 > 0) {
		memcpy (ret, str1, n1);
	}

	if (n2 > 0) {
		memcpy (ret + n1, str2, n2);
	}

	ret[n1 + n2] = 0;

	return (ret);
}

char *str_cut_alloc (const char *str, unsigned long cnt)
{
	unsigned long i;
	char          *ret;

	ret = malloc (cnt + 1);

	if (ret == NULL) {
		return (NULL);
	}

	i = 0;

	while ((i < cnt) && (str[i] != 0)) {
		ret[i] = str[i];
		i += 1;
	}

	ret[i] = 0;

	return (ret);
}

char *str_copy_alloc (const char *str)
{
	char          *ret;
	unsigned long n;

	n = (str == NULL) ? 0 : strlen (str);

	ret = malloc (n + 1);

	if (ret == NULL) {
		return (NULL);
	}

	if (n > 0) {
		memcpy (ret, str, n);
	}

	ret[n] = 0;

	return (ret);
}

char *str_trim (char *str, const char *left, const char *right)
{
	unsigned long i;

	if (str == NULL) {
		return (NULL);
	}

	if (left != NULL) {
		while (*str != 0) {
			if (strchr (left, *str) == NULL) {
				break;
			}

			str += 1;
		}
	}

	if (right != NULL) {
		i = strlen (str);

		while (i > 0) {
			if (strchr (right, str[i - 1]) == NULL) {
				break;
			}

			i -= 1;
		}

		str[i] = 0;
	}

	return (str);
}

const char *str_ltrim (const char *str, const char *left)
{
	if (str == NULL) {
		return (NULL);
	}

	if (left == NULL) {
		return (str);
	}

	while ((*str != 0) && (strchr (left, *str) != NULL)) {
		str += 1;
	}

	return (str);
}

char *str_extract_alloc (const char *str, const char *sep, const char **rest)
{
	char          *ret;
	unsigned long i;

	if (str == NULL) {
		return (NULL);
	}

	i = 0;
	while ((str[i] != 0) && (strchr (sep, str[i]) == NULL)) {
		i += 1;
	}

	ret = str_cut_alloc (str, i);

	if (*rest != NULL) {
		if (str[i] != 0) {
			i += 1;
		}

		*rest = str + i;
	}

	return (ret);
}
