/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text-ibm-mfm.c                                 *
 * Created:     2017-10-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017-2024 Hampa Hug <hampa@hampa.ch>                     *
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
void mfm_dec_byte (pri_text_t *ctx)
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
			fputc ('\n', ctx->txt.fp);
		}

		fprintf (ctx->txt.fp, "%02X/%02X\n", data, clk1 ^ clk2);

		ctx->column = 0;
	}
	else {
		if (ctx->column > 0) {
			if (ctx->column >= 16) {
				fputc ('\n', ctx->txt.fp);
				ctx->column = 0;
			}
			else {
				fputc (' ', ctx->txt.fp);
			}
		}

		fprintf (ctx->txt.fp, "%02X", data);

		ctx->column += 1;

		if (ctx->need_nl) {
			fputc ('\n', ctx->txt.fp);
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
void mfm_dec_flush (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 16) {
		mfm_dec_byte (ctx);
	}

	if (ctx->shift_cnt > 0) {
		txt_dec_bits (ctx, ctx->shift_cnt);
	}
}

static
unsigned mfm_dec_align (pri_text_t *ctx)
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

int txt_mfm_dec_track (pri_text_t *ctx)
{
	unsigned long bit, align;
	unsigned long type, val;

	fprintf (ctx->txt.fp, "\nTRACK %lu %lu\n\n", ctx->c, ctx->h);
	fprintf (ctx->txt.fp, "MODE IBM-MFM\n");
	fprintf (ctx->txt.fp, "RATE %lu\n\n", pri_trk_get_clock (ctx->trk));

	if (par_text_align) {
		align = mfm_dec_align (ctx);
	}
	else {
		align = 0;
	}

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			mfm_dec_flush (ctx);
			txt_dec_event (ctx, type, val);
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (align > 0) {
			align -= 1;

			if (align == 0) {
				txt_dec_bits (ctx, ctx->shift_cnt);
			}
		}

		if (ctx->shift_cnt >= 32) {
			mfm_dec_byte (ctx);
		}

		if (ctx->shift_cnt > 16) {
			if ((ctx->shift & 0xffff) == 0x4489) {
				txt_dec_bits (ctx, ctx->shift_cnt - 16);
			}
		}
	}

	mfm_dec_flush (ctx);

	if (ctx->column > 0) {
		fputc ('\n', ctx->txt.fp);
	}

	return (0);
}


