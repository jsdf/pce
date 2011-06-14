/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkcow.c                                   *
 * Created:     2003-04-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1996-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include "blkcow.h"

#include <stdlib.h>
#include <string.h>


#define COW_MAGIC 0x434f5720

/*
 * COW image file format
 * 0   4  magic (COW )
 * 4   4  version (0)
 * 8   4  block count
 * 12  4  bitmap start
 * 16  4  data start
 */


static
int cow_read_bitmap (disk_cow_t *cow)
{
	if (dsk_read (cow->fp, cow->bitmap, cow->bitmap_offset, cow->bitmap_size)) {
		return (1);
	}

	return (0);
}

static
int cow_write_bitmap (disk_cow_t *cow)
{
	if (dsk_write (cow->fp, cow->bitmap, cow->bitmap_offset, cow->bitmap_size)) {
		return (1);
	}

	fflush (cow->fp);

	return (0);
}

/*
 * - check if block blk is copied, return true if so.
 * - check how many blocks, starting at blk have the same status
 *   and return that number in cnt.
 * - cnt on output <= cnt on input.
 */
static
int cow_get_block (disk_cow_t *cow, uint32_t blk, uint32_t *cnt)
{
	int           r;
	uint32_t      i, n;
	unsigned char m, v;

	i = blk / 8;
	n = *cnt;
	m = 0x80 >> (blk & 7);

	r = (cow->bitmap[i] & m) != 0;

	*cnt = 1;

	if (n <= 1) {
		return (r);
	}

	n = n - 1;
	v = r ? 0xff : 0x00;

	/* i, m refer to first data block */

	while (n > 0) {
		if (m == 0x01) {
			i += 1;
			m = 0x80;

			/* skip runs of 8 blocks */
			while ((n >= 8) && (cow->bitmap[i] == v)) {
				i += 1;
				n -= 8;
				*cnt += 8;
			}

			if (n == 0) {
				return (r);
			}
		}
		else {
			m = m >> 1;
		}

		if (((cow->bitmap[i] & m) != 0) != r) {
			return (r);
		}

		n -= 1;
		*cnt += 1;
	}

	return (r);
}

static
int cow_set_block (disk_cow_t *cow, uint32_t blk, uint32_t cnt, int val)
{
	uint32_t      i, i0, i1;
	unsigned char m0, m1;

	cnt = (blk + cnt - 1);

	i0 = blk / 8;
	i1 = cnt / 8;

	m0 = 0x80 >> (blk & 7);
	m1 = 0x80 >> (cnt & 7);

	m0 |= (m0 - 1);
	m1 |= (m1 - 1) ^ 0xff;

	if (val) {
		if (i0 == i1) {
			cow->bitmap[i0] |= (m0 & m1);
		}
		else {
			cow->bitmap[i0] |= m0;
			cow->bitmap[i1] |= m1;
			for (i = i0 + 1; i < i1; i++) {
				cow->bitmap[i] = 0xff;
			}
		}
	}
	else {
		if (i0 == i1) {
			cow->bitmap[i0] &= ~(m0 & m1);
		}
		else {
			cow->bitmap[i0] &= ~m0;
			cow->bitmap[i1] &= ~m1;
			for (i = i0 + 1; i < i1; i++) {
				cow->bitmap[i] = 0x00;
			}
		}
	}

	i1 = i1 - i0 + 1;

	if (dsk_write (cow->fp, cow->bitmap + i0, cow->bitmap_offset + i0, i1)) {
		return (1);
	}

	return (0);
}

static
int dsk_cow_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	unsigned char *tmp;
	uint32_t      cnt;
	uint64_t      ofs;
	disk_cow_t    *cow;

	cow = dsk->ext;
	tmp = buf;
	ofs = cow->data_offset + 512 * (uint64_t) i;

	while (n > 0) {
		if (i >= dsk->blocks) {
			return (1);
		}

		cnt = n;
		if (cow_get_block (cow, i, &cnt)) {
			if (dsk_read (cow->fp, tmp, ofs, 512 * cnt)) {
				return (1);
			}
		}
		else {
			if (dsk_read_lba (cow->orig, tmp, i, cnt)) {
				return (1);
			}
		}

		i += cnt;
		n -= cnt;
		tmp += 512 * cnt;
		ofs += 512 * cnt;
	}

	return (0);
}

static
int dsk_cow_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	const unsigned char *tmp;
	uint32_t            cnt;
	uint64_t            ofs;
	disk_cow_t          *cow;

	if (dsk->readonly) {
		return (1);
	}

	cow = dsk->ext;
	tmp = buf;

	ofs = cow->data_offset + 512 * (uint64_t) i;

	while (n > 0) {
		if (i >= dsk->blocks) {
			return (1);
		}

		cnt = n;
		if (cow_get_block (cow, i, &cnt) == 0) {
			cow_set_block (cow, i, cnt, 1);
		}

		if (dsk_write (cow->fp, tmp, ofs, 512 * cnt)) {
			return (1);
		}

		i += cnt;
		n -= cnt;
		tmp += 512 * cnt;
		ofs += 512 * cnt;
	}

	fflush (cow->fp);

	return (0);
}

