/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/fdc.c                                            *
 * Created:     2007-09-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <devices/fdc.h>

#include <drivers/block/blkfdc.h>


/*
 * This is the glue code between the 8272 FDC, block devices and
 * the rest of the emulator.
 */


static
struct {
	unsigned v1;
	unsigned v2;
} errmap[] = {
	{ PCE_BLK_FDC_NO_ID,    E8272_ERR_NO_ID },
	{ PCE_BLK_FDC_NO_DATA,  E8272_ERR_NO_DATA },
	{ PCE_BLK_FDC_CRC_ID,   E8272_ERR_CRC_ID },
	{ PCE_BLK_FDC_CRC_DATA, E8272_ERR_CRC_DATA },
	{ PCE_BLK_FDC_DATALEN,  E8272_ERR_DATALEN },
	{ PCE_BLK_FDC_WPROT,    E8272_ERR_WPROT },
	{ 0, 0 }
};


static void dev_fdc_del_dev (device_t *dev);


/*
 * Map an error code from PCE_BLK_FDC_* to E8272_ERR_*.
 */
static
unsigned dev_fdc_map_error (unsigned err)
{
	unsigned i;
	unsigned ret;

	ret = 0;

	i = 0;

	while (errmap[i].v1 != 0) {
		if (err & errmap[i].v1) {
			err &= ~errmap[i].v1;
			ret |= errmap[i].v2;
		}

		i += 1;
	}

	if (err) {
		ret |= E8272_ERR_OTHER;
	}

	return (ret);
}

static
unsigned dev_fdc_block_read (dev_fdc_t *fdc, void *buf, unsigned *cnt,
	unsigned d, unsigned pc, unsigned ph, unsigned ps, unsigned s)
{
	unsigned ret, err;
	disk_t   *dsk;

	dsk = dsks_get_disk (fdc->dsks, fdc->drive[d & 3]);

	if (dsk == NULL) {
		*cnt = 0;
		return (E8272_ERR_NO_DATA);
	}

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		if (*cnt < 512) {
			*cnt = 0;
			return (E8272_ERR_OTHER);
		}

		if (dsk_read_chs (dsk, buf, pc, ph, s, 1)) {
			*cnt = 0;
			return (E8272_ERR_NO_DATA);
		}

		*cnt = 512;

		return (0);
	}

	err = dsk_fdc_read_chs (dsk->ext, buf, cnt, pc, ph, ps, 1);

	ret = dev_fdc_map_error (err);

	return (ret);
}

static
unsigned dev_fdc_block_write (dev_fdc_t *fdc, void *buf, unsigned *cnt,
	unsigned d, unsigned pc, unsigned ph, unsigned ps, unsigned s)
{
	unsigned err, ret;
	disk_t   *dsk;

	dsk = dsks_get_disk (fdc->dsks, fdc->drive[d & 3]);

	if (dsk == NULL) {
		*cnt = 0;
		return (E8272_ERR_NO_DATA);
	}

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		if (*cnt != 512) {
			return (E8272_ERR_OTHER);
		}

		if (dsk_write_chs (dsk, buf, pc, ph, s, 1)) {
			*cnt = 0;
			return (E8272_ERR_OTHER);
		}

		return (0);
	}

	err = dsk_fdc_write_chs (dsk->ext, buf, cnt, pc, ph, ps, 1);

	ret = dev_fdc_map_error (err);

	return (ret);
}

static
int dev_fdc_block_fmt (dev_fdc_t *fdc,
	unsigned d,
	unsigned pc, unsigned ph, unsigned ps,
	unsigned lc, unsigned lh, unsigned ls, unsigned ln,
	unsigned fill)
{
	unsigned      cnt;
	disk_t        *dsk;
	unsigned char buf[512];

	dsk = dsks_get_disk (fdc->dsks, fdc->drive[d & 3]);

	if (dsk == NULL) {
		return (E8272_ERR_NO_DATA);
	}

	cnt = 128 << ln;

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		if (cnt != 512) {
			return (E8272_ERR_OTHER);
		}

		memset (buf, fill, 512);

		if (dsk_write_chs (dsk, buf, pc, ph, ps + 1, 1)) {
			return (E8272_ERR_OTHER);
		}

		return (0);
	}

	if (ps == 0) {
		dsk_fdc_erase_track (dsk->ext, pc, ph);
	}

	if (dsk_fdc_format_sector (dsk->ext, pc, ph, lc, lh, ls, cnt, fill)) {
		return (E8272_ERR_OTHER);
	}

	return (0);
}

