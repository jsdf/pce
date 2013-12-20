/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/comment.c                                      *
 * Created:     2013-12-19 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdio.h>
#include <string.h>

#include <drivers/pri/pri.h>


int pri_comment_add (pri_img_t *img, const char *str)
{
	unsigned char       c;
	const unsigned char *tmp;

	if (img->comment_size > 0) {
		c = 0x0a;

		if (pri_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	tmp = (const unsigned char *) str;

	if (pri_img_add_comment (img, tmp, strlen (str))) {
		return (1);
	}

	return (0);
}

int pri_comment_load (pri_img_t *img, const char *fname)
{
	int           c, cr;
	unsigned      i, nl;
	FILE          *fp;
	unsigned char buf[256];

	fp = fopen (fname, "r");

	if (fp == NULL) {
		return (1);
	}

	pri_img_set_comment (img, NULL, 0);

	cr = 0;
	nl = 0;
	i = 0;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			break;
		}

		if (c == 0x0d) {
			if (cr) {
				nl += 1;
			}

			cr = 1;
		}
		else if (c == 0x0a) {
			nl += 1;
			cr = 0;
		}
		else {
			if (cr) {
				nl += 1;
			}

			if (i > 0) {
				while (nl > 0) {
					buf[i++] = 0x0a;
					nl -= 1;

					if (i >= 256) {
						pri_img_add_comment (img, buf, i);
						i = 0;
					}
				}
			}

			nl = 0;
			cr = 0;

			buf[i++] = c;

			if (i >= 256) {
				pri_img_add_comment (img, buf, i);
				i = 0;
			}
		}
	}

	if (i > 0) {
		pri_img_add_comment (img, buf, i);
		i = 0;
	}

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: load comments from %s\n", arg0, fname);
	}

	return (0);
}

int pri_comment_save (pri_img_t *img, const char *fname)
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

int pri_comment_set (pri_img_t *img, const char *str)
{
	const unsigned char *tmp;

	if ((str == NULL) || (*str == 0)) {
		pri_img_set_comment (img, NULL, 0);
		return (0);
	}

	tmp = (const unsigned char *) str;

	if (pri_img_set_comment (img, tmp, strlen (str))) {
		return (1);
	}

	return (0);
}

int pri_comment_show (pri_img_t *img)
{
	unsigned i;

	fputs ("comments:\n", stdout);

	for (i = 0; i < img->comment_size; i++) {
		fputc (img->comment[i], stdout);
	}

	fputs ("\n", stdout);

	return (0);
}
