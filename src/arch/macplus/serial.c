/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/serial.c                                    *
 * Created:     2007-12-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2020 Hampa Hug <hampa@hampa.ch>                     *
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
#include "serial.h"

#include <stdlib.h>

#include <chipset/e8530.h>
#include <drivers/char/char.h>

#include <lib/string.h>


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

	ser->inp_idx = 0;
	ser->inp_cnt = 0;

	ser->out_idx = 0;
	ser->out_cnt = 0;

	ser->cdrv = NULL;
}

void mac_ser_free (mac_ser_t *ser)
{
	chr_close (ser->cdrv);
}

static
void mac_ser_flush_output (mac_ser_t *ser)
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
void mac_ser_process_output (mac_ser_t *ser)
{
	mac_ser_flush_output (ser);

	if (ser->out_cnt > 0) {
		return;
	}

	while (1) {
		if (e8530_out_empty (ser->scc, ser->chn)) {
			mac_ser_flush_output (ser);
			break;
		}

		ser->out_buf[ser->out_idx + ser->out_cnt] = e8530_send (ser->scc, ser->chn);
		ser->out_cnt += 1;

		if ((ser->out_idx + ser->out_cnt) >= MAC_SER_BUF) {
			mac_ser_flush_output (ser);

			if (ser->out_cnt > 0) {
				break;
			}
		}
	}
}

static
void mac_ser_fill_input (mac_ser_t *ser)
{
	unsigned idx, cnt;

	if (ser->inp_cnt == 0) {
		ser->inp_idx = 0;
	}

	idx = ser->inp_idx + ser->inp_cnt;
	cnt = MAC_SER_BUF - idx;

	if (cnt == 0) {
		return;
	}

	cnt = chr_read (ser->cdrv, ser->inp_buf + idx, cnt);

	ser->inp_cnt += cnt;
}

static
void mac_ser_process_input (mac_ser_t *ser)
{
	while (e8530_inp_full (ser->scc, ser->chn) == 0) {
		if (ser->inp_cnt == 0) {
			mac_ser_fill_input (ser);

		}

		if (ser->inp_cnt == 0) {
			break;
		}

		e8530_receive (ser->scc, ser->chn, ser->inp_buf[ser->inp_idx]);

		ser->inp_idx += 1;
		ser->inp_cnt -= 1;
	}
}

static
void mac_ser_status_check (mac_ser_t *ser)
{
	e8530_set_cts (ser->scc, ser->chn, 1);
}

static
void mac_ser_set_rts (void *ext, unsigned char val)
{
	unsigned  ctl;
	mac_ser_t *ser;

	ser = ext;

	if (ser->rts == val) {
		return;
	}

#ifdef DEBUG_SERIAL
	mac_log_deb ("serial %u: RTS=%d\n", ser->chn, val);
#endif

	ser->rts = (val != 0);

	ctl = PCE_CHAR_DTR;
	ctl |= (ser->rts ? PCE_CHAR_RTS : 0);

	chr_set_ctl (ser->cdrv, val);
}

static
void mac_ser_set_comm (void *ext, unsigned long bps, unsigned parity, unsigned bpc, unsigned stop)
{
	mac_ser_t *ser;

	ser = ext;

	stop = stop / 2;

	ser->bps = bps;
	ser->parity = parity;
	ser->bpc = bpc;
	ser->stop = stop;

#ifdef DEBUG_SERIAL
	mac_log_deb ("serial %u: setup (%lu/%u/%u%u)\n", ser->chn,
		ser->bps, ser->bpc, ser->parity, ser->stop
	);
#endif

	chr_set_params (ser->cdrv, bps, bpc, parity, stop);
}

int mac_ser_set_driver (mac_ser_t *ser, const char *name)
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

int mac_ser_set_file (mac_ser_t *ser, const char *fname)
{
	int  r;
	char *driver;

	driver = str_cat_alloc ("stdio:file=", fname);

	r = mac_ser_set_driver (ser, driver);

	free (driver);

	return (r);
}

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

void mac_ser_process (mac_ser_t *ser)
{
	mac_ser_process_output (ser);
	mac_ser_process_input (ser);
	mac_ser_status_check (ser);
}
