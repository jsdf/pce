/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/mouse.c                                       *
 * Created:     2003-08-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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
#include "mouse.h"

#include <stdlib.h>

#include <chipset/82xx/e8250.h>
#include <devices/memory.h>
#include <libini/libini.h>


void mse_init (mouse_t *mse, unsigned long base, ini_sct_t *sct)
{
	e8250_init (&mse->uart);

	e8250_set_setup_fct (&mse->uart, mse, mse_uart_setup);
	e8250_set_send_fct (&mse->uart, mse, mse_uart_out);
	e8250_set_recv_fct (&mse->uart, mse, mse_uart_inp);

	e8250_set_multichar (&mse->uart, 64, 64);

	mem_blk_init (&mse->port, base, 8, 0);
	mse->port.ext = &mse->uart;
	mse->port.get_uint8 = (mem_get_uint8_f) &e8250_get_uint8;
	mse->port.set_uint8 = (mem_set_uint8_f) &e8250_set_uint8;
	mse->port.get_uint16 = (mem_get_uint16_f) &e8250_get_uint16;
	mse->port.set_uint16 = (mem_set_uint16_f) &e8250_set_uint16;

	e8250_set_dsr (&mse->uart, 1);
	e8250_set_cts (&mse->uart, 1);

	if (ini_get_sint16 (sct, "speed_mul_x", &mse->scale_x[0], 1)) {
		ini_get_sint16 (sct, "speed_x_mul", &mse->scale_x[0], 1);
	}

	if (ini_get_sint16 (sct, "speed_div_x", &mse->scale_x[1], 1)) {
		ini_get_sint16 (sct, "speed_x_div", &mse->scale_x[1], 1);
	}

	if (ini_get_sint16 (sct, "speed_mul_y", &mse->scale_y[0], 1)) {
		ini_get_sint16 (sct, "speed_y_mul", &mse->scale_y[0], 1);
	}

	if (ini_get_sint16 (sct, "speed_div_y", &mse->scale_y[1], 1)) {
		ini_get_sint16 (sct, "speed_y_div", &mse->scale_y[1], 1);
	}

	if (mse->scale_x[1] == 0) {
		mse->scale_x[1] = 1;
	}

	if (mse->scale_y[1] == 0) {
		mse->scale_y[1] = 1;
	}

	mse->scale_x[2] = 0;
	mse->scale_y[2] = 0;

	mse->dtr = 0;
	mse->rts = 0;

	mse->dx = 0;
	mse->dy = 0;
	mse->button = 0;

	mse->reset_cntr = 0;
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
void mse_send_packet (mouse_t *mse)
{
	unsigned char val;
	int           dx, dy;
	unsigned char x, y;

	if (mse->dx < -127) {
		dx = -127;
	}
	else if (mse->dx > 127) {
		dx = 127;
	}
	else {
		dx = mse->dx;
	}

	if (mse->dy < -127) {
		dy = -127;
	}
	else if (mse->dy > 127) {
		dy = 127;
	}
	else {
		dy = mse->dy;
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
	val |= (mse->button & 0x01) ? 0x20 : 0x00;
	val |= (mse->button & 0x02) ? 0x10 : 0x00;
	val |= (y >> 4) & 0x0c;
	val |= (x >> 6) & 0x03;

	mse_receive (mse, val);
	mse_receive (mse, x & 0x3f);
	mse_receive (mse, y & 0x3f);

	mse->dx -= dx;
	mse->dy -= dy;
}

void mse_set (mouse_t *mse, int dx, int dy, unsigned but)
{
	int tx, ty;

	dx = mse->scale_x[0] * dx + mse->scale_x[2];
	tx = dx;
	dx = dx / mse->scale_x[1];
	mse->scale_x[2] = tx - mse->scale_x[1] * dx;

	dy = mse->scale_y[0] * dy + mse->scale_y[2];
	ty = dy;
	dy = dy / mse->scale_y[1];
	mse->scale_y[2] = ty - mse->scale_y[1] * dy;

	mse->dx += dx;
	mse->dy += dy;

	if (mse->button != but) {
		mse->button = but;
		mse_send_packet (mse);
	}
	else if ((mse->dx != 0) || (mse->dy != 0)) {
		if (e8250_inp_empty (&mse->uart)) {
			mse_send_packet (mse);
		}
	}
}

/* 8250 setup has changed */
void mse_uart_setup (mouse_t *mse, unsigned char val)
{
	int dtr, rts;

	dtr = e8250_get_dtr (&mse->uart);
	rts = e8250_get_rts (&mse->uart);

	e8250_get_inp_all (&mse->uart);

	if (rts && dtr) {
		if ((mse->dtr == 0) || (mse->rts == 0)) {
			mse->dx = 0;
			mse->dy = 0;
			mse->scale_x[2] = 0;
			mse->scale_y[2] = 0;
			mse->reset_cntr = PCE_IBMPC_CLK2 / 1000;
		}
	}

	mse->dtr = dtr;
	mse->rts = rts;
}

/* 8250 output buffer is not empty */
void mse_uart_out (mouse_t *mse, unsigned char val)
{
	unsigned char c;

	while (1) {
		if (e8250_get_out (&mse->uart, &c, 1)) {
			break;
		}
	}
}

/* 8250 input buffer is not full */
void mse_uart_inp (mouse_t *mse, unsigned char val)
{
}

void mse_clock (mouse_t *mse, unsigned long cnt)
{
	if (mse->reset_cntr > 0) {
		if (cnt >= mse->reset_cntr) {
			mse->reset_cntr = 0;
			mse_receive (mse, 'M');
			pc_log_deb ("reset mouse\n");
		}
		else {
			mse->reset_cntr -= cnt;
		}
	}

	e8250_clock (&mse->uart, cnt);
}
