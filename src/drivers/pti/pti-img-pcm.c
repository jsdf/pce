/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-pcm.c                                *
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


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pti.h"
#include "pti-io.h"
#include "pti-img-pcm.h"


static pti_pcm_param_t pcm_default = {
	.clock = 1193182,
	.srate = 44100,
	.sign = 1
};


void pti_pcm_set_default_clock (unsigned long val)
{
	if (val > 0) {
		pcm_default.clock = val;
	}
}

void pti_pcm_set_default_srate (unsigned long val)
{
	if (val > 0) {
		pcm_default.srate = val;
	}
}

static
int pcm_get_level (unsigned char smp, int sign)
{
	if (smp == 0) {
		return (0);
	}

	if (sign == 0) {
		smp ^= 0x80;
	}

	if (smp & 0x80) {
		return (-1);
	}

	return (1);
}

static
int pcm_load_img (FILE *fp, pti_img_t *img, const pti_pcm_param_t *par)
{
	int           c;
	unsigned long clk, rem;
	int           level1, level2;

	if ((c = fgetc (fp)) == EOF) {
		return (0);
	}

	rem = 0;
	clk = 0;
	level1 = pcm_get_level (c & 0xff, par->sign);

	while ((c = fgetc (fp)) != EOF) {
		rem += img->clock;
		clk += rem / par->srate;
		rem %= par->srate;

		level2 = pcm_get_level (c & 0xff, par->sign);

		if (level1 != level2) {
			if (pti_img_add_pulse (img, clk, level1)) {
				return (1);
			}

			clk = 0;
			level1 = level2;
		}
	}

	if (clk > 0) {
		if (pti_img_add_pulse (img, clk, level1)) {
			return (1);
		}
	}

	return (0);
}

pti_img_t *pti_load_pcm (FILE *fp, const pti_pcm_param_t *par)
{
	pti_img_t *img;

	if (par == NULL) {
		par = &pcm_default;
	}

	if ((img = pti_img_new()) == NULL) {
		return (NULL);
	}

	pti_img_set_clock (img, par->clock);

	if (pcm_load_img (fp, img, par)) {
		pti_img_del (img);
		return (NULL);
	}

	pti_img_clean (img);

	return (img);
}


int pti_save_pcm (FILE *fp, const pti_img_t *img, const pti_pcm_param_t *par)
{
	unsigned long i;
	unsigned long clk, rem;
	int           level;
	unsigned char val;

	if (par == NULL) {
		par = &pcm_default;
	}

	rem = 0;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		clk = pti_pulse_convert (clk, par->srate, img->clock, &rem);

		val = (level < 0) ? 0xc0 : 0x40;

		if (par->sign == 0) {
			val ^= 0x80;
		}

		while (clk-- > 0) {
			fputc (val, fp);
		}
	}

	return (0);
}


int pti_probe_pcm_fp (FILE *fp)
{
	return (0);
}

int pti_probe_pcm (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pti_probe_pcm_fp (fp);

	fclose (fp);

	return (r);
}
