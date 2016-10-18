/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-raw.h                                *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PSI_IMG_RAW_H
#define PSI_IMG_RAW_H 1


#include <drivers/psi/psi.h>


typedef struct {
	unsigned long size;
	unsigned      c;
	unsigned      h;
	unsigned      s;
	unsigned      ssize;
	unsigned      encoding;
} psi_geometry_t;


const psi_geometry_t *psi_get_geometry_from_size (unsigned long size, unsigned long mask);

psi_img_t *psi_load_st (FILE *fp);
psi_img_t *psi_load_raw_hcs (FILE *fp);
psi_img_t *psi_load_raw_hts (FILE *fp);
psi_img_t *psi_load_raw (FILE *fp);

int psi_save_st (FILE *fp, const psi_img_t *img);
int psi_save_raw_hcs (FILE *fp, const psi_img_t *img);
int psi_save_raw_hts (FILE *fp, const psi_img_t *img);
int psi_save_raw (FILE *fp, const psi_img_t *img);

int psi_probe_raw_fp (FILE *fp);
int psi_probe_raw (const char *fname);


#endif
