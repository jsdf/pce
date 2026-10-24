/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkpbi.c                                   *
 * Created:     2018-02-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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
#include <drivers/block/blkpbi.h>

#include <stdlib.h>
#include <string.h>


#define PBI_MAGIC_PBI  0x50424920
#define PBI_MAGIC_PBIN 0x5042496e

#define XLAT_OK      0
#define XLAT_ERR     1
#define XLAT_NOALLOC 2
#define XLAT_UNIFORM 3

#define PBI_TRANSFORM 1
#define PBI_UNIFORM   2


static
int pbi_read_header (disk_pbi_t *pbi)
{
	if (dsk_read (pbi->fp, pbi->header, 0, sizeof (pbi->header))) {
		return (1);
	}

	return (0);
}

static
int pbi_write_header (disk_pbi_t *pbi)
{
	if (dsk_write (pbi->fp, pbi->header, 0, sizeof (pbi->header))) {
		return (1);
	}

	pbi->header_modified = 0;

	return (0);
}

static
int pbi_write_file_size (disk_pbi_t *pbi, uint64_t size)
{
	pbi->file_size = size;

	dsk_set_uint64_be (pbi->header, 32, size);

	if (pbi_write_header (pbi)) {
		return (1);
	}

	return (0);
}

static
int pbi_read_l1 (disk_pbi_t *pbi)
{
	if (dsk_read (pbi->fp, pbi->t1, pbi->l1_table_offset, pbi->l1_table_size)) {
		return (1);
	}

	return (0);
}

static
int pbi_write_l1 (disk_pbi_t *pbi)
{
	if (dsk_write (pbi->fp, pbi->t1, pbi->l1_table_offset, pbi->l1_table_size)) {
		return (1);
	}

	fflush (pbi->fp);

	return (0);
}

static
int pbi_read_l2 (disk_pbi_t *pbi, uint64_t ofs)
{
	if (pbi->l2_table_offset == ofs) {
		return (0);
	}

	if (dsk_read (pbi->fp, pbi->t2, ofs, pbi->l2_table_size)) {
		return (1);
	}

	pbi->l2_table_offset = ofs;

	return (0);
}

static
int pbi_write_l2 (disk_pbi_t *pbi)
{
	if (dsk_write (pbi->fp, pbi->t2, pbi->l2_table_offset, pbi->l2_table_size)) {
		return (1);
	}

	fflush (pbi->fp);

	return (0);
}

static
int pbi_read_block (disk_pbi_t *pbi, uint64_t ofs)
{
	if (dsk_read (pbi->fp, pbi->bl, ofs, pbi->block_size)) {
		return (1);
	}

	return (0);
}

static
int pbi_write_block (disk_pbi_t *pbi, uint64_t ofs)
{
	if (dsk_write (pbi->fp, pbi->bl, ofs, pbi->block_size)) {
		return (1);
	}

	return (0);
}

static
void pbi_fill_block_uint32 (void *buf, unsigned long cnt, unsigned long val)
{
	unsigned long i;
	unsigned char *dst;
	unsigned char src[4];

	dsk_set_uint32_be (src, 0, val);

	dst = buf;

	for (i = 0; i < cnt; i++) {
		dst[i] = src[i & 3];
	}
}

static
int pbi_read_backing_block (disk_pbi_t *pbi, uint64_t ofs)
{
	unsigned long i, n;

	if (pbi->next == NULL) {
		memset (pbi->bl, 0, pbi->block_size);
		return (0);
	}

	ofs &= ~pbi->block_mask;

	i = ofs / 512;
	n = pbi->block_size / 512;

	if ((i + n) > pbi->next->blocks) {
		memset (pbi->bl, 0, pbi->block_size);

		if (i >= pbi->next->blocks) {
			return (0);
		}

		n = pbi->next->blocks - i;
	}

	if (dsk_read_lbaz (pbi->next, pbi->bl, i, n)) {
		return (1);
	}

	return (0);
}

