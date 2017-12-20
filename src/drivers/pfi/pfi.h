/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi.h                                        *
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


#ifndef PFI_PFI_H
#define PFI_PFI_H 1


#include <drivers/pfi/track.h>


typedef struct {
	unsigned long trk_cnt;
	pfi_trk_t     **trk;
} pfi_cyl_t;


typedef struct {
	unsigned long cyl_cnt;
	pfi_cyl_t     **cyl;

	unsigned long comment_size;
	unsigned char *comment;
} pfi_img_t;


pfi_cyl_t *pfi_cyl_new (void);
void pfi_cyl_del (pfi_cyl_t *cyl);
pfi_cyl_t *pfi_cyl_clone (const pfi_cyl_t *cyl);
unsigned long pfi_cyl_get_trk_cnt (const pfi_cyl_t *cyl);
pfi_trk_t *pfi_cyl_get_track (pfi_cyl_t *cyl, unsigned long idx, int alloc);
const pfi_trk_t *pfi_cyl_get_track_const (const pfi_cyl_t *cyl, unsigned long idx);
int pfi_cyl_set_track (pfi_cyl_t *cyl, pfi_trk_t *trk, unsigned long h);
int pfi_cyl_add_track (pfi_cyl_t *cyl, pfi_trk_t *trk);
int pfi_cyl_del_track (pfi_cyl_t *cyl, unsigned long h);

pfi_img_t *pfi_img_new (void);
void pfi_img_del (pfi_img_t *img);
unsigned long pfi_img_get_cyl_cnt (const pfi_img_t *img);
unsigned long pfi_img_get_trk_cnt (const pfi_img_t *img, unsigned long c);
unsigned long pfi_img_get_trk_max (const pfi_img_t *img);

pfi_cyl_t *pfi_img_get_cylinder (pfi_img_t *img, unsigned long c, int alloc);
const pfi_cyl_t *pfi_img_get_cylinder_const (const pfi_img_t *img, unsigned long idx);
pfi_cyl_t *pfi_img_rmv_cylinder (pfi_img_t *img, unsigned long idx);
pfi_trk_t *pfi_img_rmv_track (pfi_img_t *img, unsigned long c, unsigned long h);
int pfi_img_set_cylinder (pfi_img_t *img, pfi_cyl_t *cyl, unsigned long c);
int pfi_img_add_cylinder (pfi_img_t *img, pfi_cyl_t *cyl);
int pfi_img_del_cylinder (pfi_img_t *img, unsigned long c);

pfi_trk_t *pfi_img_get_track (pfi_img_t *img, unsigned long c, unsigned long h, int alloc);
const pfi_trk_t *pfi_img_get_track_const (const pfi_img_t *img, unsigned long c, unsigned long h);
int pfi_img_set_track (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h);
int pfi_img_del_track (pfi_img_t *img, unsigned long c, unsigned long h);

int pfi_img_add_comment (pfi_img_t *img, const unsigned char *buf, unsigned cnt);
int pfi_img_set_comment (pfi_img_t *img, const unsigned char *buf, unsigned cnt);


#endif
