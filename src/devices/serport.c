/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/serport.c                                        *
 * Created:     2003-09-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdio.h>
#include <stdlib.h>

#include <lib/log.h>

#include "serport.h"


#ifdef PCE_ENABLE_SERPORT
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif


static void ser_uart_check_setup (serport_t *ser);
static void ser_uart_check_status (serport_t *src);
static void ser_uart_check_out (serport_t *ser, unsigned char val);
static void ser_uart_check_inp (serport_t *ser, unsigned char val);


static
unsigned char ser_get_uint8 (serport_t *ser, unsigned long addr)
{
	return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift));
}

static
unsigned short ser_get_uint16 (serport_t *ser, unsigned long addr)
{
	return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift));
}

static
unsigned long ser_get_uint32 (serport_t *ser, unsigned long addr)
{
	return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift));
}

static
void ser_set_uint8 (serport_t *ser, unsigned long addr, unsigned char val)
{
	e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift, val);
}

static
void ser_set_uint16 (serport_t *ser, unsigned long addr, unsigned short val)
{
	e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift, val & 0xff);
}

static
void ser_set_uint32 (serport_t *ser, unsigned long addr, unsigned long val)
{
	e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift, val & 0xff);
}

void ser_init (serport_t *ser, unsigned long base, unsigned shift)
{
	dev_init (&ser->device, ser, "uart");

	ser->device.free = (void *) ser_free;
	ser->device.del = (void *) ser_del;
	ser->device.clock = (void *) ser_clock;

	ser->io = base;

	ser->addr_shift = shift;

	e8250_init (&ser->uart);
	e8250_set_send_fct (&ser->uart, ser, ser_uart_check_out);
	e8250_set_recv_fct (&ser->uart, ser, ser_uart_check_inp);
	e8250_set_setup_fct (&ser->uart, ser, ser_uart_check_setup);
	e8250_set_check_fct (&ser->uart, ser, ser_uart_check_status);

	e8250_set_multichar (&ser->uart, 1, 1);

	mem_blk_init (&ser->port, base, 8 << shift, 0);
	ser->port.ext = ser;
	ser->port.get_uint8 = (void *) ser_get_uint8;
	ser->port.set_uint8 = (void *) ser_set_uint8;
	ser->port.get_uint16 = (void *) ser_get_uint16;
	ser->port.set_uint16 = (void *) ser_set_uint16;
	ser->port.get_uint32 = (void *) ser_get_uint32;
	ser->port.set_uint32 = (void *) ser_set_uint32;

	ser->bps = 2400;
	ser->databits = 8;
	ser->stopbits = 1;
	ser->parity = E8250_PARITY_N;

	ser->dtr = 0;
	ser->rts = 0;

	ser->check_out = 0;
	ser->check_inp = 1;

	ser->fp = NULL;
	ser->fp_close = 0;

	ser->fd = -1;
	ser->fd_close = 0;

	/* ready to receive data */
	e8250_set_dsr (&ser->uart, 1);
	e8250_set_cts (&ser->uart, 1);
}

serport_t *ser_new (unsigned long base, unsigned shift)
{
	serport_t *ser;

	ser = malloc (sizeof (serport_t));
	if (ser == NULL) {
		return (NULL);
	}

	ser_init (ser, base, shift);

	return (ser);
}

void ser_free (serport_t *ser)
{
	e8250_free (&ser->uart);
	mem_blk_free (&ser->port);

#ifdef PCE_ENABLE_SERPORT
	if (ser->fd_close) {
		close (ser->fd);
	}
#endif

	if (ser->fp_close) {
		fclose (ser->fp);
	}
}

void ser_del (serport_t *ser)
{
	if (ser != NULL) {
		ser_free (ser);
		free (ser);
	}
}

device_t *ser_get_device (serport_t *ser)
{
	return (&ser->device);
}

mem_blk_t *ser_get_reg (serport_t *ser)
{
	return (&ser->port);
}

e8250_t *ser_get_uart (serport_t *ser)
{
	return (&ser->uart);
}

int ser_set_fp (serport_t *ser, FILE *fp, int close)
{
	if (ser->fp_close) {
		fclose (ser->fp);
	}

	ser->fp = fp;
	ser->fp_close = (fp != NULL) && close;

	return (0);
}

