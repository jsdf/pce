/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-tap.c                                *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pti.h"
#include "pti-io.h"
#include "pti-img-tap.h"


static const char    tap_magic1[] = "C64-TAPE-RAW";
static const char    tap_magic2[] = "C16-TAPE-RAW";

static unsigned long tap_default_clock = 985248;


void pti_tap_set_default_clock (unsigned long val)
{
	if (val > 0) {
		tap_default_clock = val;
	}
}

static
int tap_load_img (FILE *fp, pti_img_t *img, unsigned long clock)
{
	int           c;
	int           level;
	unsigned      vers;
	unsigned long size;
	unsigned long clk;
	unsigned char buf[20];
	uint32_t      pbuf[2];

	if (clock == 0) {
		clock = tap_default_clock;
	}

	if (pti_read (fp, buf, 20)) {
		return (1);
	}

	if (memcmp (buf, tap_magic1, 12) != 0) {
		if (memcmp (buf, tap_magic2, 12) != 0) {
			return (1);
		}
	}

	vers = buf[12];

	if ((vers != 0) && (vers != 1) && (vers != 2)) {
		return (1);
	}

	pti_img_set_clock (img, clock);

	level = -1;

	size = pti_get_uint32_le (buf, 16);

	while (size > 0) {
		if ((c = fgetc (fp)) == EOF) {
			return (1);
		}

		size -= 1;

		if (c == 0) {
			if (vers == 0) {
				clk = 8 * 256;
			}
			else {
				if (size < 3) {
					return (1);
				}

				if (pti_read (fp, buf, 3)) {
					return (1);
				}

				size -= 3;

				clk = buf[2] & 0xff;
				clk = (clk << 8) | (buf[1] & 0xff);
				clk = (clk << 8) | (buf[0] & 0xff);
			}
		}
		else {
			clk = 8 * c;
		}

		if ((vers == 0) || (vers == 1)) {
			pti_pulse_set (pbuf + 0, clk / 2, -1);
			pti_pulse_set (pbuf + 1, clk - (clk / 2), 1);

			if (pti_img_add_pulses (img, pbuf, 2)) {
				return (1);
			}
		}
		else {
			pti_pulse_set (pbuf, clk, level);

			if (pti_img_add_pulses (img, pbuf, 1)) {
				return (1);
			}

			level = -level;
		}
	}

	return (0);
}

pti_img_t *pti_load_tap (FILE *fp, unsigned long clock)
{
	pti_img_t *img;

	if ((img = pti_img_new()) == NULL) {
		return (NULL);
	}

	if (tap_load_img (fp, img, clock)) {
		pti_img_del (img);
		return (NULL);
	}

	pti_img_clean (img);

	return (img);
}


static
int tap_save_data (FILE *fp, const pti_img_t *img, unsigned long start, unsigned long *size)
{
	unsigned long idx;
	int           val;
	unsigned long clk;
	unsigned long cnt;

	*size = 0;

	if (pti_set_pos (fp, start)) {
		return (1);
	}

	idx = 0;
	cnt = 0;

	while (idx < img->pulse_cnt) {
		while (idx < img->pulse_cnt) {
			pti_pulse_get (img->pulse[idx], &clk, &val);

			if (val >= 0) {
				break;
			}

			cnt += clk;
			idx += 1;
		}

		while (idx < img->pulse_cnt) {
			pti_pulse_get (img->pulse[idx], &clk, &val);

			if (val < 0) {
				break;
			}

			cnt += clk;
			idx += 1;
		}

		if (cnt == 0) {
			continue;
		}
		else if ((cnt / 8) < 256) {
			fputc (cnt / 8, fp);
			cnt &= 7;
			*size += 1;
		}
		else {
			fputc (0, fp);
			fputc (cnt & 0xff, fp);
			fputc ((cnt >> 8) & 0xff, fp);
			fputc ((cnt >> 16) & 0xff, fp);
			cnt = 0;
			*size += 4;
		}
	}

	return (0);
}

static
int tap_save_header (FILE *fp, unsigned long size)
{
	unsigned char buf[20];

	memcpy (buf, tap_magic1, 12);

	buf[12] = 1;
	buf[13] = 0;
	buf[14] = 0;
	buf[15] = 0;

	pti_set_uint32_le (buf, 16, size);

	if (pti_write_ofs (fp, 0, buf, 20)) {
		return (1);
	}

	return (0);
}

int pti_save_tap (FILE *fp, const pti_img_t *img)
{
	unsigned long size;

	if (tap_save_data (fp, img, 20, &size)) {
		return (1);
	}

	if (tap_save_header (fp, size)) {
		return (1);
	}

	return (0);
}


int pti_probe_tap_fp (FILE *fp)
{
	unsigned char buf[20];

	if (pti_read (fp, buf, 20)) {
		return (0);
	}

	if (memcmp (buf, tap_magic1, 12) != 0) {
		if (memcmp (buf, tap_magic2, 12) != 0) {
			return (0);
		}
	}

	return (1);
}

int pti_probe_tap (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pti_probe_tap_fp (fp);

	fclose (fp);

	return (r);
}
