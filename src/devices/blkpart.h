/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/blkpart.h                                      *
 * Created:       2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_DEVICES_BLKPART_H
#define PCE_DEVICES_BLKPART_H 1


#include <config.h>

#include <devices/disk.h>

#include <stdio.h>


/*!***************************************************************************
 * @short The partitioned image file disk structure
 *****************************************************************************/
typedef struct {
  disk_t        dsk;

  unsigned      part_cnt;

  struct {
    unsigned long block_i;
    unsigned long block_n;
    unsigned long start;
    FILE          *fp;
    int           close;
    int           ro;
  } part[8];
} disk_part_t;


/*!***************************************************************************
 * @short Add a partition
 *****************************************************************************/
int dsk_part_add_partition_fp (disk_t *dsk, FILE *fp, int close,
  unsigned long start, unsigned long blk_i, unsigned long blk_n, int ro
);

int dsk_part_add_partition (disk_t *dsk, const char *fname,
  unsigned long start, unsigned long blk_i, unsigned long blk_n, int ro
);

/*!***************************************************************************
 * @short Create a new partition image disk
 *****************************************************************************/
disk_t *dsk_part_new (unsigned d, unsigned c, unsigned h, unsigned s, int ro);


#endif
