/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkpce.h                                       *
 * Created:       2004-11-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#ifndef PCE_DEVICES_BLKPCE_H
#define PCE_DEVICES_BLKPCE_H 1


#include <config.h>

#include <devices/disk.h>

#include <stdio.h>
#include <stdint.h>


/*!***************************************************************************
 * @short The pce image file disk structure
 *****************************************************************************/
typedef struct {
  disk_t   dsk;

  FILE     *fp;

  uint64_t blk_cnt;

  uint64_t dir_base;
  uint64_t blk_next;

  uint32_t cylinders;
  uint32_t heads;
  uint32_t sectors;
  uint32_t blk_size;

  uint32_t dir_cnt;
  uint32_t dir_size;
  uint32_t dir_next;
  uint32_t dir_alloc;

  uint64_t **dir;
  uint8_t  *dir_buf;
} disk_pce_t;


disk_t *dsk_pce_open_fp (FILE *fp, int ro);
disk_t *dsk_pce_open (const char *fname, int ro);

int dsk_pce_create_fp (FILE *fp, uint32_t c, uint32_t h, uint32_t s);
int dsk_pce_create (const char *fname, uint32_t c, uint32_t h, uint32_t s);


#endif
