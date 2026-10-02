/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text-ibm-fm.c                                  *
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
unsigned fm_crc (unsigned crc, const void *buf, unsigned cnt)
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
void fm_dec_byte (pri_text_t *ctx)
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
			fputc ('\n', ctx->txt.fp);
		}

		fprintf (ctx->txt.fp, "%02X/%02X\n", val, clk);

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

		fprintf (ctx->txt.fp, "%02X", val);

		ctx->column += 1;

		if (ctx->need_nl) {
			fputc ('\n', ctx->txt.fp);
			ctx->column = 0;
		}
	}
}

static
void fm_dec_flush (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 16) {
		fm_dec_byte (ctx);
	}

	if (ctx->shift_cnt > 0) {
		txt_dec_bits (ctx, ctx->shift_cnt);
	}
}

int txt_fm_dec_track (pri_text_t *ctx)
{
	unsigned long bit;
	unsigned long type, val;

	fprintf (ctx->txt.fp, "\nTRACK %lu %lu\n\n", ctx->c, ctx->h);
	fprintf (ctx->txt.fp, "MODE IBM-FM\n");
	fprintf (ctx->txt.fp, "RATE %lu\n\n", pri_trk_get_clock (ctx->trk));

	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	pri_trk_set_pos (ctx->trk, 0);

	while (ctx->trk->wrap == 0) {
		while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
			fm_dec_flush (ctx);
			txt_dec_event (ctx, type, val);
		}

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (ctx->shift_cnt >= 32) {
			fm_dec_byte (ctx);
		}

		if (ctx->shift_cnt > 16) {
			switch (ctx->shift & 0xffff) {
			case 0xf57e: /* FE/C7 */
			case 0xf56f: /* FB/C7 */
			case 0xf56a: /* F8/C7 */
			case 0xf77a: /* FC/D7 */
				txt_dec_bits (ctx, ctx->shift_cnt - 16);
				break;
			}
		}
	}

	fm_dec_flush (ctx);

	if (ctx->column > 0) {
		fputc ('\n', ctx->txt.fp);
	}

	return (0);
}


