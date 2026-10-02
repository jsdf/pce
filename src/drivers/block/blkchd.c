/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkchd.c                                   *
 * Created:     2017-12-09 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <drivers/block/block.h>
#include <drivers/block/blkchd.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define CHD_MAGIC1 0x4d436f6d
#define CHD_MAGIC2 0x70724844


static
void chd_cache_init (disk_chd_t *img)
{
	unsigned i;

	for (i = 0; i < CHD_CACHE_SIZE; i++) {
		img->cache_valid[i] = 0;
		img->cache_src[i] = 0;
		img->cache_dst[i] = 0;
	}
}

static
int chd_cache_map (disk_chd_t *img, uint64_t *ofs, uint64_t *max, int alloc)
{
	unsigned      idx;
	uint32_t      hunk, hofs;
	uint64_t      next;
	unsigned char buf[4];

	hunk = *ofs / img->hunk_size;
	hofs = *ofs % img->hunk_size;

	if (hunk >= img->hunks) {
		return (1);
	}

	idx = hunk % CHD_CACHE_SIZE;

	if ((img->cache_valid[idx] == 0) || (img->cache_src[idx] != hunk)) {
		if (dsk_read (img->fp, buf, img->map_offset + 4 * hunk, 4)) {
			return (1);
		}

		img->cache_src[idx] = hunk;
		img->cache_dst[idx] = dsk_get_uint32_be (buf, 0);
		img->cache_valid[idx] = 1;
	}

	if ((img->cache_dst[idx] == 0) && alloc) {
		next = img->hunk_size * (uint64_t) img->next_hunk;

		if (dsk_set_filesize (img->fp, next + img->hunk_size)) {
			return (1);
		}

		img->cache_src[idx] = hunk;
		img->cache_dst[idx] = img->next_hunk;
		img->cache_valid[idx] = 1;

		img->next_hunk += 1;

		dsk_set_uint32_be (buf, 0, img->cache_dst[idx]);

		if (dsk_write (img->fp, buf, img->map_offset + 4 * hunk, 4)) {
			return (1);
		}
	}

	if (img->cache_dst[idx] == 0) {
		*ofs = 0;
	}
	else {
		*ofs = img->hunk_size * (uint64_t) img->cache_dst[idx] + hofs;
	}

	*max = img->hunk_size - hofs;

	return (0);
}

static
int chd_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	disk_chd_t    *img;
	unsigned char *p;
	uint32_t      blk;
	uint64_t      ofs, max;

	img = dsk->ext;

	p = buf;

	while (n > 0) {
		ofs = 512ULL * i;

		if (chd_cache_map (img, &ofs, &max, 0)) {
			return (1);
		}

		blk = max / 512;

		if (blk == 0) {
			return (1);
		}

		if (blk > n) {
			blk = n;
		}

		if (ofs == 0) {
			memset (p, 0, 512 * blk);
		}
		else {
			if (dsk_read (img->fp, p, ofs, 512 * blk)) {
				return (1);
			}
		}

		i += blk;
		n -= blk;
		p += 512 * blk;
	}

	return (0);
}

static
int chd_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	disk_chd_t          *img;
	const unsigned char *p;
	uint32_t            blk;
	uint64_t            ofs, max;

	if (dsk->readonly) {
		return (1);
	}

	img = dsk->ext;

	p = buf;

	while (n > 0) {
		ofs = 512ULL * i;

		if (chd_cache_map (img, &ofs, &max, 1)) {
			return (1);
		}

		blk = max / 512;

		if ((blk == 0) || (ofs == 0)) {
			return (1);
		}

		if (blk > n) {
			blk = n;
		}

		if (dsk_write (img->fp, p, ofs, 512 * blk)) {
			return (1);
		}

		i += blk;
		n -= blk;
		p += 512 * blk;
	}

	return (0);
}


static
int chd_get_msg (disk_t *dsk, const char *msg, char *val, unsigned max)
{
	return (1);
}

static
int chd_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	if (strcmp (msg, "commit") == 0) {
		return (0);
	}

	return (1);
}

static
void chd_del (disk_t *dsk)
{
	disk_chd_t *img;

	img = dsk->ext;

	if (img->fp != NULL) {
		fclose (img->fp);
	}

	free (img->map);
	free (img);
}

