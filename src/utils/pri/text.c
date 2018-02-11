/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text.c                                         *
 * Created:     2014-08-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2017 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

#include <drivers/pri/pri.h>


typedef struct {
	FILE           *fp;

	unsigned       cnt;
	char           buf[256];
	unsigned       line;

	pri_img_t      *img;
	pri_trk_t      *trk;
	unsigned long  c;
	unsigned long  h;

	unsigned long  shift;
	unsigned       shift_cnt;

	unsigned char  last_val;

	unsigned short encoding;

	unsigned long  index_position;

	unsigned       column;
	char           need_nl;

	unsigned long  bit_cnt;
	unsigned long  bit_max;
	unsigned short crc;
} pri_text_t;


static
unsigned mfm_crc (unsigned crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *src;

	src = buf;

	while (cnt > 0) {
		crc ^= (unsigned) *src << 8;

		for (i = 0; i < 8; i++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ 0x1021;
			}
			else {
				crc = crc << 1;
			}
		}

		src += 1;
		cnt -= 1;
	}

	return (crc & 0xffff);
}

static
unsigned txt_guess_encoding (pri_trk_t *trk)
{
	unsigned      val;
	unsigned long bit;

	val = 0;

	pri_trk_set_pos (trk, 0);

	while (trk->wrap == 0) {
		pri_trk_get_bits (trk, &bit, 1);

		val = (val << 1) | (bit & 1);

		switch (val & 0xffff) {
		case 0x4489:
			return (PRI_TEXT_MFM);

		case 0xf57e:
		case 0xf56f:
		case 0xf56a:
			return (PRI_TEXT_FM);
		}
	}

	return (PRI_TEXT_RAW);
}

static
void txt_put_bits (pri_text_t *ctx, unsigned cnt)
{
	unsigned      i;
	unsigned long val;

	val = (ctx->shift >> (ctx->shift_cnt - cnt)) & ((1UL << cnt) - 1);

	ctx->shift_cnt -= cnt;

	if (ctx->column > 0) {
		fputc ('\n', ctx->fp);
	}

	fprintf (ctx->fp, "RAW");

	for (i = 0; i < cnt; i++) {
		fprintf (ctx->fp, " %lu", (val >> (cnt - i - 1)) & 1);
	}

	fprintf (ctx->fp, "\n");

	ctx->column = 0;

	ctx->last_val = val & 1;
}

static
void txt_put_event (pri_text_t *ctx, unsigned long type, unsigned long val)
{
	if (ctx->column > 0) {
		fputc ('\n', ctx->fp);
		ctx->column = 0;
		ctx->need_nl = 0;
	}

	if (type == PRI_EVENT_WEAK) {
		fprintf (ctx->fp, "WEAK %08lX\n", val);
	}
	else if (type == PRI_EVENT_CLOCK) {
		unsigned long long tmp;

		tmp = pri_trk_get_clock (ctx->trk);
		tmp = (tmp * val + 32768) / 65536;

		fprintf (ctx->fp, "CLOCK %lu\n", (unsigned long) tmp);
	}
	else {
		fprintf (ctx->fp, "EVENT %08lX %08lX\n", type, val);
	}
}

static
void txt_put_byte_mfm (pri_text_t *ctx)
{
	unsigned      i;
	unsigned long tmp;
	unsigned char data, clk1, clk2;

	tmp = ctx->shift >> (ctx->shift_cnt - 16);

	clk1 = 0;
	clk2 = 0;
	data = ctx->last_val & 1;

	for (i = 0; i < 8; i++) {
		clk1 = (clk1 << 1) | ((tmp >> 15) & 1);
		clk2 = clk2 << 1;
		data = (data << 1) | ((tmp >> 14) & 1);

		if ((~data & 2) && (~data & 1)) {
			clk2 |= 1;
		}

		tmp <<= 2;
	}

	ctx->last_val = data & 1;
	ctx->shift_cnt -= 16;

	if (clk1 ^ clk2) {
		if (ctx->column > 0) {
			fputc ('\n', ctx->fp);
		}

		fprintf (ctx->fp, "%02X/%02X\n", data, clk1 ^ clk2);

		ctx->column = 0;
	}
	else {
		if (ctx->column > 0) {
			if (ctx->column >= 16) {
				fputc ('\n', ctx->fp);
				ctx->column = 0;
			}
			else {
				fputc (' ', ctx->fp);
			}
		}

		fprintf (ctx->fp, "%02X", data);

		ctx->column += 1;

		if (ctx->need_nl) {
			fputc ('\n', ctx->fp);
			ctx->column = 0;
		}
	}

	if ((data == 0xa1) && ((clk1 ^ clk2) == 0x04)) {
		ctx->need_nl = 1;
	}
	else {
		ctx->need_nl = 0;
	}
}

