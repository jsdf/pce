/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkqed.c                                   *
 * Created:     2011-05-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include "blkqed.h"

#include <stdlib.h>
#include <string.h>


#define QED_MAGIC 0x00444551

#define QED_F_BACKING_FILE            0x01
#define QED_F_NEED_CHECK              0x02
#define QED_F_BACKING_FORMAT_NO_PROBE 0x04


static
int dsk_qed_read_header (disk_qed_t *qed)
{
	if (dsk_read (qed->fp, qed->header, 0, sizeof (qed->header))) {
		return (1);
	}

	return (0);
}

static
int dsk_qed_write_header (disk_qed_t *qed)
{
	if (dsk_write (qed->fp, qed->header, 0, sizeof (qed->header))) {
		return (1);
	}

	fflush (qed->fp);

	return (0);
}

static
int dsk_qed_read_l1 (disk_qed_t *qed)
{
	if (dsk_read (qed->fp, qed->t1, qed->l1_table_offset, qed->table_bytes)) {
		return (1);
	}

	return (0);
}

static
int dsk_qed_write_l1 (disk_qed_t *qed)
{
	if (dsk_write (qed->fp, qed->t1, qed->l1_table_offset, qed->table_bytes)) {
		return (1);
	}

	fflush (qed->fp);

	return (0);
}

static
int dsk_qed_read_l2 (disk_qed_t *qed, uint64_t ofs)
{
	if (qed->l2_table_offset == ofs) {
		return (0);
	}

	if (dsk_read (qed->fp, qed->t2, ofs, qed->table_bytes)) {
		return (1);
	}

	qed->l2_table_offset = ofs;

	return (0);
}

static
int dsk_qed_write_l2 (disk_qed_t *qed, uint64_t ofs)
{
	if (dsk_write (qed->fp, qed->t2, ofs, qed->table_bytes)) {
		return (1);
	}

	fflush (qed->fp);

	qed->l2_table_offset = ofs;

	return (0);
}

static
int dsk_qed_read_cluster (disk_qed_t *qed, uint64_t ofs)
{
	if (dsk_read (qed->fp, qed->cl, ofs, qed->cluster_size)) {
		return (1);
	}

	return (0);
}

static
int dsk_qed_write_cluster (disk_qed_t *qed, uint64_t ofs)
{
	if (dsk_write (qed->fp, qed->cl, ofs, qed->cluster_size)) {
		return (1);
	}

	return (0);
}

static
int dsk_qed_read_backing_cluster (disk_qed_t *qed, uint64_t ofs)
{
	unsigned long i, n;

	if (qed->next == NULL) {
		memset (qed->cl, 0, qed->cluster_size);
		return (0);
	}

	ofs &= ~qed->cluster_mask;

	i = ofs / 512;
	n = qed->cluster_size / 512;

	if ((i + n) > qed->next->blocks) {
		memset (qed->cl, 0, qed->cluster_size);

		if (i >= qed->next->blocks) {
			return (0);
		}

		n = qed->next->blocks - i;
	}

	if (dsk_read_lbaz (qed->next, qed->cl, i, n)) {
		return (1);
	}

	return (0);
}

static
int dsk_qed_translate (disk_qed_t *qed, uint64_t *ofs, int alloc)
{
	unsigned long table_entries;
	unsigned long t1idx, t2idx;
	uint64_t      t1ofs, t2ofs;
	uint64_t      cluster_offset;
	unsigned long cluster_index;

	table_entries = qed->table_size * (qed->cluster_size / 8);

	t1idx = (*ofs / qed->cluster_size) / table_entries;
	t2idx = (*ofs / qed->cluster_size) % table_entries;

	t1ofs = dsk_get_uint64_le (qed->t1 + 8 * t1idx, 0);

	cluster_index = *ofs & qed->cluster_mask;

	if (t1idx >= table_entries) {
		return (1);
	}

	if (t1ofs == 0) {
		if (alloc == 0) {
			*ofs = 0;
			return (0);
		}

		cluster_offset = qed->offset + qed->table_bytes;

		if (dsk_qed_read_backing_cluster (qed, *ofs)) {
			return (1);
		}

		if (dsk_qed_write_cluster (qed, cluster_offset)) {
			return (1);
		}

		memset (qed->t2, 0, qed->table_bytes);

		dsk_set_uint64_le (qed->t2 + 8 * t2idx, 0, cluster_offset);

		if (dsk_qed_write_l2 (qed, qed->offset)) {
			return (1);
		}

		dsk_set_uint64_le (qed->t1 + 8 * t1idx, 0, qed->offset);

		if (dsk_qed_write_l1 (qed)) {
			return (1);
		}

		*ofs = cluster_offset + cluster_index;

		qed->offset = cluster_offset + qed->cluster_size;

		return (0);
	}

	t1ofs &= ~qed->cluster_mask;

	if (dsk_qed_read_l2 (qed, t1ofs)) {
		return (1);
	}

	t2ofs = dsk_get_uint64_le (qed->t2 + 8 * t2idx, 0);

	if (t2ofs == 0) {
		if (alloc == 0) {
			*ofs = 0;
			return (0);
		}

		if (dsk_qed_read_backing_cluster (qed, *ofs)) {
			return (1);
		}

		if (dsk_write (qed->fp, qed->cl, qed->offset, qed->cluster_size)) {
			return (1);
		}

		dsk_set_uint64_le (qed->t2 + 8 * t2idx, 0, qed->offset);

		if (dsk_qed_write_l2 (qed, t1ofs)) {
			return (1);
		}

		*ofs = qed->offset + cluster_index;

		qed->offset += qed->cluster_size;

		return (0);
	}

	t2ofs &= ~qed->cluster_mask;

	*ofs = t2ofs + cluster_index;

	return (0);
}

