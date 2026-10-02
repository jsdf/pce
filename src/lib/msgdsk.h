/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/msgdsk.h                                             *
 * Created:     2019-06-22 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2019 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_LIB_MSGDSK_H
#define PCE_LIB_MSGDSK_H 1


#include <drivers/block/block.h>


void msg_dsk_print_help (void);
void dsks_print_info (disks_t *dsks);

int msg_dsk_get_disk_id (const char *val, unsigned *id);

int msg_dsk_emu_disk_commit (const char *val, disks_t *dsks, unsigned *id);
int msg_dsk_emu_disk_eject (const char *val, disks_t *dsks, unsigned *id);
int msg_dsk_emu_disk_id (const char *val, unsigned *id);
int msg_dsk_emu_disk_insert (const char *val, disks_t *dsks, unsigned id);
int msg_dsk_emu_disk_ro (const char *val, disks_t *dsks, unsigned *id);
int msg_dsk_emu_disk_rw (const char *val, disks_t *dsks, unsigned *id);

int msg_dsk_set_msg (const char *msg, const char *val, disks_t *dsks, unsigned *id);


#endif
