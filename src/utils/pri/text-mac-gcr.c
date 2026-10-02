/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text-mac-gcr.c                                 *
 * Created:     2017-10-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017-2025 Hampa Hug <hampa@hampa.ch>                     *
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
#include <drivers/pri/pri-enc-gcr.h>

#include <lib/text.h>

#include "main.h"
#include "pri-mac-gcr.h"
#include "text.h"


static const unsigned char gcr_enc_tab[64] = {
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static const unsigned char mac_dec_tab[256] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
	0xff, 0xff, 0x02, 0x03, 0xff, 0x04, 0x05, 0x06,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x08,
	0xff, 0xff, 0xff, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
	0xff, 0xff, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
	0xff, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x1b, 0xff, 0x1c, 0x1d, 0x1e,
	0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0x20, 0x21,
	0xff, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x29, 0x2a, 0x2b,
	0xff, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
	0xff, 0xff, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
	0xff, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

static const unsigned char gcr_sync[2] = {
	0x3f, 0xc0
};

static const unsigned char gcr_sync2[3] = {
	0x3f, 0xcf, 0xf0
};

static const unsigned char gcr_sync_group[6] = {
	0xff, 0x3f, 0xcf, 0xf3, 0xfc, 0xff
};


static
void mac_put_nl (pri_text_t *ctx, int force)
{
	if ((ctx->column > 0) || force) {
		fputc ('\n', ctx->txt.fp);
	}

	ctx->column = 0;
}

static
void mac_put_byte (pri_text_t *ctx, unsigned val)
{
	if (ctx->column > 0) {
		if (ctx->column >= 16) {
			fputc ('\n', ctx->txt.fp);
			ctx->column = 0;
		}
		else {
			fputc (' ', ctx->txt.fp);
		}
	}

	fprintf (ctx->txt.fp, "%02X", val & 0xff);

	ctx->column += 1;
}

static
void mac_flush (pri_text_t *ctx)
{
	while (ctx->shift_cnt >= 8) {
		mac_put_byte (ctx, ctx->shift >> (ctx->shift_cnt - 8));
		ctx->shift_cnt -= 8;
	}

	if (ctx->shift_cnt > 0) {
		txt_dec_bits (ctx, ctx->shift_cnt);
	}
}

static
void mac_dec_events (pri_text_t *ctx)
{
	unsigned long type, val;

	while (pri_trk_get_event (ctx->trk, &type, &val) == 0) {
		mac_flush (ctx);
		txt_dec_event (ctx, type, val);
	}
}

static
int mac_dec_event_occurred (pri_text_t *ctx, pri_evt_t *evt)
{
	pri_evt_t *tmp;

	if (ctx->trk->wrap) {
		return (1);
	}

	tmp = ctx->trk->cur_evt;

	while ((tmp != NULL) && (tmp->pos < ctx->trk->idx)) {
		tmp = tmp->next;
	}

	if (tmp != evt) {
		return (1);
	}

	return (0);
}

int mac_decode_sync (pri_text_t *ctx)
{
	unsigned group_cnt, sync_cnt;

	do {
		group_cnt = 0;
		sync_cnt = 0;

		mac_dec_events (ctx);

		while (txt_dec_match (ctx, gcr_sync_group, 48) == 0) {
			group_cnt += 1;
		}

		if (group_cnt > 0) {
			mac_flush (ctx);
			mac_put_nl (ctx, 0);
			fprintf (ctx->txt.fp, "SYNC GROUP %u\n", group_cnt);
		}

		mac_dec_events (ctx);

		if (txt_dec_match (ctx, gcr_sync2, 20) == 0) {
			sync_cnt += 2;

			while (txt_dec_match (ctx, gcr_sync, 10) == 0) {
				sync_cnt += 1;
			}

			mac_flush (ctx);
			mac_put_nl (ctx, 0);
			fprintf (ctx->txt.fp, "SYNC %u\n", sync_cnt);
		}
	} while ((group_cnt > 0) || (sync_cnt > 0));

	return (0);
}

static
int mac_decode_id (pri_text_t *ctx)
{
	unsigned       i;
	unsigned long  val;
	unsigned       c, h;
	unsigned char  buf[8];
	unsigned char  dec[8];
	pri_text_pos_t pos;

	mac_dec_events (ctx);

	txt_save_pos (ctx, &pos);

	for (i = 0; i < 8; i++) {
		pri_trk_get_bits (ctx->trk, &val, 8);
		buf[i] = val;
		dec[i] = mac_dec_tab[val & 0xff];
	}

	if (mac_dec_event_occurred (ctx, pos.evt)) {
		txt_restore_pos (ctx, &pos);
		return (0);
	}

	c = (dec[0] & 0x3f) | ((dec[2] & 0x1f) << 6);
	h = (dec[2] >> 5) & 1;

	fprintf (ctx->txt.fp,
		"  %02X %02X %02X %02X %02X  %02X %02X %02X"
		"\t\t; SECT %02X %02X %02X %02X\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
		c, h, dec[1], dec[3]
	);

	ctx->s = dec[1];

	ctx->column = 0;

	return (0);
}

static
int mac_check_data (const unsigned char *buf, unsigned size)
{
	unsigned      i;
	unsigned char val;

	val = buf[0];

	for (i = 1 ; i < size; i++) {
		if (buf[i] != val) {
			return (0);
		}
	}

	return (1);
}

static
int mac_decode_end (pri_text_t *ctx)
{
	if (txt_dec_match (ctx, "\xde\xaa", 16)) {
		return (0);
	}

	mac_put_nl (ctx, 0);
	mac_put_byte (ctx, 0xde);
	mac_put_byte (ctx, 0xaa);

	if (txt_dec_match (ctx, "\xff", 8) == 0) {
		mac_put_byte (ctx, 0xff);
		mac_put_nl (ctx, 0);
	}

	mac_put_nl (ctx, 1);

	return (0);
}

static
int mac_decode_data_nibbles (pri_text_t *ctx)
{
	unsigned       i, j;
	unsigned long  val;
	unsigned char  s;
	unsigned char  buf[699];
	unsigned char  chk[4];
	pri_text_pos_t pos;

	mac_dec_events (ctx);

	txt_save_pos (ctx, &pos);

	pri_trk_get_bits (ctx->trk, &val, 8);
	s = val & 0xff;

	for (i = 0; i < 699; i++) {
		pri_trk_get_bits (ctx->trk, &val, 8);
		buf[i] = val & 0xff;
	}

	for (i = 0; i < 4; i++) {
		pri_trk_get_bits (ctx->trk, &val, 8);
		chk[i] = val & 0xff;
	}

	if (mac_dec_event_occurred (ctx, pos.evt)) {
		txt_restore_pos (ctx, &pos);
		return (1);
	}

	fprintf (ctx->txt.fp, " %02X\n", s);
	fprintf (ctx->txt.fp, ";CHECK START\n");

	for (i = 0; i < 699; i++) {
		j = i & 15;

		if (j > 0) {
			fputc (' ', ctx->txt.fp);
		}

		fprintf (ctx->txt.fp, "%02X", buf[i]);

		if (j == 15) {
			fputc ('\n', ctx->txt.fp);
		}
	}

	fprintf (ctx->txt.fp, "\n;CHECK END\n");
	fprintf (ctx->txt.fp, "%02X %02X %02X %02X\n",
		chk[0], chk[1], chk[2], chk[3]
	);

	ctx->column = 0;

	return (0);
}

static
int mac_decode_data_check (pri_text_t *ctx)
{
	unsigned       i, j;
	unsigned long  val, high, dec;
	unsigned char  s;
	unsigned char  chk[4];
	unsigned char  buf[527];
	pri_text_pos_t pos;

	mac_dec_events (ctx);

	txt_save_pos (ctx, &pos);

	pri_trk_get_bits (ctx->trk, &val, 8);
	s = val & 0xff;

	for (i = 0; i < 524; i++) {
		if ((i % 3) == 0) {
			pri_trk_get_bits (ctx->trk, &high, 8);

			dec = mac_dec_tab[high & 0xff];

			if (dec > 64) {
				fprintf (stderr,
					"bad nibble %02lX %lu/%lu/%u at %u\n",
					high, ctx->c, ctx->h, ctx->s, i
				);
				txt_restore_pos (ctx, &pos);
				return (1);
			}

			high = (dec & 0x3f) << 2;
		}

		pri_trk_get_bits (ctx->trk, &val, 8);

		dec = mac_dec_tab[val & 0xff];

		if (dec > 64) {
			fprintf (stderr, "bad nibble %02lX %lu/%lu/%u at %u\n",
				val, ctx->c, ctx->h, ctx->s, i
			);
			txt_restore_pos (ctx, &pos);
			return (1);
		}

		val = (dec & 0x3f) | (high & 0xc0);
		high <<= 2;

		buf[i] = val & 0xff;
	}

	for (i = 0; i < 4; i++) {
		pri_trk_get_bits (ctx->trk, &val, 8);
		chk[i] = val & 0xff;
	}

	if (mac_dec_event_occurred (ctx, pos.evt)) {
		txt_restore_pos (ctx, &pos);
		return (1);
	}

	pri_mac_gcr_checksum (buf, buf, 0);

	fprintf (ctx->txt.fp, " %02X\n", s);
	fprintf (ctx->txt.fp, "CHECK START\n");
	fprintf (ctx->txt.fp, "%02X", buf[0]);

	for (i = 1; i < 12; i++) {
		fprintf (ctx->txt.fp, " %02X", buf[i]);
	}

	fputc ('\n', ctx->txt.fp);

	if (mac_check_data (buf + 12, 512)) {
		fprintf (ctx->txt.fp, "REP 512 %02X\n", buf[12]);
	}
	else {
		for (i = 0; i < 512; i++) {
			j = i & 15;

			if (j > 0) {
				fputc (' ', ctx->txt.fp);
			}

			fprintf (ctx->txt.fp, "%02X", buf[i + 12]);

			if (j == 15) {
				fputc ('\n', ctx->txt.fp);
			}
		}
	}

	fprintf (ctx->txt.fp, "CHECK END\n");
	fprintf (ctx->txt.fp, "%02X %02X %02X %02X\n",
		chk[0], chk[1], chk[2], chk[3]
	);

	ctx->column = 0;

	return (0);
}

static
int mac_decode_data (pri_text_t *ctx)
{
	if (mac_decode_data_check (ctx) == 0) {
		return (0);
	}

	if (mac_decode_data_nibbles (ctx) == 0) {
		return (0);
	}

	return (0);
}

static
int mac_dec_rotate (pri_text_t *ctx, unsigned long *idx)
{
	unsigned long pos;
	long          rot;
	pri_trk_t     *trk;

	pri_mac_align_pos (ctx->trk, ctx->mac_align, &pos);

	if (pos == 0) {
		return (0);
	}

	if ((trk = pri_trk_clone (ctx->trk)) == NULL) {
		return (1);
	}

	ctx->trk = trk;
	ctx->free_track = 1;

	rot = (long) trk->size - (long) pos;

	fprintf (ctx->txt.fp, "ROTATE %ld\t; %lu - %lu\n\n", rot, trk->size, pos);

	pri_trk_rotate (trk, pos);

	if (idx != NULL) {
		*idx = (trk->size - pos) % trk->size;
	}

	return (0);
}

int txt_mac_dec_track (pri_text_t *ctx)
{
	unsigned long bit;
	unsigned char buf[3];

	fprintf (ctx->txt.fp, "\nTRACK %lu %lu\n\n", ctx->c, ctx->h);
	fprintf (ctx->txt.fp, "MODE MAC-GCR\n");
	fprintf (ctx->txt.fp, "RATE %lu\n\n", pri_trk_get_clock (ctx->trk));

	ctx->shift = 0;
	ctx->shift_cnt = 0;
	ctx->last_val = 0;

	ctx->column = 0;
	ctx->need_nl = 0;

	if (ctx->mac_align != PRI_MAC_ALIGN_NONE) {
		if (mac_dec_rotate (ctx, NULL)) {
			return (1);
		}
	}

	pri_trk_set_pos (ctx->trk, 0);

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;

	while (ctx->trk->wrap == 0) {
		if (mac_decode_sync (ctx)) {
			return (1);
		}

		if (ctx->trk->wrap) {
			break;
		}

		mac_dec_events (ctx);

		pri_trk_get_bits (ctx->trk, &bit, 1);

		ctx->shift = (ctx->shift << 1) | (bit & 1);
		ctx->shift_cnt += 1;

		if (ctx->shift_cnt < 8) {
			continue;
		}

		if (ctx->shift_cnt >= 16) {
			mac_put_byte (ctx, ctx->shift >> 8);
			ctx->shift_cnt -= 8;
			buf[2] = 0;
		}

		if ((ctx->shift & 0x80) || ctx->mac_no_slip) {
			if (ctx->shift_cnt > 8) {
				txt_dec_bits (ctx, ctx->shift_cnt - 8);
			}

			mac_put_byte (ctx, ctx->shift & 0xff);

			buf[0] = buf[1];
			buf[1] = buf[2];
			buf[2] = ctx->shift & 0xff;

			if ((buf[0] == 0xd5) && (buf[1] == 0xaa) && (buf[2] == 0x96)) {
				mac_decode_id (ctx);
			}
			else if ((buf[0] == 0xd5) && (buf[1] == 0xaa) && (buf[2] == 0xad)) {
				mac_decode_data (ctx);
				mac_decode_end (ctx);
				ctx->s = 0;
			}
			else if ((buf[0] == 0xde) && (buf[1] == 0xaa) && (buf[2] == 0xff)) {
				mac_put_nl (ctx, 0);
				mac_put_nl (ctx, 1);
			}

			ctx->shift = 0;
			ctx->shift_cnt = 0;
		}
	}

	mac_flush (ctx);
	mac_put_nl (ctx, 0);

	mac_dec_events (ctx);

	return (0);
}


static
int mac_enc_triple (pri_text_t *ctx, unsigned v1, unsigned v2, unsigned v3, unsigned cnt)
{
	unsigned char hi;

	if (cnt == 0) {
		return (0);
	}

	hi = ((v1 & 0xc0) >> 2) | ((v2 & 0xc0) >> 4) | ((v3 & 0xc0) >> 6);

	if (txt_enc_bits_raw (ctx, gcr_enc_tab[hi & 0x3f], 8)) {
		return (1);
	}

	if (txt_enc_bits_raw (ctx, gcr_enc_tab[v1 & 0x3f], 8)) {
		return (1);
	}

	if (cnt >= 2) {
		if (txt_enc_bits_raw (ctx, gcr_enc_tab[v2 & 0x3f], 8)) {
			return (1);
		}
	}

	if (cnt >= 3) {
		if (txt_enc_bits_raw (ctx, gcr_enc_tab[v3 & 0x3f], 8)) {
			return (1);
		}
	}

	return (0);
}

static
void mac_check_init (pri_text_t *ctx)
{
	ctx->mac_check_cnt = 0;

	ctx->mac_check_buf[0] = 0;
	ctx->mac_check_buf[1] = 0;
	ctx->mac_check_buf[2] = 0;
}

static
int mac_enc_bit (pri_text_t *ctx)
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
int mac_enc_byte (pri_text_t *ctx, unsigned char val)
{
	unsigned short tmp;
	unsigned short *chk;
	unsigned char  *buf;

	if (ctx->trk == NULL) {
		return (1);
	}

	if (ctx->mac_check_active == 0) {
		if (txt_enc_bits_raw (ctx, val, 8)) {
			return (1);
		}

		return (0);
	}

	chk = ctx->mac_check_chk;
	buf = ctx->mac_check_buf;

	if (ctx->mac_check_cnt == 0) {
		chk[0] = ((chk[0] << 1) | ((chk[0] >> 7) & 1)) & 0x1ff;
	}

	tmp = chk[2] + (val & 0xff) + ((chk[0] >> 8) & 1);
	chk[2] = chk[1];
	chk[1] = chk[0] & 0xff;
	chk[0] = tmp;

	val ^= chk[1];

	buf[ctx->mac_check_cnt++] = val & 0xff;

	if (ctx->mac_check_cnt >= 3) {
		if (mac_enc_triple (ctx, buf[0], buf[1], buf[2], 3)) {
			return (1);
		}

		mac_check_init (ctx);
	}

	return (0);
}

static
int mac_enc_check_start (pri_text_t *ctx)
{
	ctx->mac_check_active = 1;

	ctx->mac_check_chk[0] = 0;
	ctx->mac_check_chk[1] = 0;
	ctx->mac_check_chk[2] = 0;

	mac_check_init (ctx);

	return (0);
}

static
int mac_enc_check_stop (pri_text_t *ctx, int sum)
{
	unsigned char  *buf;
	unsigned short *chk;

	if (ctx->mac_check_active == 0) {
		return (1);
	}

	buf = ctx->mac_check_buf;
	chk = ctx->mac_check_chk;

	if (ctx->mac_check_cnt > 0) {
		if (mac_enc_triple (ctx, buf[0], buf[1], buf[2], ctx->mac_check_cnt)) {
			return (1);
		}
	}

	ctx->mac_check_active = 0;

	if (sum) {
		if (mac_enc_triple (ctx, chk[1], chk[0], chk[2], 3)) {
			return (1);
		}
	}

	return (0);
}

static
int mac_enc_check (pri_text_t *ctx)
{
	if (txt_match (&ctx->txt, "START", 1)) {
		return (mac_enc_check_start (ctx));
	}

	if (txt_match (&ctx->txt, "END", 1)) {
		return (mac_enc_check_stop (ctx, 0));
	}

	if (txt_match (&ctx->txt, "SUM", 1)) {
		return (mac_enc_check_stop (ctx, 3));
	}

	return (1);
}

static
int mac_enc_fill_pattern (pri_text_t *ctx, unsigned long max, unsigned long val, unsigned size)
{
	unsigned long cnt;

	while (ctx->bit_cnt < max) {
		cnt = max - ctx->bit_cnt;

		if (cnt < size) {
			val >>= (size - cnt);
		}
		else {
			cnt = size;
		}

		if (txt_enc_bits_raw (ctx, val, cnt)) {
			return (1);
		}
	}

	return (0);
}

static
int mac_enc_fill_sync (pri_text_t *ctx, unsigned long max)
{
	unsigned n;

	while (ctx->bit_cnt < max) {
		n = (max - ctx->bit_cnt) % 10;

		if (n == 0) {
			n = 10;
		}

		if (txt_enc_bits_raw (ctx, 0xff, n)) {
			return (1);
		}

	}

	return (0);
}

static
int mac_enc_fill_sync_group (pri_text_t *ctx, unsigned long max)
{
	unsigned n;

	while (ctx->bit_cnt < max) {
		n = (max - ctx->bit_cnt) % 48;

		if (n == 0) {
			n = 48;
		}

		if (n > 32) {
			if (txt_enc_bits_raw (ctx, 0xff3f, n - 32)) {
				return (1);
			}

			n = 32;
		}

		if (txt_enc_bits_raw (ctx, 0xcff3fcff, n)) {
			return (1);
		}

	}

	return (0);
}

static
int mac_enc_fill (pri_text_t *ctx)
{
	unsigned long max, val, bits;

	if (txt_match (&ctx->txt, "TRACK", 1)) {
		max = pri_get_mac_gcr_track_length (ctx->c);
	}
	else {
		if (txt_match_uint (&ctx->txt, 10, &max) == 0) {
			return (1);
		}
	}

	if (txt_match (&ctx->txt, "SYNC", 1)) {
		if (txt_match (&ctx->txt, "GROUP", 1)) {
			return (mac_enc_fill_sync_group (ctx, max));
		}

		if (mac_enc_fill_sync (ctx, max)) {
			return (1);
		}

		return (0);
	}

	if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
		return (1);
	}

	if (txt_match (&ctx->txt, "/", 1)) {
		if (txt_match_uint (&ctx->txt, 10, &bits) == 0) {
			return (1);
		}
	}
	else {
		bits = 8;
	}

	if (mac_enc_fill_pattern (ctx, max, val, bits)) {
		return (1);
	}

	return (0);
}

