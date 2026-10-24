/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text.c                                         *
 * Created:     2014-08-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pri/pri.h>
#include <lib/text.h>

#include "main.h"
#include "text.h"


static int txt_enc_track_finish (pri_text_t *ctx);


void txt_save_pos (const pri_text_t *ctx, pri_text_pos_t *pos)
{
	pos->pos = ctx->trk->idx;
	pos->wrap = ctx->trk->wrap;
	pos->evt = ctx->trk->cur_evt;
}

void txt_restore_pos (pri_text_t *ctx, const pri_text_pos_t *pos)
{
	ctx->trk->idx = pos->pos;
	ctx->trk->wrap = pos->wrap;
	ctx->trk->cur_evt = pos->evt;
}

static
unsigned txt_guess_encoding (pri_trk_t *trk)
{
	unsigned long val, bit;
	unsigned      fm, mfm, gcr;

	pri_trk_set_pos (trk, 0);

	val = 0;

	fm = 0;
	mfm = 0;
	gcr = 0;

	while (trk->wrap == 0) {
		pri_trk_get_bits (trk, &bit, 1);

		val = (val << 1) | (bit & 1);

		switch (val & 0xffffffff) {
		case 0xaaaaf56f:
		case 0xaaaaf57e:
			fm += 1;
			break;

		case 0x44894489:
			mfm += 1;
			break;
		}

		switch (val & 0xffffff) {
		case 0xd5aa96:
		case 0xd5aaad:
			gcr += 1;
			break;
		}
	}

	fm *= 2;
	gcr *= 2;

	if ((gcr > 0) && (gcr > fm) && (gcr > mfm)) {
		return (PRI_TEXT_MAC);
	}

	if ((fm > 0) && (fm > gcr) && (fm > mfm)) {
		return (PRI_TEXT_FM);
	}

	if ((mfm > 0) && (mfm > gcr) && (mfm > fm)) {
		return (PRI_TEXT_MFM);
	}

	return (PRI_TEXT_RAW);
}

int txt_dec_match (pri_text_t *ctx, const void *buf, unsigned cnt)
{
	unsigned            i;
	unsigned long       type, val;
	unsigned long       bit;
	const unsigned char *ptr;
	pri_text_pos_t      pos;

	txt_save_pos (ctx, &pos);

	ptr = buf;

	for (i = 0; i < cnt; i++) {
		if (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			break;
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		if (((ptr[i >> 3] >> (~i & 7)) ^ bit) & 1) {
			break;
		}
	}

	if ((i < cnt) || (ctx->trk->wrap && ctx->trk->idx > 0)) {
		txt_restore_pos (ctx, &pos);

		return (1);
	}

	return (0);
}

void txt_dec_bits (pri_text_t *ctx, unsigned cnt)
{
	unsigned      i;
	unsigned long val;

	val = (ctx->shift >> (ctx->shift_cnt - cnt)) & ((1UL << cnt) - 1);

	ctx->shift_cnt -= cnt;

	if (ctx->column > 0) {
		fputc ('\n', ctx->txt.fp);
	}

	fprintf (ctx->txt.fp, "RAW");

	for (i = 0; i < cnt; i++) {
		fprintf (ctx->txt.fp, " %lu", (val >> (cnt - i - 1)) & 1);
	}

	fprintf (ctx->txt.fp, "\n");

	ctx->column = 0;

	ctx->last_val = val & 1;
}

void txt_dec_event (pri_text_t *ctx, unsigned long type, unsigned long val)
{
	if (ctx->column > 0) {
		fputc ('\n', ctx->txt.fp);
		ctx->column = 0;
		ctx->need_nl = 0;
	}

	if (type == PRI_EVENT_WEAK) {
		fprintf (ctx->txt.fp, "WEAK %08lX\n", val);
	}
	else if (type == PRI_EVENT_CLOCK) {
		unsigned long long tmp;

		tmp = pri_trk_get_clock (ctx->trk);
		tmp = (tmp * val + 32768) / 65536;

		fprintf (ctx->txt.fp, "CLOCK %lu\n", (unsigned long) tmp);
	}
	else {
		fprintf (ctx->txt.fp, "EVENT %08lX %08lX\n", type, val);
	}
}

static
void txt_dec_init (pri_text_t *ctx, FILE *fp, pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h)
{
	memset (ctx, 0, sizeof (pri_text_t));

	txt_init (&ctx->txt, fp);

	ctx->img = img;
	ctx->trk = trk;
	ctx->c = c;
	ctx->h = h;
	ctx->s = 0;

	ctx->mac_align = par_mac_align;
	ctx->mac_no_slip = par_mac_no_slip;
}

static
void txt_dec_free (pri_text_t *ctx)
{
	if (ctx->free_track) {
		pri_trk_del (ctx->trk);
		ctx->trk = NULL;
	}

	txt_free (&ctx->txt);
}

static
int pri_decode_text_auto_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned   enc;
	pri_text_t ctx;

	txt_dec_init (&ctx, opaque, img, trk, c, h);

	enc = txt_guess_encoding (trk);

	if (enc == PRI_TEXT_FM) {
		txt_fm_dec_track (&ctx);
	}
	else if (enc == PRI_TEXT_MFM) {
		txt_mfm_dec_track (&ctx);
	}
	else if (enc == PRI_TEXT_MAC) {
		txt_mac_dec_track (&ctx);
	}
	else {
		txt_raw_dec_track (&ctx);
	}

	txt_dec_free (&ctx);

	return (0);
}

