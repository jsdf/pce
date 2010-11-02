/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/read.c                                            *
 * Created:     2001-08-24 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdio.h>
#include <string.h>

#include <libini/libini.h>
#include <libini/scanner.h>


extern int ini_eval (scanner_t *scn, ini_sct_t *sct, ini_val_t *val);

static int parse_section (scanner_t *scn, ini_sct_t *sct, char *buf);


static
void parse_error (scanner_t *scn, const char *text, int src)
{
	char       c;
	unsigned   i;
	const char *name;

	if (scn->file != NULL) {
		name = scn->file->name;
	}
	else {
		name = NULL;
	}

	if (name == NULL) {
		name = "<none>";
	}

	fprintf (stderr, "%s:%lu: %s",
		name, scn->line + 1, text
	);

	if (src) {
		fputs (": ", stderr);

		for (i = 0; i < 64; i++) {
			c = scn_get_chr (scn, i);

			if (c == 0) {
				fputs ("<eof>", stderr);
				break;
			}

			if ((c == 0x0d) || (c == 0x0a)) {
				fputs ("<nl>", stderr);
				break;
			}

			fputc (c, stderr);
		}
	}

	fputs ("\n", stderr);
}

static
int parse_block (scanner_t *scn, ini_sct_t *sct, ini_sct_t *sub, char *buf)
{
	int r, s;

	if (scn_match (scn, "{") == 0) {
		return (1);
	}

	if (sub == NULL) {
		sub = ini_sct_new (NULL);

		if (sub == NULL) {
			return (1);
		}

		sub->parent = sct;

		s = 1;
	}
	else {
		s = 0;
	}


	r = parse_section (scn, sub, buf);

	if (s) {
		ini_sct_del (sub);
	}

	if (r) {
		return (1);
	}

	if (scn_match (scn, "}") == 0) {
		return (1);
	}

	return (0);
}

static
int parse_assign_undef (scanner_t *scn, ini_sct_t *sct, ini_val_t *tmp, const char *name)
{
	ini_val_t *val;

	if (ini_eval (scn, sct, tmp)) {
		return (1);
	}

	if (tmp->type == INI_VAL_NONE) {
		return (1);
	}

	val = ini_get_val (sct, name, 0);

	if (val != NULL) {
		return (0);
	}

	val = ini_get_val (sct, name, 1);

	if (val == NULL) {
		return (1);
	}

	ini_val_copy (val, tmp);

	return (0);
}

static
int parse_if (scanner_t *scn, ini_sct_t *sct, char *buf)
{
	int       done;
	ini_sct_t *sub;
	ini_val_t val;

	ini_val_init (&val, NULL);

	if (ini_eval (scn, sct, &val)) {
		ini_val_free (&val);
		return (1);
	}

	if ((val.type == INI_VAL_INT) && (val.val.u32 != 0)) {
		sub = sct;
		done = 1;
	}
	else {
		sub = NULL;
		done = 0;
	}

	ini_val_free (&val);

	if (parse_block (scn, sct, sub, buf)) {
		return (1);
	}

	while (scn_match_ident (scn, "else")) {
		if (scn_match_ident (scn, "if")) {
			ini_val_init (&val, NULL);

			if (ini_eval (scn, sct, &val)) {
				ini_val_free (&val);
				return (1);
			}

			if ((val.type == INI_VAL_INT) && (val.val.u32 != 0)) {
				sub = done ? NULL : sct;
			}
			else {
				sub = NULL;
			}

			if (sub != NULL) {
				done = 1;
			}

			ini_val_free (&val);

			if (parse_block (scn, sct, sub, buf)) {
				return (1);
			}
		}
		else {
			sub = done ? NULL : sct;

			if (parse_block (scn, sct, sub, buf)) {
				return (1);
			}

			break;
		}
	}

	return (0);
}

static
int parse_section (scanner_t *scn, ini_sct_t *sct, char *buf)
{
	int       r;
	ini_sct_t *sub;
	ini_val_t *val;

	while (1) {
		if (scn_match_name (scn, buf, 256) == 0) {
			return (0);
		}

		if (strcmp (buf, "section") == 0) {
			if (scn_match_name (scn, buf, 256) == 0) {
				return (1);
			}
		}

		if (scn_match (scn, "{")) {
			sub = ini_get_sct (sct, buf, 1);

			if (sub == NULL) {
				return (1);
			}

			if (parse_section (scn, sub, buf)) {
				return (1);
			}

			if (scn_match (scn, "}") == 0) {
				return (1);
			}
		}
		else if (scn_match (scn, "?=")) {
			ini_val_t tmp;

			ini_val_init (&tmp, NULL);

			r = parse_assign_undef (scn, sct, &tmp, buf);

			ini_val_free (&tmp);

			if (r) {
				return (1);
			}
		}
		else if (scn_match (scn, "=")) {
			val = ini_get_val (sct, buf, 1);

			if (val == NULL) {
				return (1);
			}

			if (ini_eval (scn, sct, val)) {
				return (1);
			}

			if (val->type == INI_VAL_NONE) {
				return (1);
			}
		}
		else if (strcmp (buf, "if") == 0) {
			if (parse_if (scn, sct, buf)) {
				return (1);
			}
		}
		else if (strcmp (buf, "include") == 0) {
			if (scn_match_string (scn, buf, 256) == 0) {
				return (1);
			}

			if (scn_add_file (scn, buf, NULL, 1)) {
				parse_error (scn, "can't open include file:", 0);
				parse_error (scn, buf, 0);
				return (1);
			}
		}
		else {
			return (1);
		}

		scn_match (scn, ";");
	}

	return (1);
}

int ini_read_str (ini_sct_t *sct, const char *str)
{
	scanner_t scn;
	char      buf[256];

	scn_init (&scn);
	scn_set_str (&scn, str);

	if (parse_section (&scn, sct, buf)) {
		parse_error (&scn, "parse error before", 1);
		scn_free (&scn);
		return (1);
	}

	if (scn_get_chr (&scn, 0) != 0) {
		parse_error (&scn, "parse error before", 1);
		scn_free (&scn);
		return (1);
	}

	scn_free (&scn);

	return (0);
}

int ini_read_fp (ini_sct_t *sct, FILE *fp, const char *fname)
{
	scanner_t scn;
	char      buf[256];

	scn_init (&scn);

	if (scn_add_file (&scn, fname, fp, 0)) {
		ini_sct_del (sct);
		return (1);
	}

	if (parse_section (&scn, sct, buf)) {
		parse_error (&scn, "parse error before", 1);
		scn_free (&scn);
		return (1);
	}

	if (scn_get_chr (&scn, 0) != 0) {
		parse_error (&scn, "parse error before", 1);
		scn_free (&scn);
		return (1);
	}

	scn_free (&scn);

	return (0);
}

int ini_read (ini_sct_t *sct, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (1);
	}

	r = ini_read_fp (sct, fp, fname);

	fclose (fp);

	return (r);
}
