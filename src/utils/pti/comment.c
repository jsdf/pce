/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/comment.c                                      *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "comment.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pti/pti.h>


int pti_comment_add (pti_img_t *img, const char *str)
{
	unsigned char       c;
	const unsigned char *tmp;

	if (img->comment_size > 0) {
		c = 0x0a;

		if (pti_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	tmp = (const unsigned char *) str;

	if (pti_img_add_comment (img, tmp, strlen (str))) {
		return (1);
	}

	return (0);
}

int pti_comment_load (pti_img_t *img, const char *fname)
{
	int           c, cr;
	unsigned      i, nl;
	FILE          *fp;
	unsigned char buf[256];

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	pti_img_set_comment (img, NULL, 0);

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
						pti_img_add_comment (img, buf, i);
						i = 0;
					}
				}
			}

			nl = 0;
			cr = 0;

			buf[i++] = c;

			if (i >= 256) {
				pti_img_add_comment (img, buf, i);
				i = 0;
			}
		}
	}

	if (i > 0) {
		pti_img_add_comment (img, buf, i);
		i = 0;
	}

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: load comments from %s\n", arg0, fname);
	}

	return (0);
}

int pti_comment_save (pti_img_t *img, const char *fname)
{
	unsigned cnt;
	FILE     *fp;

	if ((fp = fopen (fname, "w")) == NULL) {
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

int pti_comment_set (pti_img_t *img, const char *str)
{
	const unsigned char *tmp;

	if ((str == NULL) || (*str == 0)) {
		pti_img_set_comment (img, NULL, 0);
		return (0);
	}

	tmp = (const unsigned char *) str;

	if (pti_img_set_comment (img, tmp, strlen (str))) {
		return (1);
	}

	return (0);
}

int pti_comment_show (pti_img_t *img)
{
	unsigned i;

	fputs ("comments:\n", stdout);

	for (i = 0; i < img->comment_size; i++) {
		fputc (img->comment[i], stdout);
	}

	fputs ("\n", stdout);

	return (0);
}
