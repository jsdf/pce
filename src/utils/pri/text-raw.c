/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text-raw.c                                     *
 * Created:     2017-10-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017-2023 Hampa Hug <hampa@hampa.ch>                     *
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
void raw_dec_byte (pri_text_t *ctx)
{
	unsigned char val;

	val = (ctx->shift >> (ctx->shift_cnt - 8)) & 0xff;

	ctx->last_val = val & 1;
	ctx->shift_cnt -= 8;

	if (ctx->column > 0) {
		if (ctx->column >= 16) {
			fputc ('\n', ctx->txt.fp);
			ctx->column = 0;
		}
		else {
			fputc (' ', ctx->txt.fp);
		}
	}

	fprintf (ctx->txt.fp, "%02X", val);

	ctx->column += 1;
}

void raw_dec_flush (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 8) {
		raw_dec_byte (ctx);
	}

	if (ctx->shift_cnt > 0) {
		txt_dec_bits (ctx, ctx->shift_cnt);
	}
}

int txt_raw_dec_track (pri_text_t *ctx)
{
	unsigned long bit;
	unsigned long type, val;

	fprintf (ctx->txt.fp, "TRACK %lu %lu\n\n", ctx->c, ctx->h);
	fprintf (ctx->txt.fp, "MODE RAW\n");
	fprintf (ctx->txt.fp, "RATE %lu\n\n", pri_trk_get_clock (ctx->trk));

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			raw_dec_flush (ctx);
			txt_dec_event (ctx, type, val);
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (ctx->shift_cnt >= 8) {
			raw_dec_byte (ctx);
		}
	}

	raw_dec_flush (ctx);

	if (ctx->column > 0) {
		fputc ('\n', ctx->txt.fp);
	}

	return (0);
}

static
int raw_enc_bit (pri_text_t *ctx)
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
int raw_enc_fill (pri_text_t *ctx)
{
	unsigned long max;
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 10, &max) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
		return (1);
	}

	max *= 8;

	while (ctx->bit_cnt < max) {
		if (txt_enc_bits_raw (ctx, val, 8)) {
			return (1);
		}
	}

	return (0);
}

static
int raw_enc_hex (pri_text_t *ctx, unsigned val)
{
	if (txt_enc_bits_raw (ctx, val, 8)) {
		return (1);
	}

	return (0);
}

static
int raw_enc_rep (pri_text_t *ctx)
{
	unsigned long cnt;
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
		return (1);
	}

	while (cnt > 0) {
		if (txt_enc_bits_raw (ctx, val, 8)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

int txt_encode_pri0_raw (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match (&ctx->txt, "BIT", 1)) {
		return (raw_enc_bit (ctx));
	}
	else if (txt_match (&ctx->txt, "FILL", 1)) {
		return (raw_enc_fill (ctx));
	}
	else if (txt_match (&ctx->txt, "REP", 1)) {
		return (raw_enc_rep (ctx));
	}
	else if (txt_match_uint (&ctx->txt, 16, &val)) {
		return (raw_enc_hex (ctx, val));
	}

	return (-1);
}
