/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/speaker.c                                        *
 * Created:     2022-02-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <devices/speaker.h>

#include <stdlib.h>

#include <drivers/sound/sound.h>


#ifndef DEBUG_SPEAKER
#define DEBUG_SPEAKER 0
#endif


static void spk_flush (speaker_t *spk);


void spk_init (speaker_t *spk, int level)
{
	spk->drv = NULL;

	spk->srate = 44100;
	spk->input_clock = 1000000;
	spk->speed_mul = 1;

	spk->enabled = 0;

	spk->speaker_inp = level;
	spk->speaker_val = 0;
	spk->sample_acc = 0;

	spk->timeout_val = 0;
	spk->timeout_clk = 0;
	spk->timeout_max = 5 * spk->input_clock;

	spk->clk = 0;
	spk->rem = 0;

	spk->lowpass_freq = 0;

	snd_iir2_init (&spk->iir);

	spk->buf_cnt = 0;

	spk_set_volume (spk, 500);
}

void spk_free (speaker_t *spk)
{
	spk_flush (spk);

	if (spk->drv != NULL) {
		snd_close (spk->drv);
	}
}

speaker_t *spk_new (int level)
{
	speaker_t *spk;

	if ((spk = malloc (sizeof (speaker_t))) == NULL) {
		return (NULL);
	}

	spk_init (spk, level);

	return (spk);
}

void spk_del (speaker_t *spk)
{
	if (spk != NULL) {
		spk_free (spk);
		free (spk);
	}
}

void spk_set_clock_fct (speaker_t *spk, void *ext, void *fct)
{
	spk->get_clock_ext = ext;
	spk->get_clock = fct;
}

void spk_set_input_clock (speaker_t *spk, unsigned long clk)
{
	spk->input_clock = clk;
	spk->timeout_max = 5 * clk;
}

void spk_set_speed (speaker_t *spk, unsigned mul)
{
	if (mul > 0) {
		spk->speed_mul = mul;
	}
}

int spk_set_driver (speaker_t *spk, const char *driver, unsigned long srate)
{
	if (spk->drv != NULL) {
		snd_close (spk->drv);
	}

	if ((spk->drv = snd_open (driver)) == NULL) {
		return (1);
	}

	spk->srate = srate;

	snd_iir2_set_lowpass (&spk->iir, spk->lowpass_freq, spk->srate);

	if (snd_set_params (spk->drv, 1, srate, 1)) {
		snd_close (spk->drv);
		spk->drv = NULL;
		return (1);
	}

	return (0);
}

void spk_set_lowpass (speaker_t *spk, unsigned long freq)
{
	spk->lowpass_freq = freq;

	snd_iir2_set_lowpass (&spk->iir, spk->lowpass_freq, spk->srate);
}

void spk_set_volume (speaker_t *spk, unsigned vol)
{
	if (vol > 1000) {
		vol = 1000;
	}

	vol = (32768UL * vol) / 1000;

	if (vol > 32767) {
		vol = 32767;
	}

	spk->val_p = 0x8000 + vol;
	spk->val_n = 0x8000 - vol;
}

void spk_set_input (speaker_t *spk, int val)
{
	spk_check (spk);

	val = (val != 0);

	if (spk->speaker_inp == val) {
		return;
	}

	spk->speaker_inp = val;
	spk->speaker_val = val ? spk->val_p : spk->val_n;

#if DEBUG_SPEAKER >= 1
	fprintf (stderr, "speaker input %d (%04X)\n",
		spk->speaker_inp, spk->speaker_val
	);
#endif
}

static
void spk_write (speaker_t *spk, uint16_t *buf, unsigned cnt)
{
	if (spk->lowpass_freq > 0) {
		snd_iir2_filter (&spk->iir, buf, buf, cnt, 1, 1);
	}

	snd_write (spk->drv, buf, cnt);
}

static
void spk_put_sample (speaker_t *spk, uint16_t val, unsigned long cnt)
{
	unsigned idx;

	if (spk->drv == NULL) {
		return;
	}

	idx = spk->buf_cnt;

	while (cnt > 0) {
		spk->buf[idx++] = val;

		if (idx >= SPEAKER_BUF) {
			spk_write (spk, spk->buf, idx);

			idx = 0;
		}

		cnt -= 1;
	}

	spk->buf_cnt = idx;
}

static
void spk_flush (speaker_t *spk)
{
	if (spk->buf_cnt == 0) {
		return;
	}

	spk_write (spk, spk->buf, spk->buf_cnt);

	snd_iir2_reset (&spk->iir);

	spk->buf_cnt = 0;
}

static
void spk_on (speaker_t *spk)
{
#if DEBUG_SPEAKER >= 1
	fprintf (stderr, "speaker on (%lu)\n", spk->input_clock);
#endif

	spk->enabled = 1;
	spk->sample_acc = 0;
	spk->timeout_val = spk->speaker_val;
	spk->timeout_clk = 0;
	spk->rem = 0;

	/* Fill the sound buffer a bit so we don't underrun immediately */
	spk_put_sample (spk, 0, spk->srate / 16);
}

static
void spk_off (speaker_t *spk)
{
#if DEBUG_SPEAKER >= 1
	fprintf (stderr, "speaker off\n");
#endif

	spk->enabled = 0;

	spk_flush (spk);
}

void spk_check (speaker_t *spk)
{
	unsigned long clk, tmp, acc;

	tmp = spk->get_clock (spk->get_clock_ext);
	clk = tmp - spk->clk;
	spk->clk = tmp;

	if (spk->enabled == 0) {
		if (spk->timeout_val != spk->speaker_val) {
			spk_on (spk);
		}
		else {
			return;
		}
	}

	if (spk->timeout_val == spk->speaker_val) {
		spk->timeout_clk += clk;

		if (spk->timeout_clk > spk->timeout_max) {
			spk_off (spk);
			return;
		}
	}
	else {
		spk->timeout_val = spk->speaker_val;
		spk->timeout_clk = clk;
	}

	acc = spk->sample_acc;

	while (clk >= spk->speed_mul) {
		acc = ((256 - 6) * acc + 6 * spk->speaker_val) / 256;

		spk->rem += spk->srate;

		if (spk->rem >= spk->input_clock) {
			spk_put_sample (spk, acc ^ 0x8000, 1);
			spk->rem -= spk->input_clock;

			if (spk->speaker_val < 0x8000) {
				spk->speaker_val = (255UL * spk->speaker_val + 0x8000 + 255) / 256;
			}
			else {
				spk->speaker_val = (255UL * spk->speaker_val + 0x8000) / 256;
			}
		}

		clk -= spk->speed_mul;
	}

	spk->clk -= clk;

	spk->sample_acc = acc;
}