static
int pbi_alloc_l2 (disk_pbi_t *pbi, uint64_t ofs)
{
	unsigned long l1idx;
	uint64_t      l1val;

	l1idx = ofs >> (pbi->blbits + pbi->l2bits);

	if (l1idx >= pbi->l1_table_entries) {
		return (1);
	}

	l1val = dsk_get_uint64_be (pbi->t1, l1idx << 3);

	if (l1val == 0) {
		pbi->l2_table_offset = pbi->file_size;

		memset (pbi->t2, 0, pbi->l2_table_size);

		if (pbi_write_l2 (pbi)) {
			return (1);
		}

		if (pbi_write_file_size (pbi, pbi->file_size + pbi->l2_table_size)) {
			return (1);
		}

		dsk_set_uint64_be (pbi->t1, l1idx << 3, pbi->l2_table_offset);

		if (pbi_write_l1 (pbi)) {
			return (1);
		}

		return (0);
	}
	else if (l1val & 0x1ff) {
		return (1);
	}

	l1val &= ~pbi->block_mask;

	if (pbi_read_l2 (pbi, l1val)) {
		return (1);
	}

	return (0);
}

static
unsigned pbi_translate_read (disk_pbi_t *pbi, uint64_t *ofs)
{
	unsigned long block_index;
	unsigned long t1idx, t2idx;
	uint64_t      t1val, t2val;

	t1idx = *ofs >> (pbi->blbits + pbi->l2bits);
	t2idx = (*ofs >> pbi->blbits) & pbi->l2_mask;

	if (t1idx >= pbi->l1_table_entries) {
		return (XLAT_ERR);
	}

	t1val = dsk_get_uint64_be (pbi->t1 + 8 * t1idx, 0);

	if (t1val == 0) {
		return (XLAT_NOALLOC);
	}
	else if (t1val & 0x1ff) {
		return (XLAT_ERR);
	}

	block_index = *ofs & pbi->block_mask;

	t1val &= ~pbi->block_mask;

	if (pbi_read_l2 (pbi, t1val)) {
		return (XLAT_ERR);
	}

	t2val = dsk_get_uint64_be (pbi->t2 + 8 * t2idx, 0);

	if (t2val == 0) {
		return (XLAT_NOALLOC);
	}
	else if (t2val & PBI_UNIFORM) {
		*ofs = t2val >> 32;
		return (XLAT_UNIFORM);
	}
	else if (t2val & 0x1ff) {
		return (XLAT_ERR);
	}

	*ofs = (t2val & ~pbi->block_mask) + block_index;

	return (XLAT_OK);
}

static
int pbi_translate_write (disk_pbi_t *pbi, uint64_t *ofs)
{
	unsigned long block_index;
	unsigned long l2idx;
	uint64_t      l2val;
	uint64_t      block_offset;

	if (pbi_alloc_l2 (pbi, *ofs)) {
		return (1);
	}

	l2idx = (*ofs >> pbi->blbits) & pbi->l2_mask;
	l2val = dsk_get_uint64_be (pbi->t2 + 8 * l2idx, 0);

	block_index = *ofs & pbi->block_mask;

	if ((l2val == 0) || (l2val & PBI_UNIFORM)) {
		block_offset = pbi->file_size;

		if (l2val == 0) {
			if (pbi_read_backing_block (pbi, *ofs)) {
				return (1);
			}
		}
		else {
			pbi_fill_block_uint32 (pbi->bl, pbi->block_size, l2val >> 32);
		}

		if (pbi_write_block (pbi, block_offset)) {
			return (1);
		}

		dsk_set_uint64_be (pbi->t2, 8 * l2idx, block_offset);

		if (pbi_write_l2 (pbi)) {
			return (1);
		}

		*ofs = block_offset + block_index;

		if (pbi_write_file_size (pbi, pbi->file_size + pbi->block_size)) {
			return (1);
		}

		return (0);
	}
	else if (l2val & 0x1ff) {
		return (1);
	}

	*ofs = (l2val & ~pbi->block_mask) + block_index;

	return (0);
}

