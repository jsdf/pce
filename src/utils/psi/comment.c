/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/comment.c                                      *
 * Created:     2013-06-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#include "main.h"
#include "comment.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


int psi_add_comment (psi_img_t *img, const char *str)
{
	unsigned char       c;
	const unsigned char *tmp;

	if (img->comment_size > 0) {
		c = 0x0a;

		if (psi_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	tmp = (const unsigned char *) str;

	if (psi_img_add_comment (img, tmp, strlen (str))) {
		return (1);
	}

	psi_img_clean_comment (img);

	return (0);
}

int psi_load_comment (psi_img_t *img, const char *fname)
{
	unsigned      n;
	FILE          *fp;
	unsigned char buf[256];

	fp = fopen (fname, "r");

	if (fp == NULL) {
		return (1);
	}

	psi_img_set_comment (img, NULL, 0);

	while (1) {
		n = fread (buf, 1, 256, fp);

		if (n == 0) {
			break;
		}

		psi_img_add_comment (img, buf, n);
	}

	fclose (fp);

	psi_img_clean_comment (img);

	if (par_verbose) {
		fprintf (stderr, "%s: load comments from %s\n", arg0, fname);
	}

	return (0);
}

int psi_save_comment (psi_img_t *img, const char *fname)
{
	unsigned cnt;
	FILE     *fp;

	fp = fopen (fname, "w");

	if (fp == NULL) {
		return (1);
	}

	cnt = img->comment_size;

	if (cnt > 0) {
		if (fwrite (img->comment, 1, cnt, fp) != cnt) {
			fclose (fp);
			return (1);
		}

		fputc (0x0a, fp);
	}

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: save comments to %s\n", arg0, fname);
	}

	return (0);
}

int psi_set_comment (psi_img_t *img, const char *str)
{
	const unsigned char *tmp;

	if ((str == NULL) || (*str == 0)) {
		psi_img_set_comment (img, NULL, 0);
		return (0);
	}

	tmp = (const unsigned char *) str;

	if (psi_img_set_comment (img, tmp, strlen (str))) {
		return (1);
	}

	psi_img_clean_comment (img);

	return (0);
}

int psi_show_comment (psi_img_t *img)
{
	unsigned i;

	fputs ("comments:\n", stdout);

	for (i = 0; i < img->comment_size; i++) {
		fputc (img->comment[i], stdout);
	}

	fputs ("\n", stdout);

	return (0);
}
