/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/speaker.h                                     *
 * Created:     2012-07-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_RC759_SPEAKER_H
#define PCE_RC759_SPEAKER_H 1


#include <drivers/sound/sound.h>


#define RC759_SPEAKER_BUF 1024


typedef struct {
	sound_drv_t    *drv;

	char           playing;

	char           speaker_out;

	uint16_t       timeout_val;
	unsigned long  timeout_clk;

	unsigned long  clk;
	unsigned long  rem;

	unsigned long  sample_acc;

	unsigned long  srate;

	unsigned long  input_clock;

	unsigned long  lowpass_freq;
	sound_iir2_t   iir;

	unsigned       buf_cnt;
	uint16_t       buf[RC759_SPEAKER_BUF];

	uint16_t       val_msk;
	uint16_t       val_on;
	uint16_t       val_off;

	void           *get_clk_ext;
	unsigned long  (*get_clk) (void *ext);
} rc759_speaker_t;


void rc759_spk_init (rc759_speaker_t *spk);
void rc759_spk_free (rc759_speaker_t *spk);

rc759_speaker_t *rc759_spk_new (void);
void rc759_spk_del (rc759_speaker_t *spk);

void rc759_spk_set_clk_fct (rc759_speaker_t *spk, void *ext, void *fct);

void rc759_spk_set_input_clock (rc759_speaker_t *spk, unsigned long clk);

int rc759_spk_set_driver (rc759_speaker_t *spk, const char *driver, unsigned long srate);

void rc759_spk_set_lowpass (rc759_speaker_t *spk, unsigned long freq);

void rc759_spk_set_volume (rc759_speaker_t *spk, unsigned vol);

void rc759_spk_set_out (rc759_speaker_t *spk, unsigned char val);

void rc759_spk_clock (rc759_speaker_t *spk, unsigned long cnt);


#endif
