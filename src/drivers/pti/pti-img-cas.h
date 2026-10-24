/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-cas.h                                *
 * Created:     2020-04-27 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PTI_IMG_CAS_H
#define PTI_IMG_CAS_H 1


#include <drivers/pti/pti.h>


pti_img_t *pti_load_cas (FILE *fp, unsigned long clock);

int pti_save_cas (FILE *fp, const pti_img_t *img);

int pti_probe_cas_fp (FILE *fp);
int pti_probe_cas (const char *fname);


#endif
