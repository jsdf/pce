/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-img-wav.c                                *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2023 Hampa Hug <hampa@hampa.ch>                     *
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
#include <math.h>

#include "pti.h"
#include "pti-io.h"
#include "pti-img-wav.h"


#define WAV_RIFF 0x46464952
#define WAV_WAVE 0x45564157
#define WAV_FMT  0x20746d66
#define WAV_DATA 0x61746164

#define WAV_BUF_SIZE 256


typedef struct {
	double a[3];
	double b[3];
	double x[3];
	double y[3];
} wav_iir2_t;


typedef struct {
	FILE          *fp;
	pti_img_t     *img;

	unsigned long rsize;

	unsigned long srate;
	unsigned      channels;
	unsigned      bits;

	unsigned      bytes;
	unsigned      frame;

	unsigned      minvol;

	unsigned char buf[WAV_BUF_SIZE];
} wav_load_t;


typedef struct {
	FILE                  *fp;
	const pti_img_t       *img;
	const pti_wav_param_t *par;

	uint32_t              size;
	uint32_t              start_riff;
	uint32_t              start_fmt;
	uint32_t              start_data;
	uint32_t              start_pcm;

	int16_t               smp0;
	int16_t               smp1;

	unsigned long         rem;

	long                  lp0_smp;

	double                lp1_smp;
	double                lp1_a;
	double                lp1_b;

	unsigned              iir_cnt;
	wav_iir2_t            iir[8];
} wav_save_t;


static unsigned long wav_default_clock = 1000000;
static unsigned      wav_min_volume = 256;

static pti_wav_param_t wav_default = {
	.srate       = 48000,
	.bits        = 16,
	.channels    = 1,
	.lowpass     = 8000,
	.order       = 0,
};


void pti_wav_init (pti_wav_param_t *par)
{
	par->srate = 48000;
	par->bits = 16;
	par->channels = 1;
	par->lowpass = 8000;
	par->order = 0;
}

void pti_wav_set_default_clock (unsigned long val)
{
	if (val > 0) {
		wav_default_clock = val;
	}
}

void pti_wav_set_srate (unsigned long val)
{
	if (val > 0) {
		wav_default.srate = val;
	}
}

void pti_wav_set_bits (unsigned val)
{
	if (val <= 8) {
		wav_default.bits = 8;
	}
	else {
		wav_default.bits = 16;
	}
}

void pti_wav_set_lowpass (unsigned long val)
{
	wav_default.lowpass = val;
}

void pti_wav_set_lowpass_order (unsigned val)
{
	wav_default.order = val;
}

void pti_wav_set_min_volume (unsigned val)
{
	wav_min_volume = val;
}


static
int wav_read (wav_load_t *wav, unsigned char *buf, unsigned long size)
{
	if (size > wav->rsize) {
		return (1);
	}

	if (pti_read (wav->fp, buf, size)) {
		return (1);
	}

	wav->rsize -= size;

	return (0);
}

static
int wav_skip_chunk (wav_load_t *wav, unsigned long size)
{
	unsigned cnt;

	while (size > 0) {
		cnt = (size < WAV_BUF_SIZE) ? size : WAV_BUF_SIZE;

		if (wav_read (wav, wav->buf, cnt)) {
			return (1);
		}

		size -= cnt;
	}

	return (0);
}

static
int wav_load_fmt (wav_load_t *wav, unsigned long size)
{
	unsigned cnt, format;

	if (size < 16) {
		return (1);
	}

	cnt = (size < 64) ? size : 64;

	if (wav_read (wav, wav->buf, cnt)) {
		return (1);
	}

	format = pti_get_uint16_le (wav->buf, 0);

	if (format == 0xfffe) {
		if (cnt < 40) {
			return (1);
		}

		if (pti_get_uint16_le (wav->buf, 16) < 22) {
			return (1);
		}

		format = pti_get_uint16_le (wav->buf, 24);
	}

	if (format != 1) {
		fprintf (stderr, "wav: unsupported sample format (%u)", format);
		return (1);
	}

	wav->channels = pti_get_uint16_le (wav->buf, 2);
	wav->srate = pti_get_uint32_le (wav->buf, 4);
	wav->bits = pti_get_uint16_le (wav->buf, 14);

	if ((wav->channels == 0) || (wav->srate == 0) || (wav->bits == 0)) {
		return (1);
	}

	if (wav->bits > 16) {
		return (1);
	}

	wav->bytes = (wav->bits + 7) / 8;
	wav->frame = wav->channels * wav->bytes;

	if (wav_skip_chunk (wav, size - cnt)) {
		return (1);
	}

	return (0);
}

