/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/disk.h                                           *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-10-06 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: disk.h,v 1.1 2003/12/20 01:01:33 hampa Exp $ */


#ifndef PCE_FLOPPY_H
#define PCE_FLOPPY_H 1


#define PCE_MAX_DISKS 16


typedef struct {
  unsigned char drive;

  struct {
    unsigned c;
    unsigned h;
    unsigned s;
  } geom;

  int           readonly;

  unsigned long start;
  unsigned long blocks;

  unsigned char *data;
  FILE          *fp;
  int           fp_close;
} disk_t;


typedef struct {
  unsigned cnt;
  disk_t   *dsk[PCE_MAX_DISKS];

  unsigned char last_status;
} disks_t;



disk_t *dsk_new (unsigned drive);

void dsk_free (disk_t *dsk);

void dsk_del (disk_t *dsk);

int dsk_set_mem (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro);

int dsk_set_image (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro);

int dsk_set_hdimage (disk_t *dsk, const char *fname, int ro);

int dsk_set_auto (disk_t *dsk, char **type,
  unsigned c, unsigned h, unsigned s,
  const char *fname, int ro);

int dsk_get_lba (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  unsigned long *lba);

int dsk_read_lba (disk_t *dsk, void *buf,
  unsigned long blk_i, unsigned long blk_n);

int dsk_read_chs (disk_t *dsk, void *buf,
  unsigned c, unsigned h, unsigned s, unsigned long blk_n);

int dsk_write_lba (disk_t *dsk, const void *buf,
  unsigned long blk_i, unsigned long blk_n);

int dsk_write_chs (disk_t *dsk, const void *buf,
  unsigned c, unsigned h, unsigned s, unsigned long blk_n);

void dsk_set_drive (disk_t *dsk, unsigned drive);


disks_t *dsks_new (void);
void dsks_del (disks_t *dsks);
int dsks_add_disk (disks_t *dsks, disk_t *dsk);
void dsks_rmv_disk (disks_t *dsks, disk_t *dsk);
disk_t *dsks_get_disk (disks_t *dsks, unsigned drive);
unsigned dsks_get_hd_cnt (disks_t *dsks);

void dsk_int13 (disks_t *dsks, e8086_t *cpu);


#endif
