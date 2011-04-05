/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-null.c                                 *
 * Created:     2009-03-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/char/char.h>
#include <drivers/char/char-null.h>


static
void chr_null_close (char_drv_t *cdrv)
{
	char_null_t *drv;

	drv = cdrv->ext;

	free (drv);
}

static
unsigned chr_null_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
#if 0
	char_null_t *drv;

	drv = cdrv->ext;
#endif

	return (0);
}

static
unsigned chr_null_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
#if 0
	char_null_t *drv;

	drv = cdrv->ext;
#endif

	return (cnt);
}

static
int chr_null_get_ctl (char_drv_t *cdrv, unsigned *ctl)
{
#if 0
	char_null_t *drv;

	drv = cdrv->ext;
#endif

	*ctl = PCE_CHAR_DSR | PCE_CHAR_CTS | PCE_CHAR_CD;

	return (0);
}

static
int chr_null_set_ctl (char_drv_t *cdrv, unsigned ctl)
{
#if 0
	char_null_t *drv;

	drv = cdrv->ext;
#endif

	return (0);
}

static
int chr_null_set_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop)
{
#if 0
	char_null_t *drv;

	drv = cdrv->ext;
#endif

	return (0);
}

static
int chr_null_init (char_null_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_null_close;
	drv->cdrv.read = chr_null_read;
	drv->cdrv.write = chr_null_write;
	drv->cdrv.get_ctl = chr_null_get_ctl;
	drv->cdrv.set_ctl = chr_null_set_ctl;
	drv->cdrv.set_params = chr_null_set_params;

	return (0);
}

char_drv_t *chr_null_open (const char *name)
{
	char_null_t *drv;

	drv = malloc (sizeof (char_null_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_null_init (drv, name)) {
		free (drv);
		return (NULL);
	}

	return (&drv->cdrv);
}
