/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   pcever.c                                                     *
 * Created:     2018-12-22 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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

#include "config.h"
#include "pce.h"


const char *arg0 = NULL;


int main (int argc, char **argv)
{
	unsigned ver[3];
	char     str[256];

	arg0 = argv[0];

	if (pce_check()) {
		fprintf (stderr, "%s: not running under pce\n", arg0);
		return (1);
	}

	if (pce_get_version (ver)) {
		fprintf (stderr, "%s: can't get version\n", arg0);
		return (1);
	}

	if (pce_get_version_str (str, sizeof (str))) {
		fprintf (stderr, "%s: can't get version string\n", arg0);
		return (1);
	}

	printf ("%s\n", str);

	return (0);
}
