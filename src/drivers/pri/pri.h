/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri.h                                        *
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


#ifndef PCE_PRI_H
#define PCE_PRI_H 1


typedef struct {
	unsigned long clock;

	unsigned long size;
	unsigned char *data;

	unsigned long idx;
	char          wrap;
} pri_trk_t;


typedef struct {
	unsigned long trk_cnt;
	pri_trk_t    **trk;
} pri_cyl_t;


typedef struct {
	unsigned long cyl_cnt;
	pri_cyl_t    **cyl;

	unsigned long comment_size;
	unsigned char *comment;
} pri_img_t;


pri_trk_t *pri_trk_new (unsigned long size, unsigned long clock);
void pri_trk_del (pri_trk_t *trk);
pri_trk_t *pri_trk_clone (const pri_trk_t *trk);
void pri_trk_clear (pri_trk_t *trk, unsigned val);
void pri_trk_clear_16 (pri_trk_t *trk, unsigned val);
void pri_trk_set_clock (pri_trk_t *trk, unsigned long clock);
unsigned long pri_trk_get_clock (const pri_trk_t *trk);
unsigned long pri_trk_get_size (const pri_trk_t *trk);
int pri_trk_set_size (pri_trk_t *trk, unsigned long size);

void pri_trk_set_pos (pri_trk_t *trk, unsigned long pos);
int pri_trk_get_bits (pri_trk_t *trk, unsigned long *val, unsigned cnt);
int pri_trk_set_bits (pri_trk_t *trk, unsigned long val, unsigned cnt);
int pri_trk_rotate (pri_trk_t *trk, unsigned long idx);

pri_cyl_t *pri_cyl_new (void);
void pri_cyl_del (pri_cyl_t *cyl);
unsigned long pri_cyl_get_trk_cnt (const pri_cyl_t *cyl);
pri_trk_t *pri_cyl_get_track (pri_cyl_t *cyl, unsigned long idx, int alloc);
int pri_cyl_set_track (pri_cyl_t *cyl, pri_trk_t *trk, unsigned long h);
int pri_cyl_add_track (pri_cyl_t *cyl, pri_trk_t *trk);
int pri_cyl_del_track (pri_cyl_t *cyl, unsigned long h);

pri_img_t *pri_img_new (void);
void pri_img_del (pri_img_t *img);
unsigned long pri_img_get_cyl_cnt (const pri_img_t *img);
unsigned long pri_img_get_trk_cnt (const pri_img_t *img, unsigned long c);

pri_cyl_t *pri_img_get_cylinder (pri_img_t *img, unsigned long c, int alloc);
pri_cyl_t *pri_img_rmv_cylinder (pri_img_t *img, unsigned long idx);
int pri_img_set_cylinder (pri_img_t *img, pri_cyl_t *cyl, unsigned long c);
int pri_img_add_cylinder (pri_img_t *img, pri_cyl_t *cyl);
int pri_img_del_cylinder (pri_img_t *img, unsigned long c);

pri_trk_t *pri_img_get_track (pri_img_t *img, unsigned long c, unsigned long h, int alloc);
int pri_img_set_track (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h);
int pri_img_del_track (pri_img_t *img, unsigned long c, unsigned long h);

int pri_img_add_comment (pri_img_t *img, const unsigned char *buf, unsigned cnt);
int pri_img_set_comment (pri_img_t *img, const unsigned char *buf, unsigned cnt);


#endif
