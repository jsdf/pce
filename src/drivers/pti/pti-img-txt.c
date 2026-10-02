/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-txt.c                                *
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


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pti.h"
#include "pti-io.h"
#include "pti-img-txt.h"

#include <lib/text.h>


typedef struct {
	pce_text_t txt;
	pti_img_t  *img;

	char      invert;
} txt_load_t;


static int txt_load_seq (txt_load_t *txt);


static
int txt_add_pulse (txt_load_t *txt, unsigned long val, int level)
{
	if (txt->invert) {
		level = -level;
	}

	if (pti_img_add_pulse (txt->img, val, level)) {
		return (1);
	}

	return (0);
}

static
int txt_load_clock (txt_load_t *txt)
{
	unsigned long val;

	if (txt_match (&txt->txt, "PET", 1)) {
		val = PTI_CLOCK_VIC20_NTSC;
	}
	else if (txt_match (&txt->txt, "VIC20-NTSC", 1)) {
		val = PTI_CLOCK_VIC20_NTSC;
	}
	else if (txt_match (&txt->txt, "VIC20-PAL", 1)) {
		val = PTI_CLOCK_VIC20_PAL;
	}
	else if (txt_match (&txt->txt, "C64-NTSC", 1)) {
		val = PTI_CLOCK_C64_NTSC;
	}
	else if (txt_match (&txt->txt, "C64-PAL", 1)) {
		val = PTI_CLOCK_C64_PAL;
	}
	else if (txt_match (&txt->txt, "C16-NTSC", 1)) {
		val = PTI_CLOCK_C16_NTSC;
	}
	else if (txt_match (&txt->txt, "C16-PAL", 1)) {
		val = PTI_CLOCK_C16_PAL;
	}
	else if (txt_match (&txt->txt, "PC-PIT", 1)) {
		val = PTI_CLOCK_PC_PIT;
	}
	else if (txt_match (&txt->txt, "PC-CPU", 1)) {
		val = PTI_CLOCK_PC_CPU;
	}
	else if (txt_match (&txt->txt, "SPECTRUM", 1)) {
		val = PTI_CLOCK_SPECTRUM;
	}
	else if (txt_match_uint (&txt->txt, 10, &val)) {
		;
	}
	else {
		return (1);
	}

	if (txt->img == NULL) {
		return (1);
	}

	pti_img_set_clock (txt->img, val);

	return (0);
}

static
int txt_load_flag (txt_load_t *txt)
{
	int val;

	val = 1;

	if (txt_match (&txt->txt, "+", 1)) {
		val = 1;
	}
	else if (txt_match (&txt->txt, "-", 1)) {
		val = 0;
	}

	if (txt_match (&txt->txt, "INVERT", 1)) {
		pti_img_set_inverted (txt->img, val);
		txt->invert = val;
	}
	else {
		return (1);
	}

	return (0);
}

static
int txt_load_pulse (txt_load_t *txt, int type)
{
	unsigned long val;

	if (!txt_match_uint (&txt->txt, 10, &val)) {
		return (1);
	}

	if (type == '/') {
		if (txt_add_pulse (txt, val / 2, -1)) {
			return (1);
		}

		if (txt_add_pulse (txt, val - val / 2, 1)) {
			return (1);
		}
	}
	else if (type == '\\') {
		if (txt_add_pulse (txt, val / 2, 1)) {
			return (1);
		}

		if (txt_add_pulse (txt, val - val / 2, -1)) {
			return (1);
		}
	}
	else if (type == '+') {
		if (txt_add_pulse (txt, val, 1)) {
			return (1);
		}
	}
	else if (type == '-') {
		if (txt_add_pulse (txt, val, -1)) {
			return (1);
		}
	}
	else if (type == '=') {
		if (txt_add_pulse (txt, val, 0)) {
			return (1);
		}
	}
	else {
		return (1);
	}

	return (0);
}

