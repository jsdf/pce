/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/sound-oss.c                                *
 * Created:     2010-08-12 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <drivers/options.h>
#include <drivers/sound/sound.h>
#include <drivers/sound/sound-sdl.h>

#include <SDL.h>

#ifndef DEBUG_SND_SDL
#define DEBUG_SND_SDL 0
#endif


static
sound_sdl_buf_t *snd_sdl_buf_new (sound_sdl_t *drv, unsigned size)
{
	unsigned char   *tmp;
	sound_sdl_buf_t *buf;

	if (drv->free != NULL) {
		buf = drv->free;
		drv->free = drv->free->next;
	}
	else {
		if (drv->buf_cnt > 32) {
			return (NULL);
		}

		buf = malloc (sizeof (sound_sdl_buf_t));

		if (buf == NULL) {
			return (NULL);
		}

		buf->max = 0;
		buf->data = NULL;

		drv->buf_cnt += 1;
	}

	buf->next = NULL;

	buf->idx = 0;
	buf->cnt = 0;

	if (buf->max < size) {
		tmp = realloc (buf->data, size);

		if (tmp == NULL) {
			free (buf->data);
			free (buf);

			drv->buf_cnt -= 1;

			return (NULL);
		}

		buf->max = size;
		buf->data = tmp;
	}

	return (buf);
}

static
void snd_sdl_buf_free_list (sound_sdl_buf_t *buf)
{
	sound_sdl_buf_t *tmp;

	while (buf != NULL) {
		tmp = buf;
		buf = buf->next;

		free (tmp->data);
		free (tmp);
	}

}

static
void snd_sdl_close (sound_drv_t *sdrv)
{
	sound_sdl_t *drv;

	drv = sdrv->ext;

	if (drv->is_open) {
		SDL_CloseAudio();
	}

	snd_sdl_buf_free_list (drv->head);
	snd_sdl_buf_free_list (drv->free);

	free (drv);
}

static
int snd_sdl_write (sound_drv_t *sdrv, const uint16_t *buf, unsigned cnt)
{
	int             sign;
	unsigned long   bcnt, scnt;
	sound_sdl_buf_t *bbuf;
	sound_sdl_t     *drv;

	drv = sdrv->ext;

	scnt = (unsigned long) sdrv->channels * (unsigned long) cnt;
	bcnt = 2 * scnt;

	SDL_LockAudio();
	bbuf = snd_sdl_buf_new (drv, bcnt);
	SDL_UnlockAudio();

	if (bbuf == NULL) {
#if DEBUG_SND_SDL >= 1
		fprintf (stderr, "snd-sdl: buffer overrun\n");
#endif
		return (1);
	}

	sign = (sdrv->sample_sign != drv->sign);

	snd_set_buf (bbuf->data, buf, scnt, sign, drv->big_endian);

	bbuf->idx = 0;
	bbuf->cnt = bcnt;

	SDL_LockAudio();

	if (drv->tail == NULL) {
		drv->head = bbuf;
	}
	else {
		drv->tail->next = bbuf;
	}

	drv->tail = bbuf;

	SDL_UnlockAudio();

	if (drv->is_paused) {
		SDL_PauseAudio (0);
		drv->is_paused = 0;
	}

	return (0);
}

static
void snd_sdl_callback (void *user, Uint8 *buf, int cnt)
{
	int             n;
	sound_sdl_t     *drv;
	sound_sdl_buf_t *src;

	drv = user;

	if (drv->head == NULL) {
		SDL_PauseAudio (1);
		drv->is_paused = 1;
		return;
	}

	while (cnt > 0) {
		if (drv->head == NULL) {
#if DEBUG_SND_SDL >= 1
			fprintf (stderr, "snd-sdl: buffer underrun\n");
#endif
			memset (buf, 0, cnt);
			return;
		}

		src = drv->head;

		n = src->cnt - src->idx;

		if (n <= cnt) {
			memcpy (buf, src->data + src->idx, n);

			buf += n;
			cnt -= n;

			drv->head = src->next;

			if (drv->head == NULL) {
				drv->tail = NULL;
			}

			src->next = drv->free;
			drv->free = src;
		}
		else {
			memcpy (buf, src->data + src->idx, cnt);
			src->idx += cnt;
			cnt = 0;
		}
	}
}

static
int snd_sdl_set_params (sound_drv_t *sdrv, unsigned chn, unsigned long srate, int sign)
{
	sound_sdl_t   *drv;
	SDL_AudioSpec req;

	drv = sdrv->ext;

	if (SDL_WasInit (SDL_INIT_AUDIO) == 0) {
		if (SDL_InitSubSystem (SDL_INIT_AUDIO) < 0) {
			fprintf (stderr,
				"snd-sdl: error initializing audio subsystem (%s)\n",
				SDL_GetError()
			);
			return (1);
		}
	}

	if (drv->is_open) {
		SDL_CloseAudio();
		drv->is_open = 0;
	}

	req.freq = srate;
	req.format = AUDIO_S16LSB;
	req.channels = chn;
	req.samples = 1024;
	req.callback = snd_sdl_callback;
	req.userdata = drv;

	if (SDL_OpenAudio (&req, NULL) < 0) {
		fprintf (stderr, "snd-sdl: error opening output (%s)\n",
			SDL_GetError()
		);
		return (1);
	}

	SDL_PauseAudio (1);

	drv->is_open = 1;
	drv->is_paused = 1;

	drv->sign = 1;
	drv->big_endian = 0;

	return (0);
}

static
int snd_sdl_init (sound_sdl_t *drv, const char *name)
{
	snd_init (&drv->sdrv, drv);

	drv->sdrv.close = snd_sdl_close;
	drv->sdrv.write = snd_sdl_write;
	drv->sdrv.set_params = snd_sdl_set_params;

	drv->is_open = 0;
	drv->is_paused = 1;

	drv->buf_cnt = 0;

	drv->head = NULL;
	drv->tail = NULL;
	drv->free = NULL;

	return (0);
}

sound_drv_t *snd_sdl_open (const char *name)
{
	sound_sdl_t *drv;

	drv = malloc (sizeof (sound_sdl_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (snd_sdl_init (drv, name)) {
		snd_sdl_close (&drv->sdrv);
		return (NULL);
	}

	return (&drv->sdrv);
}
