/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim6502/console.c                                 *
 * Created:       2004-05-31 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-05-31 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>

#include "main.h"


void con_init (console_t *con, ini_sct_t *sct)
{
  unsigned long iobase;

  iobase = ini_get_lng_def (sct, "io", 0xf000U);

  con->io = mem_blk_new (iobase, 4, 0);
  con->io->ext = con;
  con->io->get_uint8 = (mem_get_uint8_f) &con_get_uint8;
  con->io->set_uint8 = (mem_set_uint8_f) &con_set_uint8;

  con->irq_ext = NULL;
  con->irq = NULL;

  con->brk_ext = NULL;
  con->brk = NULL;

  con->status = 0;

  con->buf_i = 0;
  con->buf_j = 0;
}

void con_free (console_t *con)
{
  mem_blk_del (con->io);
}

mem_blk_t *con_get_io (console_t *con)
{
  return (con->io);
}

static
int con_readable (int fd, int t)
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
int con_add_char (console_t *con, unsigned char c)
{
  unsigned j;

  j = (con->buf_j + 1) % CON_BUF_CNT;

  if (j == con->buf_i) {
    return (1);
  }

  con->buf[con->buf_j] = c;
  con->buf_j = j;

  con->status |= 0x80;

  if (con->irq != NULL) {
    con->irq (con->irq_ext, 1);
  }

  return (0);
}

static
int con_get_char (console_t *con, unsigned char *c)
{
  if (con->buf_i == con->buf_j) {
    return (1);
  }

  *c = con->buf[con->buf_i];

  con->buf_i = (con->buf_i + 1) % CON_BUF_CNT;

  if (con->buf_i == con->buf_j) {
    con->status &= 0x7f;

    if (con->irq != NULL) {
      con->irq (con->irq_ext, 0);
    }
  }

  return (0);
}

unsigned char con_get_uint8 (console_t *con, unsigned long addr)
{
  unsigned char val;

  if (addr == 0) {
    return (con->status);
  }

  if (addr == 1) {
    if (con_get_char (con, &val)) {
      return (0);
    }

    return (val);
  }

  return (0);
}

void con_set_uint8 (console_t *con, unsigned long addr, unsigned char val)
{
  if (addr == 2) {
    fputc (val, stdout);
    fflush (stdout);
  }
}

static
void con_break (console_t *con, unsigned char val)
{
  if (con->brk != NULL) {
    con->brk (con->brk_ext, val);
  }
}

void con_check (console_t *con)
{
  unsigned      i, n;
  unsigned char buf[8];
  ssize_t       r;

  if (!con_readable (0, 0)) {
    return;
  }

  r = read (0, buf, 8);
  if (r <= 0) {
    return;
  }

  n = (unsigned) r;


  if ((n == 1) && (buf[0] == 0)) {
    con_break (con, PCE_BRK_STOP);
    return;
  }
  else if ((n == 1) && (buf[0] == 0xe0)) {
    con_break (con, PCE_BRK_ABORT);
    return;
  }

  for (i = 0; i < n; i++) {
    con_add_char (con, buf[i]);
  }
}
