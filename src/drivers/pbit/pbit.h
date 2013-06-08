/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pbit/pbit.h                                      *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_PBIT_H
#define PCE_PBIT_H 1


typedef struct {
	unsigned long clock;

	unsigned long size;
	unsigned char *data;

	unsigned long idx;
	char          wrap;
} pbit_trk_t;


typedef struct {
	unsigned long trk_cnt;
	pbit_trk_t    **trk;
} pbit_cyl_t;


typedef struct {
	unsigned long cyl_cnt;
	pbit_cyl_t    **cyl;

	unsigned long comment_size;
	unsigned char *comment;
} pbit_img_t;


pbit_trk_t *pbit_trk_new (unsigned long size, unsigned long clock);
void pbit_trk_del (pbit_trk_t *trk);
pbit_trk_t *pbit_trk_clone (const pbit_trk_t *trk);
void pbit_trk_clear (pbit_trk_t *trk, unsigned val);
void pbit_trk_clear_16 (pbit_trk_t *trk, unsigned val);
void pbit_trk_set_clock (pbit_trk_t *trk, unsigned long clock);
unsigned long pbit_trk_get_clock (const pbit_trk_t *trk);
unsigned long pbit_trk_get_size (const pbit_trk_t *trk);
int pbit_trk_set_size (pbit_trk_t *trk, unsigned long size);

void pbit_trk_set_pos (pbit_trk_t *trk, unsigned long pos);
int pbit_trk_get_bits (pbit_trk_t *trk, unsigned long *val, unsigned cnt);
int pbit_trk_set_bits (pbit_trk_t *trk, unsigned long val, unsigned cnt);
int pbit_trk_rotate (pbit_trk_t *trk, unsigned long idx);

pbit_cyl_t *pbit_cyl_new (void);
void pbit_cyl_del (pbit_cyl_t *cyl);
unsigned long pbit_cyl_get_trk_cnt (const pbit_cyl_t *cyl);
pbit_trk_t *pbit_cyl_get_track (pbit_cyl_t *cyl, unsigned long idx, int alloc);
int pbit_cyl_set_track (pbit_cyl_t *cyl, pbit_trk_t *trk, unsigned long h);
int pbit_cyl_add_track (pbit_cyl_t *cyl, pbit_trk_t *trk);
int pbit_cyl_del_track (pbit_cyl_t *cyl, unsigned long h);

pbit_img_t *pbit_img_new (void);
void pbit_img_del (pbit_img_t *img);
unsigned long pbit_img_get_cyl_cnt (const pbit_img_t *img);
unsigned long pbit_img_get_trk_cnt (const pbit_img_t *img, unsigned long c);

pbit_cyl_t *pbit_img_get_cylinder (pbit_img_t *img, unsigned long c, int alloc);
pbit_cyl_t *pbit_img_rmv_cylinder (pbit_img_t *img, unsigned long idx);
int pbit_img_set_cylinder (pbit_img_t *img, pbit_cyl_t *cyl, unsigned long c);
int pbit_img_add_cylinder (pbit_img_t *img, pbit_cyl_t *cyl);
int pbit_img_del_cylinder (pbit_img_t *img, unsigned long c);

pbit_trk_t *pbit_img_get_track (pbit_img_t *img, unsigned long c, unsigned long h, int alloc);
int pbit_img_set_track (pbit_img_t *img, pbit_trk_t *trk, unsigned long c, unsigned long h);
int pbit_img_del_track (pbit_img_t *img, unsigned long c, unsigned long h);

int pbit_img_add_comment (pbit_img_t *img, const unsigned char *buf, unsigned cnt);
int pbit_img_set_comment (pbit_img_t *img, const unsigned char *buf, unsigned cnt);


#endif
