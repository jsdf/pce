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

#include "serport.h"


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
	ser->io = base;

	ser->addr_shift = shift;

	e8250_init (&ser->uart);
	e8250_set_send_fct (&ser->uart, ser, ser_uart_check_out);
	e8250_set_recv_fct (&ser->uart, ser, ser_uart_check_inp);
	e8250_set_setup_fct (&ser->uart, ser, ser_uart_check_setup);
	e8250_set_check_fct (&ser->uart, ser, ser_uart_check_status);

	e8250_set_multichar (&ser->uart, 1, 1);
	e8250_set_bit_clk_div (&ser->uart, 10);

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

	ser->inp_idx = 0;
	ser->inp_cnt = 0;

	ser->out_idx = 0;
	ser->out_cnt = 0;

	ser->check_out = 0;
	ser->check_inp = 1;

	ser->cdrv = NULL;

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

	if (ser->cdrv != NULL) {
		chr_close (ser->cdrv);
	}
}

void ser_del (serport_t *ser)
{
	if (ser != NULL) {
		ser_free (ser);
		free (ser);
	}
}

mem_blk_t *ser_get_reg (serport_t *ser)
{
	return (&ser->port);
}

e8250_t *ser_get_uart (serport_t *ser)
{
	return (&ser->uart);
}

int ser_set_driver (serport_t *ser, const char *name)
{
	if (ser->cdrv != NULL) {
		chr_close (ser->cdrv);
	}

	ser->cdrv = chr_open (name);

	if (ser->cdrv == NULL) {
		return (1);
	}

	return (0);
}

int ser_set_log (serport_t *ser, const char *fname)
{
	if (ser->cdrv == NULL) {
		return (1);
	}

	return (chr_set_log (ser->cdrv, fname));
}

static
void ser_flush_output (serport_t *ser)
{
	unsigned cnt;

	if (ser->out_cnt == 0) {
		return;
	}

	cnt = chr_write (ser->cdrv, ser->out_buf + ser->out_idx, ser->out_cnt);

	ser->out_idx += cnt;
	ser->out_cnt -= cnt;

	if (ser->out_cnt == 0) {
		ser->out_idx = 0;
	}
}

static
void ser_process_output (serport_t *ser)
{
	ser_flush_output (ser);

	if (ser->out_cnt > 0) {
		return;
	}

	while (1) {
		if (e8250_get_out (&ser->uart, ser->out_buf + ser->out_idx + ser->out_cnt, 1)) {
			ser_flush_output (ser);
			break;
		}

		ser->out_cnt += 1;

		if ((ser->out_idx + ser->out_cnt) >= PCE_SERPORT_BUF) {
			ser_flush_output (ser);

			if (ser->out_cnt > 0) {
				break;
			}
		}
	}

	ser->check_out = ((e8250_out_empty (&ser->uart) == 0) || (ser->out_cnt > 0));
}

static
void ser_fill_input (serport_t *ser)
{
	unsigned idx, cnt;

	if (ser->inp_cnt == 0) {
		ser->inp_idx = 0;
	}

	idx = ser->inp_idx + ser->inp_cnt;
	cnt = PCE_SERPORT_BUF - idx;

	if (cnt == 0) {
		return;
	}

	cnt = chr_read (ser->cdrv, ser->inp_buf + idx, cnt);

	ser->inp_cnt += cnt;
}

static
void ser_process_input (serport_t *ser)
{
	while (e8250_inp_full (&ser->uart) == 0) {
		if (ser->inp_cnt == 0) {
			ser_fill_input (ser);

		}

		if (ser->inp_cnt == 0) {
			break;
		}

		e8250_receive (&ser->uart, ser->inp_buf[ser->inp_idx]);

		ser->inp_idx += 1;
		ser->inp_cnt -= 1;
	}

	ser->check_inp = (e8250_inp_full (&ser->uart) == 0);
}

static
void ser_uart_check_setup (serport_t *ser)
{
	unsigned val;

	ser->bps = e8250_get_bps (&ser->uart);
	ser->databits = e8250_get_databits (&ser->uart);
	ser->stopbits = e8250_get_stopbits (&ser->uart);
	ser->parity = e8250_get_parity (&ser->uart);

	chr_set_params (ser->cdrv, ser->bps, ser->databits, ser->parity, ser->stopbits);

	ser->dtr = e8250_get_dtr (&ser->uart);
	ser->rts = e8250_get_rts (&ser->uart);

	val = 0;
	val |= ser->dtr ? PCE_CHAR_DTR : 0;
	val |= ser->rts ? PCE_CHAR_RTS : 0;

	chr_set_ctl (ser->cdrv, val);
}

static
void ser_uart_check_status (serport_t *ser)
{
	unsigned val;

	if (chr_get_ctl (ser->cdrv, &val)) {
		val = PCE_CHAR_DSR | PCE_CHAR_CTS | PCE_CHAR_CD;
	}

	e8250_set_dsr (&ser->uart, (val & PCE_CHAR_DSR) != 0);
	e8250_set_cts (&ser->uart, (val & PCE_CHAR_CTS) != 0);
	e8250_set_dcd (&ser->uart, (val & PCE_CHAR_CD) != 0);
	e8250_set_ri (&ser->uart, (val & PCE_CHAR_RI) != 0);
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
