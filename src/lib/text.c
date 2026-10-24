/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/text.c                                               *
 * Created:     2021-11-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021-2023 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include "text.h"


void txt_init (pce_text_t *ctx, FILE *fp)
{
	memset (ctx, 0, sizeof (pce_text_t));

	ctx->fp = fp;
}

void txt_free (pce_text_t *ctx)
{
}

int txt_putc (pce_text_t *ctx, int c)
{
	return (fputc (c, ctx->fp));
}

int txt_error (pce_text_t *ctx, const char *str)
{
	int c;

	if (str == NULL) {
		str = "unknown";
	}

	c = txt_getc (ctx, 0);

	fprintf (stderr, "pce-text:%u: error (%s), next = %02X\n",
		ctx->line + 1, str, c
	);

	return (1);
}

int txt_getc (pce_text_t *ctx, unsigned idx)
{
	int      c;
	unsigned i;

	if (idx >= sizeof (ctx->buf)) {
		return (EOF);
	}

	for (i = ctx->cnt; i <= idx; i++) {
		if ((c = fgetc (ctx->fp)) == EOF) {
			return (EOF);
		}

		ctx->buf[ctx->cnt++] = c;
	}

	return (ctx->buf[idx]);
}

void txt_skip (pce_text_t *ctx, unsigned cnt)
{
	unsigned i;

	if (cnt >= ctx->cnt) {
		cnt -= ctx->cnt;
		ctx->cnt = 0;

		while (cnt-- > 0) {
			if (fgetc (ctx->fp) == EOF) {
				return;
			}
		}
	}
	else {
		for (i = cnt; i < ctx->cnt; i++) {
			ctx->buf[i - cnt] = ctx->buf[i];
		}

		ctx->cnt -= cnt;
	}
}

int txt_match_space (pce_text_t *ctx)
{
	int c, cr, ok, line;

	line = 0;
	ok = 0;
	cr = 0;

	while (1) {
		if ((c = txt_getc (ctx, 0)) == EOF) {
			return (ok);
		}

		if (c == 0x0d) {
			line = 0;
			ctx->line += 1;
		}
		else if (c == 0x0a) {
			line = 0;

			if (cr == 0) {
				ctx->line += 1;
			}
		}
		else if (line) {
			;
		}
		else if ((c == '\t') || (c == ' ')) {
			;
		}
		else if ((c == '#') || (c == ';')) {
			line = 1;
		}
		else {
			return (ok);
		}

		cr = (c == 0x0d);
		ok = 1;

		txt_skip (ctx, 1);
	}
}

int txt_match_eol (pce_text_t *ctx)
{
	int c, line;

	line = 0;

	while (1) {
		if ((c = txt_getc (ctx, 0)) == EOF) {
			return (1);
		}

		if (c == 0x0d) {
			ctx->line += 1;
			txt_skip (ctx, (txt_getc (ctx, 1) == 0x0a) ? 2 : 1);

			return (1);
		}
		else if (c == 0x0a) {
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
		else if ((c == '#') || (c == ';')) {
			line = 1;
		}
		else {
			return (0);
		}

		txt_skip (ctx, 1);
	}
}

int txt_match (pce_text_t *ctx, const char *str, int skip)
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

int txt_match_string (pce_text_t *ctx, char *str, unsigned max)
{
	int      c, esc, quote;
	unsigned idx;

	txt_match_space (ctx);

	if ((c = txt_getc (ctx, 0)) != '"') {
		return (0);
	}

	quote = '"';

	txt_skip (ctx, 1);

	idx = 0;
	esc = 0;

	while (idx < max) {
		if ((c = txt_getc (ctx, 0)) == EOF) {
			return (0);
		}

		txt_skip (ctx, 1);

		if (esc) {
			idx -= 1;

			if (c == 'n') {
				c = 0x0a;
			}
			else if (c == 'r') {
				c = 0x0d;
			}
			else if (c == 't') {
				c = 0x09;
			}
			else if (c == '\\') {
				c = '\\';
			}
			else if (c == '"') {
				c = '"';
			}
			else {
				idx += 1;
			}

			esc = 0;
		}
		else if (c == '\\') {
			esc = 1;
		}
		else if (c == quote) {
			break;
		}

		str[idx++] = c;
	}

	if (idx >= max) {
		return (0);
	}

	str[idx] = 0;

	return (1);
}

int txt_match_hex_digit (pce_text_t *ctx, unsigned *val)
{
	int c;

	c = txt_getc (ctx, 0);

	if ((c >= '0') && (c <= '9')) {
		*val = c - '0';
	}
	else if ((c >= 'A') && (c <= 'F')) {
		*val = c - 'A' + 10;
	}
	else if ((c >= 'a') && (c <= 'f')) {
		*val = c - 'a' + 10;
	}
	else {
		return (0);
	}

	txt_skip (ctx, 1);

	return (1);
}

int txt_match_uint (pce_text_t *ctx, unsigned base, unsigned long *val)
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

	txt_skip (ctx, i);

	return (1);
}
