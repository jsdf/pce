/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/main.h                                         *
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


#ifndef PSI_MAIN_H
#define PSI_MAIN_H 1


#include <config.h>

#include <drivers/psi/psi.h>


#define PSI_TRK_ALTERNATE (1 << 0)
#define PSI_TRK_BAD_ID    (1 << 1)
#define PSI_TRK_RANGE     (1 << 2)
#define PSI_TRK_SIZE      (1 << 3)
#define PSI_TRK_ENCODING  (1 << 4)
#define PSI_TRK_TIME      (1 << 5)

#define PSI_FLAG_MFM_SIZE (1UL << 16)


typedef int (*psi_trk_cb) (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque
);

typedef int (*psi_sct_cb) (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque
);


extern const char    *arg0;
extern int           par_verbose;
extern unsigned      par_filler;
extern unsigned long par_cnt;

extern char          par_cyl_all;
extern unsigned      par_cyl[2];

extern char          par_trk_all;
extern unsigned      par_trk[2];

extern char          par_sct_all;
extern unsigned      par_sct[2];

extern char          par_rsc_all;
extern unsigned      par_rsc[2];

extern char          par_alt_all;
extern unsigned      par_alt[2];

int psi_parse_int_list (const char **str, unsigned *val);

int psi_sel_match (unsigned c, unsigned h, unsigned s, unsigned r, unsigned a);

int psi_for_all_sectors (psi_img_t *img, psi_sct_cb fct, void *opaque);
int psi_for_all_tracks (psi_img_t *img, psi_trk_cb fct, void *opaque);


#endif
