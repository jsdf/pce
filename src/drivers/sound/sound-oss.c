/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/sound-oss.c                                *
 * Created:     2009-10-17 by Hampa Hug <hampa@hampa.ch>                     *
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
#include <limits.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include <drivers/options.h>
#include <drivers/sound/sound.h>
#include <drivers/sound/sound-oss.h>

#ifndef DEBUG_SND_OSS
#define DEBUG_SND_OSS 0
#endif


static
void snd_oss_close (sound_drv_t *sdrv)
{
	sound_oss_t *drv;

	drv = sdrv->ext;

	if (drv->fd >= 0) {
		close (drv->fd);
	}

	if (drv->dev != NULL) {
		free (drv->dev);
	}

	free (drv);
}

static
int snd_oss_write (sound_drv_t *sdrv, const uint16_t *buf, unsigned cnt)
{
	ssize_t       r;
	int           sign;
	unsigned long bcnt, scnt;
	unsigned char *bbuf;
	sound_oss_t   *drv;

	drv = sdrv->ext;

	scnt = (unsigned long) sdrv->channels * (unsigned long) cnt;
	bcnt = 2 * scnt;

	if (bcnt > SSIZE_MAX) {
		return (1);
	}

	bbuf = snd_get_bbuf (sdrv, bcnt);

	if (bbuf == NULL) {
		return (1);
	}

	sign = (sdrv->sample_sign != drv->sign);

	snd_set_buf (bbuf, buf, scnt, sign, drv->big_endian);

	r = write (drv->fd, bbuf, bcnt);

	if (r != (ssize_t) bcnt) {
#if DEBUG_SND_OSS >= 1
		fprintf (stderr, "snd-oss: buffer overrun\n");
#endif
		return (1);
	}

	return (0);
}

static
int snd_oss_set_format (sound_oss_t *drv, int fmt)
{
	int val;

	val = fmt;

	if (ioctl (drv->fd, SNDCTL_DSP_SETFMT, &val) == -1) {
		return (1);
	}

	if (val != fmt) {
		return (1);
	}

	return (0);
}

static
int snd_oss_set_params (sound_drv_t *sdrv, unsigned chn, unsigned long srate, int sign)
{
	int         val;
	sound_oss_t *drv;

	drv = sdrv->ext;

	if (snd_oss_set_format (drv, AFMT_S16_LE) == 0) {
		drv->sign = 1;
		drv->big_endian = 0;
	}
	else if (snd_oss_set_format (drv, AFMT_S16_BE) == 0) {
		drv->sign = 1;
		drv->big_endian = 1;
	}
	else if (snd_oss_set_format (drv, AFMT_U16_LE) == 0) {
		drv->sign = 0;
		drv->big_endian = 0;
	}
	else if (snd_oss_set_format (drv, AFMT_U16_BE) == 0) {
		drv->sign = 0;
		drv->big_endian = 1;
	}
	else {
		return (1);
	}

	val = chn;
	if (ioctl (drv->fd, SNDCTL_DSP_CHANNELS, &val) == -1) {
		return (1);
	}
	if (val != (int) chn) {
		return (1);
	}

	val = srate;
	if (ioctl (drv->fd, SNDCTL_DSP_SPEED, &val) == -1) {
		return (1);
	}
	if (val != (int) srate) {
		return (1);
	}

	return (0);
}

static
int snd_oss_init (sound_oss_t *drv, const char *name)
{
	snd_init (&drv->sdrv, drv);

	drv->sdrv.close = snd_oss_close;
	drv->sdrv.write = snd_oss_write;
	drv->sdrv.set_params = snd_oss_set_params;

	drv->fd = -1;
	drv->dev = drv_get_option (name, "dev");

	if (drv->dev == NULL) {
		return (1);
	}

	drv->fd = open (drv->dev, O_WRONLY | O_NDELAY, 0);

	if (drv->fd < 0) {
		return (1);
	}

	return (0);
}

sound_drv_t *snd_oss_open (const char *name)
{
	sound_oss_t *drv;

	drv = malloc (sizeof (sound_oss_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (snd_oss_init (drv, name)) {
		snd_oss_close (&drv->sdrv);
		return (NULL);
	}

	return (&drv->sdrv);
}
