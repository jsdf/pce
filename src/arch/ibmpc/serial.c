/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/serial.c                                         *
 * Created:       2003-09-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-20 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: serial.c,v 1.1 2003/12/20 01:01:34 hampa Exp $ */


#include <stdio.h>

#include "pce.h"

/* fix this */
#ifdef HAVE_SYS_POLL_H
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#ifdef PCE_HOST_LINUX
#include <sys/ioctl.h>
#include <linux/serial_reg.h>
#endif


serial_t *ser_new (unsigned base)
{
  serial_t *ser;

  ser = (serial_t *) malloc (sizeof (serial_t));
  if (ser == NULL) {
    return (NULL);
  }

  ser->io = base;

  e8250_init (&ser->uart);

  ser->uart.setup_ext = ser;
  ser->uart.setup = (e8250_setup_f) ser_setup;
  ser->uart.send_ext = ser;
  ser->uart.send = (e8250_send_f) ser_send;
  ser->uart.recv_ext = ser;
  ser->uart.recv = (e8250_recv_f) ser_recv;

  ser->prt = mem_blk_new (base, 8, 0);
  ser->prt->ext = &ser->uart;
  ser->prt->get_uint8 = (geta_uint8_f) &e8250_get_uint8;
  ser->prt->set_uint8 = (seta_uint8_f) &e8250_set_uint8;
  ser->prt->get_uint16 = (geta_uint16_f) &e8250_get_uint16;
  ser->prt->set_uint16 = (seta_uint16_f) &e8250_set_uint16;

  ser->bps = 2400;
  ser->databits = 8;
  ser->stopbits = 1;
  ser->parity = E8250_PARITY_N;

  ser->dtr = 0;
  ser->rts = 0;

  ser->fd = -1;
  ser->close = 0;

  return (ser);
}

void ser_del (serial_t *ser)
{
  if (ser != NULL) {
    if (ser->close) {
      close (ser->fd);
    }

    e8250_free (&ser->uart);

    mem_blk_del (ser->prt);

    free (ser);
  }
}

#ifdef HAVE_TERMIOS_H

int ser_line_setup (serial_t *ser)
{
  struct termios tio;
  speed_t        spd;

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
  tio.c_lflag &= ~ECHOPRT;      /* Enable echo */
  tio.c_lflag &= ~ECHOK;        /* Enable echo */
  tio.c_lflag &= ~ECHONL;       /* Enable echo */
  tio.c_lflag &= ~ECHOCTL;      /* Enable echo */
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
      tio.c_cflag |= ~PARENB;
      break;

    case E8250_PARITY_E:
      tio.c_cflag |= ~PARENB;
      tio.c_cflag &= ~PARODD;
      break;

    case E8250_PARITY_O:
      tio.c_cflag |= ~PARENB;
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
  else if (ser->bps < ((38400 + 57600) / 2)) {
    spd = B19200;
  }
  else {
    spd = B115200;
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

#else

int ser_line_setup (serial_t *ser)
{
  return (0);
}

#endif


#ifdef PCE_HOST_LINUX

void ser_modem_setup (serial_t *ser)
{
  int val;

  ioctl (ser->fd, TIOCMGET, &val);
  val &= ~(UART_MCR_DTR | UART_MCR_RTS);
  val |= e8250_get_dtr (&ser->uart) ? UART_MCR_DTR : 0;
  val |= e8250_get_rts (&ser->uart) ? UART_MCR_RTS : 0;
  ioctl (ser->fd, TIOCMSET, &val);
}

#else

void ser_modem_setup (serial_t *ser)
{
}

#endif


void ser_set_fd (serial_t *ser, int fd, int cls)
{
  if (ser->close) {
    close (ser->fd);
  }

  ser->fd = fd;
  ser->close = (fd < 0) ? 0 : cls;

  ser_line_setup (ser);
  ser_modem_setup (ser);
}

int ser_set_fname (serial_t *ser, const char *fname)
{
  int fd;

  fd = open (fname,
    O_RDWR | O_CREAT | O_NOCTTY | O_TRUNC,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
  );

  if (fd < 0) {
    return (1);
  }

  ser_set_fd (ser, fd, 1);

  return (0);
}

void ser_setup (serial_t *ser, unsigned char val)
{
  int           chg;
  unsigned long bps;
  unsigned      data, stop, parity;
  int           dtr, rts;

  chg = 0;

  bps = e8250_get_bps (&ser->uart);
  if (bps != ser->bps) {
    ser->bps = bps;
    chg = 1;
  }

  data = e8250_get_databits (&ser->uart);
  if (data != ser->databits) {
    ser->databits = data;
    chg = 1;
  }

  stop = e8250_get_stopbits (&ser->uart);
  if (stop != ser->stopbits) {
    ser->stopbits = stop;
    chg = 1;
  }

  parity = e8250_get_parity (&ser->uart);
  if (parity != ser->parity) {
    ser->parity = parity;
    chg = 1;
  }

  if (chg) {
    ser_line_setup (ser);
  }

  chg = 0;

  dtr = e8250_get_dtr (&ser->uart);
  if (dtr != ser->dtr) {
    ser->dtr = dtr;
    chg = 1;
  }

  rts = e8250_get_rts (&ser->uart);
  if (rts != ser->rts) {
    ser->rts = rts;
    chg = 1;
  }

  if (chg) {
    ser_modem_setup (ser);
  }
}

void ser_send (serial_t *ser, unsigned char val)
{
  int           r;
  unsigned char c;

  if (ser->fd < 0) {
    return;
  }

  while (1) {
#ifdef HAVE_SYS_POLL_H
    {
      struct pollfd pfd[1];

      pfd[0].fd = ser->fd;
      pfd[0].events = POLLOUT;

      r = poll (pfd, 1, 0);
      if (r < 0) {
        return;
      }

      if ((pfd[0].revents & POLLOUT) == 0) {
        return;
      }
    }
#endif

    if (e8250_get_out (&ser->uart, &c)) {
      return;
    }

    write (ser->fd, &c, 1);
  }
}

void ser_recv (serial_t *ser, unsigned char val)
{
}

void ser_clock (serial_t *ser, unsigned long clk)
{
  if (ser->fd < 0) {
    return;
  }

  ser_send (ser, 1);

  while (1) {
    if (e8250_inp_full (&ser->uart)) {
      break;
    }

#ifdef HAVE_SYS_POLL_H
    {
      int           r;
      unsigned char c;
      struct pollfd pfd[1];

      pfd[0].fd = ser->fd;
      pfd[0].events = POLLIN;

      r = poll (pfd, 1, 0);
      if (r < 0) {
        break;
      }

      if ((pfd[0].revents & POLLIN) == 0) {
        break;
      }

      read (ser->fd, &c, 1);

      if (e8250_set_inp (&ser->uart, c)) {
        break;
      }
    }
#else
    break;
#endif
  }

  e8250_clock (&ser->uart, clk);
}