static
int mac_enc_eots (pri_text_t *ctx)
{
	unsigned long max;

	max = pri_get_mac_gcr_track_length (ctx->c);

	if (mac_enc_fill_sync (ctx, max)) {
		return (1);
	}

	return (0);
}

static
int mac_enc_eotg (pri_text_t *ctx)
{
	unsigned long max;

	max = pri_get_mac_gcr_track_length (ctx->c);

	if (mac_enc_fill_sync_group (ctx, max)) {
		return (1);
	}

	return (0);
}

static
int mac_enc_hex (pri_text_t *ctx, unsigned val, unsigned cnt)
{
	unsigned long bits;

	if (txt_match (&ctx->txt, "/", 1)) {
		if (txt_match_uint (&ctx->txt, 10, &bits) == 0) {
			return (1);
		}

		while (cnt-- > 0) {
			if (txt_enc_bits_raw (ctx, val, bits)) {
				return (1);
			}
		}
	}
	else {
		if (ctx->mac_nibble) {
			if (val > 63) {
				return (1);
			}

			val = gcr_enc_tab[val];
		}

		while (cnt-- > 0) {
			if (mac_enc_byte (ctx, val)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int mac_enc_nibble_start (pri_text_t *ctx)
{
	if (ctx->mac_nibble) {
		return (1);
	}

	ctx->mac_nibble = 1;

	return (0);
}

static
int mac_enc_nibble_stop (pri_text_t *ctx)
{
	if (ctx->mac_nibble == 0) {
		return (1);
	}

	ctx->mac_nibble = 0;

	return (0);
}

static
int mac_enc_rep (pri_text_t *ctx)
{
	unsigned long cnt;
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 16, &val)) {
		return (mac_enc_hex (ctx, val, cnt));
	}

	return (1);
}

static
int mac_enc_sect_end (pri_text_t *ctx)
{
	if (mac_enc_check_stop (ctx, 1)) {
		return (1);
	}

	if (txt_enc_bits_raw (ctx, 0xdeaaff, 24)) {
		return (1);
	}

	return (0);
}

static
int mac_enc_sect (pri_text_t *ctx)
{
	unsigned      i;
	unsigned long val;
	unsigned char buf[4];
	unsigned char enc[5];

	if (txt_match (&ctx->txt, "END", 1)) {
		return (mac_enc_sect_end (ctx));
	}

	for (i = 0; i < 4; i++) {
		if (txt_match_uint (&ctx->txt, 16, &val) == 0) {
			return (1);
		}

		buf[i] = val & 0xff;
	}

	if (txt_enc_bits_raw (ctx, 0xd5aa96, 24)) {
		return (1);
	}

	enc[0] = buf[0] & 0x3f;
	enc[1] = buf[2] & 0x3f;
	enc[2] = ((buf[1] << 5) & 0x20) | ((buf[0] >> 6) & 0x1f);
	enc[3] = buf[3] & 0x3f;
	enc[4] = enc[0] ^ enc[1] ^ enc[2] ^ enc[3];

	for (i = 0; i < 5; i++) {
		if (txt_enc_bits_raw (ctx, gcr_enc_tab[enc[i] & 0x3f], 8)) {
			return (1);
		}
	}

	if (txt_enc_bits_raw (ctx, 0xdeaaff, 24)) {
		return (1);
	}

	if (txt_enc_bits_raw (ctx, 0xff, 8)) {
		return (1);
	}

	for (i = 0; i < 4; i++) {
		if (txt_enc_bits_raw (ctx, 0xff, 10)) {
			return (1);
		}
	}

	if (txt_enc_bits_raw (ctx, 0xd5aaad, 24)) {
		return (1);
	}

	if (txt_enc_bits_raw (ctx, gcr_enc_tab[enc[1] & 0x3f], 8)) {
		return (1);
	}

	if (mac_enc_check_start (ctx)) {
		return (1);
	}

	return (0);
}

static
int mac_enc_sync_group (pri_text_t *ctx)
{
	unsigned long cnt;

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	while (cnt > 0) {
		if (txt_enc_bits_raw (ctx, 0xff3fcf, 24)) {
			return (1);
		}

		if (txt_enc_bits_raw (ctx, 0xf3fcff, 24)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int mac_enc_sync (pri_text_t *ctx)
{
	unsigned long cnt;

	if (txt_match (&ctx->txt, "GROUP", 1)) {
		return (mac_enc_sync_group (ctx));
	}

	if (txt_match_uint (&ctx->txt, 10, &cnt) == 0) {
		return (1);
	}

	while (cnt > 0) {
		if (txt_enc_bits_raw (ctx, 0xff, 10)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

int txt_encode_pri0_mac (pri_text_t *ctx)
{
	unsigned long val;

	if (txt_match (&ctx->txt, "BIT", 1)) {
		return (mac_enc_bit (ctx));
	}
	else if (txt_match (&ctx->txt, "CHECK", 1)) {
		return (mac_enc_check (ctx));
	}
	else if (txt_match (&ctx->txt, "EOTG", 1)) {
		return (mac_enc_eotg (ctx));
	}
	else if (txt_match (&ctx->txt, "EOTS", 1)) {
		return (mac_enc_eots (ctx));
	}
	else if (txt_match (&ctx->txt, "EOT", 1)) {
		return (mac_enc_eotg (ctx));
	}
	else if (txt_match (&ctx->txt, "FILL", 1)) {
		return (mac_enc_fill (ctx));
	}
	else if (txt_match (&ctx->txt, "REP", 1)) {
		return (mac_enc_rep (ctx));
	}
	else if (txt_match (&ctx->txt, "SECT", 1)) {
		return (mac_enc_sect (ctx));
	}
	else if (txt_match (&ctx->txt, "SYNC", 1)) {
		return (mac_enc_sync (ctx));
	}
	else if (txt_match (&ctx->txt, "<", 1)) {
		return (mac_enc_nibble_start (ctx));
	}
	else if (txt_match (&ctx->txt, ">", 1)) {
		return (mac_enc_nibble_stop (ctx));
	}
	else if (txt_match_uint (&ctx->txt, 16, &val)) {
		return (mac_enc_hex (ctx, val, 1));
	}

	return (-1);
}
