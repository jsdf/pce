/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-io.h                                     *
 * Created:     2012-01-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2017 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_IO_H
#define PFI_IO_H 1


#include <drivers/pfi/pfi.h>


#define PFI_FORMAT_NONE     0
#define PFI_FORMAT_PFI      1
#define PFI_FORMAT_KRYOFLUX 2
#define PFI_FORMAT_SCP      3
#define PFI_FORMAT_A2R      4


unsigned pfi_get_uint16_be (const void *buf, unsigned idx);
unsigned pfi_get_uint16_le (const void *buf, unsigned idx);

unsigned long pfi_get_uint32_be (const void *buf, unsigned idx);
unsigned long pfi_get_uint32_le (const void *buf, unsigned idx);

void pfi_set_uint16_be (void *buf, unsigned idx, unsigned val);
void pfi_set_uint16_le (void *buf, unsigned idx, unsigned val);

void pfi_set_uint32_be (void *buf, unsigned idx, unsigned long val);
void pfi_set_uint32_le (void *buf, unsigned idx, unsigned long val);


int pfi_set_pos (FILE *fp, unsigned long ofs);
int pfi_read (FILE *fp, void *buf, unsigned long cnt);
int pfi_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt);
int pfi_write (FILE *fp, const void *buf, unsigned long cnt);
int pfi_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt);
int pfi_skip (FILE *fp, unsigned long cnt);


pfi_img_t *pfi_img_load_fp (FILE *fp, unsigned type);
pfi_img_t *pfi_img_load (const char *fname, unsigned type);

int pfi_img_save_fp (FILE *fp, pfi_img_t *img, unsigned type);
int pfi_img_save (const char *fname, pfi_img_t *img, unsigned type);


#endif
