/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/speaker.c                                     *
 * Created:     2010-02-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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
#include "speaker.h"

#include <stdlib.h>

#include <drivers/sound/sound.h>


#ifndef DEBUG_SPEAKER
#define DEBUG_SPEAKER 0
#endif


static
void pc_speaker_play (pc_speaker_t *spk, uint16_t *buf, unsigned cnt)
{
	if (spk->lowpass_freq > 0) {
		snd_iir2_filter (&spk->iir, buf, buf, cnt, 1, 1);
	}

	snd_write (spk->drv, buf, cnt);
}

static
void pc_speaker_write (pc_speaker_t *spk, uint16_t val, unsigned long cnt)
{
	unsigned idx;

	if (spk->drv == NULL) {
		return;
	}

	idx = spk->buf_cnt;

	while (cnt > 0) {
		spk->buf[idx++] = val;

		if (idx >= PC_SPEAKER_BUF) {
			pc_speaker_play (spk, spk->buf, idx);

			idx = 0;
		}

		cnt -= 1;
	}

	spk->buf_cnt = idx;
}

static
void pc_speaker_flush (pc_speaker_t *spk)
{
	if (spk->buf_cnt == 0) {
		return;
	}

	pc_speaker_play (spk, spk->buf, spk->buf_cnt);

	snd_iir2_reset (&spk->iir);

	spk->buf_cnt = 0;
}

static
void pc_speaker_on (pc_speaker_t *spk)
{
#if DEBUG_SPEAKER >= 1
	pc_log_deb ("speaker on\n");
#endif

	spk->playing = 1;

	spk->timeout_val = 0x8000;
	spk->timeout_clk = 0;

	spk->sample_acc = 0x8000;

	spk->rem = 0;

	/* Fill the sound buffer a bit so we don't underrun immediately */
	pc_speaker_write (spk, 0, spk->srate / 8);
}

static
void pc_speaker_off (pc_speaker_t *spk)
{
#if DEBUG_SPEAKER >= 1
	pc_log_deb ("speaker off\n");
#endif

	spk->playing = 0;

	pc_speaker_flush (spk);
}

static
void pc_speaker_check (pc_speaker_t *spk)
{
	unsigned long  clk, tmp, acc;
	uint16_t       val;

	if (spk->speaker_msk == 0) {
		val = 0x8000;
	}
	else {
		val = spk->speaker_out ? spk->val_on : spk->val_off;
	}

	tmp = spk->get_clk (spk->get_clk_ext);
	clk = tmp - spk->clk;
	spk->clk = tmp;

	if (spk->playing == 0) {
		if (spk->timeout_val != val) {
			pc_speaker_on (spk);
		}
		return;
	}
	else if (spk->timeout_val == val) {
		spk->timeout_clk += clk;

		if (spk->timeout_clk > (2 * PCE_IBMPC_CLK2)) {
			pc_speaker_off (spk);
			return;
		}
	}
	else {
		spk->timeout_val = val;
		spk->timeout_clk = clk;
	}

	acc = spk->sample_acc;

	while (clk > 0) {
		/* about 3000 Hz cut-off */
		acc = (63 * acc + val) / 64;

		spk->rem += spk->srate;

		if (spk->rem >= PCE_IBMPC_CLK2) {
			pc_speaker_write (spk, acc ^ 0x8000, 1);
			spk->rem -= PCE_IBMPC_CLK2;
		}

		clk -= 1;
	}

	spk->sample_acc = acc;
}

void pc_speaker_init (pc_speaker_t *spk)
{
	spk->drv = NULL;

	spk->playing = 0;

	spk->speaker_msk = 0;
	spk->speaker_out = 0;

	spk->timeout_clk = 0;
	spk->timeout_val = 0x8000;

	spk->clk = 0;
	spk->rem = 0;

	spk->sample_acc = 0x8000;

	spk->srate = 44100;

	spk->lowpass_freq = 0;

	snd_iir2_init (&spk->iir);

	spk->buf_cnt = 0;

	pc_speaker_set_volume (spk, 500);
}

void pc_speaker_free (pc_speaker_t *spk)
{
	pc_speaker_flush (spk);

	if (spk->drv != NULL) {
		snd_close (spk->drv);
	}
}

pc_speaker_t *pc_speaker_new (void)
{
	pc_speaker_t *spk;

	spk = malloc (sizeof (pc_speaker_t));

	if (spk == NULL) {
		return (NULL);
	}

	pc_speaker_init (spk);

	return (spk);
}

void pc_speaker_del (pc_speaker_t *spk)
{
	if (spk != NULL) {
		pc_speaker_free (spk);
		free (spk);
	}
}

void pc_speaker_set_clk_fct (pc_speaker_t *spk, void *ext, void *fct)
{
	spk->get_clk_ext = ext;
	spk->get_clk = fct;
}

int pc_speaker_set_driver (pc_speaker_t *spk, const char *driver, unsigned long srate)
{
	if (spk->drv != NULL) {
		snd_close (spk->drv);
	}

	spk->drv = snd_open (driver);

	if (spk->drv == NULL) {
		return (1);
	}

	spk->srate = srate;

	if (snd_set_params (spk->drv, 1, srate, 1)) {
		snd_close (spk->drv);
		spk->drv = NULL;
		return (1);
	}

	return (0);
}

void pc_speaker_set_lowpass (pc_speaker_t *spk, unsigned long freq)
{
	spk->lowpass_freq = freq;

	snd_iir2_set_lowpass (&spk->iir, spk->lowpass_freq, spk->srate);
}

void pc_speaker_set_volume (pc_speaker_t *spk, unsigned vol)
{
	if (vol > 1000) {
		vol = 1000;
	}

	vol = (32767UL * vol) / 1000;

	spk->val_msk = 0;
	spk->val_on = 0x8000 + vol;
	spk->val_off = 0x8000 - vol;
}

void pc_speaker_set_msk (pc_speaker_t *spk, unsigned char val)
{
	pc_speaker_check (spk);

	spk->speaker_msk = (val != 0);
}

void pc_speaker_set_out (pc_speaker_t *spk, unsigned char val)
{
	pc_speaker_check (spk);

	spk->speaker_out = (val != 0);
}

void pc_speaker_clock (pc_speaker_t *spk, unsigned long cnt)
{
	pc_speaker_check (spk);
}
