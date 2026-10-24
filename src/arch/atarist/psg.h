/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/psg.h                                       *
 * Created:     2013-06-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_PSG_H
#define PCE_ATARIST_PSG_H 1


#include <stdint.h>
#include <stdio.h>

#include <drivers/sound/sound.h>


#define PSG_BUF_SIZE 512


typedef struct {
	unsigned char  reg_sel;
	unsigned char  reg[16];

	unsigned long  clock;
	unsigned long  clock_div;

	unsigned long  srate;

	unsigned long  silence_cnt;
	unsigned long  silence_max;

	unsigned long  tone_per[3];
	unsigned long  tone_cnt[3];
	unsigned char  tone_val[3];

	unsigned long  noise_per;
	unsigned long  noise_cnt;
	unsigned long  noise_val;

	unsigned long  env_per;
	unsigned long  env_cnt;
	unsigned char  env_per2;
	unsigned char  env_cnt2;
	unsigned char  env_val;
	unsigned char  env_inc;
	unsigned char  env_idx;

	unsigned long  out_cnt;
	unsigned long  out_freq;
	unsigned long  inp_freq;

	char           speaker_on;

	char           filter_lp;
	char           filter_hp;
	long           filter_lp_x0;
	long           filter_lp_y0;
	long           filter_hp_x0;
	long           filter_hp_y0;

	unsigned long  lowpass_freq;
	sound_iir2_t   iir;

	unsigned short buf_cnt;
	uint16_t       buf[PSG_BUF_SIZE];
	uint16_t       last_smp;

	FILE           *aym;
	unsigned short aym_reg;
	unsigned long  aym_res;
	unsigned long  aym_clk;

	sound_drv_t   *drv;

	void           *seta_ext;
	void           (*seta_fct) (void *ext, unsigned char val);

	void           *setb_ext;
	void           (*setb_fct) (void *ext, unsigned char val);
} st_psg_t;


void st_psg_init (st_psg_t *psg);
void st_psg_free (st_psg_t *psg);

void st_psg_reset (st_psg_t *psg);

/*
 * Set the sound driver.
 */
int st_psg_set_driver (st_psg_t *psg, const char *drv);

/*
 * Get the sound driver.
 */
sound_drv_t *st_psg_get_driver (st_psg_t *psg);

/*
 * Set the output sample rate.
 *
 * This function should be called before the sound driver is set
 * using st_psg_set_driver().
 */
int st_psg_set_srate (st_psg_t *psg, unsigned long srate);

/*
 * Set the AYM output file.
 */
int st_psg_set_aym (st_psg_t *psg, const char *fname);

/*
 * Set the AYM resolution.
 *
 * Delays below res microseconds will be rounded down to zero.
 */
void st_psg_set_aym_resolution (st_psg_t *psg, unsigned long res);

/*
 * Set the cut off frequency for the lowpass filter.
 *
 * Zero disables the filter.
 */
void st_psg_set_lowpass (st_psg_t *psg, unsigned freq);

/*
 * Enable the highpass filter.
 */
void st_psg_set_highpass (st_psg_t *psg, int enable);

void st_psg_set_port_a_fct (st_psg_t *psg, void *ext, void *fct);
void st_psg_set_port_b_fct (st_psg_t *psg, void *ext, void *fct);

unsigned char st_psg_get_select (st_psg_t *psg);
void st_psg_set_select (st_psg_t *psg, unsigned char val);

unsigned char st_psg_get_data (st_psg_t *psg);
void st_psg_set_data (st_psg_t *psg, unsigned char val);

void st_psg_clock (st_psg_t *psg, unsigned long cnt);


#endif
