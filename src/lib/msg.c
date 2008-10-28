/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/msg.c                                                *
 * Created:     2005-12-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2008 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include "msg.h"


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