static
void txt_flush_mfm (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 16) {
		txt_put_byte_mfm (ctx);
	}

	if (ctx->shift_cnt > 0) {
		txt_put_bits (ctx, ctx->shift_cnt);
	}
}

static
unsigned txt_align_mfm (pri_text_t *ctx)
{
	unsigned long bit;

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (ctx->shift_cnt > 16) {
			if ((ctx->shift & 0xffff) == 0x4489) {
				return (ctx->shift_cnt & 15);
			}
		}
	}

	return (0);
}

static
int txt_decode_track_mfm (pri_text_t *ctx)
{
	unsigned long bit, align;
	unsigned long type, val;

	fprintf (ctx->fp, "TRACK MFM %lu %lu %lu\n\n",
		pri_trk_get_clock (ctx->trk), ctx->c, ctx->h
	);

	align = txt_align_mfm (ctx);

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			txt_flush_mfm (ctx);
			txt_put_event (ctx, type, val);
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (align > 0) {
			align -= 1;

			if (align == 0) {
				txt_put_bits (ctx, ctx->shift_cnt);
			}
		}

		if (ctx->shift_cnt >= 32) {
			txt_put_byte_mfm (ctx);
		}

		if (ctx->shift_cnt > 16) {
			if ((ctx->shift & 0xffff) == 0x4489) {
				txt_put_bits (ctx, ctx->shift_cnt - 16);
			}
		}
	}

	txt_flush_mfm (ctx);

	if (ctx->column > 0) {
		fputc ('\n', ctx->fp);
	}

	return (0);
}

static
void txt_put_byte_fm (pri_text_t *ctx)
{
	unsigned      i;
	unsigned long tmp;
	unsigned char val, clk;

	tmp = ctx->shift >> (ctx->shift_cnt - 16);

	val = 0;
	clk = 0;

	for (i = 0; i < 8; i++) {
		clk = (clk << 1) | ((tmp >> 15) & 1);
		val = (val << 1) | ((tmp >> 14) & 1);

		tmp <<= 2;
	}

	ctx->last_val = val & 1;
	ctx->shift_cnt -= 16;

	if (clk != 0xff) {
		if (ctx->column > 0) {
			fputc ('\n', ctx->fp);
		}

		fprintf (ctx->fp, "%02X/%02X\n", val, clk);

		ctx->column = 0;
	}
	else {
		if (ctx->column > 0) {
			if (ctx->column >= 16) {
				fputc ('\n', ctx->fp);
				ctx->column = 0;
			}
			else {
				fputc (' ', ctx->fp);
			}
		}

		fprintf (ctx->fp, "%02X", val);

		ctx->column += 1;

		if (ctx->need_nl) {
			fputc ('\n', ctx->fp);
			ctx->column = 0;
		}
	}
}

static
void txt_flush_fm (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 16) {
		txt_put_byte_fm (ctx);
	}

	if (ctx->shift_cnt > 0) {
		txt_put_bits (ctx, ctx->shift_cnt);
	}
}

static
int txt_decode_track_fm (pri_text_t *ctx)
{
	unsigned long bit;
	unsigned long type, val;

	fprintf (ctx->fp, "TRACK FM %lu %lu %lu\n\n",
		pri_trk_get_clock (ctx->trk), ctx->c, ctx->h
	);

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			txt_flush_fm (ctx);
			txt_put_event (ctx, type, val);
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (ctx->shift_cnt >= 32) {
			txt_put_byte_fm (ctx);
		}

		if (ctx->shift_cnt > 16) {
			switch (ctx->shift & 0xffff) {
			case 0xf57e: /* FE/C7 */
			case 0xf56f: /* FB/C7 */
			case 0xf56a: /* F8/C7 */
			case 0xf77a: /* FC/D7 */
				txt_put_bits (ctx, ctx->shift_cnt - 16);
				break;
			}
		}
	}

	txt_flush_fm (ctx);

	if (ctx->column > 0) {
		fputc ('\n', ctx->fp);
	}

	return (0);
}

