/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/text.h                                               *
 * Created:     2021-11-28 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_TEXT_H
#define PCE_LIB_TEXT_H 1


#include <config.h>

#include <stdio.h>


typedef struct {
	FILE           *fp;

	unsigned       cnt;
	char           buf[256];
	unsigned       line;
} pce_text_t;


void txt_init (pce_text_t *ctx, FILE *fp);
void txt_free (pce_text_t *ctx);

int txt_error (pce_text_t *ctx, const char *str);

int txt_putc (pce_text_t *ctx, int c);

int txt_getc (pce_text_t *ctx, unsigned idx);

void txt_skip (pce_text_t *ctx, unsigned cnt);

int txt_match_space (pce_text_t *ctx);
int txt_match_eol (pce_text_t *ctx);
int txt_match (pce_text_t *ctx, const char *str, int skip);
int txt_match_string (pce_text_t *ctx, char *str, unsigned max);
int txt_match_hex_digit (pce_text_t *ctx, unsigned *val);
int txt_match_uint (pce_text_t *ctx, unsigned base, unsigned long *val);


#endif