int ser_set_fname (serport_t *ser, const char *fname)
{
	FILE *fp;

	fp = fopen (fname, "wb");
	if (fp == NULL) {
		return (1);
	}

	if (ser_set_fp (ser, fp, 1)) {
		fclose (fp);
		return (1);
	}

	return (0);
}

#ifdef PCE_ENABLE_SERPORT

static
int ser_line_setup (serport_t *ser)
{
	struct termios tio;
	speed_t        spd;

	if (ser->fd == -1) {
		return (0);
	}

	if (tcgetattr (ser->fd, &tio)) {
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

	switch (ser->databits) {
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

	if (ser->stopbits == 1) {
		tio.c_cflag &= ~CSTOPB;
	}
	else {
		tio.c_cflag |= CSTOPB;
	}

	switch (ser->parity) {
	case E8250_PARITY_N:
		tio.c_cflag &= ~PARENB;
		break;

	case E8250_PARITY_S:
	case E8250_PARITY_M:
		/* termios does not support these */
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		break;

	case E8250_PARITY_E:
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		break;

	case E8250_PARITY_O:
		tio.c_cflag |= PARENB;
		tio.c_cflag |= PARODD;
		break;
	}

	if (ser->bps < ((50 + 75) / 2)) {
		spd = B50;
	}
	else if (ser->bps < ((75 + 110) / 2)) {
		spd = B75;
	}
	else if (ser->bps < ((110 + 134) / 2)) {
		spd = B110;
	}
	else if (ser->bps < ((134 + 150) / 2)) {
		spd = B134;
	}
	else if (ser->bps < ((150 + 200) / 2)) {
		spd = B150;
	}
	else if (ser->bps < ((200 + 300) / 2)) {
		spd = B200;
	}
	else if (ser->bps < ((300 + 600) / 2)) {
		spd = B300;
	}
	else if (ser->bps < ((600 + 1200) / 2)) {
		spd = B600;
	}
	else if (ser->bps < ((1200 + 2400) / 2)) {
		spd = B1200;
	}
	else if (ser->bps < ((2400 + 4800) / 2)) {
		spd = B2400;
	}
	else if (ser->bps < ((4800 + 9600) / 2)) {
		spd = B4800;
	}
	else if (ser->bps < ((9600 + 19200) / 2)) {
		spd = B9600;
	}
	else if (ser->bps < ((19200 + 38400) / 2)) {
		spd = B19200;
	}
#ifdef B38400
	else if (ser->bps < ((38400 + 57600) / 2)) {
		spd = B38400;
	}
#endif
#ifdef B57600
	else if (ser->bps < ((57600 + 115200) / 2)) {
		spd = B57600;
	}
#endif
#ifdef B115200
	else if (ser->bps < ((115200 + 230400) / 2)) {
		spd = B115200;
	}
#endif
	else {
		pce_log (MSG_ERR,
			"serport: can't set speed to %lu\n", ser->bps
		);

		spd = B9600;
	}

	cfsetispeed (&tio, spd);
	cfsetospeed (&tio, spd);

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr (ser->fd, TCSANOW, &tio)) {
		return (1);
	}

	tcflush (ser->fd, TCIOFLUSH);

	return (0);
}

static
void ser_modem_setup (serport_t *ser)
{
	int val;

	if (ser->fd != -1) {
		ioctl (ser->fd, TIOCMGET, &val);
		val &= ~(TIOCM_DTR | TIOCM_RTS);
		val |= ser->dtr ? TIOCM_DTR : 0;
		val |= ser->rts ? TIOCM_RTS : 0;
		ioctl (ser->fd, TIOCMSET, &val);
	}
}

static
void ser_status_check (serport_t *ser)
{
	int val;

	if (ser->fd != -1) {
		ioctl (ser->fd, TIOCMGET, &val);
	}
	else {
		val = TIOCM_DSR | TIOCM_CTS | TIOCM_CD;
	}

	e8250_set_dsr (&ser->uart, (val & TIOCM_DSR) != 0);
	e8250_set_cts (&ser->uart, (val & TIOCM_CTS) != 0);
	e8250_set_dcd (&ser->uart, (val & TIOCM_CD) != 0);
	e8250_set_ri (&ser->uart, (val & TIOCM_RI) != 0);
}

static
int fd_readable (int fd, int t)
{
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
}

static
int fd_writeable (int fd, int t)
{
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
}

static
void ser_process_output (serport_t *ser)
{
	unsigned char c;

	while (1) {
		if ((ser->fd != -1) && (fd_writeable (ser->fd, 0) == 0)) {
			break;
		}

		if (e8250_get_out (&ser->uart, &c, 1)) {
			break;
		}

		if (ser->fd != -1) {
			write (ser->fd, &c, 1);
		}

		if (ser->fp != NULL) {
			fputc (c, ser->fp);
		}
	}

	if (ser->fp != NULL) {
		fflush (ser->fp);
	}

	ser->check_out = (e8250_out_empty (&ser->uart) == 0);
}

static
void ser_process_input (serport_t *ser)
{
	ssize_t       r;
	unsigned char c;

	while (e8250_inp_full (&ser->uart) == 0) {
		if ((ser->fd == -1) || (fd_readable (ser->fd, 0) == 0)) {
			break;
		}

		r = read (ser->fd, &c, 1);
		if (r != 1) {
			break;
		}

		e8250_receive (&ser->uart, c);
	}

	ser->check_inp = (e8250_inp_full (&ser->uart) == 0);
}

int ser_set_fd (serport_t *ser, int fd, int fdclose)
{
	if (ser->fd_close) {
		close (ser->fd);
	}

	ser->fd = fd;
	ser->fd_close = (fd != -1) && fdclose;

	return (0);
}

int ser_set_dname (serport_t *ser, const char *dname)
{
	int fd;

	fd = open (dname,
		O_RDWR | O_CREAT | O_NOCTTY | O_TRUNC,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
	);

	if (fd < 0) {
		return (1);
	}

	ser_set_fd (ser, fd, 1);

	return (0);
}

#else

static
int ser_line_setup (serport_t *ser)
{
	return (0);
}

static
void ser_modem_setup (serport_t *ser)
{
}

static
void ser_status_check (serport_t *ser)
{
}

static
void ser_process_output (serport_t *ser)
{
	unsigned char c;

	ser->check_out = 0;

	while (1) {
		if (e8250_get_out (&ser->uart, &c, 1)) {
			break;
		}

		if (ser->fp != NULL) {
			fputc (c, ser->fp);
		}
	}

	if (ser->fp != NULL) {
		fflush (ser->fp);
	}

	ser->check_out = (e8250_out_empty (&ser->uart) == 0);
}

static
void ser_process_input (serport_t *ser)
{
	ser->check_inp = 0;
}

int ser_set_fd (serport_t *ser, int fd, int close)
{
	return (1);
}

int ser_set_dname (serport_t *ser, const char *dname)
{
	return (1);
}

#endif

static
void ser_uart_check_setup (serport_t *ser)
{
	ser->bps = e8250_get_bps (&ser->uart);
	ser->databits = e8250_get_databits (&ser->uart);
	ser->stopbits = e8250_get_stopbits (&ser->uart);
	ser->parity = e8250_get_parity (&ser->uart);

	ser_line_setup (ser);

	ser->dtr = e8250_get_dtr (&ser->uart);
	ser->rts = e8250_get_rts (&ser->uart);

	ser_modem_setup (ser);
}

static
void ser_uart_check_status (serport_t *ser)
{
	ser_status_check (ser);
}

/* 8250 output buffer is not empty */
static
void ser_uart_check_out (serport_t *ser, unsigned char val)
{
	ser->check_out = 1;
}

/* 8250 input buffer is not full */
static
void ser_uart_check_inp (serport_t *ser, unsigned char val)
{
	ser->check_inp = 1;
}

void ser_receive (serport_t *ser, unsigned char val)
{
	e8250_receive (&ser->uart, val);
}

void ser_clock (serport_t *ser, unsigned n)
{
	if (ser->check_out) {
		ser_process_output (ser);
	}

	if (ser->check_inp) {
		ser_process_input (ser);
	}
}