static
void txt_put_byte_raw (pri_text_t *ctx)
{
	unsigned char val;

	val = (ctx->shift >> (ctx->shift_cnt - 8)) & 0xff;

	ctx->last_val = val & 1;
	ctx->shift_cnt -= 8;

	if (ctx->column > 0) {
		if (ctx->column >= 16) {
			fputc ('\n', ctx->fp);
			ctx->column = 0;
		}
		else {
			fputc (' ', ctx->fp);
		}
	}

	fprintf (ctx->fp, "%02X", val);

	ctx->column += 1;
}

static
void txt_flush_raw (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 8) {
		txt_put_byte_raw (ctx);
	}

	if (ctx->shift_cnt > 0) {
		txt_put_bits (ctx, ctx->shift_cnt);
	}
}

static
int txt_decode_track_raw (pri_text_t *ctx)
{
	unsigned long bit;
	unsigned long type, val;

	fprintf (ctx->fp, "TRACK RAW %lu %lu %lu\n\n",
		pri_trk_get_clock (ctx->trk), ctx->c, ctx->h
	);

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			txt_flush_raw (ctx);
			txt_put_event (ctx, type, val);
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (ctx->shift_cnt >= 8) {
			txt_put_byte_raw (ctx);
		}
	}

	txt_flush_raw (ctx);

	if (ctx->column > 0) {
		fputc ('\n', ctx->fp);
	}

	return (0);
}

static
int pri_decode_text_auto_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned enc;

	pri_text_t ctx;

	ctx.fp = opaque;
	ctx.img = img;
	ctx.trk = trk;
	ctx.c = c;
	ctx.h = h;

	enc = txt_guess_encoding (trk);

	if (enc == PRI_TEXT_FM) {
		txt_decode_track_fm (&ctx);
	}
	else if (enc == PRI_TEXT_MFM) {
		txt_decode_track_mfm (&ctx);
	}
	else {
		txt_decode_track_raw (&ctx);
	}

	return (0);
}

static
int pri_decode_text_mfm_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	ctx.fp = opaque;
	ctx.img = img;
	ctx.trk = trk;
	ctx.c = c;
	ctx.h = h;

	txt_decode_track_mfm (&ctx);

	return (0);
}

static
int pri_decode_text_fm_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	ctx.fp = opaque;
	ctx.img = img;
	ctx.trk = trk;
	ctx.c = c;
	ctx.h = h;

	txt_decode_track_fm (&ctx);

	return (0);
}

static
int pri_decode_text_raw_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_text_t ctx;

	ctx.fp = opaque;
	ctx.img = img;
	ctx.trk = trk;
	ctx.c = c;
	ctx.h = h;

	txt_decode_track_raw (&ctx);

	return (0);
}

int pri_decode_text (pri_img_t *img, const char *fname, unsigned enc)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "w")) == NULL) {
		return (1);
	}

	fprintf (fp, "VERSION 2\n\n");

	if (enc == PRI_TEXT_AUTO) {
		r = pri_for_all_tracks (img, pri_decode_text_auto_cb, fp);
	}
	else if (enc == PRI_TEXT_MFM) {
		r = pri_for_all_tracks (img, pri_decode_text_mfm_cb, fp);
	}
	else if (enc == PRI_TEXT_FM) {
		r = pri_for_all_tracks (img, pri_decode_text_fm_cb, fp);
	}
	else {
		r = pri_for_all_tracks (img, pri_decode_text_raw_cb, fp);
	}

	fclose (fp);

	return (r);
}


static
void txt_init (pri_text_t *ctx, FILE *fp)
{
	ctx->fp = fp;
	ctx->cnt = 0;
	ctx->line = 0;
}

