/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/covox.c                                       *
 * Created:     2010-08-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "covox.h"

#include <stdint.h>
#include <stdlib.h>

#include <drivers/sound/sound.h>


#define PC_COVOX_SRATE 7000

#define PC_COVOX_CLOCK PCE_IBMPC_CLK2


#ifndef DEBUG_COVOX
#define DEBUG_COVOX 0
#endif


static void pc_covox_flush (pc_covox_t *cov);


void pc_covox_init (pc_covox_t *cov)
{
	cov->drv = NULL;

	cov->disney = 0;

	cov->playing = 0;

	cov->data_val = 0;
	cov->ctrl_val = 0;

	cov->smp = 0;

	cov->fifo_i = 0;
	cov->fifo_n = 0;

	cov->timeout_clk = 0;
	cov->timeout_val = 0;

	cov->clk = 0;
	cov->rem = 0;

	cov->srate = 44100;

	cov->srate_div = 0;

	cov->lowpass_freq = 0;

	snd_iir2_init (&cov->iir);

	cov->buf_cnt = 0;

	pc_covox_set_volume (cov, 500);
}

void pc_covox_free (pc_covox_t *cov)
{
	pc_covox_flush (cov);

	if (cov->drv != NULL) {
		snd_close (cov->drv);
	}
}

pc_covox_t *pc_covox_new (void)
{
	pc_covox_t *cov;

	if ((cov = malloc (sizeof (pc_covox_t))) == NULL) {
		return (NULL);
	}

	pc_covox_init (cov);

	return (cov);
}

void pc_covox_del (pc_covox_t *cov)
{
	if (cov != NULL) {
		pc_covox_free (cov);
		free (cov);
	}
}


void pc_covox_set_clk_fct (pc_covox_t *cov, void *ext, void *fct)
{
	cov->get_clk_ext = ext;
	cov->get_clk = fct;
}

void pc_covox_set_mode (pc_covox_t *cov, unsigned mode)
{
	if (mode == 0) {
		cov->disney = 0;
	}
	else {
		cov->disney = 1;
	}
}

int pc_covox_set_driver (pc_covox_t *cov, const char *driver, unsigned long srate)
{
	if (cov->drv != NULL) {
		snd_close (cov->drv);
	}

	if ((cov->drv = snd_open (driver)) == NULL) {
		return (1);
	}

	cov->srate = srate;

	if (snd_set_params (cov->drv, 1, srate, 1)) {
		snd_close (cov->drv);
		cov->drv = NULL;
		return (1);
	}

	return (0);
}

void pc_covox_set_lowpass (pc_covox_t *cov, unsigned long freq)
{
	cov->lowpass_freq = freq;

	snd_iir2_set_lowpass (&cov->iir, cov->lowpass_freq, cov->srate);
}

void pc_covox_set_volume (pc_covox_t *cov, unsigned vol)
{
	if (vol > 1000) {
		vol = 1000;
	}

	cov->vol = (257UL * vol) / 1000;
}


static
void pc_covox_play (pc_covox_t *cov, uint16_t *buf, unsigned cnt)
{
	if (cov->lowpass_freq > 0) {
		snd_iir2_filter (&cov->iir, buf, buf, cnt, 1, 1);
	}

	snd_write (cov->drv, buf, cnt);
}

static
void pc_covox_write (pc_covox_t *cov, uint16_t val, unsigned long cnt)
{
	unsigned idx;

	if (cov->drv == NULL) {
		return;
	}

	idx = cov->buf_cnt;

	while (cnt > 0) {
		cov->buf[idx++] = val;

		if (idx >= PC_COVOX_BUF) {
			pc_covox_play (cov, cov->buf, idx);

			idx = 0;
		}

		cnt -= 1;
	}

	cov->buf_cnt = idx;
}

static
void pc_covox_flush (pc_covox_t *cov)
{
	if (cov->buf_cnt == 0) {
		return;
	}

	pc_covox_play (cov, cov->buf, cov->buf_cnt);

	snd_iir2_reset (&cov->iir);

	cov->buf_cnt = 0;
}

static inline
uint16_t pc_covox_get_smp (pc_covox_t *cov, unsigned val)
{
	unsigned ret;

	ret = cov->vol * ((long) (val & 0xff) - 0x80);

	return (ret & 0xffff);
}

static
int pc_covox_fifo_add (pc_covox_t *cov, unsigned char val)
{
	unsigned idx;

	if (cov->fifo_n >= PC_COVOX_FIFO) {
		return (1);
	}

	idx = (cov->fifo_i + cov->fifo_n) % PC_COVOX_FIFO;

	cov->fifo[idx] = val;

	cov->fifo_n += 1;

	return (0);
}

