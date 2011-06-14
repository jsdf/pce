/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkpart.c                                  *
 * Created:     2004-09-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include "blkpart.h"

#include <stdlib.h>
#include <string.h>


static
int dsk_part_find (disk_t *dsk, uint32_t blk_i, unsigned *part)
{
	unsigned    i;
	disk_part_t *p;

	p = dsk->ext;

	if (p->part_cnt == 0) {
		return (1);
	}

	*part = 0;

	for (i = 0; i < p->part_cnt; i++) {
		if (p->part[i].block_i <= blk_i) {
			if ((blk_i - p->part[i].block_i) < p->part[i].block_n) {
				*part = i;
				return (0);
			}
		}
		else {
			if (p->part[i].block_i < p->part[*part].block_i) {
				*part = i;
			}
		}
	}

	if (p->part[*part].block_i <= blk_i) {
		return (1);
	}

	return (0);
}

static
int dsk_part_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	unsigned    pi;
	uint32_t    j, m;
	uint64_t    ofs, cnt;
	disk_part_t *p;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	p = dsk->ext;

	while (n > 0) {
		if (dsk_part_find (dsk, i, &pi)) {
			memset (buf, 0xfe, 512UL * n);
			return (0);
		}

		if (p->part[pi].block_i <= i) {
			j = i - p->part[pi].block_i;
			m = p->part[pi].block_n - j;

			if (m > n) {
				m = n;
			}

			ofs = p->part[pi].start + 512 * (uint64_t) j;
			cnt = 512 * (uint64_t) m;

			if (dsk_read (p->part[pi].fp, buf, ofs, cnt)) {
				return (1);
			}
		}
		else {
			m = p->part[pi].block_i - i;
			if (m > n) {
				m = n;
			}

			cnt = 512 * (uint64_t) m;

			memset (buf, 0xfe, cnt);
		}

		i += m;
		n -= m;
		buf = (unsigned char *) buf + cnt;
	}

	return (0);
}

static
int dsk_part_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	unsigned    pi;
	uint32_t    j, m;
	uint64_t    ofs, cnt;
	disk_part_t *p;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	p = dsk->ext;

	while (n > 0) {
		if (dsk_part_find (dsk, i, &pi)) {
			return (0);
		}

		if (p->part[pi].block_i <= i) {
			if (p->part[pi].ro) {
				return (1);
			}

			j = i - p->part[pi].block_i;
			m = p->part[pi].block_n - j;

			if (m > n) {
				m = n;
			}

			ofs = p->part[pi].start + 512 * (uint64_t) j;
			cnt = 512 * (uint64_t) m;

			if (dsk_write (p->part[pi].fp, buf, ofs, cnt)) {
				return (1);
			}

			fflush (p->part[pi].fp);
		}
		else {
			m = p->part[pi].block_i - i;
			if (m > n) {
				m = n;
			}
			cnt = 512 * (uint64_t) m;
		}

		i += m;
		n -= m;
		buf = (unsigned char *) buf + cnt;
	}

	return (0);
}

static
void dsk_part_del (disk_t *dsk)
{
	unsigned    i;
	disk_part_t *p;

	p = dsk->ext;

	for (i = 0; i < p->part_cnt; i++) {
		if (p->part[i].close) {
			fclose (p->part[i].fp);
		}
	}

	free (p);
}

int dsk_part_add_partition_fp (disk_t *dsk, FILE *fp, int close,
	uint64_t start, uint32_t blk_i, uint32_t blk_n, int ro)
{
	disk_part_t *p;

	p = dsk->ext;

	p->part[p->part_cnt].block_i = blk_i;
	p->part[p->part_cnt].block_n = blk_n;
	p->part[p->part_cnt].start = start;
	p->part[p->part_cnt].fp = fp;
	p->part[p->part_cnt].close = (close != 0);
	p->part[p->part_cnt].ro = (ro != 0);

	p->part_cnt += 1;

	return (0);
}

int dsk_part_add_partition (disk_t *dsk, const char *fname,
	uint64_t start, uint32_t blk_i, uint32_t blk_n, int ro)
{
	int   r;
	FILE *fp;

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		fp = fopen (fname, "r+b");
	}

	if (fp == NULL) {
		return (1);
	}

	r = dsk_part_add_partition_fp (dsk, fp, 1, start, blk_i, blk_n, ro);

	if (r) {
		fclose (fp);
		return (1);
	}

	return (0);
}

disk_t *dsk_part_open (uint32_t c, uint32_t h, uint32_t s, int ro)
{
	disk_part_t *p;

	p = malloc (sizeof (disk_part_t));
	if (p == NULL) {
		return (NULL);
	}

	dsk_init (&p->dsk, p, 0, c, h, s);

	dsk_set_readonly (&p->dsk, ro);

	p->dsk.del = dsk_part_del;
	p->dsk.read = dsk_part_read;
	p->dsk.write = dsk_part_write;

	p->part_cnt = 0;

	return (&p->dsk);
}
