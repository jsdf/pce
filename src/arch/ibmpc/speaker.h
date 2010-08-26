/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/speaker.h                                     *
 * Created:     2010-02-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_IBMPC_SPEAKER_H
#define PCE_IBMPC_SPEAKER_H 1


#include <drivers/sound/sound.h>


#define PC_SPEAKER_BUF 1024


typedef struct {
	sound_drv_t    *drv;

	char           playing;

	char           speaker_msk;
	char           speaker_out;

	uint16_t       timeout_val;
	unsigned long  timeout_clk;

	unsigned long  clk;
	unsigned long  rem;

	unsigned long  sample_acc;

	unsigned long  srate;

	unsigned long  lowpass_freq;
	sound_iir2_t   iir;

	unsigned       buf_cnt;
	uint16_t       buf[PC_SPEAKER_BUF];

	uint16_t       val_msk;
	uint16_t       val_on;
	uint16_t       val_off;

	void           *get_clk_ext;
	unsigned long  (*get_clk) (void *ext);
} pc_speaker_t;


void pc_speaker_init (pc_speaker_t *spk);
void pc_speaker_free (pc_speaker_t *cas);

pc_speaker_t *pc_speaker_new (void);
void pc_speaker_del (pc_speaker_t *spk);

void pc_speaker_set_clk_fct (pc_speaker_t *spk, void *ext, void *fct);

int pc_speaker_set_driver (pc_speaker_t *spk, const char *driver, unsigned long srate);

void pc_speaker_set_lowpass (pc_speaker_t *spk, unsigned long freq);

void pc_speaker_set_volume (pc_speaker_t *spk, unsigned vol);

void pc_speaker_set_msk (pc_speaker_t *spk, unsigned char val);
void pc_speaker_set_out (pc_speaker_t *spk, unsigned char val);

void pc_speaker_clock (pc_speaker_t *spk, unsigned long cnt);


#endif
