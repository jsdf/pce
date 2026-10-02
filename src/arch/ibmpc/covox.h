/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/covox.h                                       *
 * Created:     2010-08-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_IBMPC_COVOX_H
#define PCE_IBMPC_COVOX_H 1


#include <stdint.h>

#include <drivers/sound/sound.h>

#define PC_COVOX_BUF  2048
#define PC_COVOX_FIFO 16


typedef struct {
	sound_drv_t    *drv;

	char           disney;

	char           playing;

	unsigned char  data_val;
	unsigned char  ctrl_val;

	uint16_t       smp;
	unsigned       vol;

	unsigned char  fifo[PC_COVOX_FIFO];
	unsigned       fifo_i;
	unsigned       fifo_n;

	uint16_t       timeout_val;
	unsigned long  timeout_clk;

	unsigned long  clk;
	unsigned long  rem;

	unsigned long  srate;

	unsigned long  srate_div;

	unsigned long  lowpass_freq;
	sound_iir2_t   iir;

	unsigned       buf_cnt;
	uint16_t       buf[PC_COVOX_BUF];

	void           *get_clk_ext;
	unsigned long  (*get_clk) (void *ext);
} pc_covox_t;


void pc_covox_init (pc_covox_t *cov);
void pc_covox_free (pc_covox_t *cas);

pc_covox_t *pc_covox_new (void);
void pc_covox_del (pc_covox_t *cov);

void pc_covox_set_clk_fct (pc_covox_t *cov, void *ext, void *fct);

void pc_covox_set_mode (pc_covox_t *cov, unsigned mode);

int pc_covox_set_driver (pc_covox_t *cov, const char *driver, unsigned long srate);

void pc_covox_set_lowpass (pc_covox_t *cov, unsigned long freq);

void pc_covox_set_volume (pc_covox_t *cov, unsigned vol);

void pc_covox_set_data (pc_covox_t *cov, unsigned char val);
void pc_covox_set_ctrl (pc_covox_t *cov, unsigned char val);
unsigned char pc_covox_get_status (pc_covox_t *cov);

void pc_covox_clock (pc_covox_t *cov, unsigned long cnt);


#endif
