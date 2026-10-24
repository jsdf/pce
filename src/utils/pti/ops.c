/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/ops.c                                          *
 * Created:     2020-04-30 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "comment.h"
#include "ops.h"
#include "space.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/getopt.h>

#include <drivers/pti/pti.h>
#include <drivers/pti/pti-io.h>


static
int pti_get_opt (int argc, char **argv, char ***optarg, unsigned cnt)
{
	if (pce_getoptarg (argc, argv, optarg, cnt) != 0) {
		fprintf (stderr, "%s: missing argument\n", arg0);
		return (1);
	}

	return (0);
}

int pti_cat (pti_img_t **img, const char *fname)
{
	unsigned long i;
	unsigned long clk, rem;
	int           level;
	pti_img_t     *src;

	if ((src = pti_load_image (fname)) == NULL) {
		return (1);
	}

	if (*img == NULL) {
		*img = src;
		return (0);
	}

	rem = 0;

	for (i = 0; i < src->pulse_cnt; i++) {
		pti_pulse_get (src->pulse[i], &clk, &level);

		clk = pti_pulse_convert (clk, (*img)->clock, src->clock, &rem);

		if (pti_img_add_pulse ((*img), clk, level)) {
			pti_img_del (src);
			return (1);
		}
	}

	pti_img_del (src);

	return (0);
}

int pti_info (pti_img_t *img)
{
	unsigned      msc;
	unsigned long min, sec, clk;

	pti_img_get_length (img, &sec, &clk);

	min = sec / 60;
	sec = sec % 60;
	msc = (unsigned) ((1000.0 * clk) / img->clock);

	printf ("time:   %02lu:%02lu.%03u\n", min, sec, msc);
	printf ("clock:  %lu\n", pti_img_get_clock (img));
	printf ("pulses: %lu\n", img->pulse_cnt);

	if (img->comment_size > 0) {
		fputs ("\n", stdout);
		pti_comment_show (img);
	}

	return (0);
}

int pti_invert (pti_img_t *img)
{
	unsigned long i;
	unsigned long clk;
	int           level;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);
		pti_pulse_set (&img->pulse[i], clk, -level);
	}

	return (0);
}

int pti_new (pti_img_t **img)
{
	pti_img_del (*img);

	if ((*img = pti_img_new()) == NULL) {
		return (1);
	}

	pti_img_set_clock (*img, par_default_clock);

	return (0);
}

int pti_scale (pti_img_t **img, unsigned long dclk, int set)
{
	unsigned long i;
	unsigned long sclk, rem;
	unsigned long clk;
	int           level;
	pti_img_t     *dst, *src;

	src = *img;

	if ((dst = pti_img_clone (src, 0)) == NULL) {
		return (1);
	}

	sclk = pti_img_get_clock (src);

	rem = 0;

	for (i = 0; i < src->pulse_cnt; i++) {
		pti_pulse_get (src->pulse[i], &clk, &level);

		clk = pti_pulse_convert (clk, dclk, sclk, &rem);

		if (pti_img_add_pulse (dst, clk, level)) {
			pti_img_del (src);
			return (1);
		}
	}

	pti_img_del (src);

	if (set) {
		pti_img_set_clock (dst, dclk);
	}

	*img = dst;

	return (0);
}

int pti_trim_left (pti_img_t *img, unsigned long sec, unsigned long clk)
{
	unsigned long i, j;
	unsigned long sec2, clk2;
	int           level;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk2, &level);

		sec2 = clk2 / img->clock;
		clk2 = clk2 % img->clock;

		if ((sec2 > sec) || ((sec2 == sec) && (clk2 > clk))) {
			break;
		}

		if (clk2 > clk) {
			sec -= 1;
			clk += img->clock;
		}

		clk -= clk2;
		sec -= sec2;
	}

	if (i >= img->pulse_cnt) {
		img->pulse_cnt = 0;
		return (0);
	}

	if (clk > clk2) {
		sec2 -= 1;
		clk2 += img->clock;
	}

	clk2 -= clk;
	sec2 -= sec;

	clk2 += img->clock * sec2;

	pti_pulse_set (img->pulse + i, clk2, level);

	j = 0;
	while (i < img->pulse_cnt) {
		img->pulse[j++] = img->pulse[i++];
	}

	img->pulse_cnt = j;

	return (0);
}

