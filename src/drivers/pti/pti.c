/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti.c                                        *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pti.h"


pti_img_t *pti_img_new (void)
{
	pti_img_t *img;

	if ((img = malloc (sizeof (pti_img_t))) == NULL) {
		return (NULL);
	}

	img->clock = 1;

	img->pulse_cnt = 0;
	img->pulse_max = 0;
	img->pulse = 0;

	img->comment_size = 0;
	img->comment = NULL;

	img->save_inverted = 0;

	return (img);
}

void pti_img_del (pti_img_t *img)
{
	if (img != NULL) {
		free (img->comment);
		free (img->pulse);
		free (img);
	}
}

pti_img_t *pti_img_clone (const pti_img_t *img, int data)
{
	pti_img_t *ret;

	if ((ret = pti_img_new()) == NULL) {
		return (NULL);
	}

	ret->clock = img->clock;

	if (data && (img->pulse_cnt > 0)) {
		ret->pulse = malloc (img->pulse_cnt * sizeof (uint32_t));

		if (ret->pulse == NULL) {
			pti_img_del (ret);
			return (NULL);
		}

		ret->pulse_cnt = img->pulse_cnt;
		ret->pulse_max = img->pulse_cnt;

		memcpy (ret->pulse, img->pulse, img->pulse_cnt * sizeof (uint32_t));
	}

	return (ret);
}

int pti_img_set_pulse_max (pti_img_t *img, unsigned long cnt)
{
	uint32_t *tmp;

	if (cnt < img->pulse_cnt) {
		return (1);
	}

	if (cnt == img->pulse_max) {
		return (0);
	}

	if ((tmp = realloc (img->pulse, cnt * sizeof (uint32_t))) == NULL) {
		return (1);
	}

	img->pulse_max = cnt;

	return (0);
}

int pti_img_add_pulses (pti_img_t *img, const uint32_t *buf, unsigned long cnt)
{
	unsigned long i;
	unsigned long max;
	uint32_t      *tmp;

	if ((img->pulse_cnt + cnt) > img->pulse_max) {
		max = (img->pulse_max < 256) ? 256 : img->pulse_max;

		while (max < (img->pulse_cnt + cnt)) {
			max *= 2;

			if ((max & (max - 1)) != 0) {
				max &= max - 1;
			}
		}

		tmp = realloc (img->pulse, max * sizeof (uint32_t));

		if (tmp == NULL) {
			return (1);
		}

		img->pulse = tmp;
		img->pulse_max = max;
	}

	for (i = 0; i < cnt; i++) {
		img->pulse[img->pulse_cnt++] = buf[i];
	}

	return (0);
}

void pti_img_clean (pti_img_t *img)
{
	unsigned long i, j;
	uint32_t      *p;

	if (img->pulse_cnt > 1) {
		j = 1;
		p = img->pulse;

		for (i = 1; i < img->pulse_cnt; i++) {
			if ((p[j - 1] & 3) == (p[i] & 3)) {
				p[j - 1] += p[i] & ~3UL;
			}
			else {
				p[j++] = p[i];
			}
		}

		img->pulse_cnt = j;
	}

	if (img->pulse_max > img->pulse_cnt) {
		p = realloc (img->pulse, img->pulse_cnt * sizeof (uint32_t));

		if (p == NULL) {
			return;
		}

		img->pulse = p;
		img->pulse_max = img->pulse_cnt;
	}
}

void pti_img_set_clock (pti_img_t *img, unsigned long clock)
{
	img->clock = (clock > 0) ? clock : 1;
}

unsigned long pti_img_get_clock (const pti_img_t *img)
{
	return (img->clock);
}

void pti_img_set_inverted (pti_img_t *img, int val)
{
	img->save_inverted = (val != 0);
}

int pti_img_get_inverted (const pti_img_t *img)
{
	return (img->save_inverted != 0);
}

void pti_pulse_get (uint32_t pulse, unsigned long *clk, int *level)
{
	*clk = pulse >> 2;

	switch (pulse & 3) {
	case 0:
		*level = 1;
		break;

	case 1:
		*level = 0;
		break;

	case 3:
		*level = -1;
		break;

	default:
		*level = 0;
		break;
	}
}

