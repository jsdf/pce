/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-pcm.h                                *
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


#ifndef PTI_IMG_PCM_H
#define PTI_IMG_PCM_H 1


#include <drivers/pti/pti.h>


typedef struct {
	unsigned long clock;
	unsigned long srate;
	char          sign;
} pti_pcm_param_t;


pti_img_t *pti_load_pcm (FILE *fp, const pti_pcm_param_t *par);

int pti_save_pcm (FILE *fp, const pti_img_t *img, const pti_pcm_param_t *par);

int pti_probe_pcm_fp (FILE *fp);
int pti_probe_pcm (const char *fname);


#endif
