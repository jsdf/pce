/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-io.h                                     *
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


#ifndef PTI_IO_H
#define PTI_IO_H 1


#include <stdio.h>

#include <drivers/pti/pti.h>


#define PTI_FORMAT_NONE 0
#define PTI_FORMAT_PTI  1
#define PTI_FORMAT_TXT  2
#define PTI_FORMAT_TAP  3
#define PTI_FORMAT_WAV  4
#define PTI_FORMAT_CAS  5
#define PTI_FORMAT_PCM  6

#define PTI_CLOCK_PET        1000000
#define PTI_CLOCK_VIC20_NTSC 1022727
#define PTI_CLOCK_VIC20_PAL  1108405
#define PTI_CLOCK_C64_NTSC   1022727
#define PTI_CLOCK_C64_PAL    985248
#define PTI_CLOCK_C16_NTSC   894886
#define PTI_CLOCK_C16_PAL    886724
#define PTI_CLOCK_PC_PIT     1193182
#define PTI_CLOCK_PC_CPU     4772728
#define PTI_CLOCK_SPECTRUM   3500000


unsigned pti_get_uint16_be (const void *buf, unsigned idx);
unsigned pti_get_uint16_le (const void *buf, unsigned idx);

unsigned long pti_get_uint32_be (const void *buf, unsigned idx);
unsigned long pti_get_uint32_le (const void *buf, unsigned idx);

void pti_set_uint16_be (void *buf, unsigned idx, unsigned val);
void pti_set_uint16_le (void *buf, unsigned idx, unsigned val);

void pti_set_uint32_be (void *buf, unsigned idx, unsigned long val);
void pti_set_uint32_le (void *buf, unsigned idx, unsigned long val);


int pti_set_pos (FILE *fp, unsigned long ofs);
int pti_read (FILE *fp, void *buf, unsigned long cnt);
int pti_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt);
int pti_write (FILE *fp, const void *buf, unsigned long cnt);
int pti_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt);
int pti_skip (FILE *fp, unsigned long cnt);


void pti_cas_set_default_clock (unsigned long val);
void pti_pcm_set_default_clock (unsigned long val);
void pti_pcm_set_default_srate (unsigned long val);
void pti_tap_set_default_clock (unsigned long val);
void pti_wav_set_default_clock (unsigned long val);
void pti_wav_set_bits (unsigned val);
void pti_wav_set_lowpass (unsigned long val);
void pti_wav_set_lowpass_order (unsigned val);
void pti_wav_set_sine_wave (int val);
void pti_wav_set_srate (unsigned long val);
void pti_wav_set_min_volume (unsigned val);
void pti_set_default_clock (unsigned long val);


pti_img_t *pti_img_load_fp (FILE *fp, unsigned type);
pti_img_t *pti_img_load (const char *fname, unsigned type);

int pti_img_save_fp (FILE *fp, pti_img_t *img, unsigned type);
int pti_img_save (const char *fname, pti_img_t *img, unsigned type);


#endif
