/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/scanner.c                                         *
 * Created:     2000-12-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2000-2010 Hampa Hug <hampa@hampa.ch>                     *
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
#include <stdlib.h>
#include <string.h>

#include <libini/scanner.h>


#ifdef PCE_HOST_WINDOWS
#define scn_is_dir_sep(c) (((c) == '/') || ((c) == '\\'))
#else
#define scn_is_dir_sep(c) ((c) == PCE_DIR_SEP)
#endif


/*
 * If name is an absolute path or base is NULL, return a copy of name.
 * Otherwise return a copy of (directory name of base) + name.
 */
static
char *scn_file_get_name (const char *base, const char *name)
{
	unsigned n1, n2;
	char     *ret;

	if (scn_is_dir_sep (*name)) {
		n1 = 0;
	}
#ifdef PCE_HOST_WINDOWS
	else if ((name[0] != 0) && (name[1] == ':')) {
		n1 = 0;
	}
#endif
	else if (base != NULL) {
		n1 = strlen (base);

		while ((n1 > 0) && (base[n1 - 1] == PCE_DIR_SEP)) {
			n1 -= 1;
		}

		while ((n1 > 0) && (base[n1 - 1] != PCE_DIR_SEP)) {
			n1 -= 1;
		}
	}
	else {
		n1 = 0;
	}

	n2 = strlen (name);

	ret = malloc (n1 + n2 + 1);

	if (ret == NULL) {
		return (NULL);
	}

	if (n1 > 0) {
		memcpy (ret, base, n1);
	}

	memcpy (ret + n1, name, n2);

	ret[n1 + n2] = 0;

	return (ret);
}

static
scn_file_t *scn_file_new (void)
{
	scn_file_t *scf;

	scf = malloc (sizeof (scn_file_t));

	if (scf == NULL) {
		return (NULL);
	}

	scf->next = NULL;
	scf->name = NULL;
	scf->fp = NULL;
	scf->del = 0;

	return (scf);
}

static
void scn_file_del (scn_file_t *scf)
{
	scn_file_t *tmp;

	while (scf != NULL) {
		tmp = scf;
		scf = scf->next;

		if (tmp->del) {
			fclose (tmp->fp);
		}

		free (tmp->name);
		free (tmp);
	}
}

void scn_init (scanner_t *scn)
{
	scn->cnt = 0;

	scn->line = 0;
	scn->offset = 0;

	scn->file = NULL;
	scn->str = NULL;
}

void scn_free (scanner_t *scn)
{
	if (scn != NULL) {
		scn_file_del (scn->file);
	}
}

void scn_set_str (scanner_t *scn, const char *str)
{
	scn->str = str;
}

int scn_add_file (scanner_t *scn, const char *fname, FILE *fp, int del)
{
	char       *name;
	scn_file_t *scf;

	scf = scn_file_new();

	if (scf == NULL) {
		return (1);
	}

	if (scn->file == NULL) {
		name = scn_file_get_name (NULL, fname);
	}
	else {
		name = scn_file_get_name (scn->file->name, fname);

		scn->file->line = scn->line;
		scn->file->offset = scn->offset;
	}

	if (name == NULL) {
		scn_file_del (scf);
		return (1);
	}

	scf->name = name;

	if (fp != NULL) {
		scf->fp = fp;
	}
	else {
		scf->fp = fopen (name, "r");
	}

	if (scf->fp == NULL) {
		scn_file_del (scf);
		return (1);
	}

	scf->del = (fp != NULL) && del;

	scf->next = scn->file;
	scn->file = scf;

	scn->line = 0;
	scn->offset = 0;

	return (0);
}

static
int scn_rmv_file (scanner_t *scn)
{
	scn_file_t *scf;

	if (scn->file == NULL) {
		return (0);
	}

	scf = scn->file;
	scn->file = scn->file->next;
	scf->next = NULL;

	scn_file_del (scf);

	if (scn->file != NULL) {
		scn->line = scn->file->line;
		scn->offset = scn->file->offset;
	}

	return (0);
}

