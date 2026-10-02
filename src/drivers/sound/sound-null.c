/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/sound-null.c                               *
 * Created:     2009-10-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/sound/sound.h>
#include <drivers/sound/sound-null.h>


static
void snd_null_close (sound_drv_t *sdrv)
{
	sound_null_t *drv;

	drv = sdrv->ext;

	snd_free (sdrv);

	free (drv);
}

static
int snd_null_write (sound_drv_t *sdrv, const uint16_t *buf, unsigned cnt)
{
#if 0
	sound_null_t *drv;

	drv = sdrv->ext;
#endif

	return (0);
}

static
int snd_null_set_params (sound_drv_t *sdrv, unsigned chn, unsigned long srate, int sign)
{
#if 0
	sound_null_t *drv;

	drv = sdrv->ext;
#endif

	return (0);
}

static
int snd_null_init (sound_null_t *drv, const char *name)
{
	snd_init (&drv->sdrv, drv);

	drv->sdrv.close = snd_null_close;
	drv->sdrv.write = snd_null_write;
	drv->sdrv.set_params = snd_null_set_params;

	return (0);
}

sound_drv_t *snd_null_open (const char *name)
{
	sound_null_t *drv;

	drv = malloc (sizeof (sound_null_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (snd_null_init (drv, name)) {
		free (drv);
		return (NULL);
	}

	return (&drv->sdrv);
}
