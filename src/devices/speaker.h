/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/speaker.h                                        *
 * Created:     2022-02-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_SPEAKER_H
#define PCE_DEVICES_SPEAKER_H 1


#include <drivers/sound/sound.h>


#define SPEAKER_BUF 1024


typedef struct {
	sound_drv_t    *drv;

	unsigned long  srate;
	unsigned long  input_clock;
	unsigned       speed_mul;

	char           enabled;

	int            speaker_inp;
	uint16_t       speaker_val;
	uint16_t       sample_acc;

	uint16_t       val_n;
	uint16_t       val_p;

	uint16_t       timeout_val;
	unsigned long  timeout_clk;
	unsigned long  timeout_max;

	unsigned long  clk;
	unsigned long  rem;

	unsigned long  lowpass_freq;
	sound_iir2_t   iir;

	unsigned       buf_cnt;
	uint16_t       buf[SPEAKER_BUF];

	void           *get_clock_ext;
	unsigned long  (*get_clock) (void *ext);
} speaker_t;


void spk_init (speaker_t *spk, int level);
void spk_free (speaker_t *spk);

speaker_t *_spk_new (int level);
void spk_del (speaker_t *spk);

void spk_set_clock_fct (speaker_t *spk, void *ext, void *fct);

void spk_set_input_clock (speaker_t *spk, unsigned long clk);

int spk_set_driver (speaker_t *spk, const char *driver, unsigned long srate);

void spk_set_lowpass (speaker_t *spk, unsigned long freq);

void spk_set_volume (speaker_t *spk, unsigned vol);

void spk_set_input (speaker_t *spk, int val);

void spk_check (speaker_t *spk);


#endif
