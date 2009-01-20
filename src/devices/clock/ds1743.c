/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/clock/ds1743.c                                   *
 * Created:     2006-12-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2009 Hampa Hug <hampa@hampa.ch>                     *
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
#include <time.h>

#include <devices/clock/ds1743.h>


static void dev_ds1743_del_dev (device_t *dev);
static void dev_ds1743_cbread (dev_ds1743_t *rtc, unsigned char val);
static void dev_ds1743_cbwrite (dev_ds1743_t *rtc, unsigned char val);


dev_ds1743_t *dev_ds1743_new (unsigned long addr, unsigned long size)
{
	dev_ds1743_t *rtc;

	rtc = malloc (sizeof (dev_ds1743_t));
	if (rtc == NULL) {
		return (NULL);
	}

	dev_init (&rtc->dev, rtc, "ds1743");
	rtc->dev.del = dev_ds1743_del_dev;

	ds1743_init (&rtc->ds1743, size);
	ds1743_set_cbread (&rtc->ds1743, rtc, dev_ds1743_cbread);
	ds1743_set_cbwrite (&rtc->ds1743, rtc, dev_ds1743_cbwrite);

	mem_blk_init (&rtc->blk, addr, size, 0);

	mem_blk_set_fget (&rtc->blk, &rtc->ds1743,
		ds1743_get_uint8, ds1743_get_uint16, ds1743_get_uint32
	);

	mem_blk_set_fset (&rtc->blk, &rtc->ds1743,
		ds1743_set_uint8, ds1743_set_uint16, ds1743_set_uint32
	);

	rtc->fp = NULL;
	rtc->fpclose = 0;

	return (rtc);
}

void dev_ds1743_del (dev_ds1743_t *rtc)
{
	dev_ds1743_save (rtc);

	mem_blk_free (&rtc->blk);
	ds1743_free (&rtc->ds1743);

	free (rtc);
}

static
void dev_ds1743_del_dev (device_t *dev)
{
	dev_ds1743_del (dev->ext);
}

static
void dev_ds1743_cbread (dev_ds1743_t *rtc, unsigned char val)
{
	/* RTC is about to be read */
	dev_ds1743_set_date (rtc);
}

static
void dev_ds1743_cbwrite (dev_ds1743_t *rtc, unsigned char val)
{
	/* RTC was just written */
}

void dev_ds1743_set_date (dev_ds1743_t *rtc)
{
	time_t    tm;
	struct tm *tms;

	time (&tm);
	tms = gmtime (&tm);

	ds1743_set_date (&rtc->ds1743, 1900 + tms->tm_year, tms->tm_mon, tms->tm_mday - 1);
	ds1743_set_time (&rtc->ds1743, tms->tm_hour, tms->tm_min, tms->tm_sec);
	ds1743_set_day (&rtc->ds1743, tms->tm_wday);
}

int dev_ds1743_set_fp (dev_ds1743_t *rtc, FILE *fp, int close)
{
	if (rtc->fpclose) {
		fclose (rtc->fp);
	}

	rtc->fp = fp;
	rtc->fpclose = (fp != NULL) && close;

	if (fp != NULL) {
		if (fseek (fp, 0, SEEK_SET) != 0) {
			return (1);
		}

		if (fread (rtc->ds1743.data, rtc->ds1743.cnt, 1, fp) != 1) {
			return (1);
		}
	}

	return (0);
}

int dev_ds1743_set_fname (dev_ds1743_t *rtc, const char *fname)
{
	FILE *fp;

	if (fname == NULL) {
		dev_ds1743_set_fp (rtc, NULL, 0);
		return (0);
	}

	fp = fopen (fname, "r+b");
	if (fp == NULL) {
		fp = fopen (fname, "w+b");
	}

	if (fp == NULL) {
		return (1);
	}

	dev_ds1743_set_fp (rtc, fp, 1);

	return (0);
}

int dev_ds1743_save (dev_ds1743_t *rtc)
{
	if (rtc->fp == NULL) {
		return (1);
	}

	if (fseek (rtc->fp, 0, SEEK_SET) != 0) {
		return (1);
	}

	if (fwrite (rtc->ds1743.data, rtc->ds1743.cnt, 1, rtc->fp) != 1) {
		return (1);
	}

	return (0);
}
