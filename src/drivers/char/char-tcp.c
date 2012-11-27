/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-tcp.c                                  *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
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


#define TELNET_SE   240
#define TELNET_NOP  241
#define TELNET_GA   249
#define TELNET_WILL 251
#define TELNET_WONT 252
#define TELNET_DO   253
#define TELNET_DONT 254
#define TELNET_IAC  255


#define TELNET_OPT_BINARY            0
#define TELNET_OPT_ECHO              1
#define TELNET_OPT_SUPPRESS_GO_AHEAD 3


enum {
	CHAR_TCP_DATA,
	CHAR_TCP_IAC,
	CHAR_TCP_WILL,
	CHAR_TCP_WONT,
	CHAR_TCP_DO,
	CHAR_TCP_DONT,
	CHAR_TCP_OPTION
};


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
int tcp_set_nodelay (int fd, int val)
{
#ifdef HAVE_LINUX_TCP_H
	val = (val != 0);

	if (setsockopt (fd, TCP, TCP_NODELAY, &val, sizeof (int))) {
		return (1);
	}

	return (0);
#else
	return (1);
#endif
}

static
int tcp_set_reuseaddr (int fd, int val)
{
	val = (val != 0);

	if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (int))) {
		return (1);
	}

	return (0);
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

	tcp_set_reuseaddr (fd, 1);

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


#define telnet_do(drv, option) telnet_send_request(drv, option, TELNET_DO)
#define telnet_dont(drv, option) telnet_send_request(drv, option, TELNET_DONT)
#define telnet_will(drv, option) telnet_send_request(drv, option, TELNET_WILL)
#define telnet_wont(drv, option) telnet_send_request(drv, option, TELNET_WONT)

static
void telnet_send_request (char_tcp_t *drv, unsigned char option, unsigned char verb)
{
	unsigned char buf[3];

	buf[0] = TELNET_IAC;
	buf[1] = verb;
	buf[2] = option;

	write(drv->fd, buf, 3);
}

static
void telnet_process_request (char_tcp_t *drv, unsigned option)
{
	unsigned st;

	st = drv->telnet_state;

	switch (option) {
	case TELNET_OPT_BINARY:
		if ((st == CHAR_TCP_DO) || (st == CHAR_TCP_DONT)) {
			/* We insist on being binary. */
			telnet_will (drv, TELNET_OPT_BINARY);
		}
		else if ((st == CHAR_TCP_WILL) || (st == CHAR_TCP_WONT)) {
			/* We insist on being binary. */
			telnet_do (drv, TELNET_OPT_BINARY);
		}
		return;

	case TELNET_OPT_ECHO:
		if (st == CHAR_TCP_WILL) {
			/* We refuse that the client performs echoing itself. */
			telnet_dont (drv, TELNET_OPT_ECHO);
		}
		else if (st == CHAR_TCP_DONT) {
			/* We insist on echoing. */
			telnet_will (drv, TELNET_OPT_ECHO);
		}
		return;

	case TELNET_OPT_SUPPRESS_GO_AHEAD:
		if (st == CHAR_TCP_DONT) {
			/* We will never send go-aheads. */
			telnet_will (drv, TELNET_OPT_SUPPRESS_GO_AHEAD);
		}
		else if ((st == CHAR_TCP_WILL) || (st == CHAR_TCP_WONT)) {
			/* The client may or may not suppress go-aheads at its own
			 * volition. */
			telnet_send_request (drv, TELNET_OPT_SUPPRESS_GO_AHEAD,
				(st == CHAR_TCP_WILL) ? CHAR_TCP_DO : CHAR_TCP_DONT
			);
		}
		return;

	default:
		/* unknown option, refuse it. */
		telnet_send_request (drv, option,
			(st == CHAR_TCP_WILL) ? TELNET_DONT : TELNET_WONT
		);
	}
}

