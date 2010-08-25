/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-tios.c                                 *
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
#include <limits.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-tios.h>


/*
 * Check if file descriptor fd is readable and/or writeable
 */
static
int chr_tios_check_fd (int fd, int rd, int wr)
{
	int           r;
	struct pollfd pfd[1];

	if (fd < 0) {
		return (0);
	}

	pfd[0].fd = fd;
	pfd[0].events = (rd ? POLLIN : 0) | (wr ? POLLOUT : 0);

	r = poll (pfd, 1, 0);

	if (r < 0) {
		return (0);
	}

	if ((pfd[0].revents & (POLLIN | POLLOUT)) == 0) {
		return (0);
	}

	return (1);
}

static
void chr_tios_close (char_drv_t *cdrv)
{
	char_tios_t *drv;

	drv = cdrv->ext;

	if (drv->fname != NULL) {
		free (drv->fname);
	}

	if (drv->fd >= 0) {
		close (drv->fd);
	}

	free (drv);
}

static
unsigned chr_tios_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	char_tios_t *drv;
	ssize_t     r;

	drv = cdrv->ext;

	if (chr_tios_check_fd (drv->fd, 1, 0) == 0) {
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
unsigned chr_tios_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_tios_t *drv;
	ssize_t      r;

	drv = cdrv->ext;

	if (drv->fd < 0) {
		return (cnt);
	}

	if (chr_tios_check_fd (drv->fd, 0, 1) == 0) {
		return (0);
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
int chr_tios_get_ctl (char_drv_t *cdrv, unsigned *ctl)
{
	char_tios_t *drv;
	int         val;

	drv = cdrv->ext;

	*ctl = 0;

	if (drv->fd >= 0) {
		ioctl (drv->fd, TIOCMGET, &val);
	}
	else {
		val = TIOCM_DSR | TIOCM_CTS | TIOCM_CD;
	}

	*ctl |= (val & TIOCM_DSR) ? PCE_CHAR_DSR : 0;
	*ctl |= (val & TIOCM_CTS) ? PCE_CHAR_CTS : 0;
	*ctl |= (val & TIOCM_CD) ? PCE_CHAR_CD : 0;
	*ctl |= (val & TIOCM_RI) ? PCE_CHAR_RI : 0;

	*ctl |= PCE_CHAR_DSR;
	*ctl |= PCE_CHAR_CTS;

	return (0);
}

static
int chr_tios_set_ctl (char_drv_t *cdrv, unsigned ctl)
{
	char_tios_t *drv;
	int         val;

	drv = cdrv->ext;

	if (drv->fd < 0) {
		return (0);
	}

	ioctl (drv->fd, TIOCMGET, &val);
	val &= ~(TIOCM_DTR | TIOCM_RTS);
	val |= (ctl & PCE_CHAR_DTR) ? TIOCM_DTR : 0;
	val |= (ctl & PCE_CHAR_RTS) ? TIOCM_RTS : 0;
	ioctl (drv->fd, TIOCMSET, &val);

	return (0);
}

static
int chr_tios_set_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop)
{
	char_tios_t    *drv;
	struct termios tio;
	speed_t        spd;

	drv = cdrv->ext;

	if (drv->fd < 0) {
		return (0);
	}

	if (tcgetattr (drv->fd, &tio)) {
		return (1);
	}

	tio.c_iflag &= ~INPCK;        /* Input parity check */
	tio.c_iflag &= ~ISTRIP;       /* Strip to 7 bits */
	tio.c_iflag &= ~IGNBRK;       /* Ignore break conditions */
	tio.c_iflag &= ~BRKINT;       /* SIGINT on break condition */
	tio.c_iflag &= ~IGNCR;        /* Discard CR on intput */
	tio.c_iflag &= ~ICRNL;        /* Convert CR to LF on input */
	tio.c_iflag &= ~INLCR;        /* Convert LF to CR on input */
	tio.c_iflag &= ~IXOFF;        /* Enable input flow control */
	tio.c_iflag &= ~IXON;         /* Enable output flow control */
	tio.c_iflag &= ~IXANY;        /* Any character as START */
	tio.c_iflag &= ~IMAXBEL;      /* Send BEL on buffer overflow */

	tio.c_oflag &= ~OPOST;        /* Enable output post processing */

	tio.c_cflag |= CLOCAL;        /* Don't use modem control lines */
	tio.c_cflag &= ~HUPCL;        /* Hang up modem on device close */
	tio.c_cflag |= CREAD;         /* Input can be read */
	tio.c_cflag &= ~CRTSCTS;

	tio.c_lflag &= ~ICANON;       /* Canonical mode */
	tio.c_lflag &= ~ECHO;         /* Enable echo */
	tio.c_lflag &= ~ECHOE;        /* Enable echo */
#ifdef ECHOPRT
	tio.c_lflag &= ~ECHOPRT;      /* Enable echo */
#endif
	tio.c_lflag &= ~ECHOK;        /* Enable echo */
	tio.c_lflag &= ~ECHONL;       /* Enable echo */
#ifdef ECHOCTL
	tio.c_lflag &= ~ECHOCTL;      /* Enable echo */
#endif
	tio.c_lflag &= ~ISIG;         /* Enable signals on special characters */
	tio.c_lflag |= NOFLSH;        /* Don't flush queues on special characters */

	tio.c_cflag &= ~CSIZE;

	switch (bpc) {
	case 8:
		tio.c_cflag |= CS8;
		break;

	case 7:
		tio.c_cflag |= CS7;
		break;

	case 6:
		tio.c_cflag |= CS6;
		break;

	case 5:
		tio.c_cflag |= CS5;
		break;

	default:
		tio.c_cflag |= CS8;
		break;
	}

	if (stop == 1) {
		tio.c_cflag &= ~CSTOPB;
	}
	else {
		tio.c_cflag |= CSTOPB;
	}

	switch (parity) {
	case 0:
		tio.c_cflag &= ~PARENB;
		break;

	case 1: /* odd */
		tio.c_cflag |= PARENB;
		tio.c_cflag |= PARODD;
		break;

	case 2: /* even */
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		break;

	case 3: /* mark */
	case 4: /* space */
		/* termios does not support these */
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		break;

	default:
		tio.c_cflag &= !PARENB;
		break;
	}

	if (bps < ((50 + 75) / 2)) {
		spd = B50;
	}
	else if (bps < ((75 + 110) / 2)) {
		spd = B75;
	}
	else if (bps < ((110 + 134) / 2)) {
		spd = B110;
	}
	else if (bps < ((134 + 150) / 2)) {
		spd = B134;
	}
	else if (bps < ((150 + 200) / 2)) {
		spd = B150;
	}
	else if (bps < ((200 + 300) / 2)) {
		spd = B200;
	}
	else if (bps < ((300 + 600) / 2)) {
		spd = B300;
	}
	else if (bps < ((600 + 1200) / 2)) {
		spd = B600;
	}
	else if (bps < ((1200 + 2400) / 2)) {
		spd = B1200;
	}
	else if (bps < ((2400 + 4800) / 2)) {
		spd = B2400;
	}
	else if (bps < ((4800 + 9600) / 2)) {
		spd = B4800;
	}
	else if (bps < ((9600 + 19200) / 2)) {
		spd = B9600;
	}
	else if (bps < ((19200 + 38400) / 2)) {
		spd = B19200;
	}
#ifdef B38400
	else if (bps < ((38400 + 57600) / 2)) {
		spd = B38400;
	}
#endif
#ifdef B57600
	else if (bps < ((57600 + 115200) / 2)) {
		spd = B57600;
	}
#endif
#ifdef B115200
	else if (bps < ((115200 + 230400) / 2)) {
		spd = B115200;
	}
#endif
	else {
		spd = B9600;
	}

	cfsetispeed (&tio, spd);
	cfsetospeed (&tio, spd);

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr (drv->fd, TCSANOW, &tio)) {
		return (1);
	}

	tcflush (drv->fd, TCIOFLUSH);

	return (0);
}

static
int chr_tios_init (char_tios_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_tios_close;
	drv->cdrv.read = chr_tios_read;
	drv->cdrv.write = chr_tios_write;
	drv->cdrv.get_ctl = chr_tios_get_ctl;
	drv->cdrv.set_ctl = chr_tios_set_ctl;
	drv->cdrv.set_params = chr_tios_set_params;

	drv->fd = -1;

	drv->fname = drv_get_option (name, "file");

	if (drv->fname != NULL) {
		drv->fd = open (drv->fname, O_RDWR | O_NOCTTY);

		if (drv->fd < 0) {
			return (1);
		}

		chr_tios_set_params (&drv->cdrv, 9600, 8, 0, 1);
	}

	return (0);
}

char_drv_t *chr_tios_open (const char *name)
{
	char_tios_t *drv;

	drv = malloc (sizeof (char_tios_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_tios_init (drv, name)) {
		chr_tios_close (&drv->cdrv);

		return (NULL);
	}

	return (&drv->cdrv);
}
