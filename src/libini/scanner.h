/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/scanner.h                                         *
 * Created:     2001-03-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef LIBINI_SCANNER_H
#define LIBINI_SCANNER_H 1


#include <stdio.h>


#define SCN_BUF_MAX 256


typedef struct scn_file_t {
	struct scn_file_t *next;
	char              *name;
	FILE              *fp;
	int               del;

	unsigned long     line;
	unsigned long     offset;
} scn_file_t;


/*!***************************************************************************
 * @short The scanner type
 *****************************************************************************/
typedef struct {
	unsigned      cnt;
	unsigned      buf[SCN_BUF_MAX];

	unsigned long line;
	unsigned long offset;

	int           nl;

	scn_file_t    *file;

	const char    *str;
} scanner_t;


void scn_init (scanner_t *scn);
void scn_free (scanner_t *scn);

void scn_set_str (scanner_t *scn, const char *str);

int scn_add_file (scanner_t *scn, const char *fname, FILE *fp, int del);

char scn_get_chr (scanner_t *scn, unsigned idx);
void scn_rmv_chr (scanner_t *scn, unsigned cnt);

unsigned long scn_get_line (const scanner_t *scn);
unsigned long scn_get_offset (const scanner_t *scn);

int scn_match_space (scanner_t *scn);
int scn_match_name (scanner_t *scn, char *str, unsigned max);
int scn_match_string (scanner_t *scn, char *str, unsigned max);
int scn_peek (scanner_t *scn, const char *str);
int scn_match_ident (scanner_t *scn, const char *str);
int scn_match (scanner_t *scn, const char *str);


#endif
