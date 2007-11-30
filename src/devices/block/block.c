/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/block.c                                  *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2007 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include "block.h"

#include "blkraw.h"
#include "blkpce.h"
#include "blkdosem.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>


uint16_t dsk_get_uint16_be (const void *buf, unsigned i)
{
	const unsigned char *tmp;
	uint16_t            v;

	tmp = (const unsigned char *) buf + i;

	v = tmp[0];
	v = (v << 8) | tmp[1];

	return (v);
}

uint32_t dsk_get_uint32_be (const void *buf, unsigned i)
{
	const unsigned char *tmp;
	uint32_t            v;

	tmp = (const unsigned char *) buf + i;

	v = tmp[0];
	v = (v << 8) | tmp[1];
	v = (v << 8) | tmp[2];
	v = (v << 8) | tmp[3];

	return (v);
}

uint64_t dsk_get_uint64_be (const void *buf, unsigned i)
{
	const unsigned char *tmp;
	uint64_t            v;

	tmp = (const unsigned char *) buf + i;

	v = tmp[0];
	v = (v << 8) | tmp[1];
	v = (v << 8) | tmp[2];
	v = (v << 8) | tmp[3];
	v = (v << 8) | tmp[4];
	v = (v << 8) | tmp[5];
	v = (v << 8) | tmp[6];
	v = (v << 8) | tmp[7];

	return (v);
}

void dsk_set_uint16_be (void *buf, unsigned i, uint16_t v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = (v >> 8) & 0xff;
	tmp[1] = v & 0xff;
}

void dsk_set_uint32_be (void *buf, unsigned i, uint32_t v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = (v >> 24) & 0xff;
	tmp[1] = (v >> 16) & 0xff;
	tmp[2] = (v >> 8) & 0xff;
	tmp[3] = v & 0xff;
}

void dsk_set_uint64_be (void *buf, unsigned i, uint64_t v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = (v >> 56) & 0xff;
	tmp[1] = (v >> 48) & 0xff;
	tmp[2] = (v >> 40) & 0xff;
	tmp[3] = (v >> 32) & 0xff;
	tmp[4] = (v >> 24) & 0xff;
	tmp[5] = (v >> 16) & 0xff;
	tmp[6] = (v >> 8) & 0xff;
	tmp[7] = v & 0xff;
}

uint16_t dsk_get_uint16_le (const void *buf, unsigned i)
{
	const unsigned char *tmp;
	uint16_t            v;

	tmp = (const unsigned char *) buf + i;

	v = tmp[1];
	v = (v << 8) | tmp[0];

	return (v);
}

uint32_t dsk_get_uint32_le (const void *buf, unsigned i)
{
	const unsigned char *tmp;
	uint32_t            v;

	tmp = (const unsigned char *) buf + i;

	v = tmp[3];
	v = (v << 8) | tmp[2];
	v = (v << 8) | tmp[1];
	v = (v << 8) | tmp[0];

	return (v);
}

void dsk_set_uint32_le (void *buf, unsigned i, uint32_t v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = v & 0xff;
	tmp[1] = (v >> 8) & 0xff;
	tmp[2] = (v >> 16) & 0xff;
	tmp[3] = (v >> 24) & 0xff;
}


int dsk_read (FILE *fp, void *buf, uint64_t ofs, uint64_t cnt)
{
	size_t r;

#ifdef HAVE_FSEEKO
	if (fseeko (fp, ofs, SEEK_SET)) {
		return (1);
	}
#else
	if (ofs > (uint64_t) LONG_MAX) {
		return (1);
	}

	if (fseek (fp, (long) ofs, SEEK_SET)) {
		return (1);
	}
#endif

	r = fread (buf, 1, cnt, fp);

	if (r < cnt) {
		memset ((unsigned char *) buf + r, 0x00, cnt - r);
	}

	return (0);
}

