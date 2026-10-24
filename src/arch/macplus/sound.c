/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/sound.c                                     *
 * Created:     2008-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2020 Hampa Hug <hampa@hampa.ch>                     *
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
#include "sound.h"

#include <stdlib.h>
#include <string.h>

#include <drivers/sound/sound.h>


#define MAC_SOUND_CLK 352

#define MAC_SOUND_SILENCE 60

#ifndef DEBUG_SOUND
#define DEBUG_SOUND 0
#endif


void mac_sound_init (mac_sound_t *ms)
{
	ms->drv = NULL;

	ms->sbuf = NULL;

	ms->idx = 0;
	ms->cnt = 0;
	ms->clk = 0;

	ms->enable = 0;
	ms->volume = 0;

	ms->lowpass_freq = 8000;

	snd_iir2_init (&ms->iir);

	ms->last_val = 0x8000;
	ms->changed = 0;
	ms->silence_cnt = MAC_SOUND_SILENCE;
}

mac_sound_t *mac_sound_new (void)
{
	mac_sound_t *ms;

	ms = malloc (sizeof (mac_sound_t));

	if (ms == NULL) {
		return (NULL);
	}

	mac_sound_init (ms);

	return (ms);
}

void mac_sound_free (mac_sound_t *ms)
{
	if (ms->drv != NULL) {
		snd_close (ms->drv);
	}
}

void mac_sound_del (mac_sound_t *ms)
{
	if (ms != NULL) {
		mac_sound_free (ms);
		free (ms);
	}
}

void mac_sound_set_sbuf (mac_sound_t *ms, const unsigned char *sbuf)
{
	ms->sbuf = sbuf;
}

void mac_sound_set_lowpass (mac_sound_t *ms, unsigned freq)
{
	ms->lowpass_freq = freq;

	snd_iir2_set_lowpass (&ms->iir, freq, 22255);
}

void mac_sound_set_volume (mac_sound_t *ms, unsigned vol)
{
#if DEBUG_SOUND >= 1
	mac_log_deb ("sound: volume=%u\n", vol);
#endif

	ms->volume = vol;
}

void mac_sound_set_enable (mac_sound_t *ms, int val)
{
	val = (val != 0);

	if (ms->enable == val) {
		return;
	}

#if DEBUG_SOUND >= 1
	mac_log_deb ("sound: enable=%d\n", val);
#endif

	ms->enable = val;
}

int mac_sound_set_driver (mac_sound_t *ms, const char *driver)
{
	if (ms->drv != NULL) {
		snd_close (ms->drv);
	}

	ms->drv = snd_open (driver);

	if (ms->drv == NULL) {
		return (1);
	}

	if (snd_set_params (ms->drv, 1, 22255, 0)) {
		snd_close (ms->drv);
		ms->drv = NULL;
		return (1);
	}

	return (0);
}

static
void mac_sound_get_bytes (mac_sound_t *ms, unsigned cnt)
{
	unsigned val, div;

	if (ms->cnt >= 370) {
		return;
	}

	if (ms->sbuf == NULL) {
		return;
	}

	div = 8 - ms->volume;

	while ((cnt > 0) && (ms->cnt < 370)) {
		if (ms->enable) {
			val = ms->sbuf[2 * ms->idx];
			val = val << 8;

			if (val < 32768) {
				val = 32768 - (32768 - val) / div;
			}
			else {
				val = 32768 + (val - 32768) / div;
			}
		}
		else {
			val = 0x8000;
		}

		ms->idx += 1;
		if (ms->idx >= 370) {
			ms->idx = 0;
		}

		if (val != ms->last_val) {
			ms->last_val = val;
			ms->changed = 1;
		}

		ms->buf[ms->cnt] = val;

		ms->cnt += 1;
		cnt -= 1;
	}
}

static
void mac_sound_fill (mac_sound_t *ms, unsigned cnt)
{
	if ((cnt == 0) || (ms->cnt >= 370)) {
		return;
	}

	if (ms->last_val != 0x8000) {
		ms->changed = 1;
	}

	while ((cnt > 0) && (ms->cnt < 370)) {
		ms->buf[ms->cnt] = 0x8000;

		ms->idx += 1;
		if (ms->idx >= 370) {
			ms->idx = 0;
		}

		ms->cnt += 1;
		cnt -= 1;
	}

	ms->last_val = 0x8000;
}

static
void mac_sound_speaker_on (mac_sound_t *ms)
{
	unsigned i;
	uint16_t buf[1024];

#if DEBUG_SOUND >= 1
	mac_log_deb ("sound: output on\n");
#endif

	for (i = 0; i < 1024; i++) {
		buf[i] = 0x8000;
	}

	for (i = 0; i < 4; i++) {
		snd_write (ms->drv, buf, 1024);
	}
}

static
void mac_sound_speaker_off (mac_sound_t *ms)
{
#if DEBUG_SOUND >= 1
	mac_log_deb ("sound: output off\n");
#endif
}

void mac_sound_vbl (mac_sound_t *ms)
{
	if (ms->sbuf == NULL) {
		return;
	}

	if (ms->drv == NULL) {
		return;
	}

	if (ms->cnt < 370) {
		if (ms->enable) {
			mac_sound_get_bytes (ms, 370 - ms->cnt);
		}
		else if ((ms->silence_cnt < MAC_SOUND_SILENCE) || (ms->cnt > 0)) {
			mac_sound_fill (ms, 370 - ms->cnt);
		}
	}

	if (ms->changed) {
		if (ms->silence_cnt >= MAC_SOUND_SILENCE) {
			mac_sound_speaker_on (ms);
		}

		ms->silence_cnt = 0;
	}
	else {
		if (ms->silence_cnt < MAC_SOUND_SILENCE) {
			ms->silence_cnt += 1;

			if (ms->silence_cnt == MAC_SOUND_SILENCE) {
				mac_sound_speaker_off (ms);
			}
		}
	}

	if (ms->silence_cnt < MAC_SOUND_SILENCE) {
		if (ms->lowpass_freq > 0) {
			snd_iir2_filter (&ms->iir, ms->buf, ms->buf, ms->cnt, 1, 0);
		}

		snd_write (ms->drv, ms->buf, ms->cnt);
	}

	ms->changed = 0;

	ms->idx = 16;
	ms->cnt = 0;
	ms->clk = 0;
}

void mac_sound_clock (mac_sound_t *ms, unsigned long n)
{
	unsigned cnt;

	if (ms->enable == 0) {
		return;
	}

	if (ms->sbuf == NULL) {
		return;
	}

	if (ms->cnt >= 370) {
		return;
	}

	ms->clk += n;

	if (ms->clk >= MAC_SOUND_CLK) {
		cnt = ms->clk / MAC_SOUND_CLK;
		ms->clk %= MAC_SOUND_CLK;

		mac_sound_get_bytes (ms, cnt);
	}
}