static
int txt_getc (pri_text_t *ctx, unsigned idx)
{
	int      c;
	unsigned i;

	for (i = ctx->cnt; i <= idx; i++) {
		if ((c = fgetc (ctx->fp)) == EOF) {
			return (EOF);
		}

		ctx->buf[ctx->cnt++] = c;
	}

	return (ctx->buf[idx]);
}

static
void txt_error (pri_text_t *ctx, const char *str)
{
	int c;

	c = txt_getc (ctx, 0);

	fprintf (stderr, "pri-text:%u: error (%s), next = %02X\n", ctx->line + 1, str, c);
}

static
void txt_skip (pri_text_t *ctx, unsigned cnt)
{
	unsigned i;

	if (cnt >= ctx->cnt) {
		ctx->cnt = 0;
	}
	else {
		for (i = cnt; i < ctx->cnt; i++) {
			ctx->buf[i - cnt] = ctx->buf[i];
		}

		ctx->cnt -= cnt;
	}
}

static
void txt_match_space (pri_text_t *ctx)
{
	int c, line;

	line = 0;

	while (1) {
		c = txt_getc (ctx, 0);

		if (c == EOF) {
			return;
		}

		if ((c == 0x0d) || (c == 0x0a)) {
			line = 0;
			ctx->line += 1;
		}
		else if (line) {
			;
		}
		else if ((c == '\t') || (c == ' ')) {
			;
		}
		else if (c == '#') {
			line = 1;
		}
		else {
			return;
		}

		txt_skip (ctx, 1);
	}
}

static
int txt_match_eol (pri_text_t *ctx)
{
	int c, line;

	line = 0;

	while (1) {
		c = txt_getc (ctx, 0);

		if (c == EOF) {
			return (1);
		}

		if ((c == 0x0d) || (c == 0x0a)) {
			ctx->line += 1;
			txt_skip (ctx, 1);

			return (1);
		}
		else if (line) {
			;
		}
		else if ((c == '\t') || (c == ' ')) {
			;
		}
		else if (c == '#') {
			line = 1;
		}
		else {
			return (0);
		}

		txt_skip (ctx, 1);
	}
}

static
int txt_match (pri_text_t *ctx, const char *str, int skip)
{
	int      c;
	unsigned i, n;

	txt_match_space (ctx);

	n = 0;

	while (str[n] != 0) {
		if (n >= ctx->cnt) {
			if ((c = fgetc (ctx->fp)) == EOF) {
				return (0);
			}

			ctx->buf[ctx->cnt++] = c;
		}

		if (ctx->buf[n] != str[n]) {
			return (0);
		}

		n += 1;
	}

	if (skip) {
		for (i = n; i < ctx->cnt; i++) {
			ctx->buf[i - n] = ctx->buf[i];
		}

		ctx->cnt -= n;
	}

	return (1);
}

static
int txt_match_uint (pri_text_t *ctx, unsigned base, unsigned long *val)
{
	unsigned i, dig;
	int      c, s, ok;

	txt_match_space (ctx);

	i = 0;
	s = 0;
	ok = 0;

	c = txt_getc (ctx, i);

	if ((c == '-') || (c == '+')) {
		s = (c == '-');

		c = txt_getc (ctx, ++i);
	}

	*val = 0;

	while (1) {
		if ((c >= '0') && (c <= '9')) {
			dig = c - '0';
		}
		else if ((c >= 'a') && (c <= 'z')) {
			dig = c - 'a' + 10;
		}
		else if ((c >= 'A') && (c <= 'Z')) {
			dig = c - 'A' + 10;
		}
		else {
			break;
		}

		if (dig >= base) {
			return (0);
		}

		*val = base * *val + dig;
		ok = 1;

		c = txt_getc (ctx, ++i);
	}

	if (ok == 0) {
		return (0);
	}

	if (s) {
		*val = ~*val + 1;
	}

	*val &= 0xffffffff;

	txt_skip (ctx, i);

	return (1);
}