static
int cow_commit_block (disk_cow_t *cow, uint32_t blk, unsigned cnt)
{
	unsigned      i, n;
	uint64_t      ofs;
	unsigned char buf[8 * 512];

	ofs = cow->data_offset + 512 * (uint64_t) blk;

	while (cnt > 0) {
		n = (cnt <= 8) ? cnt : 8;

		if (dsk_read (cow->fp, buf, ofs, 512 * n)) {
			return (1);
		}

		if (dsk_write_lba (cow->orig, buf, blk, n)) {
			return (1);
		}

		memset (buf, 0, 512 * n);

		if (dsk_write (cow->fp, buf, ofs, 512 * n)) {
			return (1);
		}

		for (i = 0; i < n; i++) {
			cow->bitmap[(blk + i) >> 3] &= ~(0x80 >> ((blk + i) & 7));
		}

		blk += n;
		ofs += 512 * n;
		cnt -= n;
	}

	return (0);
}

static
int dsk_cow_commit (disk_t *dsk)
{
	int           r;
	unsigned      k;
	uint32_t      i, n;
	uint32_t      blk;
	unsigned char msk;
	disk_cow_t    *cow;

	cow = dsk->ext;

	r = 0;
	n = 0;

	for (i = 0; i < cow->bitmap_size; i++) {
		msk = cow->bitmap[i];
		blk = 8 * i;

		while (msk != 0) {
			while ((msk & 0x80) == 0) {
				msk = (msk << 1) & 0xff;
				blk += 1;
			}

			k = 0;
			while (msk & 0x80) {
				k += 1;
				msk = (msk << 1) & 0xff;
			}

			if (cow_commit_block (cow, blk, k) == 0) {
				n += k;
			}
			else {
				r = 1;
			}

			blk += k;
		}
	}

	if (n > 0) {
		if (cow_write_bitmap (cow)) {
			return (1);
		}
	}

	fflush (cow->fp);

	return (r);
}

static
int dsk_cow_get_msg (disk_t *dsk, const char *msg, char *val, unsigned max)
{
	return (1);
}

static
int dsk_cow_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	if (strcmp (msg, "commit") == 0) {
		return (dsk_cow_commit (dsk));
	}

	return (1);
}

static
void dsk_cow_del (disk_t *dsk)
{
	disk_cow_t *cow;

	cow = dsk->ext;

	dsk_del (cow->orig);

	fclose (cow->fp);

	free (cow->bitmap);
	free (cow);
}

static
int dsk_cow_create (disk_cow_t *cow)
{
	unsigned char buf[32];

	cow->bitmap_size = (cow->dsk.blocks + 7) / 8;
	cow->bitmap_offset = 20;
	cow->data_offset = (20 + cow->bitmap_size + 511) & (~511UL);

	dsk_set_uint32_be (buf, 0, COW_MAGIC);
	dsk_set_uint32_be (buf, 4, 0);
	dsk_set_uint32_be (buf, 8, cow->dsk.blocks);
	dsk_set_uint32_be (buf, 12, cow->bitmap_offset);
	dsk_set_uint32_be (buf, 16, cow->data_offset);

	if (dsk_write (cow->fp, buf, 0, 20)) {
		return (1);
	}

	memset (cow->bitmap, 0, cow->bitmap_size);

	if (cow_write_bitmap (cow)) {
		return (1);
	}

	return (0);
}

static
int dsk_cow_open (disk_cow_t *cow)
{
	unsigned char buf[32];

	if (dsk_read (cow->fp, buf, 0, 20)) {
		return (1);
	}

	if (dsk_get_uint32_be (buf, 0) != COW_MAGIC) {
		return (1);
	}

	if (dsk_get_uint32_be (buf, 4) != 0) {
		return (1);
	}

	if (dsk_get_uint32_be (buf, 8) != cow->dsk.blocks) {
		return (1);
	}

	cow->bitmap_offset = dsk_get_uint32_be (buf, 12);
	cow->data_offset = dsk_get_uint32_be (buf, 16);

	if (cow_read_bitmap (cow)) {
		return (1);
	}

	return (0);
}

static
int dsk_cow_open_file (disk_cow_t *cow, const char *fname)
{
	cow->fp = fopen (fname, "r+b");
	if (cow->fp != NULL) {
		if (dsk_cow_open (cow)) {
			fclose (cow->fp);
			return (1);
		}

		return (0);
	}

	cow->fp = fopen (fname, "w+b");
	if (cow->fp == NULL) {
		return (1);
	}

	if (dsk_cow_create (cow)) {
		fclose (cow->fp);
		return (1);
	}

	return (0);
}

disk_t *dsk_cow_new (disk_t *dsk, const char *fname)
{
	disk_cow_t *cow;

	cow = (disk_cow_t *) malloc (sizeof (disk_cow_t));
	if (cow == NULL) {
		return (NULL);
	}

	cow->dsk = *dsk;

	dsk_set_type (&cow->dsk, PCE_DISK_COW);

	cow->dsk.del = dsk_cow_del;
	cow->dsk.read = dsk_cow_read;
	cow->dsk.write = dsk_cow_write;
	cow->dsk.get_msg = dsk_cow_get_msg;
	cow->dsk.set_msg = dsk_cow_set_msg;
	cow->dsk.fname = NULL;
	cow->dsk.ext = cow;

	cow->orig = dsk;

	cow->bitmap_size = (cow->dsk.blocks + 7) / 8;
	cow->bitmap = (unsigned char *) malloc (cow->bitmap_size);
	if (cow->bitmap == NULL) {
		free (cow);
		return (NULL);
	}

	if (dsk_cow_open_file (cow, fname)) {
		free (cow->bitmap);
		free (cow);
		return (NULL);
	}

	dsk_set_fname (&cow->dsk, fname);

	return (&cow->dsk);
}
