/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/sysdep.c                                             *
 * Created:     2006-06-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2010 Hampa Hug <hampa@hampa.ch>                     *
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include "sysdep.h"


int pce_usleep (unsigned long usec)
{
#if defined(HAVE_NANOSLEEP)
	struct timespec t;

	t.tv_sec = usec / 1000000;
	t.tv_nsec = 1000 * (usec % 1000000);

	return (nanosleep (&t, NULL));
#elif defined(HAVE_USLEEP)
	unsigned long n;

	while (usec > 0) {
		n = (usec < 500000) ? usec : 500000;
		usleep (n);
		usec -= n;
	}

	return (0);
#else
	return (-1);
#endif
}

unsigned long pce_get_interval_us (unsigned long *val)
{
#ifdef HAVE_GETTIMEOFDAY
	unsigned long  clk0, clk1;
	struct timeval tv;

	if (gettimeofday (&tv, NULL)) {
		return (0);
	}

	clk1 = (1000000UL * (unsigned long) tv.tv_sec + tv.tv_usec) & 0xffffffff;
	clk0 = (clk1 - *val) & 0xffffffff;

	*val = clk1;

	return (clk0);
#else
	return (0);
#endif
}

int pce_fd_readable (int fd, int t)
{
#ifdef HAVE_SYS_POLL_H
	int           r;
	struct pollfd pfd[1];

	pfd[0].fd = fd;
	pfd[0].events = POLLIN;

	r = poll (pfd, 1, t);
	if (r < 0) {
		return (0);
	}

	if ((pfd[0].revents & POLLIN) == 0) {
		return (0);
	}

	return (1);
#else
	return (0);
#endif
}

int pce_fd_writeable (int fd, int t)
{
#ifdef HAVE_SYS_POLL_H
	int           r;
	struct pollfd pfd[1];

	pfd[0].fd = fd;
	pfd[0].events = POLLOUT;

	r = poll (pfd, 1, t);
	if (r < 0) {
		return (0);
	}

	if ((pfd[0].revents & POLLOUT) == 0) {
		return (0);
	}

	return (1);
#else
	return (0);
#endif
}

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

void pce_start (unsigned *brk)
{
	if (brk != NULL) {
		*brk = 0;
	}

	pce_set_fd_interactive (0, 0);
}

void pce_stop (void)
{
	pce_set_fd_interactive (0, 1);
}