static
int pri_decode_text_mfm_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	txt_dec_init (&ctx, opaque, img, trk, c, h);
	txt_mfm_dec_track (&ctx);
	txt_dec_free (&ctx);

	return (0);
}

static
int pri_decode_text_fm_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	txt_dec_init (&ctx, opaque, img, trk, c, h);
	txt_fm_dec_track (&ctx);
	txt_dec_free (&ctx);

	return (0);
}

static
int pri_decode_text_mac_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	txt_dec_init (&ctx, opaque, img, trk, c, h);
	txt_mac_dec_track (&ctx);
	txt_dec_free (&ctx);

	return (0);
}

static
int pri_decode_text_raw_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	txt_dec_init (&ctx, opaque, img, trk, c, h);
	txt_raw_dec_track (&ctx);
	txt_dec_free (&ctx);

	return (0);
}

int pri_decode_text (pri_img_t *img, const char *fname, unsigned enc)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "w")) == NULL) {
		return (1);
	}

	fprintf (fp, "PRI 0\n");

	if (enc == PRI_TEXT_AUTO) {
		r = pri_for_all_tracks (img, pri_decode_text_auto_cb, fp);
	}
	else if (enc == PRI_TEXT_MFM) {
		r = pri_for_all_tracks (img, pri_decode_text_mfm_cb, fp);
	}
	else if (enc == PRI_TEXT_FM) {
		r = pri_for_all_tracks (img, pri_decode_text_fm_cb, fp);
	}
	else if (enc == PRI_TEXT_MAC) {
		r = pri_for_all_tracks (img, pri_decode_text_mac_cb, fp);
	}
	else {
		r = pri_for_all_tracks (img, pri_decode_text_raw_cb, fp);
	}

	fclose (fp);

	return (r);
}


unsigned long txt_get_position (pri_text_t *ctx)
{
	unsigned long val;

	val = (ctx->bit_cnt + ctx->offset) & 0xffffffff;

	ctx->offset = 0;

	return (val);
}

int txt_enc_bits_raw (pri_text_t *ctx, unsigned long val, unsigned cnt)
{
	pri_trk_set_bits (ctx->trk, val, cnt);
	ctx->bit_cnt += cnt;

	if ((2 * ctx->bit_cnt) > ctx->bit_max) {
		ctx->bit_max *= 2;

		if (pri_trk_set_size (ctx->trk, ctx->bit_max)) {
			return (1);
		}

		pri_trk_set_pos (ctx->trk, ctx->bit_cnt);
	}

	ctx->last_val = val & 0xff;

	return (0);
}

