/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/inidsk.h                                             *
 * Created:     2004-12-13 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_INIDSK_H
#define PCE_LIB_INIDSK_H 1


#include <libini/libini.h>

#include <drivers/block/block.h>


int dsk_insert (disks_t *dsks, const char *str, int eject);

disk_t *ini_get_cow (ini_sct_t *sct, disk_t *dsk);

int ini_get_disk (ini_sct_t *sct, disk_t **ret);

disks_t *ini_get_disks (ini_sct_t *ini);


#endif