int dsk_write (FILE *fp, const void *buf, uint64_t ofs, uint64_t cnt)
{
	size_t r;

#ifdef HAVE_FSEEKO
	if (fseeko (fp, ofs, SEEK_SET)) {
		return (1);
	}
#else
	if (ofs > (uint64_t) LONG_MAX) {
		return (1);
	}

	if (fseek (fp, (long) ofs, SEEK_SET)) {
		return (1);
	}
#endif

	r = fwrite (buf, 1, cnt, fp);

	if (r != cnt) {
		return (1);
	}

	return (0);
}

int dsk_get_filesize (FILE *fp, uint64_t *cnt)
{
#ifdef HAVE_FSEEKO
	off_t val;

	if (fseeko (fp, 0, SEEK_END)) {
		return (1);
	}

	val = ftello (fp);
	if (val == (off_t) -1) {
		return (1);
	}
#else
	long val;

	if (fseek (fp, 0, SEEK_END)) {
		return (1);
	}

	val = ftell (fp);
	if (val == -1) {
		return (1);
	}
#endif

	*cnt = val;

	return (0);
}


void dsk_init (disk_t *dsk, void *ext, uint32_t c, uint32_t h, uint32_t s)
{
	dsk->del = NULL;
	dsk->read = NULL;
	dsk->write = NULL;
	dsk->get_msg = NULL;
	dsk->set_msg = NULL;

	dsk->drive = 0;

	dsk->c = c;
	dsk->h = h;
	dsk->s = s;

	dsk->blocks = c * h * s;

	dsk->visible_c = c;
	dsk->visible_h = h;
	dsk->visible_s = s;

	dsk->readonly = 0;

	dsk->ext = ext;
}

void dsk_del (disk_t *dsk)
{
	if ((dsk != NULL) && (dsk->del != NULL)) {
		dsk->del (dsk);
	}
}


void dsk_set_drive (disk_t *dsk, unsigned d)
{
	dsk->drive = d;
}

int dsk_get_readonly (disk_t *dsk)
{
	return (dsk->readonly != 0);
}

void dsk_set_readonly (disk_t *dsk, int v)
{
	dsk->readonly = (v != 0);
}

void dsk_set_visible_chs (disk_t *dsk, uint32_t c, uint32_t h, uint32_t s)
{
	dsk->visible_c = c;
	dsk->visible_h = h;
	dsk->visible_s = s;
}

unsigned dsk_get_drive (const disk_t *dsk)
{
	return (dsk->drive);
}

uint32_t dsk_get_block_cnt (const disk_t *dsk)
{
	return (dsk->blocks);
}