static
int wav_add_pulse (wav_load_t *wav, unsigned smp1, unsigned smp2, unsigned long *clk, unsigned long *rem, int level)
{
	unsigned      s1, s2;
	unsigned long tmp;

	s1 = ((smp1 & 0x8000) ? (~smp1 + 1) : smp1) & 0xffff;
	s2 = ((smp2 & 0x8000) ? (~smp2 + 1) : smp2) & 0xffff;

	if ((s1 + s2) == 0) {
		return (1);
	}

	tmp = ((unsigned long long) wav->img->clock * s1) / (s1 + s2);

	*rem += tmp;
	*clk += *rem / wav->srate;
	*rem %= wav->srate;

	if (pti_img_add_pulse (wav->img, *clk, level)) {
		return (1);
	}

	*rem += wav->img->clock - tmp;
	*clk = *rem / wav->srate;
	*rem %= wav->srate;

	return (0);
}

static
void wav_get_smp (wav_load_t *wav, const unsigned char *buf, unsigned *smp, int *level)
{
	if (wav->bits <= 8) {
		*smp = (buf[0] ^ 0x80) & 0xff;
		*smp = (*smp << 8) | *smp;
	}
	else {
		*smp = ((buf[1] & 0xff) << 8) | (buf[0] & 0xff);
	}

	*level = (*smp & 0x8000) ? -1 : 1;
}

static
int wav_load_data (wav_load_t *wav, unsigned long size)
{
	int           level1, level2;
	unsigned      smp1, smp2, abs, max;
	unsigned long clk, rem;

	if (wav->frame > WAV_BUF_SIZE) {
		return (1);
	}

	clk = 0;
	rem = 0;

	smp2 = 0;
	level2 = 0;
	max = 0;

	while (size >= wav->frame) {
		if (wav_read (wav, wav->buf, wav->frame)) {
			return (1);
		}

		size -= wav->frame;

		smp1 = smp2;
		level1 = level2;

		wav_get_smp (wav, wav->buf, &smp2, &level2);

		abs = ((smp2 & 0x8000) ? (~smp2 + 1) : smp2) & 0xffff;

		if (level1 == level2) {
			if (abs > max) {
				max = abs;
			}
		}
		else {
			if (max < wav->minvol) {
				level1 = 0;
			}

			max = abs;

			if (wav_add_pulse (wav, smp1, smp2, &clk, &rem, level1) == 0) {
				continue;
			}
		}

		rem += wav->img->clock;
		clk += rem / wav->srate;
		rem %= wav->srate;
	}

	if (clk > 0) {
		clk += (rem + wav->srate / 2) / wav->srate;

		if (pti_img_add_pulse (wav->img, clk, level2)) {
			return (1);
		}
	}

	if (wav_skip_chunk (wav, size)) {
		return (1);
	}

	return (0);
}