static
char scn_next_char (scanner_t *scn)
{
	int c;

	if (scn->file != NULL) {
		while (scn->file != NULL) {
			c = fgetc (scn->file->fp);

			if (c != EOF) {
				return (c);
			}

			scn_rmv_file (scn);
		}

		return (0);
	}

	if (scn->str != NULL) {
		if (scn->str[0] == 0) {
			return (0);
		}

		c = scn->str[0];

		scn->str += 1;

		return (c);
	}

	return (0);
}

static
void scn_process_char (scanner_t *scn, char c)
{
	if (c == 0) {
		return;
	}

	scn->offset += 1;

	if (c == 0x0d) {
		scn->line += 1;
		scn->nl = 1;
	}
	else if (c == 0x0a) {
		if (scn->nl == 0) {
			scn->line += 1;
		}

		scn->nl = 0;
	}
	else {
		scn->nl = 0;
	}
}

char scn_get_chr (scanner_t *scn, unsigned idx)
{
	if (idx >= SCN_BUF_MAX) {
		return (0);
	}

	while (idx >= scn->cnt) {
		scn->buf[scn->cnt] = scn_next_char (scn);

		if (scn->buf[scn->cnt] == 0) {
			return (0);
		}

		scn->cnt += 1;
	}

	return (scn->buf[idx]);
}

void scn_rmv_chr (scanner_t *scn, unsigned cnt)
{
	unsigned i;

	if (cnt < scn->cnt) {
		for (i = 0; i < cnt; i++) {
			scn_process_char (scn, scn->buf[i]);
		}

		for (i = cnt; i < scn->cnt; i++) {
			scn->buf[i - cnt] = scn->buf[i];
		}

		scn->cnt -= cnt;
	}
	else {
		for (i = 0; i < scn->cnt; i++) {
			scn_process_char (scn, scn->buf[i]);
		}

		cnt -= scn->cnt;

		scn->cnt = 0;

		while (cnt > 0) {
			scn_process_char (scn, scn_next_char (scn));
			cnt -= 1;
		}
	}
}

unsigned long scn_get_line (const scanner_t *scn)
{
	return (scn->line);
}

unsigned long scn_get_offset (const scanner_t *scn)
{
	return (scn->offset);
}

static
int scn_is_space (char c)
{
	if ((c == ' ') || (c == '\t')) {
		return (1);
	}

	if ((c == 0x0d) || (c == 0x0a)) {
		return (1);
	}

	return (0);
}

static
int scn_is_alpha (char c)
{
	if ((c >= 'a') && (c <= 'z')) {
		return (1);
	}

	if ((c >= 'A') && (c <= 'Z')) {
		return (1);
	}

	if (c == '_') {
		return (1);
	}

	return (0);
}

static
int scn_is_numeric (char c)
{
	if ((c >= '0') && (c <= '9')) {
		return (1);
	}

	return (0);
}

static
void scn_skip_line (scanner_t *scn)
{
	char c;

	while (1) {
		c = scn_get_chr (scn, 0);

		if (c == 0) {
			return;
		}

		scn_rmv_chr (scn, 1);

		if (c == 0x0d) {
			if (scn_get_chr (scn, 0) == 0x0a) {
				scn_rmv_chr (scn, 1);
			}

			return;
		}
		else if (c == 0x0a) {
			return;
		}
	}
}

static
void scn_skip_comment (scanner_t *scn)
{
	unsigned level;
	char     buf[2];

	level = 0;

	buf[0] = 0;
	buf[1] = 0;

	while (1) {
		buf[0] = buf[1];
		buf[1] = scn_get_chr (scn, 0);

		if (buf[1] == 0) {
			return;
		}

		scn_rmv_chr (scn, 1);

		if ((buf[0] == '*') && (buf[1] == '/')) {
			if (level == 0) {
				return;
			}

			buf[1] = 0;
			level -= 1;
		}
		else if ((buf[0] == '/') && (buf[1] == '*')) {
			buf[1] = 0;
			level += 1;
		}
	}
}

int scn_match_space (scanner_t *scn)
{
	int  r;
	char c;

	r = 0;

	while (1) {
		c = scn_get_chr (scn, 0);

		if (scn_is_space (c)) {
			scn_rmv_chr (scn, 1);
		}
		else if (c == '#') {
			scn_skip_line (scn);
		}
		else if ((c == '/') && (scn_get_chr (scn, 1) == '*')) {
			scn_rmv_chr (scn, 2);
			scn_skip_comment (scn);
		}
		else {
			return (r);
		}

		r = 1;
	}

	return (1);
}