void pti_pulse_set (uint32_t *pulse, unsigned long clk, int level)
{
	*pulse = clk << 2;

	if (level < 0) {
		*pulse |= 3;
	}
	else if (level == 0) {
		*pulse |= 1;
	}
}

unsigned long pti_pulse_convert (unsigned long val, unsigned long dclk, unsigned long sclk, unsigned long *rem)
{
	unsigned long long tmp;

	tmp = (unsigned long long) dclk * val + *rem;
	val = tmp / sclk;
	*rem = tmp % sclk;

	return (val);
}

int pti_img_get_pulse (const pti_img_t *img, unsigned long idx, unsigned long *clk, int *level)
{
	if (idx >= img->pulse_cnt) {
		return (1);
	}

	pti_pulse_get (img->pulse[idx], clk, level);

	return (0);
}

int pti_img_set_pulse (pti_img_t *img, unsigned long idx, unsigned long clk, int level)
{
	if (idx >= img->pulse_cnt) {
		return (1);
	}

	pti_pulse_set (img->pulse + idx, clk, level);

	return (0);
}

int pti_img_add_pulse (pti_img_t *img, unsigned long clk, int level)
{
	unsigned long clk1;
	int           level1;
	uint32_t      *p;
	uint32_t      pulse;

	if (img->pulse_cnt > 0) {
		p = img->pulse + img->pulse_cnt - 1;

		pti_pulse_get (*p, &clk1, &level1);

		if (level1 == level) {
			pti_pulse_set (p, clk1 + clk, level1);
			return (0);
		}
	}

	pti_pulse_set (&pulse, clk, level);

	return (pti_img_add_pulses (img, &pulse, 1));
}

void pti_time_set (unsigned long *sec, unsigned long *clk, double val, unsigned long clock)
{
	*sec = (unsigned long) val;
	*clk = (unsigned long) (clock * (val - *sec));

	if (*clk >= clock) {
		*clk = clock - 1;
	}
}

void pti_time_get (unsigned long sec, unsigned long clk, double *val, unsigned long clock)
{
	*val = (double) sec + (double) clk / (double) clock;
}


void pti_img_get_length (const pti_img_t *img, unsigned long *sec, unsigned long *clk)
{
	unsigned long i;
	unsigned long tmp;
	int           level;

	*sec = 0;
	*clk = 0;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &tmp, &level);

		*clk += tmp;

		while (*clk >= img->clock) {
			*sec += 1;
			*clk -= img->clock;
		}
	}
}

void pti_img_get_time (const pti_img_t *img, unsigned long idx, unsigned long *sec, unsigned long *clk)
{
	unsigned long i;
	unsigned long tmp;
	int           level;

	if (idx > img->pulse_cnt) {
		idx = img->pulse_cnt;
	}

	*sec = 0;
	*clk = 0;

	for (i = 0; i < idx; i++) {
		pti_pulse_get (img->pulse[i], &tmp, &level);

		*clk += tmp;

		while (*clk >= img->clock) {
			*sec += 1;
			*clk -= img->clock;
		}
	}
}

void pti_img_get_index (const pti_img_t *img, unsigned long *idx, unsigned long sec, unsigned long clk)
{
	unsigned long i;
	unsigned long tmp;
	int           level;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &tmp, &level);

		while (clk < tmp) {
			if (sec == 0) {
				*idx = i;
				return;
			}

			sec -= 1;
			clk += img->clock;
		}

		clk -= tmp;
	}

	*idx = i;
}

int pti_img_add_comment (pti_img_t *img, const void *buf, unsigned cnt)
{
	unsigned char *tmp;

	tmp = realloc (img->comment, img->comment_size + cnt);

	if (tmp == NULL) {
		return (1);
	}

	memcpy (tmp + img->comment_size, buf, cnt);

	img->comment_size += cnt;
	img->comment = tmp;

	return (0);
}

int pti_img_set_comment (pti_img_t *img, const void *buf, unsigned cnt)
{
	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;

	if ((buf == NULL) || (cnt == 0)) {
		return (0);
	}

	if (pti_img_add_comment (img, buf, cnt)) {
		return (1);
	}

	return (0);
}