static
int dsk_qed_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	unsigned long k, m;
	uint64_t      ofs;
	disk_qed_t    *qed;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	qed = dsk->ext;

	while (n > 0) {
		k = i % (qed->cluster_size / 512);
		m = (qed->cluster_size / 512) - k;

		if (m > n) {
			m = n;
		}

		ofs = 512 * (uint64_t) i;

		if (dsk_qed_translate (qed, &ofs, 0)) {
			return (1);
		}

		if (ofs == 0) {
			if (qed->next != NULL) {
				if (dsk_read_lbaz (qed->next, buf, i, m)) {
					return (1);
				}
			}
			else {
				memset (buf, 0, 512 * m);
			}
		}
		else {
			if (dsk_read (qed->fp, buf, ofs, 512 * m)) {
				return (1);
			}
		}

		buf = (unsigned char *) buf + 512 * m;

		i += m;
		n -= m;
	}

	return (0);
}

static
int dsk_qed_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	unsigned long k, m;
	uint64_t      ofs;
	disk_qed_t    *qed;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	qed = dsk->ext;

	while (n > 0) {
		k = i % (qed->cluster_size / 512);
		m = (qed->cluster_size / 512) - k;

		if (m > n) {
			m = n;
		}

		ofs = 512 * (uint64_t) i;

		if (dsk_qed_translate (qed, &ofs, 1)) {
			return (1);
		}

		if (dsk_write (qed->fp, buf, ofs, 512 * m)) {
			return (1);
		}

		buf = (unsigned char *) buf + 512 * m;

		i += m;
		n -= m;
	}

	return (0);
}

static
int dsk_qed_commit (disk_qed_t *qed)
{
	unsigned long i, j;
	unsigned long blki, blkn, blkm;
	unsigned long table_entries;
	uint64_t      ofs;

	if (qed->next == NULL) {
		return (1);
	}

	table_entries = qed->table_bytes / 8;

	blki = 0;
	blkn = qed->cluster_size / 512;

	for (i = 0; i < table_entries; i++) {
		ofs = dsk_get_uint64_le (qed->t1, 8 * i);

		if (ofs == 0) {
			blki += table_entries * blkn;
			continue;
		}

		ofs &= ~qed->cluster_mask;

		if (dsk_qed_read_l2 (qed, ofs)) {
			return (1);
		}

		for (j = 0; j < table_entries; j++) {
			ofs = dsk_get_uint64_le (qed->t2, 8 * j);

			if (ofs != 0) {
				ofs &= ~qed->cluster_mask;

				if (dsk_qed_read_cluster (qed, ofs)) {
					return (1);
				}

				if ((blki + blkn) > qed->next->blocks) {
					blkm = qed->next->blocks - blki;
				}
				else {
					blkm = blkn;
				}

				if (dsk_write_lba (qed->next, qed->cl, blki, blkm)) {
					return (1);
				}
			}

			blki += blkn;
		}

		dsk_set_uint64_le (qed->t1, 8 * i, 0);
	}

	if (dsk_qed_write_l1 (qed)) {
		return (1);
	}

	qed->offset = qed->l1_table_offset + qed->table_bytes;

	dsk_set_filesize (qed->fp, qed->offset);

	return (0);
}

static
int dsk_qed_get_msg (disk_t *dsk, const char *msg, char *val, unsigned max)
{
	return (1);
}

static
int dsk_qed_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	if (strcmp (msg, "commit") == 0) {
		return (dsk_qed_commit (dsk->ext));
	}

	return (1);
}
static
void dsk_qed_del (disk_t *dsk)
{
	disk_qed_t *qed;

	qed = dsk->ext;

	if (qed->next != NULL) {
		dsk_del (qed->next);
	}

	free (qed->cl);
	free (qed->t2);
	free (qed->t1);

	fclose (qed->fp);
	free (qed);
}

