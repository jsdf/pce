/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/mouse.c                                     *
 * Created:       2003-08-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2008 Hampa Hug <hampa@hampa.ch>                   *
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


#include "main.h"


void mse_init (mouse_t *mse, unsigned long base, ini_sct_t *sct)
{
	e8250_init (&mse->uart);

	e8250_set_setup_fct (&mse->uart, mse, mse_uart_setup);
	e8250_set_send_fct (&mse->uart, mse, mse_uart_out);
	e8250_set_recv_fct (&mse->uart, mse, mse_uart_inp);

	mem_blk_init (&mse->port, base, 8, 0);
	mse->port.ext = &mse->uart;
	mse->port.get_uint8 = (mem_get_uint8_f) &e8250_get_uint8;
	mse->port.set_uint8 = (mem_set_uint8_f) &e8250_set_uint8;
	mse->port.get_uint16 = (mem_get_uint16_f) &e8250_get_uint16;
	mse->port.set_uint16 = (mem_set_uint16_f) &e8250_set_uint16;

	e8250_set_dsr (&mse->uart, 1);
	e8250_set_cts (&mse->uart, 1);

	ini_get_sint16 (sct, "speed_x_mul", &mse->fct_x[0], 1);
	ini_get_sint16 (sct, "speed_x_div", &mse->fct_x[1], 1);
	ini_get_sint16 (sct, "speed_y_mul", &mse->fct_y[0], 1);
	ini_get_sint16 (sct, "speed_y_div", &mse->fct_y[1], 1);

	mse->dtr = 0;
	mse->rts = 0;

	mse->accu_ok = 0;
}

mouse_t *mse_new (unsigned long base, ini_sct_t *sct)
{
	mouse_t *mse;

	mse = malloc (sizeof (mouse_t));
	if (mse == NULL) {
		return (NULL);
	}

	mse_init (mse, base, sct);

	return (mse);
}

void mse_free (mouse_t *mse)
{
	mem_blk_free (&mse->port);
	e8250_free (&mse->uart);
}

void mse_del (mouse_t *mse)
{
	if (mse != NULL) {
		mse_free (mse);
		free (mse);
	}
}

mem_blk_t *mse_get_reg (mouse_t *mse)
{
	return (&mse->port);
}

static
void mse_receive (mouse_t *mse, unsigned char val)
{
	e8250_receive (&mse->uart, val);
}

static
void mse_accu_check (mouse_t *mse)
{
	unsigned char val;
	int           dx, dy;
	unsigned char x, y;

	if (mse->accu_ok == 0) {
		return;
	}

	if (mse->accu_dx < -127) {
		dx = -127;
	}
	else if (mse->accu_dx > 127) {
		dx = 127;
	}
	else {
		dx = mse->accu_dx;
	}

	if (mse->accu_dy < -127) {
		dy = -127;
	}
	else if (mse->accu_dy > 127) {
		dy = 127;
	}
	else {
		dy = mse->accu_dy;
	}

	if (dx < 0) {
		x = -dx;
		x = (~x + 1) & 0xff;
	}
	else {
		x = dx;
	}

	if (dy < 0) {
		y = -dy;
		y = (~y + 1) & 0xff;
	}
	else {
		y = dy;
	}

	val = 0x40;
	val |= (mse->accu_b & 0x01) ? 0x20 : 0x00;
	val |= (mse->accu_b & 0x02) ? 0x10 : 0x00;
	val |= (y >> 4) & 0x0c;
	val |= (x >> 6) & 0x03;

	mse_receive (mse, val);
	mse_receive (mse, x & 0x3f);
	mse_receive (mse, y & 0x3f);

	mse->accu_dx -= dx;
	mse->accu_dy -= dy;

	mse->accu_ok = ((mse->accu_dx != 0) || (mse->accu_dy != 0));
}

void mse_set (mouse_t *mse, int dx, int dy, unsigned but)
{
	dx = (mse->fct_x[0] * dx) / mse->fct_x[1];
	dy = (mse->fct_y[0] * dy) / mse->fct_y[1];

	if (mse->accu_ok) {
		if (mse->accu_b != but) {
			mse_accu_check (mse);
		}

		mse->accu_ok = 1;
		mse->accu_dx += dx;
		mse->accu_dy += dy;
		mse->accu_b = but;
	}
	else {
		mse->accu_ok = 1;
		mse->accu_dx = dx;
		mse->accu_dy = dy;
		mse->accu_b = but;
	}

	if (e8250_inp_empty (&mse->uart)) {
		mse_accu_check (mse);
	}
}

/* 8250 setup has changed */
void mse_uart_setup (mouse_t *mse, unsigned char val)
{
	int dtr, rts;

	dtr = e8250_get_dtr (&mse->uart);
	rts = e8250_get_rts (&mse->uart);

	if (rts != dtr) {
		e8250_get_inp_all (&mse->uart);
	}
	else if (dtr == 0) {
		e8250_get_inp_all (&mse->uart);
	}
	else if ((mse->dtr == 0) || (mse->rts == 0)) {
		e8250_get_inp_all (&mse->uart);

		/* this should not be necessary */
		e8250_get_uint8 (&mse->uart, 0);

		mse_receive (mse, 'M');
	}

	mse->dtr = dtr;
	mse->rts = rts;
}

/* 8250 output buffer is not empty */
void mse_uart_out (mouse_t *mse, unsigned char val)
{
	unsigned char c;

	while (1) {
		if (e8250_get_out (&mse->uart, &c)) {
			break;
		}
	}
}

/* 8250 input buffer is not full */
void mse_uart_inp (mouse_t *mse, unsigned char val)
{
}
