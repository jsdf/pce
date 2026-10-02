/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-wav.h                                *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2023 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PTI_IMG_WAV_H
#define PTI_IMG_WAV_H 1


#include <drivers/pti/pti.h>


typedef struct {
	unsigned long srate;
	unsigned      bits;
	unsigned      channels;
	unsigned long lowpass;
	unsigned      order;
} pti_wav_param_t;


void pti_wav_init (pti_wav_param_t *par);

void pti_wav_set_default_clock (unsigned long val);
void pti_wav_set_srate (unsigned long val);
void pti_wav_set_bits (unsigned val);
void pti_wav_set_lowpass (unsigned long val);
void pti_wav_set_lowpass_order (unsigned val);
void pti_wav_set_min_volume (unsigned val);

pti_img_t *pti_load_wav (FILE *fp, unsigned long clock);

int pti_save_wav (FILE *fp, const pti_img_t *img, const pti_wav_param_t *par);

int pti_probe_wav_fp (FILE *fp);
int pti_probe_wav (const char *fname);


#endif
