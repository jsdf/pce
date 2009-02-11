/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/serial.c                                    *
 * Created:     2007-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"


#ifdef PCE_ENABLE_SERPORT
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif


void mac_ser_init (mac_ser_t *ser)
{
	ser->scc = NULL;
	ser->chn = 0;

	ser->bps = 0;
	ser->bpc = 0;
	ser->stop = 0;
	ser->parity = 0;

	ser->dtr = 0;
	ser->rts = 0;

	ser->fp = NULL;
	ser->fp_close = 0;

	ser->fd = -1;
	ser->fd_close = 0;
}

void mac_ser_free (mac_ser_t *ser)
{
#ifdef PCE_ENABLE_SERPORT
	if (ser->fd_close) {
		close (ser->fd);
	}
#endif

	if (ser->fp_close) {
		fclose (ser->fp);
	}
}

#ifdef PCE_ENABLE_SERPORT

static
void mac_ser_process_output (mac_ser_t *ser)
{
	unsigned char c;

	if (e8530_out_empty (ser->scc, ser->chn)) {
		return;
	}

	while (e8530_out_empty (ser->scc, ser->chn) == 0) {
		if (ser->fd != -1) {
			if (pce_fd_writeable (ser->fd, 0) == 0) {
				break;
			}
		}

		c = e8530_send (ser->scc, ser->chn);

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
}

static
void mac_ser_process_input (mac_ser_t *ser)
{
	ssize_t       r;
	unsigned char c;

	if (ser->fd == -1) {
		return;
	}

	while (e8530_inp_full (ser->scc, ser->chn) == 0) {
		if (pce_fd_readable (ser->fd, 0) == 0) {
			break;
		}

		r = read (ser->fd, &c, 1);
		if (r != 1) {
			break;
		}

		e8530_receive (ser->scc, ser->chn, c);
	}
}

static
void mac_ser_status_check (mac_ser_t *ser)
{
	int val;

	if (ser->fd != -1) {
		ioctl (ser->fd, TIOCMGET, &val);
	}
	else {
		val = TIOCM_DSR | TIOCM_CTS | TIOCM_CD;
	}

	e8530_set_cts (ser->scc, ser->chn, (val & TIOCM_CTS) != 0);
}

static
void mac_ser_set_rts (void *ext, unsigned char val)
{
	int       reg;
	mac_ser_t *ser;

	ser = ext;

	if (ser->rts == val) {
		return;
	}

#ifdef DEBUG_SERIAL
	mac_log_deb ("serial %u: RTS=%d\n", ser->chn, val);
#endif

	ser->rts = (val != 0);

	if (ser->fd != -1) {
		ioctl (ser->fd, TIOCMGET, &reg);

		if (val) {
			reg |= TIOCM_RTS;
		}
		else {
			reg &= ~TIOCM_RTS;
		}

		ioctl (ser->fd, TIOCMSET, &val);
	}
}

static
void mac_ser_fix_bps (unsigned long *bps, speed_t *spd)
{
	unsigned i;

	struct bps_lst_s {
		unsigned long bps;
		speed_t       speed;
	};

	static struct bps_lst_s lst[] = {
		{ 0, B0 },
		{ 50, B50 },
		{ 75, B75 },
		{ 110, B110 },
		{ 134, B134 },
		{ 150, B150 },
		{ 200, B200 },
		{ 300, B300 },
		{ 600, B600 },
		{ 1200, B1200 },
		{ 1800, B1800 },
		{ 2400, B2400 },
		{ 4800, B4800 },
		{ 9600, B9600 },
		{ 19200, B19200 },
#ifdef B38400
		{ 38400, B38400 },
#endif
#ifdef B57600
		{ 57600, B57600 },
#endif
#ifdef B115200
		{ 115200, B115200 },
#endif
#ifdef B230400
		{ 230400, B230400 },
#endif
		{ 0, B0 }
	};

	i = 0;
	while (lst[i + 1].bps != 0) {
		if (*bps < (lst[i].bps + lst[i + 1].bps) / 2) {
			*bps = lst[i].bps;
			*spd = lst[i].speed;
			return;
		}

		i += 1;
	}

	*bps = lst[i].bps;
	*spd = lst[i].speed;
}

static
void mac_ser_set_comm (void *ext, unsigned long bps, unsigned parity, unsigned bpc, unsigned stop)
{
	int            mod;
	struct termios tio;
	speed_t        spd;
	mac_ser_t      *ser;

	ser = ext;

	mod = 0;

	if (ser->bps != bps) {
		mod = 1;
		ser->bps = bps;
	}

	if (ser->parity != parity) {
		mod = 1;
		ser->parity = parity;
	}

	if (ser->bpc != bpc) {
		mod = 1;
		ser->bpc = bpc;
	}

	if (ser->stop != stop) {
		mod = 1;
		ser->stop = stop;
	}

#ifdef DEBUG_SERIAL
	mac_log_deb ("serial %u: setup (%lu/%u/%u%u)\n", ser->chn,
		ser->bps, ser->bpc, ser->parity, ser->stop
	);
#endif

	if (mod == 0) {
		return;
	}

	if (ser->fd == -1) {
		return;
	}

	if (tcgetattr (ser->fd, &tio)) {
		return;
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

	switch (ser->bpc) {
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

	if (ser->stop <= 2) {
		tio.c_cflag &= ~CSTOPB;
	}
	else {
		tio.c_cflag |= CSTOPB;
	}

	switch (ser->parity) {
	case 0:
		tio.c_cflag &= ~PARENB;
		break;

	case 1:
		tio.c_cflag |= PARENB;
		tio.c_cflag |= PARODD;
		break;

	case 2:
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		break;
	}

	mac_ser_fix_bps (&bps, &spd);

	if (spd == B0) {
		spd = B9600;
	}

	cfsetispeed (&tio, spd);
	cfsetospeed (&tio, spd);

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr (ser->fd, TCSANOW, &tio)) {
		return;
	}

	tcflush (ser->fd, TCIOFLUSH);

	return;
}

int mac_ser_set_fd (mac_ser_t *ser, int fd, int fdclose)
{
	if (ser->fd_close) {
		close (ser->fd);
	}

	ser->fd = fd;
	ser->fd_close = (fd != -1) && fdclose;

	return (0);
}

int mac_ser_set_dname (mac_ser_t *ser, const char *dname)
{
	int fd;

	fd = open (dname,
		O_RDWR | O_CREAT | O_NOCTTY | O_TRUNC,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
	);

	if (fd < 0) {
		return (1);
	}

	mac_ser_set_fd (ser, fd, 1);

	return (0);
}

#else

static
void mac_ser_process_output (mac_ser_t *ser)
{
	unsigned char c;

	if (e8530_out_empty (ser->scc, ser->chn)) {
		return;
	}

	while (e8530_out_empty (ser->scc, ser->chn) == 0) {
		c = e8530_send (ser->scc, ser->chn);

		if (ser->fp != NULL) {
			fputc (c, ser->fp);
		}
	}

	if (ser->fp != NULL) {
		fflush (ser->fp);
	}
}

static
void mac_ser_process_input (mac_ser_t *ser)
{
}

static
void mac_ser_status_check (mac_ser_t *ser)
{
}

static
void mac_ser_set_rts (void *ext, unsigned char val)
{
}

static
void mac_ser_set_comm (void *ext, unsigned long bps, unsigned parity, unsigned bpc, unsigned stop)
{
}

int mac_ser_set_fd (mac_ser_t *ser, int fd, int close)
{
	return (1);
}

int mac_ser_set_dname (mac_ser_t *ser, const char *dname)
{
	return (1);
}

#endif

static
void mac_ser_set_inp (void *ext, unsigned char val)
{
	mac_ser_process_input (ext);
}

static
void mac_ser_set_out (void *ext, unsigned char val)
{
	mac_ser_process_output (ext);
}


void mac_ser_set_scc (mac_ser_t *ser, e8530_t *scc, unsigned chn)
{
	ser->scc = scc;
	ser->chn = chn;

	e8530_set_inp_fct (scc, chn, ser, mac_ser_set_inp);
	e8530_set_out_fct (scc, chn, ser, mac_ser_set_out);
	e8530_set_rts_fct (scc, chn, ser, mac_ser_set_rts);
	e8530_set_comm_fct (scc, chn, ser, mac_ser_set_comm);

	e8530_set_cts (ser->scc, ser->chn, 1);
}

int mac_ser_set_fp (mac_ser_t *ser, FILE *fp, int close)
{
	if (ser->fp_close) {
		fclose (ser->fp);
	}

	ser->fp = fp;
	ser->fp_close = (fp != NULL) && close;

	return (0);
}

int mac_ser_set_fname (mac_ser_t *ser, const char *fname)
{
	FILE *fp;

	fp = fopen (fname, "wb");
	if (fp == NULL) {
		return (1);
	}

	if (mac_ser_set_fp (ser, fp, 1)) {
		fclose (fp);
		return (1);
	}

	return (0);
}

void mac_ser_clock (mac_ser_t *ser, unsigned n)
{
	mac_ser_process_output (ser);
	mac_ser_process_input (ser);
	mac_ser_status_check (ser);
}