disk_t *dsk_chd_open_fp (FILE *fp, int ro)
{
	disk_chd_t    *img;
	uint32_t      hunk_size, hunks;
	uint64_t      image_size, file_size;
	unsigned char buf[128];

	if (dsk_read (fp, buf, 0, 124)) {
		return (NULL);
	}

	if (dsk_get_uint32_be (buf, 0) != CHD_MAGIC1) {
		return (NULL);
	}

	if (dsk_get_uint32_be (buf, 4) != CHD_MAGIC2) {
		return (NULL);
	}

	/* header size */
	if (dsk_get_uint32_be (buf, 8) < 124) {
		return (NULL);
	}

	/* version */
	if (dsk_get_uint32_be (buf, 12) != 5) {
		fprintf (stderr, "chd: unsupported version\n");
		return (NULL);
	}

	/* compressor */
	if (dsk_get_uint32_be (buf, 16) != 0) {
		fprintf (stderr, "chd: unsupported compressed image\n");
		return (NULL);
	}

	image_size = dsk_get_uint64_be (buf, 32);
	hunk_size = dsk_get_uint32_be (buf, 56);

	if ((image_size & 511) || (hunk_size & 511) || (hunk_size < 512)) {
		return (NULL);
	}

	hunks = (image_size + hunk_size - 1) / hunk_size;

	if ((img = malloc (sizeof (disk_chd_t))) == NULL) {
		return (NULL);
	}

	dsk_init (&img->dsk, img, image_size / 512, 0, 0, 0);
	dsk_set_type (&img->dsk, PCE_DISK_CHD);
	dsk_set_readonly (&img->dsk, ro);

	dsk_get_filesize (fp, &file_size);

	img->next_hunk = (file_size + hunk_size - 1) / hunk_size;

	img->dsk.del = chd_del;
	img->dsk.read = chd_read;
	img->dsk.write = chd_write;
	img->dsk.get_msg = chd_get_msg;
	img->dsk.set_msg = chd_set_msg;

	img->fp = fp;

	memcpy (img->header, buf, 124);

	img->map_offset = dsk_get_uint64_be (buf, 40);
	img->image_size = image_size;
	img->hunk_size = dsk_get_uint32_be (buf, 56);
	img->hunks = hunks;

	chd_cache_init (img);

	return (&img->dsk);
}

disk_t *dsk_chd_open (const char *fname, int ro)
{
	disk_t *dsk;
	FILE   *fp;

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		if ((fp = fopen (fname, "r+b")) == NULL) {
			fp = fopen (fname, "rb");
			ro = 1;
		}
	}

	if (fp == NULL) {
		return (NULL);
	}

	dsk = dsk_chd_open_fp (fp, ro);

	if (dsk == NULL) {
		fclose (fp);
		return (NULL);
	}

	dsk_guess_geometry (dsk);

	dsk_set_fname (dsk, fname);

	return (dsk);
}

int dsk_chd_create_fp (FILE *fp, uint32_t n, uint32_t c, uint32_t h, uint32_t s)
{
	unsigned char buf[256];
	uint32_t      k, cnt;
	uint32_t      hunk_size;
	uint64_t      image_size;
	uint64_t      ofs;

	memset (buf, 0, 256);

	image_size = 512 * (uint64_t) n;
	hunk_size = 4096;

	dsk_set_uint32_be (buf, 0, CHD_MAGIC1);
	dsk_set_uint32_be (buf, 4, CHD_MAGIC2);
	dsk_set_uint32_be (buf, 8, 124);
	dsk_set_uint32_be (buf, 12, 5);
	dsk_set_uint64_be (buf, 32, image_size);
	dsk_set_uint64_be (buf, 40, 128);
	dsk_set_uint32_be (buf, 56, hunk_size);
	dsk_set_uint32_be (buf, 60, 512);

	if (dsk_write (fp, buf, 0, 124)) {
		return (1);
	}

	memset (buf, 0, 256);

	ofs = 128;
	cnt = 4 * ((image_size + hunk_size - 1) / hunk_size);

	while (cnt > 0) {
		k = (cnt < 256) ? cnt : 256;

		if (dsk_write (fp, buf, ofs, k)) {
			return (1);
		}

		cnt -= k;
		ofs += k;
	}

	return (0);
}

int dsk_chd_create (const char *fname, uint32_t n, uint32_t c, uint32_t h, uint32_t s)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = dsk_chd_create_fp (fp, n, c, h, s);

	fclose (fp);

	return (r);
}

int dsk_chd_probe_fp (FILE *fp)
{
	unsigned char buf[16];

	if (dsk_read (fp, buf, 0, 16)) {
		return (0);
	}

	if (dsk_get_uint32_be (buf, 0) != CHD_MAGIC1) {
		return (0);
	}

	if (dsk_get_uint32_be (buf, 4) != CHD_MAGIC2) {
		return (0);
	}

	return (1);
}

int dsk_chd_probe (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = dsk_chd_probe_fp (fp);

	fclose (fp);

	return (r);
}
