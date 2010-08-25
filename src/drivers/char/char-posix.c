/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-posix.c                                *
 * Created:     2009-03-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-posix.h>


static
int chr_posix_check_fd (int fd, int rd, int wr)
{
	int           r;
	struct pollfd pfd[1];

	if (fd < 0) {
		return (1);
	}

	pfd[0].fd = fd;
	pfd[0].events = (rd ? POLLIN : 0) | (wr ? POLLOUT : 0);

	r = poll (pfd, 1, 0);

	if (r < 0) {
		return (1);
	}

	if ((pfd[0].revents & (POLLIN | POLLOUT)) == 0) {
		return (1);
	}

	return (0);
}

static
void chr_posix_close (char_drv_t *cdrv)
{
	char_posix_t *drv;

	drv = cdrv->ext;

	if (drv->fd_write > 2) {
		close (drv->fd_write);
	}

	if ((drv->fd_read > 2) && (drv->fd_read != drv->fd_write)) {
		close (drv->fd_read);
	}

	if (drv->name_write != NULL) {
		free (drv->name_write);
	}

	if (drv->name_read != NULL) {
		free (drv->name_read);
	}

	if (drv->name != NULL) {
		free (drv->name);
	}

	free (drv);
}

static
unsigned chr_posix_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	char_posix_t *drv;
	ssize_t      r;

	drv = cdrv->ext;

	if (drv->fd_read < 0) {
		return (0);
	}

	if (chr_posix_check_fd (drv->fd_read, 1, 0)) {
		return (0);
	}

#if UINT_MAX > SSIZE_MAX
	if (cnt > SSIZE_MAX) {
		cnt = SSIZE_MAX;
	}
#endif

	r = read (drv->fd_read, buf, cnt);

	if (r <= 0) {
		return (0);
	}

	return (r);
}

static
unsigned chr_posix_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_posix_t *drv;
	ssize_t      r;

	drv = cdrv->ext;

	if (drv->fd_write < 0) {
		return (cnt);
	}

	if (chr_posix_check_fd (drv->fd_write, 0, 1)) {
		return (0);
	}

#if UINT_MAX > SSIZE_MAX
	if (cnt > SSIZE_MAX) {
		cnt = SSIZE_MAX;
	}
#endif

	r = write (drv->fd_write, buf, cnt);

	if (r <= 0) {
		return (0);
	}

	return (r);
}

static
int chr_posix_init (char_posix_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_posix_close;
	drv->cdrv.read = chr_posix_read;
	drv->cdrv.write = chr_posix_write;

	drv->name = NULL;
	drv->name_read = NULL;
	drv->name_write = NULL;

	drv->fd_read = -1;
	drv->fd_write = -1;

	drv->name = drv_get_option (name, "file");

	if (drv->name == NULL) {
		drv->name_read = drv_get_option (name, "read");
		drv->name_write = drv_get_option (name, "write");
	}

	if (drv->name != NULL) {
		if (strcmp (drv->name, "-") == 0) {
			drv->fd_read = 0;
			drv->fd_write = 1;
		}
		else if (strcmp (drv->name, "--") == 0) {
			drv->fd_read = 0;
			drv->fd_write = 2;
		}
		else {
			drv->fd_read = open (drv->name,
				O_RDWR | O_CREAT | O_NOCTTY | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
			);

			if (drv->fd_read < 0) {
				return (1);
			}

			drv->fd_write = drv->fd_read;
		}
	}
	else if ((drv->name_read != NULL) || (drv->name_write != NULL)) {
		if (drv->name_read != NULL) {
			if (strcmp (drv->name_read, "-") == 0) {
				drv->fd_read = 0;
			}
			else {
				drv->fd_read = open (drv->name_read,
					O_RDONLY | O_NOCTTY, 0
				);

				if (drv->fd_read < 0) {
					return (1);
				}
			}
		}

		if (drv->name_write != NULL) {
			if (strcmp (drv->name_write, "-") == 0) {
				drv->fd_write = 1;
			}
			else if (strcmp (drv->name_write, "--") == 0) {
				drv->fd_write = 2;
			}
			else {
				drv->fd_write = open (drv->name_write,
					O_WRONLY | O_CREAT | O_NOCTTY | O_TRUNC,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
				);

				if (drv->fd_write < 0) {
					return (1);
				}
			}
		}
	}
	else {
		if (strncmp (name, "sercon", 6) == 0) {
			drv->fd_read = 0;
			drv->fd_write = 1;
		}
	}

	return (0);
}

char_drv_t *chr_posix_open (const char *name)
{
	char_posix_t *drv;

	drv = malloc (sizeof (char_posix_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_posix_init (drv, name)) {
		chr_posix_close (&drv->cdrv);
		return (NULL);
	}

	return (&drv->cdrv);
}
