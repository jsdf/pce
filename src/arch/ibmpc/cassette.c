/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/cassette.c                                    *
 * Created:     2008-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2010 Hampa Hug <hampa@hampa.ch>                     *
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
#include "cassette.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lib/console.h>


#define CAS_CLK     1193182
#define CAS_SRATE   44100
#define CAS_IIR_MUL 16384


static void pc_cas_reset (pc_cassette_t *cas);


void pc_cas_init (pc_cassette_t *cas)
{
	cas->save = 0;
	cas->pcm = 0;
	cas->filter = 0;

	cas->motor = 0;

	cas->position = 0;

	cas->position_save = 0;
	cas->position_load = 0;

	cas->data_out = 0;
	cas->data_inp = 0;

	cas->pcm_out_vol = 64;
	cas->pcm_out_val = 0;

	cas->cas_out_cnt = 0;
	cas->cas_out_buf = 0;

	cas->cas_inp_cnt = 0;
	cas->cas_inp_buf = 0;
	cas->cas_inp_bit = 0;

	cas->clk = 0;

	cas->clk_pcm = 0;

	cas->clk_out = 0;
	cas->clk_inp = 0;

	cas->close = 0;
	cas->fname = NULL;
	cas->fp = NULL;

	pc_cas_reset (cas);
}

void pc_cas_free (pc_cassette_t *cas)
{
	free (cas->fname);

	if (cas->close) {
		fclose (cas->fp);
	}
}

pc_cassette_t *pc_cas_new (void)
{
	pc_cassette_t *cas;

	cas = malloc (sizeof (pc_cassette_t));

	if (cas == NULL) {
		return (NULL);
	}

	pc_cas_init (cas);

	return (cas);
}

void pc_cas_del (pc_cassette_t *cas)
{
	if (cas != NULL) {
		pc_cas_free (cas);
		free (cas);
	}
}

/*
 * Second order butterworth low-pass filter
 */
static
void pc_cas_iir2_set_lowpass (pc_cas_iir2_t *iir, unsigned long freq, unsigned long srate)
{
	double om, b0;

	if ((2 * freq) >= srate) {
		freq = (srate / 2) - 1;
	}

	om = 1.0 / tan ((3.14159265358979312 * freq) / srate);
	b0 = om * om + om * sqrt(2.0) + 1.0;

	iir->a[0] = (long) (CAS_IIR_MUL * 1.0 / b0);
	iir->a[1] = (long) (CAS_IIR_MUL * 2.0 / b0);
	iir->a[2] = (long) (CAS_IIR_MUL * 1.0 / b0);

	iir->b[0] = (long) (CAS_IIR_MUL * 1.0);
	iir->b[1] = (long) (CAS_IIR_MUL * 2.0 * (1.0 - om * om) / b0);
	iir->b[2] = (long) (CAS_IIR_MUL * (om * om - om * sqrt(2.0) + 1.0) / b0);

	iir->x[0] = 0;
	iir->x[1] = 0;
	iir->x[2] = 0;

	iir->y[0] = 0;
	iir->y[1] = 0;
	iir->y[2] = 0;
}

static
int pc_cas_iir2 (pc_cas_iir2_t *iir, int x)
{
	iir->x[2] = iir->x[1];
	iir->x[1] = iir->x[0];
	iir->x[0] = 32 * (long) x;

	iir->y[2] = iir->y[1];
	iir->y[1] = iir->y[0];

	iir->y[0] = iir->a[0] * iir->x[0];
	iir->y[0] += iir->a[1] * iir->x[1] + iir->a[2] * iir->x[2];
	iir->y[0] -= iir->b[1] * iir->y[1] + iir->b[2] * iir->y[2];

	iir->y[0] = iir->y[0] / CAS_IIR_MUL;

	return ((int) (iir->y[0] / 32));
}

int pc_cas_set_fname (pc_cassette_t *cas, const char *fname)
{
	unsigned   n;
	const char *ext;

	if (cas->close) {
		fclose (cas->fp);
	}

	cas->close = 0;
	cas->fp = NULL;

	free (cas->fname);
	cas->fname = NULL;

	cas->position = 0;

	cas->position_save = 0;
	cas->position_load = 0;

	if (fname == NULL) {
		return (0);
	}

	cas->fp = fopen (fname, "r+b");

	if (cas->fp == NULL) {
		cas->fp = fopen (fname, "w+b");
	}

	if (cas->fp == NULL) {
		return (1);
	}

	cas->close = 1;

	pc_cas_append (cas);

	cas->position_save = cas->position;

	if (cas->save == 0) {
		pc_cas_set_position (cas, 0);
	}

	n = strlen (fname);

	cas->fname = malloc (n + 1);

	if (cas->fname != NULL) {
		memcpy (cas->fname, fname, n + 1);
	}

	if (n > 4) {
		ext = fname + (n - 4);

		if (strcmp (ext, ".pcm") == 0) {
			pc_cas_set_pcm (cas, 1);
		}
		else if (strcmp (ext, ".raw") == 0) {
			pc_cas_set_pcm (cas, 1);
		}
		else if (strcmp (ext, ".cas") == 0) {
			pc_cas_set_pcm (cas, 0);
		}
	}

	return (0);
}