static
int wav_load_image (wav_load_t *wav)
{
	int           have_fmt;
	unsigned long type, size;

	if (pti_read (wav->fp, wav->buf, 12)) {
		return (1);
	}

	wav->rsize = pti_get_uint32_le (wav->buf, 4);

	if (wav->rsize < 4) {
		return (1);
	}

	wav->rsize -= 4;

	if (pti_get_uint32_le (wav->buf, 0) != WAV_RIFF) {
		return (1);
	}

	if (pti_get_uint32_le (wav->buf, 8) != WAV_WAVE) {
		return (1);
	}

	have_fmt = 0;

	while (wav->rsize >= 8) {
		if (wav_read (wav, wav->buf, 8)) {
			return (1);
		}

		type = pti_get_uint32_le (wav->buf, 0);
		size = pti_get_uint32_le (wav->buf, 4);

		if (size > wav->rsize) {
			return (1);
		}

		if (type == WAV_FMT) {
			if (have_fmt) {
				return (1);
			}

			if (wav_load_fmt (wav, size)) {
				return (1);
			}

			have_fmt = 1;
		}
		else if (type == WAV_DATA) {
			if (have_fmt == 0) {
				return (1);
			}

			if (wav_load_data (wav, size)) {
				return (1);
			}
		}
		else {
			if (wav_skip_chunk (wav, size)) {
				return (1);
			}
		}
	}

	return (0);
}

pti_img_t *pti_load_wav (FILE *fp, unsigned long clock)
{
	wav_load_t wav;

	if (clock == 0) {
		clock = wav_default_clock;
	}

	memset (&wav, 0, sizeof (wav_load_t));

	wav.fp = fp;
	wav.minvol = wav_min_volume;

	if ((wav.img = pti_img_new()) == NULL) {
		return (NULL);
	}

	pti_img_set_clock (wav.img, clock);

	if (wav_load_image (&wav)) {
		pti_img_del (wav.img);
		return (NULL);
	}

	pti_img_clean (wav.img);

	return (wav.img);
}

