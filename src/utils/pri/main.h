/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/main.h                                         *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2025 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef PRI_MAIN_H
#define PRI_MAIN_H 1


#include <config.h>

#include <drivers/pri/pri.h>
#include <drivers/psi/psi.h>


#define PRI_TEXT_AUTO 0
#define PRI_TEXT_RAW  1
#define PRI_TEXT_MFM  2
#define PRI_TEXT_FM   3
#define PRI_TEXT_MAC  4


extern const char    *arg0;

extern int           par_verbose;

extern int           par_list;
extern int           par_list_long;
extern int           par_print_info;

extern char          par_cyl_all;
extern unsigned long par_cyl[2];

extern char          par_trk_all;
extern unsigned long par_trk[2];

extern unsigned long par_data_rate;
extern unsigned long par_track_size;

extern unsigned      par_mac_align;
extern char          par_mac_no_slip;
extern char          par_psi_dec_pos;
extern char          par_text_align;


typedef int (*pri_trk_cb) (pri_img_t *img, pri_trk_t *trk,
	unsigned long c, unsigned long h, void *opaque
);


int strcmp2 (const char *str, const char *str1, const char *str2);

int pri_parse_range (const char *str, unsigned long *v1, unsigned long *v2, char *all);

int pri_for_all_tracks (pri_img_t *img, pri_trk_cb fct, void *opaque);

int pri_comment_add (pri_img_t *img, const char *str);
int pri_comment_load (pri_img_t *img, const char *fname);
int pri_comment_save (pri_img_t *img, const char *fname);
int pri_comment_set (pri_img_t *img, const char *str);
int pri_comment_show (pri_img_t *img);

int pri_decode_raw (pri_img_t *img, const char *fname);
int pri_decode (pri_img_t *img, const char *type, const char *fname);

int pri_delete_tracks (pri_img_t *img);
int pri_double_step (pri_img_t *img, int even);

int pri_edit (pri_img_t *img, const char *what, const char *val);

int pri_encode (pri_img_t **img, const char *type, const char *fname);

int pri_event_add (pri_img_t *img, const char *type, const char *pos, const char *val);
int pri_event_clear (pri_img_t *img);
int pri_event_del (pri_img_t *img, const char *type, const char *range);
int pri_event_list (pri_img_t *img, const char *type, const char *range);

int pri_print_info (pri_img_t *img);

int pri_list_tracks (pri_img_t *img);

int pri_merge_image (pri_img_t *img, const char *fname, int overwrite);

int pri_half_step (pri_img_t *img);
int pri_new (pri_img_t *img);

int pri_half_rate (pri_img_t *img);
int pri_mac_gcr_align (pri_img_t *img, const char *mode);
int pri_mac_gcr_align_gap (pri_img_t *img);
int pri_mac_gcr_align_index (pri_img_t *img);
int pri_mac_gcr_align_sync (pri_img_t *img);
int pri_mac_gcr_align_sector (pri_img_t *img);
int pri_mfm_align_am (pri_img_t *img, const char *what, const char *idx, const char *pos);
int pri_rotate_tracks_angle (pri_img_t *img, double val);
int pri_rotate_tracks (pri_img_t *img, long ofs);
int pri_rotate_tracks_left (pri_img_t *img, unsigned long ofs);
int pri_rotate_tracks_right (pri_img_t *img, unsigned long ofs);

int pri_encode_text (pri_img_t *img, const char *fname);
int pri_decode_text (pri_img_t *img, const char *fname, unsigned enc);

int pri_weak_clean (pri_img_t *img);
int pri_weak_close (pri_img_t *img, unsigned cnt);
int pri_weak_detect (pri_img_t *img, unsigned long cnt);
int pri_weak_expand (pri_img_t *img, unsigned long left, unsigned long right);
int pri_weak_open (pri_img_t *img, unsigned long cnt);
int pri_weak_flip (pri_img_t *img, int random);
int pri_weak_load (pri_img_t *img, const char *fname);
int pri_weak_set (pri_img_t *img, int val);
int pri_weak_save (pri_img_t *img, const char *fname);


#endif
