/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/console.c                                          *
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

#include "console.h"


#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif


void pce_set_fd_interactive (int fd, int interactive)
{
#ifdef HAVE_TERMIOS_H
	static int            sios_ok = 0;
	static struct termios sios;
	struct termios        tios;

	if (sios_ok == 0) {
		tcgetattr (fd, &sios);
		sios_ok = 1;
	}

	if (interactive) {
		tcsetattr (fd, TCSANOW, &sios);
	}
	else {
		tios = sios;

		tios.c_lflag &= ~(ICANON | ECHO);
		tios.c_cc[VMIN] = 1;
		tios.c_cc[VTIME] = 0;

		tcsetattr (fd, TCSANOW, &tios);
	}
#endif
}
