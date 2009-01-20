/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/iniata.h                                             *
 * Created:     2006-12-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_INIATA_H
#define PCE_LIB_INIATA_H 1


#include <libini/libini.h>

#include <devices/ata.h>
#include <devices/pci-ata.h>
#include <devices/block/block.h>


int ini_get_ata_chn (ata_chn_t *ata, disks_t *dsks, ini_sct_t *sct);
int ini_get_pci_ata (pci_ata_t *pciata, disks_t *dsks, ini_sct_t *sct);


#endif