static
uint32_t dsk_qed_get_min_cluster_size (uint32_t min)
{
	if (min <= 4096) {
		return (4096);
	}

	if (min >= 0x8000000) {
		return (0x8000000);
	}

	min += min - 1;

	while (min & (min - 1)) {
		min &= min - 1;
	}

	return (min);
}

static
int dsk_qed_alloc_tables (disk_qed_t *qed)
{
	qed->t1 = malloc (qed->table_bytes);

	if (qed->t1 == NULL) {
		return (1);
	}

	qed->t2 = malloc (qed->table_bytes);

	if (qed->t2 == NULL) {
		return (1);
	}

	qed->cl = malloc (qed->cluster_size);

	if (qed->cl == NULL) {
		return (1);
	}

	return (0);
}

static
int dsk_qed_parse_header (disk_qed_t *qed)
{
	uint64_t features;

	if (dsk_qed_read_header (qed)) {
		return (1);
	}

	if (dsk_get_uint32_le (qed->header, 0) != QED_MAGIC) {
		return (1);
	}

	qed->header_modified = 0;

	qed->cluster_size = dsk_get_uint32_le (qed->header, 4);
	qed->table_size = dsk_get_uint32_le (qed->header, 8);
	qed->header_size = dsk_get_uint32_le (qed->header, 12);

	if (qed->cluster_size & (qed->cluster_size - 1)) {
		return (1);
	}

	if (qed->table_size & (qed->table_size - 1)) {
		return (1);
	}

	if (qed->header_size < 1) {
		return (1);
	}

	qed->table_bytes = qed->table_size * qed->cluster_size;
	qed->cluster_mask = qed->cluster_size - 1;

	qed->features = dsk_get_uint64_le (qed->header, 16);
	qed->compat_features = dsk_get_uint64_le (qed->header, 24);
	qed->autoclear_features = dsk_get_uint64_le (qed->header, 32);

	features = qed->features;
	features &= ~(uint64_t) QED_F_BACKING_FILE;
	features &= ~(uint64_t) QED_F_BACKING_FORMAT_NO_PROBE;

	if (features != 0) {
		fprintf (stderr, "qed: unknown features (0x%08llx)\n",
			(unsigned long long) features
		);
		return (1);
	}

	if (qed->autoclear_features != 0) {
		qed->header_modified = 1;
		qed->autoclear_features = 0;
		dsk_set_uint64_le (qed->header, 32, qed->autoclear_features);
	}

	qed->l1_table_offset = dsk_get_uint64_le (qed->header, 40);
	qed->l2_table_offset = 0;
	qed->image_size = dsk_get_uint64_le (qed->header, 48);

	if (qed->l1_table_offset & qed->cluster_mask) {
		return (1);
	}

	if (qed->image_size & 511) {
		return (1);
	}

	if (dsk_get_filesize (qed->fp, &qed->offset)) {
		return (1);
	}

	qed->offset = (qed->offset + qed->cluster_size - 1) & ~qed->cluster_mask;

	return (0);
}

disk_t *dsk_qed_open_fp (FILE *fp, int ro)
{
	disk_qed_t *qed;

	qed = malloc (sizeof (disk_qed_t));

	if (qed == NULL) {
		return (NULL);
	}

	qed->fp = fp;

	qed->next = NULL;

	qed->t1 = NULL;
	qed->t2 = NULL;
	qed->cl = NULL;

	if (dsk_qed_parse_header (qed)) {
		free (qed);
		return (NULL);
	}

	if (qed->header_modified && (ro == 0)) {
		if (dsk_qed_write_header (qed)) {
			free (qed);
			return (NULL);
		}
	}

	dsk_init (&qed->dsk, qed, qed->image_size / 512, 0, 0, 0);
	dsk_set_type (&qed->dsk, PCE_DISK_QED);
	dsk_set_readonly (&qed->dsk, ro);

	qed->dsk.del = dsk_qed_del;
	qed->dsk.read = dsk_qed_read;
	qed->dsk.write = dsk_qed_write;

	if (dsk_qed_alloc_tables (qed)) {
		dsk_qed_del (&qed->dsk);
		return (NULL);
	}

	if (dsk_qed_read_l1 (qed)) {
		dsk_qed_del (&qed->dsk);
		return (NULL);
	}

	dsk_guess_geometry (&qed->dsk);

	return (&qed->dsk);
}

disk_t *dsk_qed_open (const char *fname, int ro)
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

	dsk = dsk_qed_open_fp (fp, ro);

	if (dsk == NULL) {
		fclose (fp);
		return (NULL);
	}

	dsk_set_fname (dsk, fname);

	return (dsk);
}

