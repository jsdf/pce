/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim6502/console.c                                   *
 * Created:     2004-05-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "main.h"


void con_init (console_t *con, ini_sct_t *sct)
{
	unsigned long iobase;

	ini_get_uint32 (sct, "io", &iobase, 0xf000);

	con->io = mem_blk_new (iobase, 4, 0);
	con->io->ext = con;
	con->io->get_uint8 = (mem_get_uint8_f) &con_get_uint8;
	con->io->set_uint8 = (mem_set_uint8_f) &con_set_uint8;

	con->irq_ext = NULL;
	con->irq = NULL;

	con->msg_ext = NULL;

	con->status = 0x40;
	con->data_inp = 0;
	con->data_out = 0;

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
int con_rmv_char (console_t *con)
{
	if (con->buf_i == con->buf_j) {
		return (1);
	}

	con->buf_i = (con->buf_i + 1) % CON_BUF_CNT;

	if (con->buf_i == con->buf_j) {
		con->status &= 0x7f;

		if (con->irq != NULL) {
			con->irq (con->irq_ext, 0);
		}
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

	return (0);
}

unsigned char con_get_uint8 (console_t *con, unsigned long addr)
{
	if (addr == 0) {
		return (con->status);
	}

	if (addr == 1) {
		con_get_char (con, &con->data_inp);
		return (con->data_inp);
	}

	if (addr == 2) {
		return (con->data_out);
	}

	return (0);
}

void con_set_uint8 (console_t *con, unsigned long addr, unsigned char val)
{
	if (addr == 0) {
		if (val & 0x80) {
			con_rmv_char (con);
		}
	}
	else if (addr == 2) {
		con->data_out = val;
		fputc (val, stdout);
		fflush (stdout);
	}
}

void con_check (console_t *con)
{
	unsigned      i, n;
	unsigned char buf[8];
	ssize_t       r;

	if (!pce_fd_readable (0, 0)) {
		return;
	}

	r = read (0, buf, 8);
	if (r <= 0) {
		return;
	}

	n = (unsigned) r;


	if ((n == 1) && (buf[0] == 0)) {
		s6502_set_msg (con->msg_ext, "break", "stop");
		return;
	}
	else if ((n == 1) && (buf[0] == 0xe0)) {
		s6502_set_msg (con->msg_ext, "break", "abort");
		return;
	}

	for (i = 0; i < n; i++) {
		con_add_char (con, buf[i]);
	}
}
