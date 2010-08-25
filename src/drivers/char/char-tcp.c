/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-tcp.c                                  *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>
#include <limits.h>

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-tcp.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_LINUX_TCP_H
#include <linux/tcp.h>
#endif


static
unsigned tcp_get_state (int fd, int t)
{
	int           r;
	unsigned      val;
	struct pollfd pfd[1];

	pfd[0].fd = fd;
	pfd[0].events = POLLIN | POLLOUT;

	r = poll (pfd, 1, t);

	if (r < 0) {
		return (0);
	}

	val = 0;

	if (pfd[0].revents & POLLHUP) {
		val |= 4;
	}

	if (pfd[0].revents & POLLOUT) {
		val |= 2;
	}

	if (pfd[0].revents & POLLIN) {
		val |= 1;
	}

	return (val);
}

static
int tcp_connect (const char *host, unsigned short port)
{
	int                fd;
	struct sockaddr_in addr;

	fd = socket (PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return (-1);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr (host);
	addr.sin_port = htons (port);

	if (connect (fd, (struct sockaddr *) &addr, sizeof (addr)) == -1) {
		close (fd);
		return (-1);
	}

	return (fd);
}

static
int tcp_listen (const char *host, unsigned short port, unsigned queue)
{
	int                fd;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;

	if ((host == NULL) || (host[0] == 0)) {
		addr.sin_addr.s_addr = INADDR_ANY;
	}
	else {
		if (inet_aton (host, &addr.sin_addr) == 0) {
			return (-1);
		}
	}

#if USHRT_MAX != 65535
	if (port > 65535) {
		return (-1);
	}
#endif

	addr.sin_port = htons ((unsigned short) port);

	fd = socket (PF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		return (-1);
	}

	if (bind (fd, (struct sockaddr *) &addr, sizeof (addr)) == -1) {
		close (fd);
		return (-1);
	}

	if (listen (fd, (int) queue) == -1) {
		close (fd);
		return (-1);
	}

	return (fd);
}

static
int tcp_accept (int fd)
{
	int                ret;
	struct sockaddr_in addr;
	socklen_t          cnt;

	cnt = sizeof (addr);
	ret = accept (fd, (struct sockaddr *) &addr, &cnt);

#ifdef DEBUG_CHAR
	if (ret >= 0) {
		fprintf (stderr, "char-tcp: accept\n");
	}
#endif

	return (ret);
}

int tcp_nodelay (int fd, int val)
{
#ifdef HAVE_LINUX_TCP_H
	val = (val != 0);

	if (setsockopt (fd, 6, TCP_NODELAY, &val, sizeof (int))) {
		return (1);
	}

	return (0);
#else
	return (1);
#endif
}

static
char *tcp_host_resolve (const char *str)
{
	char           buf[256];
	struct hostent *ent;
	char           *ret;

	ent = gethostbyname (str);
	if (ent == NULL) {
		return (NULL);
	}

	if (inet_ntop (ent->h_addrtype, ent->h_addr_list[0], buf, 256) == NULL) {
		return (NULL);
	}

	ret = strdup (buf);

	return (ret);
}


static
void chr_tcp_close (char_drv_t *cdrv)
{
	char_tcp_t *drv;

	drv = cdrv->ext;

#ifdef DEBUG_CHAR
	fprintf (stderr, "char-tcp: close\n");
#endif

	if (drv->fd >= 0) {
		close (drv->fd);
	}

	if (drv->listen_fd >= 0) {
		close (drv->listen_fd);
	}

	free (drv);
}

static
int chr_tcp_accept (char_tcp_t *drv)
{
	if (drv->fd >= 0) {
		return (0);
	}

	if (drv->listen_fd < 0) {
		return (1);
	}

	if ((tcp_get_state (drv->listen_fd, 0) & 1) == 0) {
		return (1);
	}

	drv->fd = tcp_accept (drv->listen_fd);

	if (drv->fd < 0) {
		return (1);
	}

	tcp_nodelay (drv->fd, 1);

	return (0);
}

static
void chr_tcp_shutdown (char_tcp_t *drv)
{
	if (drv->fd < 0) {
		return;
	}

#ifdef DEBUG_CHAR
	fprintf (stderr, "char-tcp: shutdown\n");
#endif

	close (drv->fd);

	drv->fd = -1;
}

static
unsigned chr_tcp_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	char_tcp_t *drv;
	ssize_t    r;
	unsigned   st;

	drv = cdrv->ext;

	if (chr_tcp_accept (drv)) {
		return (0);
	}

	st = tcp_get_state (drv->fd, 0);

	if (st & 4) {
		chr_tcp_shutdown (drv);
		return (0);
	}

	if ((st & 1) == 0) {
		return (0);
	}

#if UINT_MAX > SSIZE_MAX
	if (cnt > SSIZE_MAX) {
		cnt = SSIZE_MAX;
	}
#endif

	r = read (drv->fd, buf, cnt);

	if (r <= 0) {
		chr_tcp_shutdown (drv);
		return (0);
	}

	return (r);
}

static
unsigned chr_tcp_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_tcp_t *drv;
	ssize_t    r;
	unsigned   st;

	drv = cdrv->ext;

	if (chr_tcp_accept (drv)) {
		return (cnt);
	}

	st = tcp_get_state (drv->fd, 0);

	if (st & 4) {
		chr_tcp_shutdown (drv);
		return (cnt);
	}

	if ((st & 2) == 0) {
		return (0);
	}

#if UINT_MAX > SSIZE_MAX
	if (cnt > SSIZE_MAX) {
		cnt = SSIZE_MAX;
	}
#endif

	r = write (drv->fd, buf, cnt);

	if (r <= 0) {
		chr_tcp_shutdown (drv);
		return (cnt);
	}

	return (r);
}

static
int chr_tcp_init (char_tcp_t *drv, const char *name)
{
	char *str;

	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_tcp_close;
	drv->cdrv.read = chr_tcp_read;
	drv->cdrv.write = chr_tcp_write;

	drv->listen_fd = -1;
	drv->fd = -1;

	drv->host = drv_get_option (name, "host");
	drv->connect = drv_get_option_bool (name, "connect", 0);
	drv->port = drv_get_option_uint (name, "port", 5555);

	if (drv->host != NULL) {
		str = tcp_host_resolve (drv->host);

		if (str == NULL) {
			return (1);
		}

		free (drv->host);

		drv->host = str;
	}

	if (drv->connect) {
		drv->fd = tcp_connect (drv->host, drv->port);

		if (drv->fd < 0) {
			return (1);
		}
	}
	else {
		drv->listen_fd = tcp_listen (NULL, drv->port, 1);

		if (drv->listen_fd < 0) {
			return (1);
		}
	}

	return (0);
}

char_drv_t *chr_tcp_open (const char *name)
{
	char_tcp_t *drv;

	drv = malloc (sizeof (char_tcp_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_tcp_init (drv, name)) {
		chr_tcp_close (&drv->cdrv);
		return (NULL);
	}

	return (&drv->cdrv);
}
