/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-stdio.c                                *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-stdio.h>


static
void chr_stdio_close (char_drv_t *cdrv)
{
	char_stdio_t *drv;

	drv = cdrv->ext;

	if (drv->fname != NULL) {
		free (drv->fname);
	}

	if (drv->fp != NULL) {
		if ((drv->fp != stdin) && (drv->fp != stdout) && (drv->fp != stderr)) {
			fclose (drv->fp);
		}
	}

	free (drv);
}

static
unsigned chr_stdio_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	return (0);
}

static
unsigned chr_stdio_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_stdio_t *drv;

	drv = cdrv->ext;

	if (drv->fp == NULL) {
		return (cnt);
	}

	cnt = fwrite (buf, 1, cnt, drv->fp);

	if (drv->flush) {
		fflush (drv->fp);
	}

	return (cnt);
}

static
int chr_stdio_init (char_stdio_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_stdio_close;
	drv->cdrv.read = chr_stdio_read;
	drv->cdrv.write = chr_stdio_write;

	drv->fp = NULL;

	drv->fname = drv_get_option (name, "file");
	drv->flush = drv_get_option_bool (name, "flush", 1);

	if (drv->fname != NULL) {
		if (strcmp (drv->fname, "-") == 0) {
			drv->fp = stdout;
		}
		else {
			drv->fp = fopen (drv->fname, "wb");

			if (drv->fp == NULL) {
				return (1);
			}
		}
	}

	return (0);
}

char_drv_t *chr_stdio_open (const char *name)
{
	char_stdio_t *drv;

	drv = malloc (sizeof (char_stdio_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_stdio_init (drv, name)) {
		chr_stdio_close (&drv->cdrv);

		return (NULL);
	}

	return (&drv->cdrv);
}
