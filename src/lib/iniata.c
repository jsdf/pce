/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/iniata.c                                           *
 * Created:       2006-12-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2006-2007 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>

#include <lib/iniata.h>
#include <lib/log.h>

#include <devices/ata.h>
#include <devices/pci-ata.h>
#include <devices/block/block.h>


int ini_get_ata_chn (ata_chn_t *ata, disks_t *dsks, ini_sct_t *ini)
{
	unsigned   dev, drv;
	unsigned   multi;
	const char *model;
	disk_t     *dsk;
	ini_sct_t  *sct;

	sct = NULL;

	while ((sct = ini_next_sct (ini, sct, "device")) != NULL) {
		ini_get_uint16 (sct, "device", &dev, 0);
		ini_get_uint16 (sct, "disk", &drv, 0);
		ini_get_uint16 (sct, "multi_count_max", &multi, 0);
		ini_get_string (sct, "model", &model, "PCEDISK");

		pce_log_tag (MSG_INF,
			"ATA-DEV:", "device=%u multi=%u model=%s drive=%u\n",
			dev, multi, model, drv
		);

		dsk = dsks_get_disk (dsks, drv);
		if (dsk == NULL) {
			pce_log (MSG_ERR, "*** no such drive (%u)\n", drv);
		}
		else {
			ata_set_multi_mode (ata, dev, multi);
			ata_set_model (ata, dev, model);

			ata_set_block (ata, dsk, dev);
		}
	}

	return (0);
}

int ini_get_pci_ata (pci_ata_t *pciata, disks_t *dsks, ini_sct_t *ini)
{
	unsigned  idx;
	ini_sct_t *sct;
	ata_chn_t *chn;

	idx = 0;
	sct = NULL;

	while ((sct = ini_next_sct (ini, sct, "channel")) != NULL) {
		ini_get_uint16 (sct, "channel", &idx, idx);

		chn = pci_ata_get_chn (pciata, idx);

		pce_log_tag (MSG_INF, "PCI-ATA:", "channel=%u\n", idx);

		if (chn == NULL) {
			pce_log (MSG_ERR, "*** no such channel (%u)\n", idx);
		}
		else {
			ini_get_ata_chn (chn, dsks, sct);
		}
	}

	return (0);
}
