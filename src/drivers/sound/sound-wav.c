/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/sound-wav.c                                *
 * Created:     2009-10-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/options.h>
#include <drivers/sound/sound.h>
#include <drivers/sound/sound-wav.h>

#define WAVE_RIFF 0x46464952
#define WAVE_WAVE 0x45564157
#define WAVE_FMT  0x20746d66
#define WAVE_DATA 0x61746164


static
void wav_set_uint16 (void *buf, unsigned i, unsigned v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = v & 0xff;
	tmp[1] = (v >> 8) & 0xff;
}

static
void wav_set_uint32 (void *buf, unsigned i, unsigned long v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = v & 0xff;
	tmp[1] = (v >> 8) & 0xff;
	tmp[2] = (v >> 16) & 0xff;
	tmp[3] = (v >> 24) & 0xff;
}

static
int snd_wav_write_header (sound_drv_t *sdrv)
{
	unsigned char buf[64];

	if (fseek (sdrv->wav_fp, 0, SEEK_SET)) {
		return (1);
	}

	wav_set_uint32 (buf, 0, WAVE_RIFF);
	wav_set_uint32 (buf, 4, sdrv->wav_cnt + 36);
	wav_set_uint32 (buf, 8, WAVE_WAVE);

	wav_set_uint32 (buf, 12, WAVE_FMT);
	wav_set_uint32 (buf, 16, 16);
	wav_set_uint16 (buf, 20, 0x0001);
	wav_set_uint16 (buf, 22, sdrv->channels);
	wav_set_uint32 (buf, 24, sdrv->sample_rate);
	wav_set_uint32 (buf, 28, 2 * sdrv->channels * sdrv->sample_rate);
	wav_set_uint16 (buf, 32, 2 * sdrv->channels);
	wav_set_uint16 (buf, 34, 16);

	wav_set_uint32 (buf, 36, WAVE_DATA);
	wav_set_uint32 (buf, 40, sdrv->wav_cnt);

	if (fwrite (buf, 1, 44, sdrv->wav_fp) != 44) {
		return (1);
	}

	if (fseek (sdrv->wav_fp, sdrv->wav_cnt + 44, SEEK_SET)) {
		return (1);
	}

	return (0);
}

void snd_wav_close (sound_drv_t *sdrv)
{
	if (sdrv->wav_fp != NULL) {
		snd_wav_write_header (sdrv);
	}
}

int snd_wav_write (sound_drv_t *sdrv, const uint16_t *buf, unsigned cnt)
{
	unsigned long bcnt, scnt;
	unsigned char *bbuf;

	if (sdrv->wav_fp == NULL) {
		return (0);
	}

	scnt = (unsigned long) sdrv->channels * (unsigned long) cnt;
	bcnt = 2 * scnt;

	bbuf = snd_get_bbuf (sdrv, bcnt);

	if (buf == NULL) {
		return (1);
	}

	snd_set_buf (bbuf, buf, scnt, sdrv->sample_sign == 0, 0);

	if (fwrite (bbuf, 1, bcnt, sdrv->wav_fp) != bcnt) {
		return (1);
	}

	sdrv->wav_cnt += bcnt;

	return (0);
}

int snd_wav_set_params (sound_drv_t *sdrv, unsigned chn, unsigned long srate, int sign)
{
	if (sdrv->wav_fp == NULL) {
		return (0);
	}

	if (snd_wav_write_header (sdrv)) {
		return (1);
	}

	return (0);
}

int snd_wav_init (sound_drv_t *sdrv, const char *name)
{
	char *wav;

	sdrv->wav_fp = NULL;
	sdrv->wav_cnt = 0;

	wav = drv_get_option (name, "wav");

	if (wav == NULL) {
		return (0);
	}

	if (*wav == 0) {
		return (0);
	}

	sdrv->wav_fp = fopen (wav, "wb");

	free (wav);

	if (sdrv->wav_fp == NULL) {
		return (1);
	}

	return (0);
}