static
int wav_save_riff (wav_save_t *wav)
{
	unsigned char buf[12];

	pti_set_uint32_le (buf, 0, WAV_RIFF);
	pti_set_uint32_le (buf, 4, wav->start_pcm + wav->size - 8);
	pti_set_uint32_le (buf, 8, WAV_WAVE);

	if (pti_write_ofs (wav->fp, wav->start_riff, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int wav_save_fmt (wav_save_t *wav)
{
	unsigned      block;
	unsigned char buf[32];

	if (wav->par->bits > 16) {
		return (1);
	}

	if (wav->par->srate == 0) {
		return (1);
	}

	block = wav->par->channels * ((wav->par->bits + 7) / 8);

	pti_set_uint32_le (buf, 0, WAV_FMT);
	pti_set_uint32_le (buf, 4, 16);

	pti_set_uint16_le (buf, 8, 0x0001); /* pcm */
	pti_set_uint16_le (buf, 10, wav->par->channels);
	pti_set_uint32_le (buf, 12, wav->par->srate);
	pti_set_uint32_le (buf, 16, block * wav->par->srate);
	pti_set_uint16_le (buf, 20, block);
	pti_set_uint16_le (buf, 22, wav->par->bits);

	if (pti_write_ofs (wav->fp, wav->start_fmt, buf, 24)) {
		return (1);
	}

	return (0);
}

static
int wav_save_data (wav_save_t *wav)
{
	unsigned char buf[8];

	pti_set_uint32_le (buf, 0, WAV_DATA);
	pti_set_uint32_le (buf, 4, wav->size);

	if (pti_write_ofs (wav->fp, wav->start_data, buf, 8)) {
		return (1);
	}

	return (0);
}

static
int wav_save_smp (wav_save_t *wav, long smp)
{
	unsigned i;
	unsigned val;

	if (smp < -32767) {
		val = 0x8000;
	}
	else if (smp > 32767) {
		val = 0x7fff;
	}
	else {
		val = ((unsigned) smp) & 0xffff;
	}

	for (i = 0; i < wav->par->channels; i++) {
		if (wav->par->bits <= 8) {
			fputc (((val >> 8) ^ 0x80) & 0xff, wav->fp);
			wav->size += 1;
		}
		else {
			fputc (val & 0xff, wav->fp);
			fputc ((val >> 8) & 0xff, wav->fp);
			wav->size += 2;
		}
	}

	return (0);
}

static
int wav_save_pulse_0 (wav_save_t *wav, unsigned long clk, long smp)
{
	unsigned long cnt;
	unsigned long clock, srate;
	double        f;

	clock = wav->img->clock;
	srate = wav->par->srate;

	f = (double) wav->rem / (double) clock;

	wav->lp0_smp = f * wav->lp0_smp + (1.0 - f) * smp;

	while (clk > 0) {
		cnt = (clk < 256) ? clk : 256;
		clk -= cnt;

		wav->rem += cnt * srate;

		while (wav->rem >= clock) {
			wav->rem -= clock;

			if (wav_save_smp (wav, wav->lp0_smp)) {
				return (1);
			}

			wav->lp0_smp = smp;
		}
	}

	return (0);
}

static
int wav_save_pcm_0 (wav_save_t *wav)
{
	unsigned long   i;
	unsigned long   clk;
	long            smp;
	int             level;
	const pti_img_t *img;

	img = wav->img;

	wav->lp0_smp = 0;

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		smp = (level == 0) ? 0 : ((level < 0) ? wav->smp0 : wav->smp1);

		if (wav_save_pulse_0 (wav, clk, smp)) {
			return (1);
		}
	}

	return (0);
}

static
void wav_setup_1 (wav_save_t *wav)
{
	double om;

	wav->lp1_smp = 0.0;

	if (wav->par->lowpass == 0) {
		wav->lp1_a = 1.0;
		wav->lp1_b = 0.0;
		return;
	}

	om = 2.0 * M_PI * (double) wav->par->lowpass / (double) wav->img->clock;

	wav->lp1_a = om / (om + 1.0);
	wav->lp1_b = 1.0 / (om + 1.0);
}

static
int wav_save_pulse_1 (wav_save_t *wav, unsigned long clk, long smp)
{
	unsigned long clock, srate;
	double        f1, f2;

	clock = wav->img->clock;
	srate = wav->par->srate;

	f1 = wav->lp1_b;
	f2 = wav->lp1_a * smp;

	while (clk--) {
		wav->lp1_smp = f1 * wav->lp1_smp + f2;

		wav->rem += srate;

		while (wav->rem >= clock) {
			wav->rem -= clock;

			if (wav_save_smp (wav, (long) wav->lp1_smp)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int wav_save_pcm_1 (wav_save_t *wav)
{
	unsigned long   i;
	unsigned long   clk;
	long            smp;
	int             level;
	const pti_img_t *img;

	img = wav->img;

	wav_setup_1 (wav);

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		smp = (level == 0) ? 0 : ((level < 0) ? wav->smp0 : wav->smp1);

		if (wav_save_pulse_1 (wav, clk, smp)) {
			return (1);
		}
	}

	return (0);
}

static
void wav_iir2_butterworth (wav_iir2_t *iir, unsigned k, unsigned N, double om)
{
	unsigned i;
	double   b0, c, f;

	c = 1.0 / tan (M_PI * om);
	f = 2.0 * c * cos (M_PI * (2.0 * k + 1.0) / (2.0 * N));
	b0 = c * c + f + 1.0;

	iir->a[0] = 1.0 / b0;
	iir->a[1] = 2.0 / b0;
	iir->a[2] = 1.0 / b0;

	iir->b[0] = 1.0;
	iir->b[1] = 2.0 * (c * c - 1.0) / b0;
	iir->b[2] = (f - c * c - 1.0) / b0;

	for (i = 0; i < 3; i++) {
		iir->x[i] = 0.0;
		iir->y[i] = 0.0;
	}
}

static
void wav_iir_butterworth (wav_iir2_t *iir, unsigned n, unsigned long cut, unsigned long srate)
{
	unsigned i;
	double   om;

	om = (double) cut / (double) srate;

	for (i = 0; i < n; i++) {
		wav_iir2_butterworth (iir + i, i, 2 * n, om);
	}
}

static
void wav_setup_n (wav_save_t *wav)
{
	unsigned cnt;

	if (wav->par->lowpass == 0) {
		wav->iir_cnt = 0;
		return;
	}

	cnt = (wav->par->order + 1) / 2;

	if (cnt < 1) {
		cnt = 1;
	}
	else if (cnt > 8) {
		cnt = 8;
	}

	wav_iir_butterworth (wav->iir, cnt, wav->par->lowpass, wav->img->clock);

	wav->iir_cnt = cnt;
}

static inline
double wav_iir2 (wav_iir2_t *iir, double x)
{
	iir->x[2] = iir->x[1];
	iir->x[1] = iir->x[0];
	iir->x[0] = x;

	iir->y[2] = iir->y[1];
	iir->y[1] = iir->y[0];

	iir->y[0] = iir->a[0] * iir->x[0];
	iir->y[0] += iir->a[1] * iir->x[1] + iir->a[2] * iir->x[2];
	iir->y[0] += iir->b[1] * iir->y[1] + iir->b[2] * iir->y[2];

	return (iir->y[0]);
}

static
double wav_iir (wav_iir2_t *iir, unsigned n, double x)
{
	unsigned i;

	for (i = 0; i < n; i++) {
		x = wav_iir2 (iir + i, x);
	}

	return (x);
}

static
int wav_save_pulse_n (wav_save_t *wav, unsigned long clk, long smp)
{
	unsigned long clock, srate;
	double        smpf;
	long          val;

	clock = wav->img->clock;
	srate = wav->par->srate;

	smpf = (double) smp;

	while (clk--) {
		val = (long) wav_iir (wav->iir, wav->iir_cnt, smpf);

		wav->rem += srate;

		while (wav->rem >= clock) {
			wav->rem -= clock;

			if (wav_save_smp (wav, val)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int wav_save_pcm_n (wav_save_t *wav)
{
	unsigned long   i;
	unsigned long   clk;
	long            smp;
	int             level;
	const pti_img_t *img;

	img = wav->img;

	wav_setup_n (wav);

	for (i = 0; i < img->pulse_cnt; i++) {
		pti_pulse_get (img->pulse[i], &clk, &level);

		smp = (level == 0) ? 0 : ((level < 0) ? wav->smp0 : wav->smp1);

		if (wav_save_pulse_n (wav, clk, smp)) {
			return (1);
		}
	}

	return (0);
}

static
int wav_save_pcm (wav_save_t *wav)
{
	if (pti_set_pos (wav->fp, wav->start_pcm)) {
		return (1);
	}

	wav->size = 0;
	wav->rem = 0;

	if (wav->par->order == 0) {
		if (wav_save_pcm_0 (wav)) {
			return (1);
		}
	}
	else if (wav->par->order == 1) {
		if (wav_save_pcm_1 (wav)) {
			return (1);
		}
	}
	else {
		if (wav_save_pcm_n (wav)) {
			return (1);
		}
	}

	if (wav->rem >= (wav->img->clock / 2)) {
		if (wav_save_smp (wav, 0)) {
			return (1);
		}
	}

	return (0);
}

int pti_save_wav (FILE *fp, const pti_img_t *img, const pti_wav_param_t *par)
{
	wav_save_t wav;

	if (par == NULL) {
		par = &wav_default;
	}

	wav.fp = fp;
	wav.img = img;
	wav.par = par;

	wav.start_riff = 0;
	wav.start_fmt = wav.start_riff + 12;
	wav.start_data = wav.start_fmt + 24;
	wav.start_pcm = wav.start_data + 8;

	wav.smp0 = -16384;
	wav.smp1 = 16384;

	wav.iir_cnt = 0;

	if (wav_save_pcm (&wav)) {
		return (1);
	}

	if (wav_save_data (&wav)) {
		return (1);
	}

	if (wav_save_fmt (&wav)) {
		return (1);
	}

	if (wav_save_riff (&wav)) {
		return (1);
	}

	return (0);
}


int pti_probe_wav_fp (FILE *fp)
{
	unsigned char buf[12];

	if (pti_read (fp, buf, 12)) {
		return (0);
	}

	if (pti_get_uint32_le (buf, 0) != WAV_RIFF) {
		return (0);
	}

	if (pti_get_uint32_le (buf, 8) != WAV_WAVE) {
		return (0);
	}

	return (1);
}

int pti_probe_wav (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pti_probe_wav_fp (fp);

	fclose (fp);

	return (r);
}