static
void pc_cas_reset (pc_cassette_t *cas)
{
	cas->clk_pcm = 0;

	cas->clk_out = cas->clk;
	cas->clk_inp = 0;

	cas->pcm_out_val = 0;

	cas->cas_out_cnt = 0;
	cas->cas_out_buf = 0;

	cas->cas_inp_cnt = 0;
	cas->cas_inp_buf = 0;
	cas->cas_inp_bit = 0;

	pc_cas_iir2_set_lowpass (&cas->pcm_out_iir, 3000, CAS_SRATE);
	pc_cas_iir2_set_lowpass (&cas->pcm_inp_iir, 3000, CAS_SRATE);
}

int pc_cas_get_mode (pc_cassette_t *cas)
{
	return (cas->save);
}

void pc_cas_set_mode (pc_cassette_t *cas, int save)
{
	save = (save != 0);

	if (cas->save == save) {
		return;
	}

	if (cas->save) {
		cas->position_save = cas->position;
		cas->position = cas->position_load;
	}
	else {
		cas->position_load = cas->position;
		cas->position = cas->position_save;
	}

	cas->save = save;

	if (cas->fp != NULL) {
		fflush (cas->fp);

		pc_cas_set_position (cas, cas->position);
	}

	pc_cas_reset (cas);
}

int pc_cas_get_pcm (pc_cassette_t *cas)
{
	return (cas->pcm);
}

void pc_cas_set_pcm (pc_cassette_t *cas, int pcm)
{
	cas->pcm = (pcm != 0);

	pc_cas_reset (cas);
}

int pc_cas_get_filter (pc_cassette_t *cas)
{
	return (cas->filter);
}

void pc_cas_set_filter (pc_cassette_t *cas, int filter)
{
	cas->filter = (filter != 0);

	if (cas->filter) {
		cas->pcm_out_vol = 128;
	}
	else {
		cas->pcm_out_vol = 64;
	}

	pc_cas_reset (cas);
}

void pc_cas_rewind (pc_cassette_t *cas)
{
	if (cas->fp != NULL) {
		rewind (cas->fp);
		cas->position = 0;
	}

	pc_cas_reset (cas);
}

void pc_cas_append (pc_cassette_t *cas)
{
	if (cas->fp != NULL) {
		fseek (cas->fp, 0, SEEK_END);
		cas->position = ftell (cas->fp);
	}

	pc_cas_reset (cas);
}

unsigned long pc_cas_get_position (pc_cassette_t *cas)
{
	return (cas->position);
}

int pc_cas_set_position (pc_cassette_t *cas, unsigned long pos)
{
	if (cas->fp == NULL) {
		return (1);
	}

	if (fseek (cas->fp, pos, SEEK_SET) != 0) {
		return (1);
	}

	cas->position = pos;

	pc_cas_reset (cas);

	return (0);
}

void pc_cas_set_motor (pc_cassette_t *cas, unsigned char val)
{
	val = (val != 0);

	if (val == cas->motor) {
		return;
	}

	pce_printf ("cassette motor %s at %lu (%s)\n",
		val ? "on " : "off",
		cas->position,
		(cas->fname != NULL) ? cas->fname : "<none>"
	);

	cas->motor = val;

	if (cas->fp != NULL) {
		fflush (cas->fp);

		pc_cas_set_position (cas, cas->position);
	}

	pc_cas_reset (cas);
}

static
void pc_cas_read_bit (pc_cassette_t *cas)
{
	int val;

	if (cas->cas_inp_cnt == 0) {
		if (cas->fp == NULL) {
			return;
		}

		if (feof (cas->fp)) {
			return;
		}

		val = fgetc (cas->fp);

		if (val == EOF) {
			pce_printf ("cassette EOF at %lu\n",
				cas->position
			);
			return;
		}

		cas->position += 1;

		cas->cas_inp_cnt = 8;
		cas->cas_inp_buf = val;
	}

	cas->cas_inp_bit = ((cas->cas_inp_buf & 0x80) != 0);

	cas->cas_inp_buf = (cas->cas_inp_buf << 1) & 0xff;
	cas->cas_inp_cnt -= 1;
}

