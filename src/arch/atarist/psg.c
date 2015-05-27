/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/psg.c                                       *
 * Created:     2013-06-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include "psg.h"

#include <drivers/sound/sound.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef DEBUG_PSG
#define DEBUG_PSG 0
#endif

#define PSG_FREQ_INP 8000000


struct psg_env_s {
	unsigned char val;
	unsigned char inc;
};


#include "psg_vol.h"

static struct psg_env_s envtab[16][3] = {
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0xff, 0xff } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0xff, 0xff } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0xff, 0xff } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0xff, 0xff } },

	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x01 } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x01 } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x01 } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x01 } },

	{ { 0xff, 0xff }, { 0xff, 0xff }, { 0xff, 0xff } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0xff, 0xff } },
	{ { 0x00, 0x01 }, { 0xff, 0xff }, { 0xff, 0xff } },
	{ { 0xff, 0x00 }, { 0xff, 0x00 }, { 0xff, 0xff } },

	{ { 0x00, 0x01 }, { 0x00, 0x01 }, { 0x00, 0x01 } },
	{ { 0xff, 0x00 }, { 0xff, 0x00 }, { 0x00, 0x01 } },
	{ { 0xff, 0xff }, { 0x00, 0x01 }, { 0x00, 0x01 } },
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x01 } },
};


void st_psg_init (st_psg_t *psg)
{
	unsigned i;

	for (i = 0; i < 16; i++) {
		psg->reg[i] = 0;
	}

	psg->clock = 0;
	psg->clock_div = 0;

	psg->srate = 44100;

	psg->silence_cnt = 0;
	psg->silence_max = 2 * psg->srate;

	psg->speaker_on = 0;

	for (i = 0; i < 3; i++) {
		psg->tone_per[i] = 1;
		psg->tone_cnt[i] = 1;
		psg->tone_val[i] = 0;
	}

	psg->noise_per = 1;
	psg->noise_cnt = 1;
	psg->noise_val = 1;

	psg->env_per = 1;
	psg->env_cnt = 1;
	psg->env_per2 = 32;
	psg->env_cnt2 = 32;
	psg->env_val = 0;
	psg->env_inc = 0;
	psg->env_idx = 0;

	psg->out_cnt = 0;
	psg->out_freq = psg->env_per2 * psg->srate;
	psg->inp_freq = PSG_FREQ_INP;

	psg->filter_hp = 0;
	psg->filter_hp_x0 = 0;
	psg->filter_hp_y0 = 0;

	psg->lowpass_freq = 0;

	psg->buf_cnt = 0;
	psg->last_smp = 0x8000;

	psg->aym = NULL;
	psg->aym_reg = 0;
	psg->aym_res = 250;
	psg->aym_clk = 0;

	psg->drv = NULL;

	psg->seta_ext = NULL;
	psg->seta_fct = NULL;

	psg->setb_ext = NULL;
	psg->setb_fct = NULL;
}

void st_psg_free (st_psg_t *psg)
{
	st_psg_set_aym (psg, NULL);

	if (psg->drv != NULL) {
		snd_close (psg->drv);
	}
}

void st_psg_reset (st_psg_t *psg)
{
	unsigned i;

	for (i = 0; i < 16; i++) {
		st_psg_set_select (psg, i);
		st_psg_set_data (psg, 0);
	}

	for (i = 0; i < 3; i++) {
		psg->tone_per[i] = 1;
		psg->tone_cnt[i] = 1;
		psg->tone_val[i] = 0;
	}

	psg->noise_per = 1;
	psg->noise_cnt = psg->noise_per;
	psg->noise_val = 1;

	psg->env_per = 1;
	psg->env_cnt = psg->env_per;
	psg->env_cnt2 = psg->env_per2;
	psg->env_val = 0;
	psg->env_inc = 0;
	psg->env_idx = 0;
}

int st_psg_set_driver (st_psg_t *psg, const char *drv)
{
	if (psg->drv != NULL) {
		snd_close (psg->drv);
		psg->drv = NULL;
	}

	if ((drv == NULL) || (*drv == 0)) {
		return (0);
	}

	psg->drv = snd_open (drv);

	if (psg->drv == NULL) {
		return (1);
	}

	if (snd_set_params (psg->drv, 1, psg->srate, 0)) {
		snd_close (psg->drv);
		psg->drv = NULL;
		return (1);
	}

	return (0);
}

sound_drv_t *st_psg_get_driver (st_psg_t *psg)
{
	return (psg->drv);
}

int st_psg_set_srate (st_psg_t *psg, unsigned long srate)
{
	if (psg->drv != NULL) {
		if (snd_set_params (psg->drv, 1, srate, 0)) {
			return (1);
		}
	}

	psg->srate = srate;
	psg->silence_max = 2 * srate;
	psg->out_freq = psg->env_per2 * srate;

	return (0);
}

