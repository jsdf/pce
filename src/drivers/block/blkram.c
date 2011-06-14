/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkram.c                                   *
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


#include "blkram.h"

#include <stdlib.h>
#include <string.h>


static
int dsk_ram_load (disk_ram_t *ram, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");
	if (fp == NULL) {
		return (1);
	}

	r = dsk_read (fp, ram->data, 0, 512 * (uint64_t) ram->dsk.blocks);

	fclose (fp);

	return (r);
}

static
int dsk_ram_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	disk_ram_t *ram;

	ram = dsk->ext;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	memcpy (buf, ram->data + 512 * i, 512 * n);

	return (0);
}

static
int dsk_ram_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	disk_ram_t *ram;

	if (dsk->readonly) {
		return (1);
	}

	ram = dsk->ext;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	memcpy (ram->data + 512 * i, buf, 512 * n);

	return (0);
}

static
void dsk_ram_del (disk_t *dsk)
{
	disk_ram_t *ram;

	ram = dsk->ext;

	free (ram->data);
	free (ram);
}

disk_t *dsk_ram_open (const char *fname, uint32_t n, uint32_t c, uint32_t h, uint32_t s, int ro)
{
	disk_ram_t *ram;

	ram = malloc (sizeof (disk_ram_t));
	if (ram == NULL) {
		return (NULL);
	}

	dsk_init (&ram->dsk, ram, n, c, h, s);

	dsk_set_type (&ram->dsk, PCE_DISK_RAM);

	dsk_set_readonly (&ram->dsk, ro);

	ram->dsk.del = dsk_ram_del;
	ram->dsk.read = dsk_ram_read;
	ram->dsk.write = dsk_ram_write;

	ram->data = malloc (512 * ram->dsk.blocks);
	if (ram->data == NULL) {
		free (ram);
		return (NULL);
	}

	memset (ram->data, 0, 512 * ram->dsk.blocks);

	if (fname != NULL) {
		if (dsk_ram_load (ram, fname)) {
			free (ram->data);
			free (ram);
			return (NULL);
		}

		dsk_set_fname (&ram->dsk, fname);
	}

	return (&ram->dsk);
}