static
int txt_enc_clock (pri_text_t *ctx)
{
	unsigned long pos, val, old;

	if (txt_match_uint (&ctx->txt, 10, &val) == 0) {
		return (1);
	}

	if (val > 131072) {
		old = pri_trk_get_clock (ctx->trk);
		val = (65536ULL * val + (old / 2)) / old;
	}

	pos = txt_get_position (ctx);

	if (pri_trk_evt_add (ctx->trk, PRI_EVENT_CLOCK, pos, val) == NULL) {
		return (1);
	}

	return (0);
}

static
int txt_enc_comm (pri_text_t *ctx)
{
	unsigned cnt;
	char     str[256];

	if (txt_match (&ctx->txt, "RESET", 1)) {
		pri_img_set_comment (ctx->img, NULL, 0);
		return (0);
	}

	if (txt_match_string (&ctx->txt, str, 256) == 0) {
		return (1);
	}

	cnt = strlen (str);

	if (cnt < 256) {
		str[cnt++] = 0x0a;
	}

	if (pri_img_add_comment (ctx->img, (unsigned char *) str, cnt)) {
		return (1);
	}

	return (0);
}

static
int txt_enc_index (pri_text_t *ctx)
{
	ctx->index_position = txt_get_position (ctx);

	return (0);
}

static
int txt_enc_mode (pri_text_t *ctx)
{
	if (txt_match (&ctx->txt, "RAW", 1)) {
		ctx->encoding = PRI_TEXT_RAW;
	}
	else if (txt_match (&ctx->txt, "IBM-MFM", 1)) {
		ctx->encoding = PRI_TEXT_MFM;
	}
	else if (txt_match (&ctx->txt, "IBM-FM", 1)) {
		ctx->encoding = PRI_TEXT_FM;
	}
	else if (txt_match (&ctx->txt, "MAC-GCR", 1)) {
		ctx->encoding = PRI_TEXT_MAC;
	}
	else if (txt_match (&ctx->txt, "MFM", 1)) {
		ctx->encoding = PRI_TEXT_MFM;
	}
	else if (txt_match (&ctx->txt, "FM", 1)) {
		ctx->encoding = PRI_TEXT_FM;
	}
	else {
		ctx->encoding = PRI_TEXT_RAW;
		return (1);
	}

	return (0);
}

static
int txt_enc_offset (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 10, &val) == 0) {
		return (1);
	}

	ctx->offset = val;

	return (0);
}

static
int txt_enc_rate (pri_text_t *ctx)
{
	unsigned long clock;

	if (ctx->trk == NULL) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &clock) == 0) {
		return (1);
	}

	pri_trk_set_clock (ctx->trk, clock);

	return (0);
}

static
int txt_enc_raw (pri_text_t *ctx)
{
	unsigned long val;

	while (txt_match_eol (&ctx->txt) == 0) {
		if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
			return (1);
		}

		if ((val != 0) && (val != 1)) {
			return (1);
		}

		if (txt_enc_bits_raw (ctx, val, 1)) {
			return (1);
		}
	}

	return (0);
}

static
int txt_enc_rotate_track (pri_text_t *ctx)
{
	unsigned long c, h, cnt, max;
	pri_trk_t     *trk;

	if (txt_enc_track_finish (ctx)) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &c) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &h) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	if (cnt == 0) {
		return (0);
	}

	if (ctx->img == NULL) {
		return (1);
	}

	if ((trk = pri_img_get_track (ctx->img, c, h, 1)) == NULL) {
		return (1);
	}

	max = pri_trk_get_size (trk);

	if (max > 0) {
		if (-cnt < cnt) {
			cnt = max - (-cnt % max);
		}

		pri_trk_rotate (trk, cnt);
	}

	return (0);
}

static
int txt_enc_rotate (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match (&ctx->txt, "TRACK", 1)) {
		return (txt_enc_rotate_track (ctx));
	}

	if (txt_match_uint (&ctx->txt, 10, &val) == 0) {
		return (1);
	}

	ctx->rotate = val;

	return (0);
}

