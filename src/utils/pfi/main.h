/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/main.h                                         *
 * Created:     2012-01-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_MAIN_H
#define PFI_MAIN_H 1


#include <config.h>

#include <drivers/pfi/pfi.h>


#define PFI_FOLD_NONE    0
#define PFI_FOLD_MAXRUN  1
#define PFI_FOLD_MINDIFF 2


typedef int (*pfi_trk_cb) (pfi_img_t *img, pfi_trk_t *trk,
	unsigned long c, unsigned long h, void *opaque
);


extern const char    *arg0;

extern int           par_verbose;

extern unsigned long par_pfi_clock;

extern unsigned      par_revolution;

extern double        par_slack1;
extern double        par_slack2;

extern int           par_weak_bits;
extern unsigned long par_weak_i1;
extern unsigned long par_weak_i2;

extern unsigned long par_clock_tolerance;

extern unsigned      par_fold_mode;
extern unsigned      par_fold_window;
extern unsigned long par_fold_max;

extern unsigned      par_pfi_fold_revolution;
extern unsigned long par_pfi_fold_window;
extern unsigned long par_pfi_fold_compare;
extern int           par_pfi_fold_right;


int pfi_parse_double (const char *str, double *val);
int pfi_parse_long (const char *str, long *val);
int pfi_parse_ulong (const char *str, unsigned long *val);
int pfi_parse_uint (const char *str, unsigned *val);
int pfi_parse_bool (const char *str, int *val);
int pfi_parse_rate (const char *str, unsigned long *val);
int pfi_parse_time (const char *str, double *val, double def);

int pfi_for_all_tracks (pfi_img_t *img, pfi_trk_cb fct, void *opaque);
int pfi_parse_range (const char *str, unsigned long *v1, unsigned long *v2, char *all);


int pfi_comment_add (pfi_img_t *img, const char *str);
int pfi_comment_load (pfi_img_t *img, const char *fname);
int pfi_comment_save (pfi_img_t *img, const char *fname);
int pfi_comment_set (pfi_img_t *img, const char *str);
int pfi_comment_show (pfi_img_t *img);

int pfi_delete_tracks (pfi_img_t *img);
int pfi_double_step (pfi_img_t *img, int even);

int pfi_decode_bits (pfi_img_t *img, const char *type, unsigned long cell, const char *fname);
int pfi_decode_bits_pbs (pfi_img_t *img, unsigned long rate, const char *fname);
int pfi_decode (pfi_img_t *img, const char *type, unsigned long rate, const char *fname);

int pfi_encode (pfi_img_t *img, const char *type, const char *fname);

int pfi_export_tracks (pfi_img_t *img, const char *fname);
int pfi_import_tracks (pfi_img_t *img, const char *fname);

int pfi_fold (pfi_img_t *img);

int pfi_print_info (pfi_img_t *img);
int pfi_list_tracks (pfi_img_t *img, int verb);

int pfi_decode_text (pfi_img_t *img, const char *fname);
int pfi_encode_text (pfi_img_t *img, const char *fname);

int pfi_quantize (pfi_img_t *img, unsigned long rate);
int pfi_revolutions (pfi_img_t *img, const char *str);
int pfi_slack (pfi_img_t *img, const char *str);
int pfi_scale_tracks (pfi_img_t *img, double factor);
int pfi_set_clock (pfi_img_t *img, unsigned long clock);
int pfi_set_rpm (pfi_img_t *img, double rpm);
int pfi_set_rpm_mac_490 (pfi_img_t *img);
int pfi_set_rpm_mac_500 (pfi_img_t *img);
int pfi_shift_index (pfi_img_t *img, long ofs);
int pfi_shift_index_us (pfi_img_t *img, long us);
int pfi_wpcom (pfi_img_t *img);


#endif