int st_psg_set_aym (st_psg_t *psg, const char *fname)
{
	const char *str;

	psg->aym_reg = 0;
	psg->aym_clk = psg->clock;

	if (psg->aym != NULL) {
		fputc (0xff, psg->aym);
		fputc (0xff, psg->aym);
		fclose (psg->aym);
		psg->aym = NULL;
	}

	if ((fname == NULL) || (*fname == 0)) {
		return (0);
	}

	if ((psg->aym = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	if (fwrite ("AYM\x1a\0\0\0\0", 1, 8, psg->aym) != 8) {
		return (1);
	}

	str = "pce-atarist " PCE_VERSION_STR " (" __DATE__ " " __TIME__ ")";

	fputc (0x80, psg->aym);
	fputc (strlen (str), psg->aym);
	fputs (str, psg->aym);

	fflush (psg->aym);

	return (0);
}

void st_psg_set_aym_resolution (st_psg_t *psg, unsigned long res)
{
	psg->aym_res = res;
}

void st_psg_set_lowpass (st_psg_t *psg, unsigned freq)
{
	psg->lowpass_freq = freq;

	if (freq > 0) {
		snd_iir2_set_lowpass (&psg->iir, freq, psg->srate);
	}
}

void st_psg_set_highpass (st_psg_t *psg, int enable)
{
	psg->filter_hp = (enable != 0);
}

void st_psg_set_port_a_fct (st_psg_t *psg, void *ext, void *fct)
{
	psg->seta_ext = ext;
	psg->seta_fct = fct;
}

void st_psg_set_port_b_fct (st_psg_t *psg, void *ext, void *fct)
{
	psg->setb_ext = ext;
	psg->setb_fct = fct;
}


static
void psg_write_buffer (st_psg_t *psg)
{
	if (psg->buf_cnt == 0) {
		return;
	}

	if (psg->speaker_on == 0) {
		psg->speaker_on = 1;
#if DEBUG_PSG >= 1
		fprintf (stderr, "PSG: speaker on\n");
#endif
	}

	if (psg->lowpass_freq > 0) {
		snd_iir2_filter (&psg->iir, psg->buf, psg->buf, psg->buf_cnt, 1, 0);
	}

	if (psg->drv != NULL) {
		snd_write (psg->drv, psg->buf, psg->buf_cnt);
	}

	psg->buf_cnt = 0;
}

/*
 * Highpass filter (15 HZ at 44100)
 */
static
uint16_t psg_filter_hp (st_psg_t *psg, uint16_t v)
{
	long x0, x1, y0;

	x1 = psg->filter_hp_x0;
	x0 = (long) v - 32768;
	y0 = psg->filter_hp_y0;

	y0 = 32733 * (x0 - x1) + 32698 * y0;

	if (y0 < 0) {
		y0 = -(-y0 / 32768);
	}
	else {
		y0 = y0 / 32768;
	}

	psg->filter_hp_x0 = x0;
	psg->filter_hp_y0 = y0;

	return (y0 + 32768);
}

static
void psg_add_sample (st_psg_t *psg)
{
	unsigned      i;
	unsigned char tone, noise;
	unsigned      vol, v[3];
	uint16_t      smp;

	tone = psg->reg[7];
	noise = psg->reg[7] >> 3;

	for (i = 0; i < 3; i++) {
		vol = (tone | psg->tone_val[i]) & (noise | psg->noise_val);
		vol = -(vol & 1);

		if (psg->reg[8 + i] & 0x10) {
			vol &= psg->env_val >> 1;
		}
		else {
			vol &= psg->reg[8 + i];
		}

		v[i] = vol & 0x0f;

		if (v[i] > 0) {
			if (((tone & 1) == 0) && (psg->tone_per[i] < 5)) {
				v[i] -= 1;
			}
		}

		tone >>= 1;
		noise >>= 1;
	}

	smp = 0x8000 + (voltab16[v[0]][v[1]][v[2]] / 4);

	if (psg->last_smp == smp) {
		if (--psg->silence_cnt == 0) {
			psg_write_buffer (psg);
			psg->speaker_on = 0;
#if DEBUG_PSG >= 1
			fprintf (stderr, "PSG: speaker off (%u)\n", psg->last_smp);
#endif
			return;
		}
	}
	else {
		psg->silence_cnt = psg->silence_max;
		psg->last_smp = smp;
	}

	if (psg->filter_hp) {
		smp = psg_filter_hp (psg, smp);
	}

	psg->buf[psg->buf_cnt++] = smp;
}

static
void psg_env_clock (st_psg_t *psg)
{
	struct psg_env_s *tab;

	if (--psg->env_cnt2 == 0) {
		psg->env_cnt2 = psg->env_per2;
		psg->env_idx += 1;

		tab = envtab[psg->reg[13] & 0x0f] + (psg->env_idx & 1);

		psg->env_val = tab->val;
		psg->env_inc = tab->inc;
	}
	else {
		psg->env_val += psg->env_inc;
	}
}


static
void psg_set_tone_period (st_psg_t *psg, unsigned chn, unsigned char v1, unsigned char v2)
{
	unsigned idx, val;

	idx = 2 * chn;

	if ((psg->reg[idx] == v1) && (psg->reg[idx + 1] == v2)) {
		return;
	}

	psg->reg[idx + 0] = v1;
	psg->reg[idx + 1] = v2;

	val = (((unsigned) v2 & 0x0f) << 8) | (v1 & 0xff);

	if (val == 0) {
		val = 1;
	}

#if DEBUG_PSG >= 3
	if (psg->tone_per[chn] != val) {
		fprintf (stderr, "PSG: chn %u: period = %u\n", chn, val);
	}
#endif

	psg->tone_per[chn] = val;
}

static
void psg_set_noise_period (st_psg_t *psg, unsigned char val)
{
	psg->reg[6] = val;

	val &= 0x1f;

	if (val == 0) {
		val = 1;
	}

#if DEBUG_PSG >= 3
	if (psg->noise_per != val) {
		fprintf (stderr, "PSG: noise period = %u\n", val);
	}
#endif

	psg->noise_per = val;
}

static
void psg_set_mixer (st_psg_t *psg, unsigned char val)
{
	if ((psg->reg[7] ^ val) & ~val & 0x3f) {
		psg->silence_cnt = psg->silence_max;
	}

#if DEBUG_PSG >= 2
	if ((psg->reg[7] ^ val) & 0x3f) {
		fprintf (stderr, "PSG: enable T=%c%c%c N=%c%c%c\n",
			(val & 1) ? 'a' : 'A',
			(val & 2) ? 'b' : 'B',
			(val & 4) ? 'c' : 'C',
			(val & 8) ? 'a' : 'A',
			(val & 16) ? 'b' : 'B',
			(val & 32) ? 'c' : 'C'
		);
	}
#endif

	psg->reg[7] = val;
}

static
void psg_set_level (st_psg_t *psg, unsigned chn, unsigned char val)
{
	if (chn > 2) {
		return;
	}

	if (psg->reg[8 + chn] != val) {
		psg->silence_cnt = psg->silence_max;
	}

	psg->reg[8 + chn] = val;
}

static
void psg_set_env_period (st_psg_t *psg, unsigned char v1, unsigned char v2)
{
	unsigned val;

	psg->reg[11] = v1;
	psg->reg[12] = v2;

	val = ((unsigned) (v2 & 0xff) << 8) | (v1 & 0xff);

	if (val == 0) {
		val = 1;
	}

#if DEBUG_PSG >= 3
	if (psg->env_per != val) {
		fprintf (stderr, "PSG: envelope period = %u\n", val);
	}
#endif

	psg->env_per = val;
	psg->env_cnt = val;
}

static
void psg_set_env_shape (st_psg_t *psg, unsigned char val)
{
	struct psg_env_s *tab;

	psg->silence_cnt = psg->silence_max;

	tab = envtab[val & 0x0f] + 2;

	psg->env_val = tab->val;
	psg->env_inc = tab->inc;
	psg->env_idx = 1;

	psg->env_cnt = psg->env_per;
	psg->env_cnt2 = psg->env_per2;

#if DEBUG_PSG >= 3
	fprintf (stderr, "PSG: envelope: S=%02X v=%u i=%u\n",
		val, psg->env_val & 31, psg->env_inc & 31
	);
#endif

	psg->reg[13] = val;
}

static
void psg_set_port_a (st_psg_t *psg, unsigned char val)
{
	if (psg->reg[14] != val) {
		psg->reg[14] = val;

		if (psg->seta_fct != NULL) {
			psg->seta_fct (psg->seta_ext, val);
		}
	}
}

static
void psg_set_port_b (st_psg_t *psg, unsigned char val)
{
	if (psg->reg[15] != val) {
		psg->reg[15] = val;

		if (psg->setb_fct != NULL) {
			psg->setb_fct (psg->setb_ext, val);
		}
	}
}

static
void psg_aym_set_reg (st_psg_t *psg, unsigned idx, unsigned char val)
{
	unsigned long clk;

	static unsigned char msk[16] = {
		0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x1f, 0x3f,
		0x1f, 0x1f, 0x1f, 0xff, 0xff, 0x0f, 0x00, 0x00
	};

	if (psg->aym == NULL) {
		return;
	}

	if (idx > 13) {
		return;
	}

	if (psg->aym_reg == 0) {
		psg->aym_clk = psg->clock;
	}

	clk = (psg->clock - psg->aym_clk) / (PSG_FREQ_INP / 1000000);

	if (clk > 5000000) {
		clk = 5000000;
		psg->aym_reg = 0;
	}
	else if (((psg->reg[idx] ^ val) & msk[idx]) == 0) {
		if ((psg->aym_reg & (1U << idx)) && (idx != 13)) {
			return;
		}
	}

	if (clk >= psg->aym_res) {
		if (clk & 0xff000000) {
			fputc (0x30, psg->aym);
			fputc ((clk >> 24) & 0xff, psg->aym);
		}

		if (clk & 0x00fff000) {
			fputc (0x20 + ((clk >> 20) & 0x0f), psg->aym);
			fputc ((clk >> 12) & 0xff, psg->aym);
		}

		if (clk & 0x00000fff) {
			fputc (0x10 + ((clk >> 8) & 0x0f), psg->aym);
			fputc (clk & 0xff, psg->aym);
		}

		psg->aym_clk = psg->clock;
	}

	fputc (idx, psg->aym);
	fputc (val & msk[idx], psg->aym);

	psg->aym_reg |= (1U << idx);
}

unsigned char st_psg_get_select (st_psg_t *psg)
{
	return (psg->reg_sel);
}

void st_psg_set_select (st_psg_t *psg, unsigned char val)
{
	psg->reg_sel = val;
}

unsigned char st_psg_get_data (st_psg_t *psg)
{
	if (psg->reg_sel > 15) {
		return (0xff);
	}

	return (psg->reg[psg->reg_sel]);
}

void st_psg_set_data (st_psg_t *psg, unsigned char val)
{
	psg_aym_set_reg (psg, psg->reg_sel, val);

	switch (psg->reg_sel) {
	case 0:
		psg_set_tone_period (psg, 0, val, psg->reg[1]);
		break;

	case 1:
		psg_set_tone_period (psg, 0, psg->reg[0], val);
		break;

	case 2:
		psg_set_tone_period (psg, 1, val, psg->reg[3]);
		break;

	case 3:
		psg_set_tone_period (psg, 1, psg->reg[2], val);
		break;

	case 4:
		psg_set_tone_period (psg, 2, val, psg->reg[5]);
		break;

	case 5:
		psg_set_tone_period (psg, 2, psg->reg[4], val);
		break;

	case 6:
		psg_set_noise_period (psg, val);
		break;

	case 7: /* mixer */
		psg_set_mixer (psg, val);
		break;

	case 8: /* level A */
		psg_set_level (psg, 0, val);
		break;

	case 9: /* level B */
		psg_set_level (psg, 1, val);
		break;

	case 10: /* level C */
		psg_set_level (psg, 2, val);
		break;

	case 11:
		psg_set_env_period (psg, val, psg->reg[12]);
		break;

	case 12:
		psg_set_env_period (psg, psg->reg[11], val);
		break;

	case 13:
		psg_set_env_shape (psg, val);
		break;

	case 14:
		psg_set_port_a (psg, val);
		break;

	case 15:
		psg_set_port_b (psg, val);
		break;
	}
}

void st_psg_clock (st_psg_t *psg, unsigned long cnt)
{
	unsigned i;

	psg->clock += cnt;

	if (psg->silence_cnt == 0) {
		return;
	}

	cnt += psg->clock_div;
	psg->clock_div = cnt & 0x1f;
	cnt = cnt >> 5;

	/* cnt is now f[master] / 8 */

	while (cnt > 0) {
		if (--psg->noise_cnt == 0) {
			psg->noise_cnt = psg->noise_per;

			if (psg->noise_val & 1) {
				psg->noise_val = (psg->noise_val >> 1) ^ 0x12000;
			}
			else {
				psg->noise_val = psg->noise_val >> 1;
			}
		}

		for (i = 0; i < 3; i++) {
			if (--psg->tone_cnt[i] == 0) {
				psg->tone_cnt[i] = psg->tone_per[i];
				psg->tone_val[i] = !psg->tone_val[i];
			}

			if (psg->tone_per[i] < 5) {
				psg->tone_val[i] = 1;
			}
		}

		if (--psg->env_cnt == 0) {
			psg->env_cnt = psg->env_per;

			psg_env_clock (psg);
		}

		psg->out_cnt += psg->out_freq;

		if (psg->out_cnt >= psg->inp_freq) {
			psg->out_cnt -= psg->inp_freq;

			psg_add_sample (psg);

			if (psg->buf_cnt >= PSG_BUF_SIZE) {
				psg_write_buffer (psg);
			}
		}

		cnt -= 1;
	}
}