static
int txt_enc_track_finish (pri_text_t *ctx)
{
	if (ctx->trk == NULL) {
		return (0);
	}

	if (ctx->bit_cnt == 0) {
		ctx->trk = NULL;

		pri_img_del_track (ctx->img, ctx->c, ctx->h);

		return (0);
	}

	if (pri_trk_set_size (ctx->trk, ctx->bit_cnt)) {
		return (1);
	}

	if ((ctx->index_position != 0) || (ctx->rotate != 0)) {
		unsigned long cnt, max, rot;

		max = pri_trk_get_size (ctx->trk);
		rot = ctx->rotate;

		if (max > 0) {
			if (-rot < rot) {
				rot = max - (-rot % max);
			}

			cnt = (ctx->index_position + rot) % max;
			pri_trk_rotate (ctx->trk, cnt);
		}
	}

	ctx->trk = NULL;

	return (0);
}

static
int txt_enc_track_size (pri_text_t *ctx)
{
	unsigned long size;

	if (txt_match_uint (&ctx->txt, 10, &size) == 0) {
		return (1);
	}

	ctx->track_size = size;

	return (0);
}

static
int txt_enc_track (pri_text_t *ctx)
{
	unsigned long c, h;

	if (txt_match_uint (&ctx->txt, 10, &c) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &h) == 0) {
		return (1);
	}

	if ((c > 255) || (h > 255)) {
		txt_error (&ctx->txt, "c/h too large");
		return (1);
	}

	ctx->trk = pri_img_get_track (ctx->img, c, h, 1);

	if (ctx->trk == NULL) {
		return (1);
	}

	pri_trk_set_clock (ctx->trk, 500000);
	pri_trk_evt_del_all (ctx->trk, PRI_EVENT_ALL);

	ctx->c = c;
	ctx->h = h;

	ctx->bit_cnt = 0;
	ctx->bit_max = 65536;

	if (pri_trk_set_size (ctx->trk, ctx->bit_max)) {
		return (1);
	}

	pri_trk_set_pos (ctx->trk, 0);

	ctx->rotate = 0;

	ctx->mac_check_active = 0;
	ctx->last_val = 0;
	ctx->crc = 0xffff;

	return (0);
}

static
int txt_enc_weak_run (pri_text_t *ctx)
{
	unsigned long pos, cnt, val;

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	pos = txt_get_position (ctx);
	val = 0xffffffff;

	while (cnt >= 32) {
		if (pri_trk_evt_add (ctx->trk, PRI_EVENT_WEAK, pos, val) == NULL) {
			return (1);
		}

		pos += 32;
		cnt -= 32;
	}

	if (cnt > 0) {
		val = (val << (32 - cnt)) & 0xffffffff;

		if (pri_trk_evt_add (ctx->trk, PRI_EVENT_WEAK, pos, val) == NULL) {
			return (1);
		}
	}

	return (0);
}

static
int txt_enc_weak (pri_text_t *ctx)
{
	int           ok;
	unsigned      dig, cnt;
	unsigned long pos, val;

	if (txt_match (&ctx->txt, "RUN", 1)) {
		return (txt_enc_weak_run (ctx));
	}

	txt_match_space (&ctx->txt);

	pos = txt_get_position (ctx);
	val = 0;
	cnt = 0;
	ok = 0;

	while (txt_match_hex_digit (&ctx->txt, &dig)) {
		val |= (unsigned long) (dig & 0x0f) << (28 - cnt);
		cnt += 4;
		ok = 1;

		if (cnt >= 32) {
			if (pri_trk_evt_add (ctx->trk, PRI_EVENT_WEAK, pos, val) == NULL) {
				return (1);
			}

			pos += 32;
			val = 0;
			cnt = 0;
		}
	}

	if (cnt > 0) {
		if (pri_trk_evt_add (ctx->trk, PRI_EVENT_WEAK, pos, val) == NULL) {
			return (1);
		}
	}

	return (ok == 0);
}

static
int txt_clean_comment (pri_text_t *ctx)
{
	pri_img_t *img;

	img = ctx->img;

	while (img->comment_size > 0) {
		if (img->comment[img->comment_size - 1] == 0x0a) {
			img->comment_size -= 1;
		}
		else {
			break;
		}
	}

	return (0);
}