static
int pbi_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	unsigned long k, m;
	unsigned long mask;
	uint64_t      ofs;
	disk_pbi_t    *pbi;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	pbi = dsk->ext;

	mask = (1UL << (pbi->blbits - 9)) - 1;

	while (n > 0) {
		k = i & mask;
		m = (pbi->block_size / 512) - k;

		if (m > n) {
			m = n;
		}

		ofs = 512 * (uint64_t) i;

		switch (pbi_translate_read (pbi, &ofs)) {
		case XLAT_OK:
			if (dsk_read (pbi->fp, buf, ofs, 512 * m)) {
				return (1);
			}
			break;

		case XLAT_NOALLOC:
			if (pbi->next != NULL) {
				if (dsk_read_lbaz (pbi->next, buf, i, m)) {
					return (1);
				}
			}
			else {
				memset (buf, 0, 512 * m);
			}
			break;

		case XLAT_UNIFORM:
			pbi_fill_block_uint32 (buf, 512 * m, ofs);
			break;

		default:
			return (1);
		}


		buf = (unsigned char *) buf + 512 * m;

		i += m;
		n -= m;
	}

	return (0);
}

static
int pbi_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	unsigned long k, m;
	unsigned long mask;
	uint64_t      ofs;
	disk_pbi_t    *pbi;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	pbi = dsk->ext;

	mask = (1UL << (pbi->blbits - 9)) - 1;

	while (n > 0) {
		k = i & mask;
		m = (pbi->block_size / 512) - k;

		if (m > n) {
			m = n;
		}

		ofs = 512 * (uint64_t) i;

		if (pbi_translate_write (pbi, &ofs)) {
			return (1);
		}

		if (dsk_write (pbi->fp, buf, ofs, 512 * m)) {
			return (1);
		}

		buf = (unsigned char *) buf + 512 * m;

		i += m;
		n -= m;
	}

	if (pbi->header_modified) {
		if (pbi_write_header (pbi)) {
			return (1);
		}
	}

	return (0);
}

static
int pbi_commit (disk_pbi_t *pbi)
{
	unsigned long i, j;
	unsigned long blki, blkn, blkm;
	uint64_t      ofs;

	if (pbi->next == NULL) {
		return (1);
	}

	blki = 0;
	blkn = pbi->block_size / 512;

	for (i = 0; i < pbi->l1_table_entries; i++) {
		ofs = dsk_get_uint64_be (pbi->t1, 8 * i);

		if (ofs == 0) {
			blki += pbi->l2_table_entries * blkn;
			continue;
		}

		ofs &= ~pbi->block_mask;

		if (pbi_read_l2 (pbi, ofs)) {
			return (1);
		}

		for (j = 0; j < pbi->l2_table_entries; j++) {
			ofs = dsk_get_uint64_be (pbi->t2, 8 * j);

			if (ofs != 0) {
				ofs &= ~pbi->block_mask;

				if (pbi_read_block (pbi, ofs)) {
					return (1);
				}

				if ((blki + blkn) > pbi->next->blocks) {
					blkm = pbi->next->blocks - blki;
				}
				else {
					blkm = blkn;
				}

				if (dsk_write_lba (pbi->next, pbi->bl, blki, blkm)) {
					return (1);
				}
			}

			blki += blkn;
		}

		dsk_set_uint64_be (pbi->t1, 8 * i, 0);
	}

	if (pbi_write_l1 (pbi)) {
		return (1);
	}

	if (pbi_write_file_size (pbi, pbi->l1_table_offset + pbi->l1_table_size)) {
		return (1);
	}

	dsk_set_filesize (pbi->fp, pbi->file_size);

	return (0);
}

static
int pbi_get_msg (disk_t *dsk, const char *msg, char *val, unsigned max)
{
	return (1);
}

static
int pbi_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	if (strcmp (msg, "commit") == 0) {
		return (pbi_commit (dsk->ext));
	}

	return (1);
}

static
void pbi_del (disk_t *dsk)
{
	disk_pbi_t *pbi;

	pbi = dsk->ext;

	if (pbi->next != NULL) {
		dsk_del (pbi->next);
	}

	free (pbi->bl);
	free (pbi->t2);
	free (pbi->t1);

	fclose (pbi->fp);
	free (pbi);
}

int dsk_pbi_set_uniform (disk_pbi_t *pbi, uint64_t ofs, unsigned long val)
{
	unsigned long l2idx;
	uint64_t      l2val;

	if (ofs & pbi->block_mask) {
		return (1);
	}

	if (pbi_alloc_l2 (pbi, ofs)) {
		return (1);
	}

	l2idx = (ofs >> pbi->blbits) & pbi->l2_mask;
	l2val = ((uint64_t) val << 32) | PBI_UNIFORM;

	dsk_set_uint64_be (pbi->t2, l2idx << 3, l2val);

	if (pbi_write_l2 (pbi)) {
		return (1);
	}

	return (0);
}

