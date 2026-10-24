/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/aym/encode.c                                       *
 * Created:     2015-05-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2016 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static
int aym_skip_space (FILE *fp)
{
	int c, com;

	com = 0;

	while ((c = fgetc (fp)) != EOF) {
		if (c == '#') {
			com = 1;
		}
		else if ((c == 0x0a) || (c == 0x0d)) {
			com = 0;
		}
		else if ((c == ' ') || (c == '\t')) {
			;
		}
		else if (com) {
			;
		}
		else {
			ungetc (c, fp);
			return (0);
		}
	}

	return (0);
}

static
int aym_get_str (FILE *fp, char *dst, unsigned max)
{
	int      c;
	unsigned i;

	aym_skip_space (fp);

	i = 0;

	while (i < max) {
		if ((c = fgetc (fp)) == EOF) {
			break;
		}

		if ((c >= 'A') && (c <= 'Z')) {
			;
		}
		else {
			ungetc (c, fp);
			break;
		}

		dst[i++] = c;
	}

	if ((i == 0) || (i >= max)) {
		return (1);
	}

	dst[i++] = 0;

	return (0);
}

static
int aym_get_int (FILE *fp, unsigned long long *val)
{
	int      c, ok;
	unsigned dig;

	aym_skip_space (fp);

	ok = 0;

	*val = 0;

	while (1) {
		if ((c = fgetc (fp)) == EOF) {
			break;
		}

		if ((c >= '0') && (c <= '9')) {
			dig = c - '0';
		}
		else {
			ungetc (c, fp);
			break;
		}

		*val = 10 * *val + dig;

		ok = 1;
	}

	return (ok == 0);
}

static
int aym_get_byte (FILE *fp, unsigned *val)
{
	int      c, ok;
	unsigned dig;

	aym_skip_space (fp);

	ok = 0;

	*val = 0;

	while (1) {
		if ((c = fgetc (fp)) == EOF) {
			break;
		}

		if ((c >= '0') && (c <= '9')) {
			dig = c - '0';
		}
		else if ((c >= 'A') && (c <= 'F')) {
			dig = c - 'A' + 10;
		}
		else if ((c >= 'a') && (c <= 'f')) {
			dig = c - 'a' + 10;
		}
		else {
			ungetc (c, fp);
			break;
		}

		*val = 16 * *val + dig;

		ok = 1;
	}

	return (ok == 0);
}

static
void aym_write_delay (FILE *out, unsigned long long val)
{
	while (val >= 0xfff000000) {
		fputc (0x3f, out);
		fputc (0xff, out);
		val -= 0xfff000000;
	}

	if (val & 0xfff000000) {
		fputc (0x30 | ((val >> 32) & 0x0f), out);
		fputc ((val >> 24) & 0xff, out);
	}

	if (val & 0x00fff000) {
		fputc (0x20 | ((val >> 20) & 0x0f), out);
		fputc ((val >> 12) & 0xff, out);
	}

	if (val & 0x00000fff) {
		fputc (0x10 | ((val >> 8) & 0x0f), out);
		fputc (val & 0xff, out);
	}
}

static
int aym_encode_text (FILE *inp, FILE *out)
{
	int                c;
	unsigned           i, n, type;
	unsigned long long val;
	char               str[256];

	if (aym_get_int (inp, &val)) {
		return (1);
	}

	type = val & 0x0f;

	n = 0;

	while ((n < 256) && ((c = fgetc (inp)) != EOF)) {
		if ((c == 0x0d) || (c == 0x0a)) {
			break;
		}

		if ((n == 0) && ((c == ' ') || (c == 't'))) {
			continue;
		}

		str[n++] = c;
	}

	if (n > 255) {
		return (1);
	}

	while ((n > 0) && ((str[n - 1] == ' ') || (str[n - 1] == '\t'))) {
		n -= 1;
	}

	fputc (0x80 | type, out);
	fputc (n, out);

	for (i = 0; i < n; i++) {
		fputc (str[i], out);
	}

	return (0);
}

static
int aym_encode_fp (FILE *inp, FILE *out, unsigned long th)
{
	unsigned long long vers, delay;
	unsigned char      buf[8];
	char               str[256];

	if (aym_get_str (inp, str, 256)) {
		return (1);
	}

	if (strcmp (str, "AYM") != 0) {
		fprintf (stderr, "%s: bad magic (%s)\n", arg0, str);
		return (1);
	}

	if (aym_get_int (inp, &vers)) {
		return (1);
	}

	if (vers != 0) {
		fprintf (stderr, "%s: unknown version (%llu)\n", arg0, vers);
		return (1);
	}

	aym_set_uint32_be (buf, 0, AYM_MAGIC);
	aym_set_uint32_be (buf, 4, 0);

	if (fwrite (buf, 1, 8, out) != 8) {
		return (1);
	}

	delay = 0;

	while (1) {
		if (aym_get_str (inp, str, 256)) {
			return (1);
		}

		if (strcmp (str, "DEL") == 0) {
			unsigned long long val;

			if (aym_get_int (inp, &val)) {
				return (1);
			}

			delay += val;
		}
		else if (strcmp (str, "END") == 0) {
			fputc (0xff, out);
			fputc (0xff, out);
			break;
		}
		else if (strcmp (str, "REG") == 0) {
			unsigned v1, v2;

			if ((delay > 0) && (delay >= th)) {
				aym_write_delay (out, delay);
				delay = 0;
			}

			if (aym_get_byte (inp, &v1)) {
				return (1);
			}

			if (aym_get_byte (inp, &v2)) {
				return (1);
			}

			fputc (v1 & 0x0f, out);
			fputc (v2 & 0xff, out);
		}
		else if (strcmp (str, "TXT") == 0) {
			if (aym_encode_text (inp, out)) {
				return (1);
			}
		}
		else {
			fprintf (stderr, "%s: bad keyword (%s)\n", arg0, str);
			return (1);
		}
	}

	return (0);
}

int aym_encode (const char *inp, const char *out, unsigned long th)
{
	int  r;
	FILE *finp, *fout;

	if (inp == NULL) {
		finp = stdin;
	}
	else if ((finp = fopen (inp, "r")) == NULL) {
		fprintf (stderr, "%s: can't open input file (%s)\n", arg0, inp);
		return (1);
	}

	if (out == NULL) {
		fout = stdout;
	}
	else if ((fout = fopen (out, "wb")) == NULL) {
		fprintf (stderr, "%s: can't open output file (%s)\n", arg0, out);
		fclose (finp);
		return (1);
	}

	r = aym_encode_fp (finp, fout, th);

	if (fout != stdout) {
		fclose (fout);
	}

	if (finp != stdin) {
		fclose (finp);
	}

	return (r);
}