static
int txt_encode_pri0 (pri_text_t *ctx)
{
	int r;

	while (1) {
		switch (ctx->encoding) {
		case PRI_TEXT_MFM:
			r = txt_encode_pri0_mfm (ctx);
			break;

		case PRI_TEXT_FM:
			r = txt_encode_pri0_fm (ctx);
			break;

		case PRI_TEXT_MAC:
			r = txt_encode_pri0_mac (ctx);
			break;

		case PRI_TEXT_RAW:
			r = txt_encode_pri0_raw (ctx);
			break;

		default:
			r = -1;
			break;
		}

		if (r >= 0) {
			if (r > 0) {
				return (1);
			}

			continue;
		}

		if (txt_match (&ctx->txt, "CLOCK", 1)) {
			if (txt_enc_clock (ctx)) {
				txt_error (&ctx->txt, "clock");
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "COMM", 1)) {
			if (txt_enc_comm (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "INDEX", 1)) {
			if (txt_enc_index (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "MODE", 1)) {
			if (txt_enc_mode (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "OFFSET", 1)) {
			if (txt_enc_offset (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "PRI", 0)) {
			break;
		}
		else if (txt_match (&ctx->txt, "RATE", 1)) {
			if (txt_enc_rate (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "RAW", 1)) {
			if (txt_enc_raw (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "ROTATE", 1)) {
			if (txt_enc_rotate (ctx)) {
				return (1);
			}
		}
		else if (txt_match (&ctx->txt, "TRACK", 1)) {
			if (txt_match (&ctx->txt, "SIZE", 1)) {
				if (txt_enc_track_size (ctx)) {
					return (1);
				}
			}
			else {
				if (txt_enc_track_finish (ctx)) {
					return (1);
				}

				if (txt_enc_track (ctx)) {
					return (1);
				}
			}
		}
		else if (txt_match (&ctx->txt, "WEAK", 1) || txt_match (&ctx->txt, "FUZZY", 1)) {
			if (txt_enc_weak (ctx)) {
				return (1);
			}
		}
		else if (feof (ctx->txt.fp)) {
			break;
		}
		else {
			return (1);
		}
	}

	if (txt_enc_track_finish (ctx)) {
		return (1);
	}

	if (txt_clean_comment (ctx)) {
		return (1);
	}

	return (0);
}

static
int txt_encode (pri_text_t *ctx)
{
	unsigned long val;

	ctx->trk = NULL;
	ctx->bit_cnt = 0;
	ctx->bit_max = 0;
	ctx->last_val = 0;
	ctx->encoding = PRI_TEXT_RAW;
	ctx->index_position = 0;
	ctx->rotate = 0;
	ctx->offset = 0;
	ctx->crc = 0xffff;

	ctx->mac_check_active = 0;
	ctx->mac_gap_size = 6;

	while (1) {
		if (txt_match (&ctx->txt, "PRI", 1)) {
			if (txt_match_uint (&ctx->txt, 10, &val) == 0) {
				return (1);
			}

			if (val == 0) {
				if (txt_encode_pri0 (ctx)) {
					return (txt_error (&ctx->txt, "PRI"));
				}
			}
			else {
				return (txt_error (&ctx->txt, "bad pri version"));
			}
		}
		else if (feof (ctx->txt.fp) == 0) {
			return (txt_error (&ctx->txt, "no pri header"));
		}
		else {
			break;
		}
	}

	if (txt_enc_track_finish (ctx)) {
		return (1);
	}

	return (0);
}

int pri_encode_text (pri_img_t *img, const char *fname)
{
	int        r;
	pri_text_t ctx;

	memset (&ctx, 0, sizeof (ctx));

	ctx.track_size = par_track_size;

	if ((ctx.txt.fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	txt_init (&ctx.txt, ctx.txt.fp);

	ctx.img = img;

	r = txt_encode (&ctx);

	txt_free (&ctx.txt);
	fclose (ctx.txt.fp);

	return (r);
}
