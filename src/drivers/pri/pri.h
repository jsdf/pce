/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri.h                                        *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#define PRI_EVENT_FUZZY 1
#define PRI_EVENT_WEAK  1
#define PRI_EVENT_CLOCK 2
#define PRI_EVENT_ALL   0xffffffff


typedef struct pri_evt_t {
	struct pri_evt_t *next;
	unsigned long    type;
	unsigned long    pos;
	unsigned long    val;
} pri_evt_t;


typedef struct {
	unsigned long clock;

	unsigned long size;
	unsigned char *data;

	pri_evt_t     *evt;

	unsigned long idx;
	pri_evt_t     *cur_evt;
	char          wrap;
} pri_trk_t;


typedef struct {
	unsigned long trk_cnt;
	pri_trk_t     **trk;
} pri_cyl_t;


typedef struct {
	unsigned long cyl_cnt;
	pri_cyl_t     **cyl;

	unsigned long comment_size;
	unsigned char *comment;

	char          readonly;

	char          woz_track_sync;
	char          woz_cleaned;
} pri_img_t;


pri_evt_t *pri_evt_new (unsigned long type, unsigned long pos, unsigned long val);
void pri_evt_del (pri_evt_t *evt);
pri_evt_t *pri_evt_next (pri_evt_t *evt, unsigned long type);
unsigned long pri_evt_get_clock (pri_evt_t *evt, unsigned long base);

pri_trk_t *pri_trk_new (unsigned long size, unsigned long clock);
void pri_trk_del (pri_trk_t *trk);
pri_trk_t *pri_trk_clone (const pri_trk_t *trk);
void pri_trk_evt_ins (pri_trk_t *trk, pri_evt_t *evt);
int pri_trk_evt_rmv (pri_trk_t *trk, const pri_evt_t *evt);
pri_evt_t *pri_trk_evt_add (pri_trk_t *trk, unsigned long type, unsigned long pos, unsigned long val);
pri_evt_t *pri_trk_evt_get_idx (pri_trk_t *trk, unsigned long type, unsigned long idx);
pri_evt_t *pri_trk_evt_get_pos (pri_trk_t *trk, unsigned long type, unsigned long pos);
pri_evt_t *pri_trk_evt_get_after (pri_trk_t *trk, unsigned long type, unsigned long pos);
pri_evt_t *pri_trk_evt_get_before (pri_trk_t *trk, unsigned long type, unsigned long pos);
int pri_trk_evt_del (pri_trk_t *trk, pri_evt_t *evt);
void pri_trk_evt_del_all (pri_trk_t *trk, unsigned long type);
unsigned pri_trk_evt_count (const pri_trk_t *trk, unsigned long type);
void pri_trk_clear (pri_trk_t *trk, unsigned val);
void pri_trk_clear_16 (pri_trk_t *trk, unsigned val);
void pri_trk_clear_slack (pri_trk_t *trk);
void pri_trk_set_clock (pri_trk_t *trk, unsigned long clock);
unsigned long pri_trk_get_clock (const pri_trk_t *trk);
unsigned long pri_trk_get_size (const pri_trk_t *trk);
int pri_trk_set_size (pri_trk_t *trk, unsigned long size);

unsigned long pri_trk_get_pos (const pri_trk_t *trk);
void pri_trk_set_pos (pri_trk_t *trk, unsigned long pos);
int pri_trk_get_bits (pri_trk_t *trk, unsigned long *val, unsigned cnt);
int pri_trk_set_bits (pri_trk_t *trk, unsigned long val, unsigned cnt);
int pri_trk_get_event (pri_trk_t *trk, unsigned long *type, unsigned long *val);
int pri_trk_rotate (pri_trk_t *trk, unsigned long idx);
int pri_trk_get_weak_mask (pri_trk_t *trk, unsigned char **buf, unsigned long *cnt);
int pri_trk_set_weak_mask (pri_trk_t *trk, const void *buf, unsigned long cnt);

pri_cyl_t *pri_cyl_new (void);
void pri_cyl_del (pri_cyl_t *cyl);
pri_cyl_t *pri_cyl_clone (const pri_cyl_t *cyl);
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

pri_trk_t *pri_img_get_track_const (const pri_img_t *img, unsigned long c, unsigned long h);
pri_trk_t *pri_img_get_track (pri_img_t *img, unsigned long c, unsigned long h, int alloc);
int pri_img_set_track (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h);
int pri_img_del_track (pri_img_t *img, unsigned long c, unsigned long h);

int pri_img_add_comment (pri_img_t *img, const unsigned char *buf, unsigned cnt);
int pri_img_set_comment (pri_img_t *img, const unsigned char *buf, unsigned cnt);


#endif
