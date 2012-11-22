/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-pty.c                                  *
 * Created:     2009-03-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2012 Hampa Hug <hampa@hampa.ch>                     *
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
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-pty.h>


static
void chr_pty_close (char_drv_t *cdrv)
{
	char_pty_t *drv;

	drv = cdrv->ext;

	if (drv->symlink != NULL) {
		unlink (drv->symlink);
		free (drv->symlink);
	}

	if (drv->ptsname != NULL) {
		free (drv->ptsname);
	}

	if (drv->fd >= 0) {
		close (drv->fd);
	}

	free (drv);
}

static
unsigned chr_pty_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	char_pty_t *drv;
	ssize_t    r;

	drv = cdrv->ext;

	if (drv->fd < 0) {
		return (0);
	}

#if UINT_MAX > SSIZE_MAX
	if (cnt > SSIZE_MAX) {
		cnt = SSIZE_MAX;
	}
#endif

	r = read (drv->fd, buf, cnt);

	if (r <= 0) {
		return (0);
	}

	return (r);
}

static
unsigned chr_pty_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_pty_t *drv;
	ssize_t    r;

	drv = cdrv->ext;

	if (drv->fd < 0) {
		return (cnt);
	}

#if UINT_MAX > SSIZE_MAX
	if (cnt > SSIZE_MAX) {
		cnt = SSIZE_MAX;
	}
#endif

	r = write (drv->fd, buf, cnt);

	if (r <= 0) {
		return (0);
	}

	return (r);
}

static
int chr_pty_init_pt (char_pty_t *drv, int fd)
{
	const char *pts;

	if (grantpt (fd)) {
		return (1);
	}

	if (unlockpt (fd)) {
		return (1);
	}

	pts = ptsname (fd);

	if (pts == NULL) {
		return (1);
	}

	if (strlen (pts) > 255) {
		return (1);
	}

	drv->ptsname = strdup (pts);

	fprintf (stderr, "char-pty: %s\n", drv->ptsname);

	return (0);
}

static
int chr_pty_set_nonblock (int fd)
{
	long fl;

	fl = fcntl (fd, F_GETFD);

	if (fl == -1) {
		return (1);
	}

	if (fcntl (fd, F_SETFL, fl | O_NONBLOCK) == -1) {
		return (1);
	}

	return (0);
}

static
int chr_pty_disable_echo (int fd)
{
	struct termios tio;

	if (tcgetattr (fd, &tio)) {
		return (1);
	}

	tio.c_iflag &= ~(IGNCR | ICRNL | INLCR);
	tio.c_lflag &= ~(ECHO | ECHOE);

	if (tcsetattr (fd, TCSANOW, &tio)) {
		return (1);
	}

	tcflush (fd, TCIOFLUSH);

	return (0);
}

static
int chr_pty_init (char_pty_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_pty_close;
	drv->cdrv.read = chr_pty_read;
	drv->cdrv.write = chr_pty_write;

	drv->ptsname = NULL;

	drv->symlink = drv_get_option (name, "symlink");

	drv->fd = posix_openpt (O_RDWR | O_NOCTTY);

	if (drv->fd < 0) {
		return (1);
	}

	if (chr_pty_init_pt (drv, drv->fd)) {
		return (1);
	}

	chr_pty_set_nonblock (drv->fd);
	chr_pty_disable_echo (drv->fd);

	if (drv->symlink != NULL) {
		if (symlink (drv->ptsname, drv->symlink)) {
			fprintf (stderr,
				"*** error creating symlink %s -> %s\n",
				drv->ptsname, drv->symlink
			);
		}
	}

	return (0);
}

char_drv_t *chr_pty_open (const char *name)
{
	char_pty_t *drv;

	drv = malloc (sizeof (char_pty_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_pty_init (drv, name)) {
		chr_pty_close (&drv->cdrv);

		return (NULL);
	}

	return (&drv->cdrv);
}