disk_t *dsk_qed_cow_open (disk_t *dsk, const char *fname)
{
	unsigned   n;
	disk_t     *cow;
	disk_qed_t *qed;
	uint64_t   features;

	if ((cow = dsk_qed_open (fname, 0)) == NULL) {
		return (NULL);
	}

	qed = cow->ext;
	qed->next = dsk;

	cow->get_msg = dsk_qed_get_msg;
	cow->set_msg = dsk_qed_set_msg;

	cow->drive = dsk->drive;

	features = dsk_get_uint64_le (qed->header, 16);
	features |= QED_F_BACKING_FILE;

	if (dsk->type == PCE_DISK_RAW) {
		features |= QED_F_BACKING_FORMAT_NO_PROBE;
	}

	dsk_set_uint64_le (qed->header, 16, features);

	n = (dsk->fname != NULL) ? strlen (dsk->fname) : 0;

	if (n > 1024) {
		n = 0;
	}

	dsk_set_uint32_le (qed->header, 56, 64);
	dsk_set_uint32_le (qed->header, 60, n);

	if (n > 0) {
		memcpy (qed->header + 64, dsk->fname, n);
	}

	dsk_qed_write_header (qed);

	dsk_set_geometry (cow, cow->blocks, 0, dsk->h, dsk->s);
	dsk_set_visible_chs (cow, cow->c, cow->h, cow->s);

	dsk_set_fname (cow, fname);

	return (cow);
}

disk_t *dsk_qed_cow_create (disk_t *dsk, const char *fname, uint32_t n, uint32_t minblk)
{
	disk_t *cow;

	if (dsk_qed_create (fname, n, minblk)) {
		return (NULL);
	}

	if ((cow = dsk_qed_cow_open (dsk, fname)) == NULL) {
		return (NULL);
	}

	return (cow);
}

int dsk_qed_create_fp (FILE *fp, uint32_t n, uint32_t minclst)
{
	unsigned long i;
	unsigned long cluster_size, table_size, header_size;
	uint64_t      image_size, max_size, table_entries;
	unsigned char *buf;

	image_size = 512 * (uint64_t) n;

	cluster_size = dsk_qed_get_min_cluster_size (minclst);
	table_size = 2;
	header_size = 1;

	table_entries = (uint64_t) (cluster_size / 8) * table_size;
	max_size = table_entries * table_entries * cluster_size;

	while ((max_size < image_size) && (table_size < 16)) {
		table_size *= 2;
		table_entries = (uint64_t) (cluster_size / 8) * table_size;
		max_size = table_entries * table_entries * cluster_size;
	}

	while ((max_size < image_size) && (cluster_size < 0x8000000)) {
		cluster_size *= 2;
		table_entries = (uint64_t) (cluster_size / 8) * table_size;
		max_size = table_entries * table_entries * cluster_size;
	}

	if (max_size < image_size) {
		return (1);
	}

	buf = malloc (cluster_size);

	if (buf == NULL) {
		return (1);
	}

	memset (buf, 0, cluster_size);

	dsk_set_uint32_le (buf, 0, QED_MAGIC);
	dsk_set_uint32_le (buf, 4, cluster_size);
	dsk_set_uint32_le (buf, 8, table_size);
	dsk_set_uint32_le (buf, 12, header_size);

	dsk_set_uint64_le (buf, 16, 0);
	dsk_set_uint64_le (buf, 24, 0);
	dsk_set_uint64_le (buf, 32, 0);

	dsk_set_uint64_le (buf, 40, (uint64_t) cluster_size * header_size);
	dsk_set_uint64_le (buf, 48, image_size);

	if (dsk_write (fp, buf, 0, cluster_size)) {
		free (buf);
		return (1);
	}

	memset (buf, 0, cluster_size);

	for (i = 1; i < (header_size + table_size); i++) {
		if (dsk_write (fp, buf, (uint64_t) i * cluster_size, cluster_size)) {
			free (buf);
			return (1);
		}
	}

	free (buf);

	return (0);
}

int dsk_qed_create (const char *fname, uint32_t n, uint32_t minclst)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = dsk_qed_create_fp (fp, n, minclst);

	fclose (fp);

	return (r);
}

int dsk_qed_probe_fp (FILE *fp)
{
	unsigned char buf[4];

	if (dsk_read (fp, buf, 0, 4)) {
		return (0);
	}

	if (dsk_get_uint32_le (buf, 0) != QED_MAGIC) {
		return (0);
	}

	return (1);
}

int dsk_qed_probe (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = dsk_qed_probe_fp (fp);

	fclose (fp);

	return (r);
}