static
int dev_fdc_block_rdid (dev_fdc_t *fdc,
	unsigned d, unsigned pc, unsigned ph, unsigned ps,
	unsigned *lc, unsigned *lh, unsigned *ls, unsigned *ln)
{
	unsigned      cnt;
	disk_t        *dsk;
	unsigned char buf[512];

	dsk = dsks_get_disk (fdc->dsks, fdc->drive[d & 3]);

	if (dsk == NULL) {
		return (1);
	}

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		if (dsk_read_chs (dsk, buf, pc, ph, ps + 1, 1)) {
			return (1);
		}

		*lc = pc;
		*lh = ph;
		*ls = ps + 1;
		*ln = 2;

		return (0);
	}

	if (dsk_fdc_read_id (dsk->ext, pc, ph, ps, lc, lh, ls, &cnt)) {
		return (1);
	}

	*ln = 0;

	while (cnt > 128) {
		cnt >>= 1;
		*ln += 1;
	}

	return (0);
}


static
void dev_fdc_clock (dev_fdc_t *fdc, unsigned n)
{
	e8272_clock (&fdc->e8272, n);
}

dev_fdc_t *dev_fdc_new (unsigned long addr)
{
	unsigned  i;
	dev_fdc_t *fdc;

	fdc = malloc (sizeof (dev_fdc_t));

	if (fdc == NULL) {
		return (NULL);
	}

	dev_init (&fdc->dev, fdc, "fdc");

	fdc->dev.del = dev_fdc_del_dev;
	fdc->dev.clock = (void *) dev_fdc_clock;

	e8272_init (&fdc->e8272);

	e8272_set_block_read_fct (&fdc->e8272, fdc, dev_fdc_block_read);
	e8272_set_block_write_fct (&fdc->e8272, fdc, dev_fdc_block_write);
	e8272_set_block_fmt_fct (&fdc->e8272, fdc, dev_fdc_block_fmt);
	e8272_set_block_rdid_fct (&fdc->e8272, fdc, dev_fdc_block_rdid);

	mem_blk_init (&fdc->blk, addr, 8, 0);

	mem_blk_set_fct (&fdc->blk, &fdc->e8272,
		e8272_get_uint8, NULL, NULL,
		e8272_set_uint8, NULL, NULL
	);

	for (i = 0; i < 4; i++) {
		fdc->drive[i] = 0xffff;
	}

	return (fdc);
}

void dev_fdc_del (dev_fdc_t *fdc)
{
	if (fdc != NULL) {
		mem_blk_free (&fdc->blk);
		e8272_free (&fdc->e8272);

		free (fdc);
	}
}

static
void dev_fdc_del_dev (device_t *dev)
{
	dev_fdc_del (dev->ext);
}

void dev_fdc_mem_add_io (dev_fdc_t *fdc, memory_t *io)
{
	mem_add_blk (io, &fdc->blk, 0);
}

void dev_fdc_mem_rmv_io (dev_fdc_t *fdc, memory_t *io)
{
	mem_rmv_blk (io, &fdc->blk);
}


void dev_fdc_reset (dev_fdc_t *fdc)
{
	e8272_reset (&fdc->e8272);
}

void dev_fdc_set_disks (dev_fdc_t *fdc, disks_t *dsks)
{
	fdc->dsks = dsks;
}

void dev_fdc_set_drive (dev_fdc_t *fdc, unsigned fdcdrv, unsigned drive)
{
	if (fdcdrv < 4) {
		fdc->drive[fdcdrv] = drive;
	}
}

unsigned dev_fdc_get_drive (dev_fdc_t *fdc, unsigned fdcdrv)
{
	if (fdcdrv < 4) {
		return (fdc->drive[fdcdrv]);
	}

	return (0xffff);
}
