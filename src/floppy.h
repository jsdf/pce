/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     floppy.h                                                   *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: floppy.h,v 1.2 2003/04/16 17:19:51 hampa Exp $ */


#ifndef PCE_FLOPPY_H
#define PCE_FLOPPY_H 1


#include <pce.h>
#include <e8086/e8086.h>


#define PCE_MAX_DISKS 16


typedef struct {
  unsigned char drive;

  struct {
    unsigned c;
    unsigned h;
    unsigned s;
  } geom;

  unsigned long start;
  unsigned long size;
  unsigned char *data;
  FILE          *fp;
  int           fp_close;
} disk_t;


typedef struct {
  unsigned cnt;
  disk_t   *dsk[PCE_MAX_DISKS];

  unsigned char last_status;
} disks_t;



disk_t *dsk_new (unsigned drive, unsigned c, unsigned h, unsigned s);
void dsk_del (disk_t *dsk);
void dsk_set_drive (disk_t *dsk, unsigned drive);
int dsk_set_file (disk_t *dsk, const char *fname);
int dsk_alloc (disk_t *dsk);
int dsk_load_img (disk_t *dsk, const char *fname);

disks_t *dsks_new (void);
void dsks_del (disks_t *dsks);
int dsks_add_disk (disks_t *dsks, disk_t *dsk);
disk_t *dsks_get_disk (disks_t *dsks, unsigned drive);
unsigned dsks_get_hd_cnt (disks_t *dsks);

void dsk_int13 (disks_t *dsks, e8086_t *cpu);


#endif
