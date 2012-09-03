/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/speaker.c                                     *
 * Created:     2012-07-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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
void rc759_spk_play (rc759_speaker_t *spk, uint16_t *buf, unsigned cnt)
{
	if (spk->lowpass_freq > 0) {
		snd_iir2_filter (&spk->iir, buf, buf, cnt, 1, 1);
	}

	snd_write (spk->drv, buf, cnt);
}

static
void rc759_spk_write (rc759_speaker_t *spk, uint16_t val, unsigned long cnt)
{
	unsigned idx;

	if (spk->drv == NULL) {
		return;
	}

	idx = spk->buf_cnt;

	while (cnt > 0) {
		spk->buf[idx++] = val;

		if (idx >= RC759_SPEAKER_BUF) {
			rc759_spk_play (spk, spk->buf, idx);

			idx = 0;
		}

		cnt -= 1;
	}

	spk->buf_cnt = idx;
}

static
void rc759_spk_flush (rc759_speaker_t *spk)
{
	if (spk->buf_cnt == 0) {
		return;
	}

	rc759_spk_play (spk, spk->buf, spk->buf_cnt);

	snd_iir2_reset (&spk->iir);

	spk->buf_cnt = 0;
}

static
void rc759_spk_on (rc759_speaker_t *spk)
{
#if DEBUG_SPEAKER >= 1
	sim_log_deb ("speaker on (%lu)\n", spk->input_clock);
#endif

	spk->playing = 1;

	spk->timeout_val = 0x8000;
	spk->timeout_clk = 0;

	spk->sample_acc = 0x8000;

	spk->rem = 0;

	/* Fill the sound buffer a bit so we don't underrun immediately */
	rc759_spk_write (spk, 0, spk->srate / 8);
}

static
void rc759_spk_off (rc759_speaker_t *spk)
{
#if DEBUG_SPEAKER >= 1
	sim_log_deb ("speaker off\n");
#endif

	spk->playing = 0;

	rc759_spk_flush (spk);
}

static
void rc759_spk_check (rc759_speaker_t *spk)
{
	unsigned long  clk, tmp, acc;
	uint16_t       val;

	val = spk->speaker_out ? spk->val_on : spk->val_off;

	tmp = spk->get_clk (spk->get_clk_ext);
	clk = tmp - spk->clk;
	spk->clk = tmp;

	if (spk->playing == 0) {
		if (spk->timeout_val != val) {
			rc759_spk_on (spk);
		}
		return;
	}
	else if (spk->timeout_val == val) {
		spk->timeout_clk += clk;

		if (spk->timeout_clk > (1 * spk->input_clock)) {
			rc759_spk_off (spk);
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

		if (spk->rem >= spk->input_clock) {
			rc759_spk_write (spk, acc ^ 0x8000, 1);
			spk->rem -= spk->input_clock;
		}

		clk -= 1;
	}

	spk->sample_acc = acc;
}

void rc759_spk_init (rc759_speaker_t *spk)
{
	spk->drv = NULL;

	spk->playing = 0;

	spk->speaker_out = 0;

	spk->timeout_clk = 0;
	spk->timeout_val = 0x8000;

	spk->clk = 0;
	spk->rem = 0;

	spk->sample_acc = 0x8000;

	spk->srate = 44100;

	spk->input_clock = 1000000;

	spk->lowpass_freq = 0;

	snd_iir2_init (&spk->iir);

	spk->buf_cnt = 0;

	rc759_spk_set_volume (spk, 500);

	spk->timeout_val = spk->val_off;
}

void rc759_spk_free (rc759_speaker_t *spk)
{
	rc759_spk_flush (spk);

	if (spk->drv != NULL) {
		snd_close (spk->drv);
	}
}

rc759_speaker_t *rc759_spk_new (void)
{
	rc759_speaker_t *spk;

	spk = malloc (sizeof (rc759_speaker_t));

	if (spk == NULL) {
		return (NULL);
	}

	rc759_spk_init (spk);

	return (spk);
}

void rc759_spk_del (rc759_speaker_t *spk)
{
	if (spk != NULL) {
		rc759_spk_free (spk);
		free (spk);
	}
}

void rc759_spk_set_clk_fct (rc759_speaker_t *spk, void *ext, void *fct)
{
	spk->get_clk_ext = ext;
	spk->get_clk = fct;
}

void rc759_spk_set_input_clock (rc759_speaker_t *spk, unsigned long clk)
{
	spk->input_clock = clk;
}

int rc759_spk_set_driver (rc759_speaker_t *spk, const char *driver, unsigned long srate)
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

void rc759_spk_set_lowpass (rc759_speaker_t *spk, unsigned long freq)
{
	spk->lowpass_freq = freq;

	snd_iir2_set_lowpass (&spk->iir, spk->lowpass_freq, spk->srate);
}

void rc759_spk_set_volume (rc759_speaker_t *spk, unsigned vol)
{
	if (vol > 1000) {
		vol = 1000;
	}

	vol = (32767UL * vol) / 1000;

	spk->val_msk = 0;
	spk->val_on = 0x8000 + vol;
	spk->val_off = 0x8000 - vol;
}

void rc759_spk_set_out (rc759_speaker_t *spk, unsigned char val)
{
	rc759_spk_check (spk);

	spk->speaker_out = (val != 0);
}

void rc759_spk_clock (rc759_speaker_t *spk, unsigned long cnt)
{
	rc759_spk_check (spk);
}
