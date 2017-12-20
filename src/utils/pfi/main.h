/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/main.h                                         *
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


#ifndef PFI_MAIN_H
#define PFI_MAIN_H 1


#include <config.h>

#include <drivers/pfi/pfi.h>


typedef int (*pfi_trk_cb) (pfi_img_t *img, pfi_trk_t *trk,
	unsigned long c, unsigned long h, void *opaque
);


extern const char    *arg0;

extern int           par_verbose;

extern unsigned long par_pfi_clock;

extern unsigned      par_revolution;

extern unsigned      par_slack1;
extern unsigned      par_slack2;

extern int           par_weak_bits;
extern unsigned long par_weak_i1;
extern unsigned long par_weak_i2;

extern unsigned long par_clock_tolerance;


int pfi_for_all_tracks (pfi_img_t *img, pfi_trk_cb fct, void *opaque);
int pfi_parse_range (const char *str, unsigned long *v1, unsigned long *v2, char *all, char *inv);


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

int pfi_print_info (pfi_img_t *img);
int pfi_list_tracks (pfi_img_t *img, int verb);

int pfi_revolutions (pfi_img_t *img, const char *str);
int pfi_scale_tracks (pfi_img_t *img, double factor);
int pfi_set_clock (pfi_img_t *img, unsigned long clock);
int pfi_set_rpm (pfi_img_t *img, double rpm);
int pfi_set_rpm_mac (pfi_img_t *img);
int pfi_shift_index (pfi_img_t *img, long ofs);


#endif