static
int pc_cas_read_smp (pc_cassette_t *cas)
{
	int smp, val;

	if (feof (cas->fp)) {
		return (0);
	}

	smp = fgetc (cas->fp);

	if (smp == EOF) {
		pce_printf ("cassette EOF at %lu\n",
			cas->position
		);
		return (0);
	}

	cas->position += 1;

	val = (smp & 0x80) ? (smp - 256) : smp;

	val = pc_cas_iir2 (&cas->pcm_inp_iir, val);

	return (val);
}

static
void pc_cas_write_bit (pc_cassette_t *cas, unsigned char val)
{
	if (val) {
		cas->cas_out_buf |= (0x80 >> cas->cas_out_cnt);
	}

	cas->cas_out_cnt += 1;

	if (cas->cas_out_cnt >= 8) {
		if (cas->fp != NULL) {
			fputc (cas->cas_out_buf, cas->fp);
			cas->position += 1;
		}

		cas->cas_out_buf = 0;
		cas->cas_out_cnt = 0;
	}
}

static
void pc_cas_write_smp (pc_cassette_t *cas, int val)
{
	unsigned char smp;

	if (cas->filter) {
		val = pc_cas_iir2 (&cas->pcm_out_iir, val);
	}

	if (val < 0) {
		smp = (val < -127) ? 0x80 : (val + 256);
	}
	else {
		smp = (val > 127) ? 0x7f : val;
	}

	fputc (smp, cas->fp);

	cas->position += 1;
}

unsigned char pc_cas_get_inp (pc_cassette_t *cas)
{
	return (cas->data_inp);
}

void pc_cas_set_out (pc_cassette_t *cas, unsigned char val)
{
	unsigned long clk;

	val = (val != 0);

	if (cas->motor == 0) {
		cas->data_inp = val;
		return;
	}

	if (cas->data_out == val) {
		return;
	}

	cas->data_out = val;

	if (cas->pcm) {
		cas->pcm_out_val = val ? -cas->pcm_out_vol : cas->pcm_out_vol;
		return;
	}

	if (cas->save == 0) {
		return;
	}

	if (val == 0) {
		return;
	}

	clk = cas->clk - cas->clk_out;
	cas->clk_out = cas->clk;

	if (clk < (CAS_CLK / 4000)) {
		;
	}
	else if (clk < ((3 * CAS_CLK) / 4000)) {
		pc_cas_write_bit (cas, 0);
	}
	else if (clk < ((5 * CAS_CLK) / 4000)) {
		pc_cas_write_bit (cas, 1);
	}
}

void pc_cas_print_state (pc_cassette_t *cas)
{
	const char *mode;

	if (cas->pcm) {
		mode = cas->filter ? "pcm/f" : "pcm/r";
	}
	else {
		mode = "cas";
	}

	pce_printf ("cassette %s %s at %lu (%s)\n",
		cas->save ? "save" : "load",
		mode,
		cas->position,
		(cas->fname != NULL) ? cas->fname : "<none>"
	);
}

static
void pc_cas_clock_pcm (pc_cassette_t *cas, unsigned long cnt)
{
	unsigned long i, n;
	int           v;

	n = CAS_SRATE * cnt + cas->clk_pcm;

	cas->clk_pcm = n % CAS_CLK;

	n = n / CAS_CLK;

	if (n == 0) {
		return;
	}

	if (cas->save) {
		for (i = 0; i < n; i++) {
			pc_cas_write_smp (cas, cas->pcm_out_val);

			if (cas->filter) {
				cas->pcm_out_val = (15 * cas->pcm_out_val) / 16;
			}
		}
	}
	else {
		for (i = 0; i < n; i++) {
			v = pc_cas_read_smp (cas);
		}

		cas->data_inp = (v < 0) ? 0 : 1;
	}
}

void pc_cas_clock (pc_cassette_t *cas, unsigned long cnt)
{
	cas->clk += cnt;

	if (cas->motor == 0) {
		return;
	}

	if (cas->pcm) {
		pc_cas_clock_pcm (cas, cnt);
		return;
	}

	if (cas->save) {
		return;
	}

	if (cas->clk_inp > cnt) {
		cas->clk_inp -= cnt;
		return;
	}

	cnt -= cas->clk_inp;

	cas->data_inp = !cas->data_inp;

	if (cas->data_inp) {
		pc_cas_read_bit (cas);
	}

	if (cas->cas_inp_bit) {
		cas->clk_inp = CAS_CLK / 2000;
	}
	else {
		cas->clk_inp = CAS_CLK / 4000;
	}

	if (cas->clk_inp > cnt) {
		cas->clk_inp -= cnt;
	}
}