disk_t *dsk_auto_open (const char *fname, uint64_t ofs, int ro)
{
	disk_t *dsk;

	dsk = dsk_pce_open (fname, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	dsk = dsk_dosemu_open (fname, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	dsk = dsk_dosimg_open (fname, ofs, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	dsk = dsk_mbrimg_open (fname, ofs, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	dsk = dsk_hfsimg_open (fname, ofs, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	dsk = dsk_macimg_open (fname, ofs, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	dsk = dsk_fdimg_open (fname, ofs, ro);
	if (dsk != NULL) {
		return (dsk);
	}

	return (NULL);
}


int dsk_get_lba (disk_t *dsk, uint32_t c, uint32_t h, uint32_t s, uint32_t *v)
{
	if ((s < 1) || (s > dsk->s)) {
		return (1);
	}

	if ((h >= dsk->h) || (c >= dsk->c)) {
		return (1);
	}

	*v = ((c * dsk->h + h) * dsk->s + s - 1);

	return (0);
}

int dsk_read_lba (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	if (dsk->read != NULL) {
		return (dsk->read (dsk, buf, i, n));
	}

	return (1);
}

int dsk_read_chs (disk_t *dsk, void *buf,
	uint32_t c, uint32_t h, uint32_t s, uint32_t n)
{
	uint32_t i;

	if (dsk_get_lba (dsk, c, h, s, &i)) {
		return (1);
	}

	return (dsk_read_lba (dsk, buf, i, n));
}

int dsk_write_lba (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	if (dsk->write != NULL) {
		return (dsk->write (dsk, buf, i, n));
	}

	return (1);
}

int dsk_write_chs (disk_t *dsk, const void *buf,
	uint32_t c, uint32_t h, uint32_t s, uint32_t n)
{
	uint32_t i;

	if (dsk_get_lba (dsk, c, h, s, &i)) {
		return (1);
	}

	return (dsk_write_lba (dsk, buf, i, n));
}

int dsk_commit (disk_t *dsk)
{
	return (dsk_set_msg (dsk, "commit", NULL));
}

int dsk_get_msg (disk_t *dsk, const char *msg, char *val, unsigned max)
{
	if (dsk->get_msg != NULL) {
		return (dsk->get_msg (dsk, msg, val, max));
	}

	return (1);
}

int dsk_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	if (dsk->set_msg != NULL) {
		if (val == NULL) {
			val = "";
		}

		return (dsk->set_msg (dsk, msg, val));
	}

	return (1);
}


disks_t *dsks_new (void)
{
	disks_t *dsks;

	dsks = (disks_t *) malloc (sizeof (disks_t));
	if (dsks == NULL) {
		return (NULL);
	}

	dsks->cnt = 0;
	dsks->dsk = NULL;

	return (dsks);
}

void dsks_del (disks_t *dsks)
{
	unsigned i;

	if (dsks == NULL) {
		return;
	}

	for (i = 0; i < dsks->cnt; i++) {
		dsk_del (dsks->dsk[i]);
	}

	free (dsks->dsk);
	free (dsks);
}

int dsks_add_disk (disks_t *dsks, disk_t *dsk)
{
	unsigned i, n;
	disk_t   **tmp;

	for (i = 0; i < dsks->cnt; i++) {
		if (dsks->dsk[i]->drive == dsk->drive) {
			return (1);
		}
	}

	n = dsks->cnt + 1;
	tmp = (disk_t **) realloc (dsks->dsk, n * sizeof (disk_t *));
	if (tmp == NULL) {
		return (1);
	}

	tmp[dsks->cnt] = dsk;

	dsks->cnt = n;
	dsks->dsk = tmp;

	return (0);
}

int dsks_rmv_disk (disks_t *dsks, disk_t *dsk)
{
	int      r;
	unsigned i, j;

	r = 0;
	j = 0;
	for (i = 0; i < dsks->cnt; i++) {
		if (dsks->dsk[i] != dsk) {
			dsks->dsk[j] = dsks->dsk[i];
			j += 1;
		}
		else {
			r = 1;
		}
	}

	dsks->cnt = j;

	return (r);
}

disk_t *dsks_get_disk (disks_t *dsks, unsigned drive)
{
	unsigned i;

	for (i = 0; i < dsks->cnt; i++) {
		if (dsks->dsk[i]->drive == drive) {
			return (dsks->dsk[i]);
		}
	}

	return (NULL);
}

int dsks_commit (disks_t *dsks)
{
	unsigned i;
	int      r;

	r = 0;

	for (i = 0; i < dsks->cnt; i++) {
		if (dsks->dsk[i]->set_msg != NULL) {
			if (dsk_set_msg (dsks->dsk[i], "commit", NULL)) {
				r = 1;
			}
		}
	}

	return (r);
}

int dsks_get_msg (disks_t *dsks, unsigned drv, const char *msg, char *val, unsigned max)
{
	disk_t *dsk;

	dsk = dsks_get_disk (dsks, drv);
	if (dsk == NULL) {
		return (1);
	}

	return (dsk_get_msg (dsk, msg, val, max));
}

int dsks_set_msg (disks_t *dsks, unsigned drv, const char *msg, const char *val)
{
	disk_t *dsk;

	dsk = dsks_get_disk (dsks, drv);
	if (dsk == NULL) {
		return (1);
	}

	if (val == NULL) {
		val = "";
	}

	return (dsk_set_msg (dsk, msg, val));
}
