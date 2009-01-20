/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sims32/sercons.c                                    *
 * Created:     2004-09-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "main.h"


void scon_check (sims32_t *sim)
{
	unsigned      i, n;
	unsigned char buf[8];
	ssize_t       r;

	if (par_sig_int) {
		par_sig_int = 0;
		ss32_set_keycode (sim, 0x03);
	}

	if (!pce_fd_readable (0, 0)) {
		return;
	}

	r = read (0, buf, 8);
	if (r <= 0) {
		return;
	}

	n = (unsigned) r;

	if ((n == 1) && (buf[0] == 0)) {
		ss32_set_msg (sim, "break", "stop");
		return;
	}
	if ((n == 1) && (buf[0] == 0x1b)) {
		ss32_set_msg (sim, "break", "stop");
		return;
	}
	else if ((n == 1) && (buf[0] == 0xe0)) {
		ss32_set_msg (sim, "break", "abort");
		return;
	}

	for (i = 0; i < n; i++) {
		ss32_set_keycode (sim, buf[i]);
	}
}
