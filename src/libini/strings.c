/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/strings.c                                         *
 * Created:     2010-09-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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

#include <libini/libini.h>


void ini_str_init (ini_strings_t *is)
{
	is->cnt = 0;
	is->max = 0;
	is->str = NULL;
}

void ini_str_free (ini_strings_t *is)
{
	free (is->str);
}

int ini_str_add (ini_strings_t *is, const char *s1, const char *s2, const char *s3)
{
	unsigned long n1, n2, n3;
	unsigned long cnt, max;
	char          *tmp;

	n1 = (s1 != NULL) ? strlen (s1) : 0;
	n2 = (s2 != NULL) ? strlen (s2) : 0;
	n3 = (s3 != NULL) ? strlen (s3) : 0;

	cnt = is->cnt + n1 + n2 + n3 + 1;

	if (cnt >= is->max) {
		max = (is->max == 0) ? 256 : is->max;

		while (cnt >= max) {
			max *= 2;
		}

		tmp = realloc (is->str, max);

		if (tmp == NULL) {
			return (1);
		}

		is->max = max;
		is->str = tmp;
	}

	if (s1 != NULL) {
		memcpy (is->str + is->cnt, s1, n1);
	}

	if (s2 != NULL) {
		memcpy (is->str + is->cnt + n1, s2, n2);
	}

	if (s3 != NULL) {
		memcpy (is->str + is->cnt + n1 + n2, s3, n3);
	}

	is->cnt += n1 + n2 + n3;

	is->str[is->cnt] = 0;

	return (0);
}

int ini_str_eval (ini_strings_t *is, ini_sct_t *sct, int free)
{
	int r;

	r = ini_read_str (sct, is->str);

	if (free) {
		ini_str_free (is);
	}

	return (r);
}