static
int pc_covox_fifo_next (pc_covox_t *cov)
{
	long val;

	if (cov->fifo_n == 0) {
		cov->smp = 0;
		return (1);
	}

	val = cov->fifo[cov->fifo_i];

	cov->smp = pc_covox_get_smp (cov, val);

	cov->fifo_i = (cov->fifo_i + 1) % PC_COVOX_FIFO;
	cov->fifo_n -= 1;

	return (0);
}

static
void pc_covox_on (pc_covox_t *cov)
{
#if DEBUG_COVOX >= 1
	pc_log_deb ("covox on\n");
#endif

	cov->playing = 1;

	cov->timeout_val = cov->data_val;
	cov->timeout_clk = 0;

	cov->rem = 0;

	cov->srate_div = 0;

	/* Fill the sound buffer a bit so we don't underrun immediately */
	pc_covox_write (cov, 0, cov->srate / 8);
}

static
void pc_covox_off (pc_covox_t *cov)
{
#if DEBUG_COVOX >= 1
	pc_log_deb ("covox off\n");
#endif

	cov->playing = 0;

	pc_covox_flush (cov);
}

static
void pc_covox_check_disney (pc_covox_t *cov)
{
	unsigned long clk, cnt, tmp;

	tmp = cov->get_clk (cov->get_clk_ext);
	clk = tmp - cov->clk;
	cov->clk = tmp;

	if (cov->playing == 0) {
		if (cov->fifo_n == 0) {
			return;
		}

		pc_covox_on (cov);

		pc_covox_fifo_next (cov);
	}

	tmp = cov->rem + cov->srate * clk;
	cov->rem = tmp % PC_COVOX_CLOCK;
	cnt = tmp / PC_COVOX_CLOCK;

	while (cnt > 0) {
		pc_covox_write (cov, cov->smp, 1);

		cov->srate_div += PC_COVOX_SRATE;

		while (cov->srate_div >= cov->srate) {
			if (pc_covox_fifo_next (cov)) {
				cov->timeout_clk += 1;

				if (cov->timeout_clk > (2 * PC_COVOX_SRATE)) {
					pc_covox_off (cov);
					return;
				}
			}
			else {
				cov->timeout_clk = 0;
			}

			cov->srate_div -= cov->srate;
		}

		cnt -= 1;
	}
}

static
void pc_covox_check_covox (pc_covox_t *cov)
{
	unsigned long clk, cnt, tmp;

	tmp = cov->get_clk (cov->get_clk_ext);
	clk = tmp - cov->clk;
	cov->clk = tmp;

	if (cov->playing == 0) {
		if (cov->timeout_val != cov->data_val) {
			pc_covox_on (cov);
		}
		return;
	}
	else if (cov->timeout_val == cov->data_val) {
		cov->timeout_clk += clk;

		if (cov->timeout_clk > (2 * PC_COVOX_CLOCK)) {
			pc_covox_off (cov);
			return;
		}
	}
	else {
		cov->timeout_val = cov->data_val;
		cov->timeout_clk = clk;
	}

	tmp = cov->rem + cov->srate * clk;
	cov->rem = tmp % PC_COVOX_CLOCK;
	cnt = tmp / PC_COVOX_CLOCK;

	if (cnt > 0) {
		pc_covox_write (cov, cov->smp, cnt);
	}
}

void pc_covox_set_data (pc_covox_t *cov, unsigned char val)
{
	if (cov->disney == 0) {
		pc_covox_check_covox (cov);

		cov->smp = pc_covox_get_smp (cov, val);
	}

	cov->data_val = val;
}

void pc_covox_set_ctrl (pc_covox_t *cov, unsigned char val)
{
	/* SEL */
	if ((cov->ctrl_val ^ val) & val & 0x08) {
		pc_covox_fifo_add (cov, cov->data_val);
	}

#if DEBUG_COVOX >= 2
	pc_log_deb ("covox: set ctrl: 0x%02x\n", val);
#endif

	cov->ctrl_val = val;
}

unsigned char pc_covox_get_status (pc_covox_t *cov)
{
	unsigned char val;

	val = 0x00;

	if (cov->fifo_n >= PC_COVOX_FIFO) {
		/* INIT */
		if (cov->ctrl_val & 0x04) {
			/* ACK */
			val |= 0x40;
		}
	}

#if DEBUG_COVOX >= 2
	pc_log_deb ("covox: get status: 0x%02x\n", val);
#endif

	return (val);
}

void pc_covox_clock (pc_covox_t *cov, unsigned long cnt)
{
	if (cov->disney) {
		pc_covox_check_disney (cov);
	}
	else {
		pc_covox_check_covox (cov);
	}
}
