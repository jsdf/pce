/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/text.c                                         *
 * Created:     2012-01-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2025 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdio.h>
#include <string.h>

#include <lib/text.h>

#include <drivers/pfi/pfi.h>


typedef struct {
	pce_text_t    txt;
	pfi_img_t     *img;
	pfi_trk_t     *trk;
	unsigned long version;
	unsigned long pos;
	char          add;
} pfi_text_t;


static
int pfi_decode_text_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      col, row, idx;
	uint32_t      val, ofs;
	unsigned long total, index;
	FILE          *fp;

	fp = opaque;

	fprintf (fp, "TRACK %lu %lu\n", c, h);
	fprintf (fp, "CLOCK %lu\n\n", pfi_trk_get_clock (trk));

	pfi_trk_rewind (trk);

	col = 0;
	row = 0;
	idx = 0;

	total = 0;
	index = 0;

	while (pfi_trk_get_pulse (trk, &val, &ofs) == 0) {
		if ((val == 0) || (ofs < val)) {
			if (col > 0) {
				fputc ('\n', fp);
				col = 0;
			}

			idx += 1;
			row = 0;

			if (trk->index_cnt > idx) {
				fprintf (fp, "\n; Revolution %u", idx);
			}

			fprintf (fp, "\n; %lu + %lu", total, (unsigned long) ofs);
			fprintf (fp, "\nINDEX %lu\n\n", (unsigned long) ofs);

			index = total + ofs;
		}

		if (val > 0) {
			total += val;

			if (col > 0) {
				fputc (' ', fp);
			}

			fprintf (fp, "%3lu", (unsigned long) val);

			if (++col >= 16) {
				if (++row >= 8) {
					fprintf (fp, "\t\t; %lu + %lu",
						index, total - index
					);
					row = 0;
				}
				fputc ('\n', fp);
				col = 0;
			}
		}
	}

	if (col > 0) {
		fputc ('\n', fp);
	}

	return (0);
}

int pfi_decode_text (pfi_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "w")) == NULL) {
		return (1);
	}

	fprintf (fp, "PFI 0\n\n");

	r = pfi_for_all_tracks (img, pfi_decode_text_cb, fp);

	fclose (fp);

	return (r);
}


static
int txt_enc_clock (pfi_text_t *ctx)
{
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 10, &val) == 0) {
		return (1);
	}

	if (ctx->trk != NULL) {
		pfi_trk_set_clock (ctx->trk, val);
	}

	return (0);
}

static
int txt_enc_index (pfi_text_t *ctx)
{
	unsigned long val;

	if (txt_match_uint (&ctx->txt, 10, &val) == 0) {
		return (1);
	}

	if (ctx->trk == NULL) {
		return (1);
	}

	if (pfi_trk_add_index (ctx->trk, ctx->pos + val)) {
		return (1);
	}

	return (0);
}

static
int txt_enc_text (pfi_text_t *ctx)
{
	unsigned      cnt;
	char          str[256];

	if (txt_match (&ctx->txt, "INIT", 1)) {
		pfi_img_set_comment (ctx->img, NULL, 0);
		return (0);
	}

	if (txt_match_string (&ctx->txt, str, 256) == 0) {
		return (1);
	}

	cnt = strlen (str);

	if (cnt < 256) {
		str[cnt++] = 0x0a;
	}

	if (pfi_img_add_comment (ctx->img, (unsigned char *) str, cnt)) {
		return (1);
	}

	return (0);
}

static
int txt_enc_track (pfi_text_t *ctx)
{
	unsigned long c, h;

	if (txt_match_uint (&ctx->txt, 10, &c) == 0) {
		return (1);
	}

	if (txt_match_uint (&ctx->txt, 10, &h) == 0) {
		return (1);
	}

	pfi_img_del_track (ctx->img, c, h);

	if ((ctx->trk = pfi_img_get_track (ctx->img, c, h, 1)) == NULL) {
		return (1);
	}

	ctx->pos = 0;

	return (0);
}

static
int txt_enc_pulse (pfi_text_t *ctx, unsigned base)
{
	unsigned long val;

	if (txt_match_uint (&ctx->txt, base, &val) == 0) {
		return (1);
	}

	if (ctx->trk == NULL) {
		return (1);
	}

	if (ctx->add) {
		if (pfi_trk_inc_pulse (ctx->trk, val)) {
			return (1);
		}
	}
	else {
		if (pfi_trk_add_pulse (ctx->trk, val)) {
			return (1);
		}
	}

	ctx->add = 0;
	ctx->pos += val;

	return (0);
}

static
int txt_encode (pfi_text_t *ctx)
{
	pce_text_t *txt;

	ctx->trk = NULL;
	ctx->version = 0;
	ctx->pos = 0;
	ctx->add = 0;

	txt = &ctx->txt;

	if (txt_match (txt, "PFI", 1)) {
		if (txt_match_uint (txt, 10, &ctx->version) == 0) {
			return (1);
		}
	}

	while (1) {
		txt_match_space (txt);

		if (feof (txt->fp)) {
			return (0);
		}

		if (txt_match (txt, "CLOCK", 1)) {
			if (txt_enc_clock (ctx)) {
				return (1);
			}
		}
		else if (txt_match (txt, "END", 1)) {
			return (0);
		}
		else if (txt_match (txt, "INDEX", 1)) {
			if (txt_enc_index (ctx)) {
				return (1);
			}
		}
		else if (txt_match (txt, "PFI", 1)) {
			if (txt_match_uint (txt, 10, &ctx->version) == 0) {
				return (1);
			}

			ctx->trk = NULL;
			ctx->pos = 0;
			ctx->add = 0;
		}
		else if (txt_match (txt, "TEXT", 1)) {
			if (txt_enc_text (ctx)) {
				return (1);
			}
		}
		else if (txt_match (txt, "TRACK", 1)) {
			if (txt_enc_track (ctx)) {
				return (1);
			}
		}
		else if (txt_match (txt, "+", 1)) {
			ctx->add = 1;
		}
		else if (txt_match (txt, "$", 1)) {
			if (txt_enc_pulse (ctx, 16)) {
				return (1);
			}
		}
		else {
			if (txt_enc_pulse (ctx, 10)) {
				return (1);
			}
		}
	}
}

int pfi_encode_text (pfi_img_t *img, const char *fname)
{
	int        r;
	FILE       *fp;
	pfi_text_t ctx;

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	txt_init (&ctx.txt, fp);

	ctx.img = img;
	ctx.trk = NULL;
	ctx.pos = 0;
	ctx.add = 0;

	r = txt_encode (&ctx);

	if (r) {
		txt_error (&ctx.txt, "PFI");
	}

	txt_free (&ctx.txt);
	fclose (fp);

	return (r);
}
