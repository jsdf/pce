/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-stdio.c                                *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-stdio.h>


static
int stdio_check_reopen (char_stdio_t *drv)
{
#ifdef HAVE_SYS_STAT_H
	struct stat st;

	if (drv->reopen == 0) {
		return (0);
	}

	if (drv->write_name == NULL) {
		return (0);
	}

	if (stat (drv->write_name, &st) == 0) {
		return (0);
	}

	if ((drv->write_fp != NULL) && (drv->write_fp != stdout)) {
		fclose (drv->write_fp);
	}

	drv->write_fp = fopen (drv->write_name, "wb");
#endif

	return (0);
}

static
void chr_stdio_fclose (char *name, FILE *fp)
{
	if (name != NULL) {
		free (name);
	}

	if ((fp != NULL) && (fp != stdin) && (fp != stdout) && (fp != stderr)) {
		fclose (fp);
	}
}

static
void chr_stdio_close (char_drv_t *cdrv)
{
	char_stdio_t *drv;

	drv = cdrv->ext;

	chr_stdio_fclose (drv->write_name, drv->write_fp);
	chr_stdio_fclose (drv->read_name, drv->read_fp);

	free (drv);
}

static
unsigned chr_stdio_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	char_stdio_t *drv;

	drv = cdrv->ext;

	if (drv->read_fp == NULL) {
		return (0);
	}

	cnt = fread (buf, 1, cnt, drv->read_fp);

	return (cnt);
}

static
unsigned chr_stdio_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_stdio_t *drv;

	drv = cdrv->ext;

	stdio_check_reopen (drv);

	if (drv->write_fp == NULL) {
		return (cnt);
	}

	cnt = fwrite (buf, 1, cnt, drv->write_fp);

	if (drv->flush) {
		fflush (drv->write_fp);
	}

	return (cnt);
}

static
int chr_stdio_init (char_stdio_t *drv, const char *name)
{
	const char *mode;

	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_stdio_close;
	drv->cdrv.read = chr_stdio_read;
	drv->cdrv.write = chr_stdio_write;

	drv->read_name = NULL;
	drv->read_fp = NULL;
	drv->write_name = NULL;
	drv->write_fp = NULL;

	drv->flush = drv_get_option_bool (name, "flush", 1);
	drv->reopen = drv_get_option_bool (name, "reopen", 0);
	drv->append = drv_get_option_bool (name, "append", 0);

	drv->write_name = drv_get_option (name, "write");

	if (drv->write_name == NULL) {
		drv->write_name = drv_get_option (name, "file");
	}

	if (drv->write_name != NULL) {
		if (strcmp (drv->write_name, "-") == 0) {
			drv->write_fp = stdout;
			drv->reopen = 0;
		}
		else {
			mode = drv->append ? "ab" : "wb";

			drv->write_fp = fopen (drv->write_name, mode);

			if (drv->write_fp == NULL) {
				return (1);
			}
		}
	}

	drv->read_name = drv_get_option (name, "read");

	if (drv->read_name != NULL) {
		if (strcmp (drv->read_name, "-") == 0) {
			drv->read_fp = stdin;
		}
		else {
			drv->read_fp = fopen (drv->read_name, "rb");

			if (drv->read_fp == NULL) {
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