/*
 * Get the number of block bits of blocks with minimal size min.
 */
static
uint32_t pbi_get_block_bits (uint32_t min)
{
	unsigned cnt;

	if (min == 0) {
		return (12);
	}

	if (min <= 512) {
		return (9);
	}

	if (min >= (1UL << 20)) {
		return (20);
	}

	min = (min + min - 1) >> 1;
	cnt = 0;

	while (min > 0) {
		min >>= 1;
		cnt += 1;
	}

	return (cnt);
}

/*
 * Get the l1 bits, l2 bits and block bits for an image of size size and
 * a minimum block size of minblk.
 */
static
int pbi_get_bits (uint64_t size, uint32_t minblk, unsigned *l1bits, unsigned *l2bits, unsigned *blbits)
{
	uint64_t max_size;

	*blbits = pbi_get_block_bits (minblk);
	*l1bits = *blbits - 3;
	*l2bits = *blbits - 3;

	max_size = 1ULL << (*l1bits + *l2bits + *blbits);

	while (max_size < size) {
		if (*l1bits < 17) {
			*l1bits += 1;
		}
		else if (*l2bits < 17) {
			*l2bits += 1;
		}
		else {
			*blbits += 1;
		}

		max_size = 1ULL << (*l1bits + *l2bits + *blbits);
	}

	return (0);
}

static
int pbi_alloc_tables (disk_pbi_t *pbi)
{
	pbi->t1 = malloc (pbi->l1_table_size);

	if (pbi->t1 == NULL) {
		return (1);
	}

	pbi->t2 = malloc (pbi->l2_table_size);

	if (pbi->t2 == NULL) {
		return (1);
	}

	pbi->bl = malloc (pbi->block_size);

	if (pbi->bl == NULL) {
		return (1);
	}

	return (0);
}

static
int pbi_parse_header (disk_pbi_t *pbi)
{
	unsigned long magic;
	unsigned char *p;

	if (pbi_read_header (pbi)) {
		return (1);
	}

	p = pbi->header;

	magic = dsk_get_uint32_be (p, 0);

	if (magic == PBI_MAGIC_PBIN) {
		unsigned long block_size;
		uint64_t      file_size;

		if ((p[14] < 9) || (p[14] > 24)) {
			return (1);
		}

		block_size = 1UL << p[14];

		if (dsk_get_filesize (pbi->fp, &file_size)) {
			return (1);
		}

		if (file_size < (3 * block_size)) {
			return (1);
		}

		file_size -= block_size;

		if (dsk_read (pbi->fp, p, file_size, sizeof (pbi->header))) {
			return (1);
		}

		pbi->header_modified = 1;

		magic = dsk_get_uint32_be (pbi->header, 0);
	}

	if (magic != PBI_MAGIC_PBI) {
		return (1);
	}

	if (dsk_get_uint32_be (p, 4) != 0) {
		return (1);
	}

	pbi->header_size = dsk_get_uint32_be (p, 8);

	if (pbi->header_size < 48) {
		return (1);
	}

	pbi->l1bits = p[12];
	pbi->l2bits = p[13];
	pbi->blbits = p[14];

	if (pbi->blbits < 9) {
		return (1);
	}

	if ((pbi->l1bits > 24) || (pbi->l2bits > 24) || (pbi->blbits > 24)) {
		return (1);
	}

	pbi->image_size = dsk_get_uint64_be (p, 16);
	pbi->l1_table_offset = dsk_get_uint64_be (p, 24);
	pbi->file_size = dsk_get_uint64_be (p, 32);

	if (pbi->image_size & 511) {
		return (1);
	}

	pbi->c = dsk_get_uint32_be (p, 40);
	pbi->h = dsk_get_uint16_be (p, 44);
	pbi->s = dsk_get_uint16_be (p, 46);

	pbi->block_size = 1UL << pbi->blbits;
	pbi->block_mask = pbi->block_size - 1;
	pbi->l2_mask = (1UL << pbi->l2bits) - 1;

	pbi->l1_table_entries = 1UL << pbi->l1bits;
	pbi->l2_table_entries = 1UL << pbi->l2bits;

	pbi->l1_table_size = pbi->l1_table_entries << 3;
	pbi->l2_table_size = pbi->l2_table_entries << 3;

	pbi->l2_table_offset = 0;

	pbi->file_size = (pbi->file_size + pbi->block_size - 1) & ~pbi->block_mask;

	return (0);
}