static
int txt_add_bits_raw (pri_text_t *ctx, unsigned long val, unsigned cnt)
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
int txt_add_bits_mfm (pri_text_t *ctx, unsigned long val, unsigned cnt)
{
	unsigned bit;

	while (cnt > 0) {
		bit = (val >> (cnt - 1)) & 1;

		if (((ctx->last_val | val) & 1) == 0) {
			bit |= 2;
		}

		if (txt_add_bits_raw (ctx, bit, 2)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int txt_add_bits_fm (pri_text_t *ctx, unsigned long val, unsigned cnt)
{
	unsigned bit;

	while (cnt > 0) {
		bit = (val >> (cnt - 1)) & 1;

		if (txt_add_bits_raw (ctx, bit | 2, 2)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int txt_add_bits (pri_text_t *ctx, unsigned long val, unsigned cnt)
{
	if (ctx->encoding == PRI_TEXT_RAW) {
		return (txt_add_bits_raw (ctx, val, cnt));
	}

	if (ctx->encoding == PRI_TEXT_MFM) {
		return (txt_add_bits_mfm (ctx, val, cnt));
	}

	if (ctx->encoding == PRI_TEXT_FM) {
		return (txt_add_bits_fm (ctx, val, cnt));
	}

	return (1);
}

static
int txt_add_byte_mfm (pri_text_t *ctx, unsigned char data, unsigned char clock)
{
	unsigned i;
	unsigned val;

	if (ctx->trk == NULL) {
		return (1);
	}

	ctx->crc = mfm_crc (ctx->crc, &data, 1);

	val = ctx->last_val & 1;

	for (i = 0; i < 8; i++) {
		val <<= 2;

		if (data & 0x80) {
			val |= 1;
		}

		if (((val & 4) == 0) && ((val & 1) == 0)) {
			val |= 2;
		}

		if (clock & 0x80) {
			val ^= 2;
		}

		data <<= 1;
		clock <<= 1;
	}

	if (txt_add_bits_raw (ctx, val, 16)) {
		return (1);
	}

	return (0);
}

static
int txt_add_byte_fm (pri_text_t *ctx, unsigned char val, unsigned char clk)
{
	unsigned i;
	unsigned tmp;

	if (ctx->trk == NULL) {
		return (1);
	}

	ctx->crc = mfm_crc (ctx->crc, &val, 1);

	tmp = 0;

	for (i = 0; i < 8; i++) {
		tmp = (tmp << 2) | ((clk >> 6) & 2) | ((val >> 7) & 1);
		val <<= 1;
		clk <<= 1;
	}

	if (txt_add_bits_raw (ctx, tmp, 16)) {
		return (1);
	}

	return (0);
}

static
int txt_add_byte (pri_text_t *ctx, unsigned char data, unsigned char clock)
{
	if (ctx->encoding == PRI_TEXT_RAW) {
		return (txt_add_bits_raw (ctx, data, 8));
	}

	if (ctx->encoding == PRI_TEXT_MFM) {
		return (txt_add_byte_mfm (ctx, data, clock));
	}

	if (ctx->encoding == PRI_TEXT_FM) {
		return (txt_add_byte_fm (ctx, data, clock));
	}

	return (1);
}

static
int txt_enc_am (pri_text_t *ctx)
{
	int           r;
	unsigned long val;

	if (ctx->encoding == PRI_TEXT_MFM) {
		if (txt_match_uint (ctx, 16, &val) == 0) {
			return (1);
		}

		ctx->crc = 0xffff;

		r = txt_add_byte (ctx, 0xa1, 0x04);
		r |= txt_add_byte (ctx, 0xa1, 0x04);
		r |= txt_add_byte (ctx, 0xa1, 0x04);
		r |= txt_add_byte (ctx, val, 0x00);
	}
	else if (ctx->encoding == PRI_TEXT_FM) {
		if (txt_match_uint (ctx, 16, &val) == 0) {
			return (1);
		}

		ctx->crc = 0xffff;

		r = txt_add_byte (ctx, val, 0xc7);
	}
	else {
		return (1);
	}

	return (0);
}

static
int txt_enc_bit (pri_text_t *ctx)
{
	unsigned long val;

	while (txt_match_eol (ctx) == 0) {
		if (txt_match_uint (ctx, 16, &val) == 0) {
			return (1);
		}

		if ((val != 0) && (val != 1)) {
			return (1);
		}

		if (txt_add_bits (ctx, val, 1)) {
			return (1);
		}
	}

	return (0);
}

static
int txt_enc_byte (pri_text_t *ctx)
{
	unsigned long val[2];

	if (txt_match_uint (ctx, 16, val) == 0) {
		return (1);
	}

	if (txt_match (ctx, "/", 1)) {
		if (txt_match_uint (ctx, 16, val + 1) == 0) {
			return (1);
		}
	}
	else {
		if (ctx->encoding == PRI_TEXT_FM) {
			val[1] = 0xff;
		}
		else {
			val[1] = 0;
		}
	}

	if (txt_add_byte (ctx, val[0], val[1])) {
		return (1);
	}

	return (0);
}

static
int txt_enc_clock (pri_text_t *ctx)
{
	unsigned long val, old;

	if (txt_match_uint (ctx, 10, &val) == 0) {
		return (1);
	}

	if (val > 131072) {
		old = pri_trk_get_clock (ctx->trk);
		val = (65536ULL * val + (old / 2)) / old;
	}

	if (pri_trk_evt_add (ctx->trk, PRI_EVENT_CLOCK, ctx->bit_cnt, val) == NULL) {
		return (1);
	}

	return (0);
}

static
int txt_enc_encoding (pri_text_t *ctx)
{
	if (txt_match (ctx, "RAW", 1)) {
		ctx->encoding = PRI_TEXT_RAW;
	}
	else if (txt_match (ctx, "MFM", 1)) {
		ctx->encoding = PRI_TEXT_MFM;
	}
	else if (txt_match (ctx, "FM", 1)) {
		ctx->encoding = PRI_TEXT_FM;
	}
	else {
		ctx->encoding = PRI_TEXT_RAW;
		return (1);
	}

	return (0);
}

static
int txt_enc_fill (pri_text_t *ctx)
{
	unsigned long max;
	unsigned long val[2];

	if (txt_match_uint (ctx, 10, &max) == 0) {
		return (1);
	}

	if (txt_match_uint (ctx, 16, val) == 0) {
		return (1);
	}

	if (txt_match (ctx, "/", 1)) {
		if (txt_match_uint (ctx, 16, val + 1) == 0) {
			return (1);
		}
	}
	else {
		val[1] = 0;
	}

	max *= 16;

	while (ctx->bit_cnt < max) {
		if (txt_add_byte (ctx, val[0], val[1])) {
			return (1);
		}
	}

	return (0);
}

static
int txt_enc_fuzzy (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match_uint (ctx, 16, &val) == 0) {
		return (1);
	}

	if (pri_trk_evt_add (ctx->trk, PRI_EVENT_WEAK, ctx->bit_cnt, val) == NULL) {
		return (1);
	}

	return (0);
}

static
int txt_enc_raw (pri_text_t *ctx)
{
	unsigned long val;

	while (txt_match_eol (ctx) == 0) {
		if (txt_match_uint (ctx, 16, &val) == 0) {
			return (1);
		}

		if ((val != 0) && (val != 1)) {
			return (1);
		}

		if (txt_add_bits_raw (ctx, val, 1)) {
			return (1);
		}
	}

	return (0);
}

static
int txt_enc_rep (pri_text_t *ctx)
{
	unsigned long cnt;
	unsigned long val[2];

	if (txt_match_uint (ctx, 10, &cnt) == 0) {
		return (1);
	}

	if (txt_match_uint (ctx, 16, val) == 0) {
		return (1);
	}

	if (txt_match (ctx, "/", 1)) {
		if (txt_match_uint (ctx, 16, val + 1) == 0) {
			return (1);
		}
	}
	else {
		val[1] = 0;
	}

	while (cnt > 0) {
		if (txt_add_byte (ctx, val[0], val[1])) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int txt_enc_sect_mfm (pri_text_t *ctx)
{
	int           r;
	unsigned      i;
	unsigned long val;
	unsigned char id[4];

	for (i = 0; i < 4; i++) {
		if (txt_match_uint (ctx, 16, &val) == 0) {
			return (1);
		}

		id[i] = val & 0xff;
	}

	r = 0;

	for (i = 0; i < 12; i++) {
		r |= txt_add_byte (ctx, 0, 0);
	}

	ctx->crc = 0xffff;

	r |= txt_add_byte (ctx, 0xa1, 0x04);
	r |= txt_add_byte (ctx, 0xa1, 0x04);
	r |= txt_add_byte (ctx, 0xa1, 0x04);
	r |= txt_add_byte (ctx, 0xfe, 0x00);

	for (i = 0; i < 4; i++) {
		r |= txt_add_byte (ctx, id[i], 0);
	}

	val = ctx->crc;

	r |= txt_add_byte (ctx, val >> 8, 0);
	r |= txt_add_byte (ctx, val, 0);

	for (i = 0; i < 22; i++) {
		r |= txt_add_byte (ctx, 0x4e, 0x00);
	}

	for (i = 0; i < 12; i++) {
		txt_add_byte (ctx, 0, 0);
	}

	ctx->crc = 0xffff;

	r = txt_add_byte (ctx, 0xa1, 0x04);
	r |= txt_add_byte (ctx, 0xa1, 0x04);
	r |= txt_add_byte (ctx, 0xa1, 0x04);
	r |= txt_add_byte (ctx, 0xfb, 0x00);

	return (r);
}

static
int txt_enc_sect_fm (pri_text_t *ctx)
{
	int           r;
	unsigned      i;
	unsigned long val;
	unsigned char id[4];

	for (i = 0; i < 4; i++) {
		if (txt_match_uint (ctx, 16, &val) == 0) {
			return (1);
		}

		id[i] = val & 0xff;
	}

	r = 0;

	for (i = 0; i < 6; i++) {
		r |= txt_add_byte_fm (ctx, 0, 0xff);
	}

	ctx->crc = 0xffff;

	r |= txt_add_byte_fm (ctx, 0xfe, 0xc7);

	for (i = 0; i < 4; i++) {
		r |= txt_add_byte_fm (ctx, id[i], 0xff);
	}

	val = ctx->crc;

	r |= txt_add_byte_fm (ctx, val >> 8, 0xff);
	r |= txt_add_byte_fm (ctx, val, 0xff);

	for (i = 0; i < 11; i++) {
		r |= txt_add_byte_fm (ctx, 0xff, 0xff);
	}

	for (i = 0; i < 6; i++) {
		txt_add_byte_fm (ctx, 0, 0xff);
	}

	ctx->crc = 0xffff;

	r |= txt_add_byte_fm (ctx, 0xfb, 0xc7);

	return (r);
}

static
int txt_enc_sect (pri_text_t *ctx)
{
	if (ctx->encoding == PRI_TEXT_MFM) {
		return (txt_enc_sect_mfm (ctx));
	}

	if (ctx->encoding == PRI_TEXT_FM) {
		return (txt_enc_sect_fm (ctx));
	}

	return (1);
}
static
int txt_enc_sync (pri_text_t *ctx)
{
	unsigned i;

	if (ctx->encoding == PRI_TEXT_MFM) {
		for (i = 0; i < 12; i++) {
			if (txt_add_byte (ctx, 0x00, 0x00)) {
				return (1);
			}
		}
	}
	else if (ctx->encoding == PRI_TEXT_FM) {
		for (i = 0; i < 6; i++) {
			if (txt_add_byte (ctx, 0x00, 0xff)) {
				return (1);
			}
		}
	}
	else {
		return (1);
	}

	return (0);
}

static
int txt_enc_track_finish (pri_text_t *ctx)
{
	if (ctx->trk == NULL) {
		return (0);
	}

	if (pri_trk_set_size (ctx->trk, ctx->bit_cnt)) {
		return (1);
	}

	if (ctx->index_position != 0) {
		unsigned long cnt, max;

		max = pri_trk_get_size (ctx->trk);

		if (max > 0) {
			cnt = ctx->index_position % max;
			pri_trk_rotate (ctx->trk, cnt);
		}
	}

	ctx->trk = NULL;

	return (0);
}

static
int txt_enc_track (pri_text_t *ctx)
{
	unsigned long c, h, clock;

	if (txt_match (ctx, "RAW", 1)) {
		ctx->encoding = PRI_TEXT_RAW;
	}
	else if (txt_match (ctx, "MFM", 1)) {
		ctx->encoding = PRI_TEXT_MFM;
	}
	else if (txt_match (ctx, "FM", 1)) {
		ctx->encoding = PRI_TEXT_FM;
	}
	else {
		txt_error (ctx, "missing track type");
		return (1);
	}

	if (txt_match_uint (ctx, 10, &clock) == 0) {
		return (1);
	}

	if (txt_match_uint (ctx, 10, &c) == 0) {
		return (1);
	}

	if (txt_match_uint (ctx, 10, &h) == 0) {
		return (1);
	}

	ctx->trk = pri_img_get_track (ctx->img, c, h, 1);

	if (ctx->trk == NULL) {
		return (1);
	}

	pri_trk_set_clock (ctx->trk, clock);
	pri_trk_evt_del_all (ctx->trk, PRI_EVENT_ALL);

	ctx->bit_cnt = 0;
	ctx->bit_max = 65536;

	if (pri_trk_set_size (ctx->trk, ctx->bit_max)) {
		return (1);
	}

	pri_trk_set_pos (ctx->trk, 0);

	ctx->last_val = 0;
	ctx->crc = 0xffff;

	return (0);
}

static
int txt_encode_v2 (pri_text_t *ctx)
{
	ctx->trk = NULL;
	ctx->last_val = 0;
	ctx->bit_cnt = 0;
	ctx->bit_max = 0;
	ctx->crc = 0xffff;

	while (1) {
		if (txt_match (ctx, "AM", 1) || txt_match (ctx, "MARK", 1)) {
			if (txt_enc_am (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "BIT", 1)) {
			if (txt_enc_bit (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "CLOCK", 1)) {
			if (txt_enc_clock (ctx)) {
				fprintf (stderr, "pri-text: syntax error (clock)\n");
				return (1);
			}
		}
		else if (txt_match (ctx, "CRC0", 1)) {
			ctx->crc = 0xffff;
		}
		else if (txt_match (ctx, "CRC", 1)) {
			unsigned crc;

			crc = ctx->crc;

			if (txt_add_byte (ctx, crc >> 8, 0)) {
				return (1);
			}

			if (txt_add_byte (ctx, crc, 0)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "ENCODING", 1)) {
			if (txt_enc_encoding (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "FILL", 1)) {
			if (txt_enc_fill (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "FUZZY", 1)) {
			if (txt_enc_fuzzy (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "INDEX", 1)) {
			ctx->index_position = ctx->bit_cnt;
		}
		else if (txt_match (ctx, "RAW", 1)) {
			if (txt_enc_raw (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "REP", 1)) {
			if (txt_enc_rep (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "SECT", 1)) {
			if (txt_enc_sect (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "SYNC", 1)) {
			if (txt_enc_sync (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "TRACK", 1)) {
			if (txt_enc_track_finish (ctx)) {
				return (1);
			}

			if (txt_enc_track (ctx)) {
				return (1);
			}
		}
		else if (txt_match (ctx, "VERSION", 0)) {
			break;
		}
		else if (feof (ctx->fp) == 0) {
			if (txt_enc_byte (ctx)) {
				txt_error (ctx, "byte");
				return (1);
			}
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

static
int txt_encode (pri_text_t *ctx)
{
	ctx->trk = NULL;
	ctx->last_val = 0;
	ctx->encoding = PRI_TEXT_RAW;
	ctx->index_position = 0;
	ctx->bit_cnt = 0;
	ctx->bit_max = 0;
	ctx->crc = 0xffff;

	while (1) {
		if (txt_match (ctx, "VERSION", 1)) {
			if (txt_match (ctx, "2", 1)) {
				if (txt_encode_v2 (ctx)) {
					return (1);
				}
			}
			else {
				txt_error (ctx, "bad version");
				return (1);
			}
		}
		else if (feof (ctx->fp) == 0) {
			if (txt_enc_byte (ctx)) {
				txt_error (ctx, "VERSION");
				return (1);
			}
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

	if ((ctx.fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	txt_init (&ctx, ctx.fp);

	ctx.img = img;

	r = txt_encode (&ctx);

	fclose (ctx.fp);

	return (r);
}
