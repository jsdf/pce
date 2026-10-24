/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/space.c                                        *
 * Created:     2020-05-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2022 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "space.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pti/pti.h>


int pti_space_add_left (pti_img_t *img, unsigned long clk)
{
	unsigned long i;
	unsigned long tmp;
	int           level;

	if (img->pulse_cnt > 0) {
		pti_pulse_get (img->pulse[0], &tmp, &level);

		if (level == 0) {
			pti_pulse_set (img->pulse, tmp + clk, 0);
			return (0);
		}
	}

	if (pti_img_set_pulse_max (img, img->pulse_cnt + 1)) {
		return (1);
	}

	i = img->pulse_cnt++;

	while (i > 0) {
		img->pulse[i] = img->pulse[i - 1];
		i -= 1;
	}

	pti_pulse_set (img->pulse, clk, 0);

	return (0);
}

int pti_space_add_right (pti_img_t *img, unsigned long clk)
{
	unsigned long tmp;
	int           level;

	if (img->pulse_cnt > 0) {
		pti_pulse_get (img->pulse[img->pulse_cnt - 1], &tmp, &level);

		if (level == 0) {
			pti_pulse_set (img->pulse + img->pulse_cnt - 1, tmp + clk, level);
			return (0);
		}
	}

	if (pti_img_add_pulse (img, clk, 0)) {
		return (1);
	}

	return (0);
}

int pti_space_add (pti_img_t *img, unsigned long clk)
{
	if (pti_space_add_left (img, clk)) {
		return (1);
	}

	if (pti_space_add_right (img, clk)) {
		return (1);
	}

	return (0);
}

int pti_space_auto (pti_img_t *img, unsigned long min)
{
	unsigned long i;
	unsigned long clk;
	int           level;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		if (clk >= min) {
			pti_pulse_set (img->pulse + i, clk, 0);
		}
	}

	return (0);
}

int pti_space_max (pti_img_t *img, unsigned long max)
{
	unsigned long i;
	unsigned long clk;
	int           level;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		if ((level == 0) && (clk > max)) {
			pti_pulse_set (img->pulse + i, max, level);
		}
	}

	return (0);
}

int pti_space_trim_left (pti_img_t *img)
{
	unsigned long i, j;
	unsigned long clk;
	int           level;

	i = 0;

	while (i < img->pulse_cnt) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		if (level != 0) {
			break;
		}

		i += 1;
	}

	if (i == 0) {
		return (0);
	}

	j = 0;

	while (i < img->pulse_cnt) {
		img->pulse[j++] = img->pulse[i++];
	}

	img->pulse_cnt = j;

	return (0);
}

int pti_space_trim_right (pti_img_t *img)
{
	unsigned long clk;
	int           level;

	while (img->pulse_cnt > 0) {
		pti_pulse_get (img->pulse[img->pulse_cnt - 1], &clk, &level);

		if (level != 0) {
			break;
		}

		img->pulse_cnt -= 1;
	}

	return (0);
}

int pti_space_trim (pti_img_t *img)
{
	if (pti_space_trim_right (img)) {
		return (1);
	}

	if (pti_space_trim_left (img)) {
		return (1);
	}

	return (0);
}