int pti_trim_right (pti_img_t *img, unsigned long sec, unsigned long clk)
{
	unsigned long idx;

	pti_img_get_index (img, &idx, sec, clk);

	if (idx < img->pulse_cnt) {
		img->pulse_cnt = idx;
	}

	return (0);
}

int pti_operation (pti_img_t **img, const char *op, int argc, char **argv)
{
	int  r;
	char **optarg;

	if (*img == NULL) {
		if ((*img = pti_img_new()) == NULL) {
			return (1);
		}

		pti_img_set_clock (*img, par_default_clock);
	}

	r = 1;

	if (strcmp (op, "cat") == 0) {
		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		r = pti_cat (img, optarg[0]);
	}
	else if (strcmp (op, "comment-add") == 0) {
		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		r = pti_comment_add (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-load") == 0) {
		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		r = pti_comment_load (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-print") == 0) {
		r = pti_comment_show (*img);
	}
	else if (strcmp (op, "comment-save") == 0) {
		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		r = pti_comment_save (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-set") == 0) {
		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		r = pti_comment_set (*img, optarg[0]);
	}
	else if (strcmp (op, "fix-clock") == 0) {
		unsigned long clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_freq (optarg[0], &clk)) {
			return (1);
		}

		pti_img_set_clock (*img, clk);

		r = 0;
	}
	else if (strcmp (op, "info") == 0) {
		r = pti_info (*img);
	}
	else if (strcmp (op, "invert") == 0) {
		r = pti_invert (*img);
	}
	else if (strcmp (op, "new") == 0) {
		r = pti_new (img);
	}
	else if (strcmp (op, "save-inverted") == 0) {
		int val;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_bool (optarg[0], &val)) {
			return (1);
		}

		pti_img_set_inverted (*img, val);

		r = 0;
	}
	else if (strcmp (op, "scale") == 0) {
		double fct;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_double (optarg[0], &fct)) {
			return (1);
		}

		r = pti_scale (img, (unsigned long) (fct * (*img)->clock), 0);
	}
	else if (strcmp (op, "set-clock") == 0) {
		unsigned long clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_freq (optarg[0], &clk)) {
			return (1);
		}

		r = pti_scale (img, clk, 1);
	}
	else if (strcmp (op, "space-add-left") == 0) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_space_add_left (*img, (*img)->clock * sec + clk);
	}
	else if ((strcmp (op, "space-add-right") == 0) || (strcmp (op, "space") == 0)) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_space_add_right (*img, (*img)->clock * sec + clk);
	}
	else if (strcmp (op, "space-add") == 0) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_space_add (*img, (*img)->clock * sec + clk);
	}
	else if (strcmp (op, "space-auto") == 0) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_space_auto (*img, (*img)->clock * sec + clk);
	}
	else if (strcmp (op, "space-max") == 0) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_space_max (*img, (*img)->clock * sec + clk);
	}
	else if (strcmp (op, "space-trim-left") == 0) {
		r = pti_space_trim_left (*img);
	}
	else if (strcmp (op, "space-trim-right") == 0) {
		r = pti_space_trim_right (*img);
	}
	else if (strcmp (op, "space-trim") == 0) {
		r = pti_space_trim (*img);
	}
	else if (strcmp (op, "trim-left") == 0) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_trim_left (*img, sec, clk);
	}
	else if (strcmp (op, "trim-right") == 0) {
		unsigned long sec, clk;

		if (pti_get_opt (argc, argv, &optarg, 1)) {
			return (1);
		}

		if (pti_parse_time_clk (optarg[0], &sec, &clk, (*img)->clock)) {
			return (1);
		}

		r = pti_trim_right (*img, sec, clk);
	}
	else {
		fprintf (stderr, "%s: unknown operation (%s)\n", arg0, op);
		return (1);
	}

	if (r) {
		fprintf (stderr, "%s: operation failed (%s)\n", arg0, op);
	}

	return (r);
}