static
unsigned telnet_filter_input (char_tcp_t *drv, unsigned char *buf, unsigned cnt)
{
	unsigned i, j;

	j = 0;

	for (i = 0; i < cnt; i++) {
		switch (drv->telnet_state) {
		case CHAR_TCP_DATA:
			if (buf[i] == TELNET_IAC) {
				drv->telnet_state = CHAR_TCP_IAC;
			}
			else {
				buf[j++] = buf[i];
			}
			break;

		case CHAR_TCP_IAC:
			switch (buf[i]) {
			case TELNET_NOP:
			case TELNET_GA:
				drv->telnet_state = CHAR_TCP_DATA;
				break;

			case TELNET_WILL:
				drv->telnet_state = CHAR_TCP_WILL;
				break;

			case TELNET_WONT:
				drv->telnet_state = CHAR_TCP_WONT;
				break;

			case TELNET_DO:
				drv->telnet_state = CHAR_TCP_DO;
				break;

			case TELNET_DONT:
				drv->telnet_state = CHAR_TCP_DONT;
				break;

			case TELNET_IAC:
				buf[j++] = buf[i];
				drv->telnet_state = CHAR_TCP_DATA;
				break;

			default:
				fprintf (stderr, "char-tcp: "
					"ignoring unknown/unexptected IAC %u\n",
					buf[i]
				);
				drv->telnet_state = CHAR_TCP_DATA;
				break;
			}
			break;

		case CHAR_TCP_WILL:
		case CHAR_TCP_DO:
		case CHAR_TCP_WONT:
		case CHAR_TCP_DONT:
			telnet_process_request (drv, buf[i]);
			drv->telnet_state = CHAR_TCP_DATA;
			break;

		default:
			fprintf (stderr,
				"unknown char-tcp state %u? resetting to DATA.\n",
				drv->telnet_state
			);
			drv->telnet_state = CHAR_TCP_DATA;
		}
	}

	return (j);
}

static
ssize_t telnet_write (char_tcp_t *drv, const void *buf, unsigned cnt)
{
	unsigned            i;
	ssize_t             r;
	const unsigned char *tmp;
	unsigned char       iac[2];

	if (cnt == 0) {
		return (0);
	}

	tmp = buf;

	i = 0;
	while ((i < cnt) && (tmp[i] != TELNET_IAC)) {
		i += 1;
	}

	if (i == 0) {
		iac[0] = TELNET_IAC;
		iac[1] = TELNET_IAC;

		r = write (drv->fd, iac, 2);

		if (r == 2) {
			r = 1;
		}
	}
	else {
		r = write (drv->fd, buf, i);
	}

	return (r);
}


static
void chr_tcp_init_connection (char_tcp_t *drv)
{
	tcp_set_nodelay (drv->fd, 1);

	drv->telnet_state = CHAR_TCP_DATA;

	if (drv->telnet && drv->telnetinit) {
		/* Instruct telnet client to switch to character mode. */
		telnet_do (drv, TELNET_OPT_BINARY);
		telnet_will (drv, TELNET_OPT_ECHO);
		telnet_will (drv, TELNET_OPT_SUPPRESS_GO_AHEAD);
	}
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

	chr_tcp_init_connection (drv);

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

	if (drv->telnet) {
		r = telnet_filter_input (drv, buf, r);
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

	if (drv->telnet) {
		r = telnet_write (drv, buf, cnt);
	}
	else {
		r = write (drv->fd, buf, cnt);
	}

	if (r <= 0) {
		chr_tcp_shutdown (drv);
		return (cnt);
	}

	return (r);
}

static
int chr_tcp_get_ctl (char_drv_t *cdrv, unsigned *ctl)
{
	char_tcp_t *drv = cdrv->ext;

	*ctl = drv->ctl | PCE_CHAR_DSR;

	if (drv->fd >= 0) {
		*ctl |= PCE_CHAR_CTS | PCE_CHAR_CD;
	}
	else {
		*ctl &= ~(PCE_CHAR_CTS | PCE_CHAR_CD);
	}

	return (0);
}

static
int chr_tcp_set_ctl (char_drv_t *cdrv, unsigned ctl)
{
	char_tcp_t *drv = cdrv->ext;

	if ((drv->ctl ^ ctl) & ~ctl & PCE_CHAR_DTR) {
		/* Hang Up */
		chr_tcp_shutdown(drv);
	}

	drv->ctl = ctl;

	return (0);
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

	drv->ctl = PCE_CHAR_DSR;

	drv->host = drv_get_option (name, "host");
	drv->connect = drv_get_option_bool (name, "connect", 0);
	drv->port = drv_get_option_uint (name, "port", 5555);

	drv->telnet = drv_get_option_bool (name, "telnet", 0);
	drv->telnetinit = drv_get_option_bool (name, "telnetinit", 1);
	drv->usectl = drv_get_option_bool (name, "usectl", 1);

	if (drv->usectl) {
		drv->cdrv.get_ctl = chr_tcp_get_ctl;
		drv->cdrv.set_ctl = chr_tcp_set_ctl;
	}

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

		chr_tcp_init_connection (drv);
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