static
int txt_load_rep (txt_load_t *txt)
{
	unsigned long i, idx1, idx2, cnt;
	unsigned long clk;
	int           level;

	if (!txt_match_uint (&txt->txt, 10, &cnt)) {
		return (1);
	}

	if (!txt_match (&txt->txt, "{", 1)) {
		return (1);
	}

	idx1 = txt->img->pulse_cnt;

	if (txt_load_seq (txt)) {
		return (1);
	}

	idx2 = txt->img->pulse_cnt;

	if (!txt_match (&txt->txt, "}", 1)) {
		return (1);
	}

	if (cnt > 0) {
		cnt -= 1;
	}

	while (cnt-- > 0) {
		for (i = idx1; i < idx2; i++) {
			pti_pulse_get (txt->img->pulse[i], &clk, &level);

			if (pti_img_add_pulse (txt->img, clk, level)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int txt_load_text (txt_load_t *txt)
{
	unsigned n;
	char     str[256];

	if (txt_match_eol (&txt->txt)) {
		str[0] = 0;
	}
	else if (txt_match_string (&txt->txt, str, 256)) {
		;
	}
	else {
		return (1);
	}

	if (txt->img->comment_size > 0) {
		if (pti_img_add_comment (txt->img, "\x0a", 1)) {
			return (1);
		}
	}

	n = strlen (str);

	if (n > 0) {
		if (pti_img_add_comment (txt->img, str, n)) {
			return (1);
		}
	}

	return (0);
}

static
int txt_load_seq (txt_load_t *txt)
{
	int c;

	while (1) {
		txt_match_space (&txt->txt);

		c = txt_getc (&txt->txt, 0);

		if ((c == '+') || (c == '-') || (c == '=') || (c == '/') || (c == '\\')) {
			txt_skip (&txt->txt, 1);

			if (txt_load_pulse (txt, c)) {
				return (1);
			}
		}
		else if (txt_match (&txt->txt, "CLOCK", 1)) {
			if (txt_load_clock (txt)) {
				return (1);
			}
		}
		else if (txt_match (&txt->txt, "FLAG", 1)) {
			if (txt_load_flag (txt)) {
				return (1);
			}
		}
		else if (txt_match (&txt->txt, "INVERT", 1)) {
			txt->invert = !txt->invert;
		}
		else if (txt_match (&txt->txt, "REP", 1)) {
			if (txt_load_rep (txt)) {
				return (1);
			}
		}
		else if (txt_match (&txt->txt, "TEXT", 1)) {
			if (txt_load_text (txt)) {
				return (1);
			}
		}
		else {
			return (0);
		}
	}

	return (0);
}

static
int txt_load (txt_load_t *txt)
{
	int           ok;
	unsigned long val;

	ok = 0;

	while (1) {
		if (txt_match (&txt->txt, "PTI", 1) == 0) {
			break;
		}

		if (txt_match_uint (&txt->txt, 10, &val) == 0) {
			return (1);
		}

		if (val != 0) {
			fprintf (stderr, "pti: bad version (%lu)\n", val);
			return (1);
		}

		ok = 1;

		if (txt_load_seq (txt)) {
			return (1);
		}

		if (txt_match (&txt->txt, "END", 1)) {
			;
		}
	}

	if (ok == 0) {
		return (1);
	}

	if (feof (txt->txt.fp) == 0) {
		return (1);
	}

	return (0);
}

pti_img_t *pti_load_txt (FILE *fp)
{
	txt_load_t txt;

	txt_init (&txt.txt, fp);

	if ((txt.img = pti_img_new()) == NULL) {
		return (NULL);
	}

	txt.invert = 0;

	if (txt_load (&txt)) {
		txt_free (&txt.txt);
		pti_img_del (txt.img);
		return (NULL);
	}

	txt_free (&txt.txt);

	pti_img_clean (txt.img);

	return (txt.img);
}


int pti_txt_save_comment (FILE *fp, const pti_img_t *img)
{
	unsigned long       i, n;
	int                 start;
	const unsigned char *p;

	if (img->comment_size == 0) {
		return (0);
	}

	p = img->comment;
	n = img->comment_size;

	while ((n > 0) && ((*p == 0x0a) || (*p == 0x0d))) {
		p += 1;
		n -= 1;
	}

	while ((n > 0) && ((p[n - 1] == 0x0a) || (p[n - 1] == 0x0d))) {
		n -= 1;
	}

	start = 0;

	fputc ('\n', fp);

	for (i = 0; i < n; i++) {
		if (start == 0) {
			fputs ("TEXT \"", fp);
			start = 1;
		}

		if (p[i] == 0x0a) {
			if (start == 0) {
				continue;
			}

			fputs ("\"\n", fp);
			start = 0;
		}
		else if ((p[i] == '"') || (p[i] == '\\')) {
			fputc ('\\', fp);
			fputc (p[i], fp);
		}
		else {
			fputc (p[i], fp);
		}
	}

	if (start) {
		fputs ("\"\n", fp);
	}

	return (0);
}

int pti_save_txt (FILE *fp, const pti_img_t *img)
{
	unsigned      j;
	unsigned long i;
	int           inv;
	unsigned long clk;
	int           val;
	unsigned      col;
	char          buf[256];

	inv = pti_img_get_inverted (img);

	fprintf (fp, "PTI 0\n\n");
	fprintf (fp, "CLOCK %lu\n", pti_img_get_clock (img));

	if (inv) {
		fprintf (fp, "FLAG  INVERT\n");
	}

	if (pti_txt_save_comment (fp, img)) {
		return (1);
	}

	fputc ('\n', fp);

	col = 0;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &val);

		if (inv) {
			val = -val;
		}

		j = 0;

		do {
			buf[j++] = '0' + (clk % 10);
			clk = clk / 10;
		} while (clk > 0);

		if (val < 0) {
			buf[j++] = '-';
		}
		else if (val > 0) {
			buf[j++] = '+';
		}
		else {
			buf[j++] = '=';
		}

		while (j < 4) {
			buf[j++] = ' ';
		}

		if (((col == 7) && (val < 0)) || ((col > 0) && (val == 0))) {
			fputc ('\n', fp);
			col = 0;
		}
		else if (col > 0) {
			fputc (' ', fp);
		}

		while (j-- > 0) {
			fputc (buf[j], fp);
		}

		col += 1;

		if ((col >= 8) || (val == 0)) {
			fputc ('\n', fp);
			col = 0;
		}
	}

	if (col > 0) {
		fputc ('\n', fp);
	}

	return (0);
}


int pti_probe_txt_fp (FILE *fp)
{
	return (0);
}

int pti_probe_txt (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pti_probe_txt_fp (fp);

	fclose (fp);

	return (r);
}
