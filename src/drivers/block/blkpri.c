/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkpri.c                                   *
 * Created:     2017-10-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>

#include <drivers/block/blkpri.h>


static
int dsk_pri_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	return (1);
}

static
int dsk_pri_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	return (1);
}

static
int dsk_pri_save (disk_pri_t *dp)
{
	if (dp->dsk.fname == NULL) {
		return (1);
	}

	if (dp->img == NULL) {
		return (1);
	}

	if (dsk_get_readonly (&dp->dsk)) {
		return (1);
	}

	dp->img->readonly = 0;

	if (pri_img_save (dp->dsk.fname, dp->img, dp->type)) {
		return (1);
	}

	dp->dirty = 0;

	return (0);
}

static
int dsk_pri_commit (disk_pri_t *dp)
{
	if (dp->dirty == 0) {
		return (0);
	}

	if (dsk_pri_save (dp)) {
		fprintf (stderr,
			"disk %u: writing back pri image to %s failed\n",
			dp->dsk.drive,
			(dp->dsk.fname != NULL) ? dp->dsk.fname : "<none>"
		);

		return (1);
	}

	return (0);
}

static
int dsk_pri_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	if (strcmp (msg, "commit") == 0) {
		return (dsk_pri_commit (dsk->ext));
	}

	return (1);
}

static
void dsk_pri_del (disk_t *dsk)
{
	disk_pri_t *dp;

	dp = dsk->ext;

	dsk_pri_commit (dp);

	if (dp->img != NULL) {
		pri_img_del (dp->img);
	}

	free (dp);
}

disk_t *dsk_pri_open_fp (FILE *fp, unsigned type, int ro)
{
	disk_t     *dsk;
	disk_pri_t *dp;

	if ((dp = malloc (sizeof (disk_pri_t))) == NULL) {
		return (NULL);
	}

	dsk = &dp->dsk;

	dsk_init (dsk, dp, 0, 0, 0, 0);
	dsk_set_type (dsk, PCE_DISK_PRI);
	dsk_set_readonly (dsk, ro);

	dsk->del = dsk_pri_del;
	dsk->read = dsk_pri_read;
	dsk->write = dsk_pri_write;
	dsk->set_msg = dsk_pri_set_msg;

	dp->dirty = 0;
	dp->type = type;

	dp->img = pri_img_load_fp (fp, type);

	if (dp->img == NULL) {
		dsk_pri_del (dsk);
		return (NULL);
	}

	if (dp->img->readonly) {
		dsk_set_readonly (dsk, 1);
	}

	return (dsk);
}

disk_t *dsk_pri_open (const char *fname, unsigned type, int ro)
{
	disk_t *dsk;
	FILE   *fp;

	if (type == PRI_FORMAT_NONE) {
		type = pri_probe (fname);

		if (type == PRI_FORMAT_NONE) {
			type = pri_guess_type (fname);
		}
	}

	if (type == PRI_FORMAT_NONE) {
		return (NULL);
	}

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		fp = fopen (fname, "r+b");

		if (fp == NULL) {
			ro = 1;
			fp = fopen (fname, "rb");
		}
	}

	if (fp == NULL) {
		return (NULL);
	}

	dsk = dsk_pri_open_fp (fp, type, ro);

	fclose (fp);

	if (dsk == NULL) {
		return (NULL);
	}

	dsk_set_fname (dsk, fname);

	return (dsk);
}


static
int dsk_pri_init_pri (pri_img_t *img, unsigned long c, unsigned long h, unsigned long s)
{
	unsigned  ci, hi;
	pri_trk_t *trk;

	if ((c > 65535) || (h > 65535) || (s > 65535)) {
		return (1);
	}

	for (ci = 0; ci < c; ci++) {
		for (hi = 0; hi < h; hi++) {
			if ((trk = pri_img_get_track (img, ci, hi, 1)) == NULL) {
				return (1);
			}
		}
	}

	return (0);
}

int dsk_pri_create_fp (FILE *fp, unsigned type, uint32_t c, uint32_t h, uint32_t s)
{
	int       r;
	pri_img_t *img;

	img = pri_img_new();

	if (img == NULL) {
		return (1);
	}

	if (dsk_pri_init_pri (img, c, h, s)) {
		pri_img_del (img);
		return (1);
	}

	r = pri_img_save_fp (fp, img, type);

	pri_img_del (img);

	return (r);
}

int dsk_pri_create (const char *name, unsigned type, uint32_t c, uint32_t h, uint32_t s)
{
	int  r;
	FILE *fp;

	if (type == PRI_FORMAT_NONE) {
		type = pri_guess_type (name);
	}

	if ((fp = fopen (name, "wb")) == NULL) {
		return (1);
	}

	r = dsk_pri_create_fp (fp, type, c, h, s);

	fclose (fp);

	return (r);
}


unsigned dsk_pri_probe_fp (FILE *fp)
{
	return (pri_probe_fp (fp));
}

unsigned dsk_pri_probe (const char *fname)
{
	return (pri_probe (fname));
}