disk_t *dsk_pbi_open_fp (FILE *fp, int ro)
{
	disk_pbi_t *pbi;

	if ((pbi = malloc (sizeof (disk_pbi_t))) == NULL) {
		return (NULL);
	}

	pbi->next = NULL;

	pbi->t1 = NULL;
	pbi->t2 = NULL;
	pbi->bl = NULL;

	pbi->header_modified = 0;

	pbi->fp = fp;

	if (pbi_parse_header (pbi)) {
		free (pbi);
		return (NULL);
	}

	dsk_init (&pbi->dsk, pbi, pbi->image_size / 512, pbi->c, pbi->h, pbi->s);
	dsk_set_type (&pbi->dsk, PCE_DISK_PBI);
	dsk_set_readonly (&pbi->dsk, ro);

	pbi->dsk.del = pbi_del;
	pbi->dsk.read = pbi_read;
	pbi->dsk.write = pbi_write;

	if (pbi_alloc_tables (pbi)) {
		pbi_del (&pbi->dsk);
		return (NULL);
	}

	if (pbi_read_l1 (pbi)) {
		pbi_del (&pbi->dsk);
		return (NULL);
	}

	if ((pbi->c == 0) || (pbi->h == 0) || (pbi->s == 0)) {
		dsk_guess_geometry (&pbi->dsk);
	}

	return (&pbi->dsk);
}

disk_t *dsk_pbi_open (const char *fname, int ro)
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

	if ((dsk = dsk_pbi_open_fp (fp, ro)) == NULL) {
		fclose (fp);
		return (NULL);
	}

	dsk_set_fname (dsk, fname);

	return (dsk);
}

disk_t *dsk_pbi_cow_open (disk_t *dsk, const char *fname)
{
	disk_t     *cow;
	disk_pbi_t *pbi;

	if ((cow = dsk_pbi_open (fname, 0)) == NULL) {
		return (NULL);
	}

	pbi = cow->ext;
	pbi->next = dsk;

	cow->get_msg = pbi_get_msg;
	cow->set_msg = pbi_set_msg;

	cow->drive = dsk->drive;

	dsk_set_fname (cow, fname);

	return (cow);
}

disk_t *dsk_pbi_cow_create (disk_t *dsk, const char *fname, uint32_t n, uint32_t c, uint32_t h, uint32_t s, uint32_t minblk)
{
	disk_t *cow;

	if (dsk_pbi_create (fname, n, c, h, s, minblk)) {
		return (NULL);
	}

	if ((cow = dsk_pbi_cow_open (dsk, fname)) == NULL) {
		return (NULL);
	}

	return (cow);
}