int scn_match_name (scanner_t *scn, char *str, unsigned max)
{
	unsigned i;
	char     c;

	scn_match_space (scn);

	c = scn_get_chr (scn, 0);

	if ((scn_is_alpha (c) == 0) && (c != '$')) {
		return (0);
	}

	i = 0;

	while (i < max) {
		str[i++] = c;

		c = scn_get_chr (scn, i);

		if (scn_is_alpha (c) || scn_is_numeric (c)) {
			;
		}
		else if (c == '.') {
			;
		}
		else if ((c == '[') || (c == ']')) {
			;
		}
		else if ((c == '+') && (i > 0) && (str[i - 1] == '[')) {
			;
		}
		else if ((c == '-') && (i > 0) && (str[i - 1] == '[')) {
			;
		}
		else {
			break;
		}
	}

	if (i >= max) {
		return (0);
	}

	str[i] = 0;

	scn_rmv_chr (scn, i);

	return (1);

}

static
int scn_get_hex (scanner_t *scn, char *val, unsigned idx)
{
	char     c;
	unsigned i, d, v;

	v = 0;

	for (i = 0; i < 2; i++) {
		c = scn_get_chr (scn, idx + i);

		if ((c >= '0') && (c <= '9')) {
			d = c - '0';
		}
		else if ((c >= 'a') && (c <= 'f')) {
			d = c - 'a' + 10;
		}
		else if ((c >= 'A') && (c <= 'F')) {
			d = c - 'A' + 10;
		}
		else {
			return (1);
		}

		v = 16 * v + d;
	}

	*val = (char) v;

	return (0);
}

int scn_match_string (scanner_t *scn, char *str, unsigned max)
{
	char     c;
	unsigned i, j;

	scn_match_space (scn);

	c = scn_get_chr (scn, 0);

	if (c != '"') {
		return (0);
	}

	i = 1;
	j = 0;

	while (j < max) {
		c = scn_get_chr (scn, i);

		if (c == 0) {
			return (1);
		}

		if (c == '\\') {
			switch (scn_get_chr (scn, i + 1)) {
			case 'a':
				c = '\a';
				i += 1;
				break;

			case 'b':
				c = '\b';
				i += 1;
				break;

			case 'f':
				c = '\f';
				i += 1;
				break;

			case 'n':
				c = '\n';
				i += 1;
				break;

			case 'r':
				c = '\r';
				i += 1;
				break;

			case 'v':
				c = '\v';
				i += 1;
				break;

			case '\\':
				c = '\\';
				i += 1;
				break;

			case '"':
				c = '"';
				i += 1;
				break;

			case 'x':
				if (scn_get_hex (scn, &c, i + 2)) {
					return (0);
				}
				i += 3;
				break;
			}
		}
		else if (c == '"') {
			break;
		}

		str[j++] = c;

		i += 1;
	}

	if (j >= max) {
		return (0);
	}

	str[j] = 0;

	scn_rmv_chr (scn, i + 1);

	return (1);
}

int scn_peek (scanner_t *scn, const char *str)
{
	unsigned cnt;

	scn_match_space (scn);

	cnt = 0;

	while (str[cnt] != 0) {
		if (scn_get_chr (scn, cnt) != str[cnt]) {
			return (0);
		}

		cnt += 1;
	}

	return (1);
}

int scn_match_ident (scanner_t *scn, const char *str)
{
	char     c;
	unsigned cnt;

	scn_match_space (scn);

	cnt = 0;

	while (str[cnt] != 0) {
		if (scn_get_chr (scn, cnt) != str[cnt]) {
			return (0);
		}

		cnt += 1;
	}

	c = scn_get_chr (scn, cnt);

	if (scn_is_alpha (c) || scn_is_numeric (c)) {
		return (0);
	}

	scn_rmv_chr (scn, cnt);

	return (1);
}

int scn_match (scanner_t *scn, const char *str)
{
	unsigned cnt;

	scn_match_space (scn);

	cnt = 0;

	while (str[cnt] != 0) {
		if (scn_get_chr (scn, cnt) != str[cnt]) {
			return (0);
		}

		cnt += 1;
	}

	scn_rmv_chr (scn, cnt);

	return (1);
}
