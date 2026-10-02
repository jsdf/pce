/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/main.h                                         *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PTI_MAIN_H
#define PTI_MAIN_H 1


#include <config.h>

#include <drivers/pti/pti.h>


extern const char    *arg0;

extern int           par_verbose;

extern unsigned long par_default_clock;


int pti_parse_double (const char *str, double *val);
int pti_parse_long (const char *str, long *val);
int pti_parse_ulong (const char *str, unsigned long *val);
int pti_parse_uint (const char *str, unsigned *val);
int pti_parse_bool (const char *str, int *val);
int pti_parse_freq (const char *str, unsigned long *val);
int pti_parse_time_sec (const char *str, double *sec);
int pti_parse_time_clk (const char *str, unsigned long *sec, unsigned long *clk, unsigned long clock);

pti_img_t *pti_load_image (const char *fname);
int pti_save_image (const char *fname, pti_img_t **img);


#endif