int dsk_pbi_create_flat_fp (FILE *fp, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk)
{
	unsigned long l1idx, l2idx;
	unsigned long l1size, l2size, blsize;
	unsigned      l1bits, l2bits, blbits, l1blk, l2blk;
	uint64_t      blidx, blcnt;
	uint64_t      l2ofs, blofs;
	uint64_t      image_size;
	unsigned char *buf, *l1, *l2;

	image_size = 512 * (uint64_t) n;

	if (pbi_get_bits (image_size, minblk, &l1bits, &l2bits, &blbits)) {
		return (1);
	}

	blsize = 1UL << blbits;
	blcnt = (image_size + blsize - 1) >> blbits;

	l1blk = 1UL << (l1bits + 3 - blbits);
	l1size = l1blk << blbits;

	l2blk = 1UL << (l2bits + 3 - blbits);
	l2size = l2blk << blbits;

	if ((buf = malloc (l1size + l2size)) == NULL) {
		return (1);
	}

	l1 = buf;
	l2 = buf + l1size;

	blofs = blsize;
	l2ofs = (blcnt + 1) << blbits;
	l1idx = 0;
	l2idx = 0;

	memset (l1, 0, l1size);

	for (blidx = 0; blidx < blcnt; blidx++) {
		if (l2idx == 0) {
			memset (l2, 0, l2size);
			dsk_set_uint64_be (l1, l1idx, l2ofs);
			l1idx += 8;
		}

		dsk_set_uint64_be (l2, l2idx, blofs);

		l2idx += 8;
		blofs += blsize;

		if ((l2idx >= blsize) || ((blidx + 1) == blcnt)) {
			if (dsk_write (fp, l2, l2ofs, l2size)) {
				free (buf);
				return (1);
			}

			l2idx = 0;
			l2ofs += l2size;
		}
	}

	if (dsk_write (fp, l1, l2ofs, l1size)) {
		free (buf);
		return (1);
	}

	memset (buf, 0, blsize);

	dsk_set_uint32_be (buf, 0, PBI_MAGIC_PBI);
	dsk_set_uint32_be (buf, 4, 0);
	dsk_set_uint32_be (buf, 8, 48);

	buf[12] = l1bits;
	buf[13] = l2bits;
	buf[14] = blbits;

	dsk_set_uint64_be (buf, 16, image_size);
	dsk_set_uint64_be (buf, 24, l2ofs);
	dsk_set_uint64_be (buf, 32, l2ofs + l1size);

	dsk_set_uint32_be (buf, 40, c);
	dsk_set_uint16_be (buf, 44, h);
	dsk_set_uint16_be (buf, 46, s);

	if (dsk_write (fp, buf, 0, blsize)) {
		free (buf);
		return (1);
	}

	free (buf);

	return (0);
}

int dsk_pbi_create_flat (const char *fname, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = dsk_pbi_create_flat_fp (fp, n, c, h, s, minblk);

	fclose (fp);

	return (r);
}

int dsk_pbi_create_fp (FILE *fp, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk)
{
	unsigned long i;
	unsigned long blsize;
	unsigned      l1bits, l2bits, blbits, l1blk;
	uint64_t      image_size;
	unsigned char *buf;

	image_size = 512 * (uint64_t) n;

	if (pbi_get_bits (image_size, minblk, &l1bits, &l2bits, &blbits)) {
		return (1);
	}

	blsize = 1UL << blbits;
	l1blk = 1UL << (l1bits + 3 - blbits);

	if ((buf = malloc (blsize)) == NULL) {
		return (1);
	}

	memset (buf, 0, blsize);

	dsk_set_uint32_be (buf, 0, PBI_MAGIC_PBI);
	dsk_set_uint32_be (buf, 4, 0);
	dsk_set_uint32_be (buf, 8, 48);

	buf[12] = l1bits;
	buf[13] = l2bits;
	buf[14] = blbits;

	dsk_set_uint64_be (buf, 16, image_size);
	dsk_set_uint64_be (buf, 24, blsize);
	dsk_set_uint64_be (buf, 32, (l1blk + 1) * blsize);

	dsk_set_uint32_be (buf, 40, c);
	dsk_set_uint16_be (buf, 44, h);
	dsk_set_uint16_be (buf, 46, s);

	if (dsk_write (fp, buf, 0, blsize)) {
		free (buf);
		return (1);
	}

	memset (buf, 0, blsize);

	for (i = 0; i < l1blk; i++) {
		if (dsk_write (fp, buf, (uint64_t) (i + 1) * blsize, blsize)) {
			free (buf);
			return (1);
		}
	}

	free (buf);

	return (0);
}

int dsk_pbi_create (const char *fname, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = dsk_pbi_create_fp (fp, n, c, h, s, minblk);

	fclose (fp);

	return (r);
}

int dsk_pbi_probe_fp (FILE *fp)
{
	unsigned long magic;
	unsigned char buf[8];

	if (dsk_read (fp, buf, 0, 8)) {
		return (0);
	}

	magic = dsk_get_uint32_be (buf, 0);

	if ((magic != PBI_MAGIC_PBI) && (magic != PBI_MAGIC_PBIN)) {
		return (0);
	}

	if (dsk_get_uint32_be (buf, 4) != 0) {
		return (0);
	}

	return (1);
}

int dsk_pbi_probe (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = dsk_pbi_probe_fp (fp);

	fclose (fp);

	return (r);
}