static
int fm_enc_bits (pri_text_t *ctx, unsigned long val, unsigned cnt)
{
	unsigned bit;

	while (cnt > 0) {
		bit = (val >> (cnt - 1)) & 1;

		if (txt_enc_bits_raw (ctx, bit | 2, 2)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int fm_enc_byte (pri_text_t *ctx, unsigned char val, unsigned char clk)
{
	unsigned i;
	unsigned tmp;

	if (ctx->trk == NULL) {
		return (1);
	}

	ctx->crc = fm_crc (ctx->crc, &val, 1);

	tmp = 0;

	for (i = 0; i < 8; i++) {
		tmp = (tmp << 2) | ((clk >> 6) & 2) | ((val >> 7) & 1);
		val <<= 1;
		clk <<= 1;
	}

	if (txt_enc_bits_raw (ctx, tmp, 16)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_bytes (pri_text_t *ctx, unsigned char val, unsigned char clk, unsigned cnt)
{
	while (cnt-- > 0) {
		if (fm_enc_byte (ctx, val, clk)) {
			return (1);
		}
	}

	return (0);
}

static
int fm_enc_am (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
		return (1);
	}

	ctx->crc = 0xffff;

	if (fm_enc_byte (ctx, val, 0xc7)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_bit (pri_text_t *ctx)
{
	unsigned long val;

	while (txt_match_eol (&ctx->txt) == 0) {
		if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
			return (1);
		}

		if ((val != 0) && (val != 1)) {
			return (1);
		}

		if (fm_enc_bits (ctx, val, 1)) {
			return (1);
		}
	}

	return (0);
}

static
int fm_enc_check_start (pri_text_t *ctx)
{
	ctx->crc = 0xffff;

	return (0);
}

static
int fm_enc_check_stop (pri_text_t *ctx, int sum)
{
	unsigned crc;

	if (sum) {
		crc = ctx->crc;

		if (fm_enc_byte (ctx, crc >> 8, 0xff)) {
			return (1);
		}

		if (fm_enc_byte (ctx, crc, 0xff)) {
			return (1);
		}
	}

	return (0);
}

static
int fm_enc_check (pri_text_t *ctx)
{
	if (txt_match (&ctx->txt, "START", 1)) {
		return (fm_enc_check_start (ctx));
	}

	if (txt_match (&ctx->txt, "END", 1)) {
		return (fm_enc_check_stop (ctx, 0));
	}

	if (txt_match (&ctx->txt, "SUM", 1)) {
		return (fm_enc_check_stop (ctx, 1));
	}

	return (0);
}

static
int fm_enc_dam (pri_text_t *ctx)
{
	if (fm_enc_bytes (ctx, 0xff, 0x00, 6)) {
		return (1);
	}

	if (fm_enc_byte (ctx, 0xfb, 0xc7)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_eot (pri_text_t *ctx)
{
	unsigned long max;

	max = 50000;

	if (ctx->bit_cnt < max) {
		while (ctx->bit_cnt < max) {
			if (fm_enc_byte (ctx, 0xff, 0xff)) {
				return (1);
			}
		}

		ctx->bit_cnt = max;
		pri_trk_set_pos (ctx->trk, max);
	}

	return (0);
}

static
int fm_enc_fill (pri_text_t *ctx)
{
	unsigned long max;
	unsigned long val[2];

	if (txt_match (&ctx->txt, "TRACK", 1)) {
		max = 50000;
	}
	else if (txt_match_uint (&ctx->txt, 10, &max)) {
		if (max <= 5000) {
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
		val[1] = 0xff;
	}

	if (ctx->bit_cnt < max) {
		while (ctx->bit_cnt < max) {
			if (fm_enc_byte (ctx, val[0], val[1])) {
				return (1);
			}
		}

		ctx->bit_cnt = max;
		pri_trk_set_pos (ctx->trk, max);
	}

	return (0);
}

static
int fm_enc_gap (pri_text_t *ctx)
{
	unsigned long cnt;

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	if (fm_enc_bytes (ctx, 0xff, 0xff, cnt)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_hex (pri_text_t *ctx, unsigned val)
{
	unsigned      clk;
	unsigned long tmp;

	clk = 0xff;

	if (txt_match (&ctx->txt, "/", 1)) {
		if (txt_match_uint (&ctx->txt, 16, &tmp) == 0) {
			return (1);
		}

		clk = tmp & 0xff;
	}

	if (fm_enc_byte (ctx, val, clk)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_iam (pri_text_t *ctx)
{
	unsigned long gap4a, gap1;

	if (txt_match_uint (&ctx->txt, 10, &gap4a) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &gap1) == 0) {
		return (1);
	}

	if (fm_enc_bytes (ctx, 0xff, 0xff, gap4a)) {
		return (1);
	}

	if (fm_enc_bytes (ctx, 0x00, 0xff, 6)) {
		return (1);
	}

	if (fm_enc_byte (ctx, 0xfc, 0xd7)) {
		return (1);
	}

	if (fm_enc_bytes (ctx, 0xff, 0xff, gap1)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_idam (pri_text_t *ctx)
{
	if (fm_enc_bytes (ctx, 0xff, 0x00, 6)) {
		return (1);
	}

	if (fm_enc_byte (ctx, 0xfe, 0xc7)) {
		return (1);
	}

	return (0);
}

static
int fm_enc_rep (pri_text_t *ctx)
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
		val[1] = 0xff;
	}

	while (cnt > 0) {
		if (fm_enc_byte (ctx, val[0], val[1])) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int fm_enc_sect (pri_text_t *ctx)
{
	int           r;
	unsigned      i;
	unsigned long val;
	unsigned char id[4];

	for (i = 0; i < 4; i++) {
		if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
			return (1);
		}

		id[i] = val & 0xff;
	}

	r = 0;

	for (i = 0; i < 6; i++) {
		r |= fm_enc_byte (ctx, 0, 0xff);
	}

	ctx->crc = 0xffff;

	r |= fm_enc_byte (ctx, 0xfe, 0xc7);

	for (i = 0; i < 4; i++) {
		r |= fm_enc_byte (ctx, id[i], 0xff);
	}

	val = ctx->crc;

	r |= fm_enc_byte (ctx, val >> 8, 0xff);
	r |= fm_enc_byte (ctx, val, 0xff);

	for (i = 0; i < 11; i++) {
		r |= fm_enc_byte (ctx, 0xff, 0xff);
	}

	for (i = 0; i < 6; i++) {
		fm_enc_byte (ctx, 0, 0xff);
	}

	ctx->crc = 0xffff;

	r |= fm_enc_byte (ctx, 0xfb, 0xc7);

	return (r);
}

static
int txt_fm_enc_sync (pri_text_t *ctx)
{
	unsigned i;

	for (i = 0; i < 6; i++) {
		if (fm_enc_byte (ctx, 0x00, 0xff)) {
			return (1);
		}
	}

	return (0);
}

int txt_encode_pri0_fm (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match (&ctx->txt, "AM", 1)) {
		return (fm_enc_am (ctx));
	}
	else if (txt_match (&ctx->txt, "BIT", 1)) {
		return (fm_enc_bit (ctx));
	}
	else if (txt_match (&ctx->txt, "CHECK", 1)) {
		return (fm_enc_check (ctx));
	}
	else if (txt_match (&ctx->txt, "CRC", 1)) {
		return (fm_enc_check_stop (ctx, 1));
	}
	else if (txt_match (&ctx->txt, "DAM", 1)) {
		return (fm_enc_dam (ctx));
	}
	else if (txt_match (&ctx->txt, "EOT", 1)) {
		return (fm_enc_eot (ctx));
	}
	else if (txt_match (&ctx->txt, "FILL", 1)) {
		return (fm_enc_fill (ctx));
	}
	else if (txt_match (&ctx->txt, "GAP", 1)) {
		return (fm_enc_gap (ctx));
	}
	else if (txt_match (&ctx->txt, "IAM", 1)) {
		return (fm_enc_iam (ctx));
	}
	else if (txt_match (&ctx->txt, "IDAM", 1)) {
		return (fm_enc_idam (ctx));
	}
	else if (txt_match (&ctx->txt, "REP", 1)) {
		return (fm_enc_rep (ctx));
	}
	else if (txt_match (&ctx->txt, "SECT", 1)) {
		return (fm_enc_sect (ctx));
	}
	else if (txt_match (&ctx->txt, "SYNC", 1)) {
		return (txt_fm_enc_sync (ctx));
	}
	else if (txt_match_uint (&ctx->txt, 16, &val)) {
		return (fm_enc_hex (ctx, val));
	}

	return (-1);
}
