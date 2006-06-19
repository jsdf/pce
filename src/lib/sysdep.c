/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/sysdep.c                                           *
 * Created:       2006-06-19 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-06-19 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2006 Hampa Hug <hampa@hampa.ch>                        *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "sysdep.h"


int pce_usleep (unsigned long usec)
{
#if defined(HAVE_NANOSLEEP)
	struct timespec t;

	t.tv_sec = usec / 1000000;
	t.tv_nsec = 1000 * (usec % 1000000);

	return (nanosleep (&t, NULL));
#elif defined(HAVE_USLEEP) && defined (HAVE_SLEEP)
	unsigned long sec;

	sec = usec / 1000000;
	usec = usec % 1000000;

	while (sec > 0) {
		sec = sleep (sec);
	}

	return (usleep (usec));
#else
	return (-1);
#endif
}
