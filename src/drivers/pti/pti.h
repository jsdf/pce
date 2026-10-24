/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti.h                                        *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PTI_PTI_H
#define PTI_PTI_H 1


#include <stdint.h>


typedef struct {
	unsigned long clock;

	unsigned long pulse_cnt;
	unsigned long pulse_max;
	uint32_t      *pulse;

	unsigned long comment_size;
	unsigned char *comment;

	char          save_inverted;
} pti_img_t;


pti_img_t *pti_img_new (void);
void pti_img_del (pti_img_t *img);

pti_img_t *pti_img_clone (const pti_img_t *img, int data);

int pti_img_set_pulse_max (pti_img_t *img, unsigned long cnt);

int pti_img_add_pulses (pti_img_t *img, const uint32_t *buf, unsigned long cnt);

void pti_img_clean (pti_img_t *img);

void pti_img_set_clock (pti_img_t *img, unsigned long clock);
unsigned long pti_img_get_clock (const pti_img_t *img);

void pti_img_set_inverted (pti_img_t *img, int val);
int pti_img_get_inverted (const pti_img_t *img);

void pti_pulse_get (uint32_t pulse, unsigned long *clk, int *level);
void pti_pulse_set (uint32_t *pulse, unsigned long clk, int level);

unsigned long pti_pulse_convert (unsigned long val, unsigned long dclk, unsigned long sclk, unsigned long *rem);

int pti_img_get_pulse (const pti_img_t *img, unsigned long idx, unsigned long *clk, int *level);
int pti_img_set_pulse (pti_img_t *img, unsigned long idx, unsigned long clk, int level);

int pti_img_add_pulse (pti_img_t *img, unsigned long clk, int level);

void pti_time_set (unsigned long *sec, unsigned long *clk, double val, unsigned long clock);
void pti_time_get (unsigned long sec, unsigned long clk, double *val, unsigned long clock);

void pti_img_get_length (const pti_img_t *img, unsigned long *sec, unsigned long *clk);
void pti_img_get_time (const pti_img_t *img, unsigned long idx, unsigned long *sec, unsigned long *clk);
void pti_img_get_index (const pti_img_t *img, unsigned long *idx, unsigned long sec, unsigned long clk);

int pti_img_add_comment (pti_img_t *img, const void *buf, unsigned cnt);
int pti_img_set_comment (pti_img_t *img, const void *buf, unsigned cnt);


#endif