static
int mfm_enc_bits (pri_text_t *ctx, unsigned long val, unsigned cnt)
{
	unsigned bit;

	while (cnt > 0) {
		bit = (val >> (cnt - 1)) & 1;

		if (((ctx->last_val | val) & 1) == 0) {
			bit |= 2;
		}

		if (txt_enc_bits_raw (ctx, bit, 2)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int mfm_enc_byte (pri_text_t *ctx, unsigned char data, unsigned char clock)
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

	if (txt_enc_bits_raw (ctx, val, 16)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_bytes (pri_text_t *ctx, unsigned char data, unsigned char clock, unsigned cnt)
{
	while (cnt-- > 0) {
		if (mfm_enc_byte (ctx, data, clock)) {
			return (1);
		}
	}

	return (0);
}

static
int mfm_enc_address_mark (pri_text_t *ctx, unsigned val)
{
	ctx->crc = 0xffff;

	if (mfm_enc_bytes (ctx, 0xa1, 0x04, 3)) {
		return (1);
	}

	if (mfm_enc_byte (ctx, val, 0x00)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_am (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
		return (1);
	}

	if (mfm_enc_address_mark (ctx, val & 0xff)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_bit (pri_text_t *ctx)
{
	unsigned long val;

	while (txt_match_eol (&ctx->txt) == 0) {
		if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
			return (1);
		}

		if ((val != 0) && (val != 1)) {
			return (1);
		}

		if (mfm_enc_bits (ctx, val, 1)) {
			return (1);
		}
	}

	return (0);
}

static
int mfm_enc_check_start (pri_text_t *ctx)
{
	ctx->crc = 0xffff;

	return (0);
}

static
int mfm_enc_check_stop (pri_text_t *ctx, int sum)
{
	unsigned crc;

	if (sum) {
		crc = ctx->crc;

		if (mfm_enc_byte (ctx, crc >> 8, 0)) {
			return (1);
		}

		if (mfm_enc_byte (ctx, crc, 0)) {
			return (1);
		}
	}

	return (0);
}

static
int mfm_enc_check (pri_text_t *ctx)
{
	if (txt_match (&ctx->txt, "START", 1)) {
		return (mfm_enc_check_start (ctx));
	}

	if (txt_match (&ctx->txt, "END", 1)) {
		return (mfm_enc_check_stop (ctx, 0));
	}

	if (txt_match (&ctx->txt, "SUM", 1)) {
		return (mfm_enc_check_stop (ctx, 1));
	}

	return (0);
}

static
int mfm_enc_dam (pri_text_t *ctx)
{
	if (mfm_enc_bytes (ctx, 0x00, 0x00, 12)) {
		return (1);
	}

	if (mfm_enc_address_mark (ctx, 0xfb)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_eot (pri_text_t *ctx)
{
	unsigned long max;

	if (ctx->track_size > 0) {
		max = ctx->track_size;
	}
	else if (pri_trk_get_clock (ctx->trk) < 750000) {
		max = 100000;
	}
	else {
		max = 200000;
	}

	if (ctx->bit_cnt < max) {
		while (ctx->bit_cnt < max) {
			if (mfm_enc_byte (ctx, 0x4e, 0x00)) {
				return (1);
			}
		}

		ctx->bit_cnt = max;
		pri_trk_set_pos (ctx->trk, max);
	}

	return (0);
}

static
int mfm_enc_fill (pri_text_t *ctx)
{
	unsigned long max;
	unsigned long val[2];

	if (txt_match (&ctx->txt, "TRACK", 1)) {
		if (pri_trk_get_clock (ctx->trk) < 750000) {
			max = 100000;
		}
		else {
			max = 200000;
		}
	}
	else if (txt_match_uint (&ctx->txt, 10, &max)) {
		if (max <= 15000) {
			max *= 16;
		}
	}
	else {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 16, val) == 0) {
		return (1);
	}

	if (txt_match (&ctx->txt, "/", 1)) {
		if (txt_match_uint (&ctx->txt, 16, val + 1) == 0) {
			return (1);
		}
	}
	else {
		val[1] = 0;
	}

	if (ctx->bit_cnt < max) {
		while (ctx->bit_cnt < max) {
			if (mfm_enc_byte (ctx, val[0], val[1])) {
				return (1);
			}
		}

		ctx->bit_cnt = max;
		pri_trk_set_pos (ctx->trk, max);
	}

	return (0);
}

static
int mfm_enc_gap (pri_text_t *ctx)
{
	unsigned long cnt;

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	if (mfm_enc_bytes (ctx, 0x4e, 0x00, cnt)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_hex (pri_text_t *ctx, unsigned val)
{
	unsigned      clk;
	unsigned long tmp;

	clk = 0;

	if (txt_match (&ctx->txt, "/", 1)) {
		if (txt_match_uint (&ctx->txt, 16, &tmp) == 0) {
			return (1);
		}

		clk = tmp & 0xff;
	}

	if (mfm_enc_byte (ctx, val, clk)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_iam (pri_text_t *ctx)
{
	unsigned long gap4a, gap1;

	if (txt_match_uint (&ctx->txt, 10, &gap4a) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &gap1) == 0) {
		return (1);
	}

	if (mfm_enc_bytes (ctx, 0x4e, 0x00, gap4a)) {
		return (1);
	}

	if (mfm_enc_bytes (ctx, 0x00, 0x00, 12)) {
		return (1);
	}

	if (mfm_enc_bytes (ctx, 0xc2, 0x08, 3)) {
		return (1);
	}

	if (mfm_enc_byte (ctx, 0xfc, 0x00)) {
		return (1);
	}

	if (mfm_enc_bytes (ctx, 0x4e, 0x00, gap1)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_idam (pri_text_t *ctx)
{
	if (mfm_enc_bytes (ctx, 0x00, 0x00, 12)) {
		return (1);
	}

	if (mfm_enc_address_mark (ctx, 0xfe)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_rep (pri_text_t *ctx)
{
	unsigned long cnt;
	unsigned long val[2];

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 16, val) == 0) {
		return (1);
	}

	if (txt_match (&ctx->txt, "/", 1)) {
		if (txt_match_uint (&ctx->txt, 16, val + 1) == 0) {
			return (1);
		}
	}
	else {
		val[1] = 0;
	}

	while (cnt > 0) {
		if (mfm_enc_byte (ctx, val[0], val[1])) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int mfm_enc_sect_end (pri_text_t *ctx)
{
	if (mfm_enc_check_stop (ctx, 1)) {
		return (1);
	}

	return (0);
}

static
int mfm_enc_sect (pri_text_t *ctx)
{
	int           r;
	unsigned      i;
	unsigned long val;
	unsigned char id[4];

	if (txt_match (&ctx->txt, "END", 1)) {
		return (mfm_enc_sect_end (ctx));
	}

	for (i = 0; i < 4; i++) {
		if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
			return (1);
		}

		id[i] = val & 0xff;
	}

	r = 0;

	for (i = 0; i < 12; i++) {
		r |= mfm_enc_byte (ctx, 0, 0);
	}

	ctx->crc = 0xffff;

	r |= mfm_enc_byte (ctx, 0xa1, 0x04);
	r |= mfm_enc_byte (ctx, 0xa1, 0x04);
	r |= mfm_enc_byte (ctx, 0xa1, 0x04);
	r |= mfm_enc_byte (ctx, 0xfe, 0x00);

	for (i = 0; i < 4; i++) {
		r |= mfm_enc_byte (ctx, id[i], 0);
	}

	val = ctx->crc;

	r |= mfm_enc_byte (ctx, val >> 8, 0);
	r |= mfm_enc_byte (ctx, val, 0);

	for (i = 0; i < 22; i++) {
		r |= mfm_enc_byte (ctx, 0x4e, 0x00);
	}

	for (i = 0; i < 12; i++) {
		mfm_enc_byte (ctx, 0, 0);
	}

	ctx->crc = 0xffff;

	r = mfm_enc_byte (ctx, 0xa1, 0x04);
	r |= mfm_enc_byte (ctx, 0xa1, 0x04);
	r |= mfm_enc_byte (ctx, 0xa1, 0x04);
	r |= mfm_enc_byte (ctx, 0xfb, 0x00);

	return (r);
}

static
int mfm_enc_sync (pri_text_t *ctx)
{
	unsigned i;

	for (i = 0; i < 12; i++) {
		if (mfm_enc_byte (ctx, 0x00, 0x00)) {
			return (1);
		}
	}

	return (0);
}

int txt_encode_pri0_mfm (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match (&ctx->txt, "AM", 1)) {
		return (mfm_enc_am (ctx));
	}
	else if (txt_match (&ctx->txt, "BIT", 1)) {
		return (mfm_enc_bit (ctx));
	}
	else if (txt_match (&ctx->txt, "CHECK", 1)) {
		return (mfm_enc_check (ctx));
	}
	else if (txt_match (&ctx->txt, "CRC", 1)) {
		return (mfm_enc_check_stop (ctx, 1));
	}
	else if (txt_match (&ctx->txt, "DAM", 1)) {
		return (mfm_enc_dam (ctx));
	}
	else if (txt_match (&ctx->txt, "EOT", 1)) {
		return (mfm_enc_eot (ctx));
	}
	else if (txt_match (&ctx->txt, "FILL", 1)) {
		return (mfm_enc_fill (ctx));
	}
	else if (txt_match (&ctx->txt, "GAP", 1)) {
		return (mfm_enc_gap (ctx));
	}
	else if (txt_match (&ctx->txt, "IAM", 1)) {
		return (mfm_enc_iam (ctx));
	}
	else if (txt_match (&ctx->txt, "IDAM", 1)) {
		return (mfm_enc_idam (ctx));
	}
	else if (txt_match (&ctx->txt, "REP", 1)) {
		return (mfm_enc_rep (ctx));
	}
	else if (txt_match (&ctx->txt, "SECT", 1)) {
		return (mfm_enc_sect (ctx));
	}
	else if (txt_match (&ctx->txt, "SYNC", 1)) {
		return (mfm_enc_sync (ctx));
	}
	else if (txt_match_uint (&ctx->txt, 16, &val)) {
		return (mfm_enc_hex (ctx, val));
	}

	return (-1);
}
