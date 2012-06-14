/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkraw.c                                   *
 * Created:     2004-09-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#include "blkraw.h"

#include <stdlib.h>


static
int dsk_img_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	disk_img_t *img;
	uint64_t   ofs, cnt;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	img = dsk->ext;

	ofs = img->start + 512 * (uint64_t) i;
	cnt = 512 * (uint64_t) n;

	if (dsk_read (img->fp, buf, ofs, cnt)) {
		return (1);
	}

	return (0);
}

static
int dsk_img_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	disk_img_t *img;
	uint64_t   ofs, cnt;

	if (dsk->readonly) {
		return (1);
	}

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	img = dsk->ext;

	ofs = img->start + 512 * (uint64_t) i;
	cnt = 512 * (uint64_t) n;

	if (dsk_write (img->fp, buf, ofs, cnt)) {
		return (1);
	}

	fflush (img->fp);

	return (0);
}

static
void dsk_img_del (disk_t *dsk)
{
	disk_img_t *img;

	img = dsk->ext;

	fclose (img->fp);
	free (img);
}

static
disk_t *dsk_img_open_size (FILE *fp, uint32_t n, uint64_t ofs, int ro)
{
	disk_img_t *img;

	img = malloc (sizeof (disk_img_t));

	if (img == NULL) {
		return (NULL);
	}

	dsk_init (&img->dsk, img, n, 0, 0, 0);

	dsk_set_type (&img->dsk, PCE_DISK_RAW);

	dsk_set_readonly (&img->dsk, ro);

	img->dsk.del = dsk_img_del;
	img->dsk.read = dsk_img_read;
	img->dsk.write = dsk_img_write;

	img->start = ofs;

	img->fp = fp;

	return (&img->dsk);
}

disk_t *dsk_img_open_fp (FILE *fp, uint64_t ofs, int ro)
{
	uint64_t cnt;
	disk_t   *dsk;

	if (dsk_get_filesize (fp, &cnt)) {
		return (NULL);
	}

	if (cnt <= ofs) {
		return (NULL);
	}

	cnt = (cnt - ofs) / 512;

	if (cnt == 0) {
		return (NULL);
	}

	dsk = dsk_img_open_size (fp, cnt, ofs, ro);

	if (dsk == NULL) {
		return (NULL);
	}

	dsk_guess_geometry (dsk);

	return (dsk);
}

disk_t *dsk_img_open (const char *fname, uint64_t ofs, int ro)
{
	disk_t *dsk;
	FILE   *fp;

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		fp = fopen (fname, "r+b");

		if (fp == NULL) {
			fp = fopen (fname, "rb");
			ro = 1;
		}
	}

	if (fp == NULL) {
		return (NULL);
	}

	dsk = dsk_img_open_fp (fp, ofs, ro);

	if (dsk == NULL) {
		fclose (fp);
		return (NULL);
	}

	dsk_set_fname (dsk, fname);

	return (dsk);
}

void dsk_img_set_offset (disk_t *dsk, uint64_t ofs)
{
	disk_img_t *img;

	img = dsk->ext;

	img->start = ofs;
}

int dsk_img_create_fp (FILE *fp, uint32_t n, uint64_t ofs)
{
	uint64_t      cnt;
	unsigned char buf;

	cnt = 512 * (uint64_t) n;

	if (cnt == 0) {
		return (1);
	}

	buf = 0;

	if (dsk_set_filesize (fp, ofs + cnt)) {
		if (dsk_write (fp, &buf, ofs + cnt - 1, 1)) {
			return (1);
		}
	}

	return (0);
}

int dsk_img_create (const char *fname, uint32_t n, uint64_t ofs)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = dsk_img_create_fp (fp, n, ofs);

	fclose (fp);

	return (r);
}
