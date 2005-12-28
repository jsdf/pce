/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/scanner.c                                       *
 * Created:       2000-12-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2000-2005 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include <stdlib.h>
#include <string.h>

#include <libini/scanner.h>


static unsigned scn_get_char (scanner_t *scn);


void scn_set_clear (scn_set_t *set)
{
	unsigned i;

	for (i = 0; i < 32; i++) {
		set->el[i] = 0;
	}
}

void scn_set_add_el (scn_set_t *set, unsigned el)
{
	if (el < 256) {
		set->el[el / 8] |= 0x80 >> (el % 8);
	}
}

void scn_set_add_els (scn_set_t *set, unsigned el1, unsigned el2)
{
	while (el1 <= el2) {
		scn_set_add_el (set, el1);
		el1 += 1;
	}
}

void scn_set_copy (scn_set_t *dst, const scn_set_t *src)
{
	unsigned i;

	for (i = 0; i < 32; i++) {
		dst->el[i] = src->el[i];
	}
}

int scn_set_is_el (const scn_set_t *set, unsigned el)
{
	unsigned r;

	if (el > 255) {
		return (0);
	}

	r = set->el[el / 8] & (0x80 >> (el % 8));

	return (r != 0);
}


scn_class_t *scn_cls_new (unsigned id)
{
	scn_class_t *cls;

	cls = (scn_class_t *) malloc (sizeof (scn_class_t));
	if (cls == NULL) {
		return (NULL);
	}

	cls->next = NULL;
	cls->cls_id = id;

	scn_set_clear (&cls->start);
	scn_set_clear (&cls->run);

	return (cls);
}

void scn_cls_del (scn_class_t *cls)
{
	free (cls);
}

scn_class_t *scn_cls_clone (const scn_class_t *cls)
{
	scn_class_t *ret;

	ret = scn_cls_new (cls->cls_id);
	if (ret == NULL) {
		return (NULL);
	}

	scn_set_copy (&ret->start, &cls->start);
	scn_set_copy (&ret->run, &cls->run);

	return (ret);
}


void scn_init (scanner_t *scn)
{
	scn->del = &scn_del;
	scn->get_char = &scn_get_char;
	scn->scan = &scn_scan;

	scn->tok_id = 0;
	scn->tok_str[0] = 0;

	scn->curcnt = 0;

	scn->cls = NULL;

	scn_set_clear (&scn->white);
	scn_set_add_el (&scn->white, 9);
	scn_set_add_el (&scn->white, 10);
	scn_set_add_el (&scn->white, 13);
	scn_set_add_el (&scn->white, 32);

	scn_set_clear (&scn->newline);
	scn_set_add_el (&scn->newline, 10);

	scn->line = 0;
	scn->offset = 0;

	scn->ext = NULL;
}

scanner_t *scn_new (void)
{
	scanner_t *scn;

	scn = (scanner_t *) malloc (sizeof (scanner_t));
	if (scn == NULL) {
		return (NULL);
	}

	scn_init (scn);

	return (scn);
}

void scn_free (scanner_t *scn)
{
	scn_class_t *cls;

	if (scn != NULL) {
		while (scn->cls != NULL) {
			cls = scn->cls;
			scn->cls = scn->cls->next;
			scn_cls_del (cls);
		}
	}
}

void scn_del (scanner_t *scn)
{
	if (scn != NULL) {
		scn_free (scn);
		free (scn);
	}
}

static
unsigned scn_get_char (scanner_t *scn)
{
	return (SCN_EOF);
}

unsigned scn_get_chr (scanner_t *scn, unsigned i)
{
	unsigned j;

	if (i < scn->curcnt) {
		return (scn->curchr[i]);
	}

	if (i >= SCN_BUF_N) {
		return (SCN_EOF);
	}

	for (j = scn->curcnt; j <= i; j++) {
		scn->curchr[j] = scn->get_char (scn);
	}

	scn->curcnt = i + 1;

	return (scn->curchr[i]);
}

void scn_rmv_chr (scanner_t *scn, unsigned n)
{
	unsigned i;

	scn->offset += n;

	if (n < scn->curcnt) {
		for (i = n; i < scn->curcnt; i++) {
			scn->curchr[i - n] = scn->curchr[i];
		}

		scn->curcnt -= n;
	}
	else {
		n -= scn->curcnt;
		scn->curcnt = 0;

		while (n > 0) {
			scn->get_char (scn);
			n -= 1;
		}
	}
}

int scn_chr (scanner_t *scn, unsigned c)
{
	if (scn->tok_id != SCN_TOK_CHAR) {
		return (0);
	}

	if ((unsigned) scn->tok_str[0] != c) {
		return (0);
	}

	if (scn->tok_str[1] != 0) {
		return (0);
	}

	return (1);
}

int scn_tok (scanner_t *scn, unsigned id, const char *str)
{
	if (scn->tok_id != id) {
		return (0);
	}

	if (strcmp (scn->tok_str, str) != 0) {
		return (0);
	}

	return (1);
}

void scn_set_white (scanner_t *scn, const scn_set_t *set)
{
	scn_set_copy (&scn->white, set);
}

void scn_class_add (scanner_t *scn, scn_class_t *cls)
{
	scn_class_t *tmp;

	tmp = scn_cls_clone (cls);
	if (tmp == NULL) {
		return;
	}

	tmp->next = scn->cls;
	scn->cls = tmp;
}

static
scn_class_t *scn_class_find (scanner_t *scn, unsigned chr)
{
	scn_class_t *cls;

	cls = scn->cls;
	while (cls != NULL) {
		if (scn_set_is_el (&cls->start, chr)) {
			return (cls);
		}

		cls = cls->next;
	}

	return (NULL);
}

void scn_set_line (scanner_t *scn, unsigned long ln)
{
	scn->line = ln;
}

unsigned long scn_get_line (const scanner_t *scn)
{
	return (scn->line);
}

void scn_set_offset (scanner_t *scn, unsigned long i)
{
	scn->offset = i;
}

unsigned long scn_get_offset (const scanner_t *scn)
{
	return (scn->offset);
}

void scn_scan (scanner_t *scn)
{
	unsigned    c;
	unsigned    i;
	scn_class_t *cls;

	scn->tok_id = SCN_TOK_NONE;
	scn->tok_str[0] = 0;

	c = scn_get_chr (scn, 0);
	while (scn_set_is_el (&scn->white, c)) {
		if (scn_set_is_el (&scn->newline, c)) {
			scn->line += 1;
		}

		scn_rmv_chr (scn, 1);
		c = scn_get_chr (scn, 0);
	}

	if (c == SCN_EOF) {
		return;
	}

	cls = scn_class_find (scn, c);
	if (cls == NULL) {
		if (scn_set_is_el (&scn->newline, c)) {
			scn->line += 1;
		}

		scn->tok_str[0] = (char)c;
		scn->tok_str[1] = 0;
		scn->tok_id = SCN_TOK_CHAR;
		scn_rmv_chr (scn, 1);
		return;
	}

	scn->tok_id = cls->cls_id;

	i = 0;
	do {
		if (scn_set_is_el (&scn->newline, c)) {
			scn->line += 1;
		}

		scn->tok_str[i] = (char) c;
		i += 1;

		scn_rmv_chr (scn, 1);
		c = scn_get_chr (scn, 0);
	} while ((i < SCN_TOK_MAX) && scn_set_is_el (&cls->run, c));

	scn->tok_str[i] = 0;
}


void scnf_init (scn_file_t *sf)
{
	scn_init (&sf->scn);

	sf->scn.ext = sf;

	sf->scn.del = &scnf_del;
	sf->scn.get_char = &scnf_get_char;

	sf->file = NULL;
}

scanner_t *scnf_new (void)
{
	scn_file_t *sf;

	sf = (scn_file_t *) malloc (sizeof (scn_file_t));
	if (sf == NULL) {
		return (NULL);
	}

	scnf_init (sf);

	return (&sf->scn);
}

void scnf_free (scn_file_t *sf)
{
	scn_free (&sf->scn);
}

void scnf_del (scanner_t *scn)
{
	scn_file_t *sf;

	if (scn != NULL) {
		sf = (scn_file_t *) scn->ext;
		scnf_free (sf);
		free (sf);
	}
}

unsigned scnf_get_char (scanner_t *scn)
{
	int         c;
	unsigned    ret;
	scn_file_t  *sf;

	sf = (scn_file_t *) scn->ext;

	c = fgetc (sf->file);

	if (c == EOF) {
		ret = SCN_EOF;
	}
	else {
		ret = (unsigned) c;
	}

	return (ret);
}

void scnf_set_file (scanner_t *scn, FILE *file)
{
	scn_file_t *sf;

	sf = (scn_file_t *) scn->ext;

	sf->file = file;
}


void scns_init (scn_str_t *ss, const char *str)
{
	unsigned long i, n;

	scn_init (&ss->scn);

	ss->scn.ext = ss;

	ss->scn.del = &scns_del;
	ss->scn.get_char = &scns_get_char;

	n = 0;
	while (str[n] != 0) {
		n += 1;
	}

	if (n > 0) {
		ss->str = (unsigned char *) malloc (n);

		for (i = 0; i < n; i++) {
			ss->str[i] = (unsigned char) str[i];
		}
	}
	else {
		ss->str = NULL;
	}

	ss->i = 0;
	ss->n = n;
}

scanner_t *scns_new (const char *str)
{
	scn_str_t *ss;

	ss = (scn_str_t *) malloc (sizeof (scn_str_t));
	if (ss == NULL) {
		return (NULL);
	}

	scns_init (ss, str);

	return (&ss->scn);
}

void scns_free (scn_str_t *ss)
{
	if (ss != NULL) {
		free (ss->str);
		ss->i = 0;
		ss->n = 0;
		ss->str = NULL;
	}

	scn_free (&ss->scn);
}

void scns_del (scanner_t *scn)
{
	scn_str_t *ss;

	if (scn != NULL) {
		ss = (scn_str_t *) scn->ext;
		scns_free (ss);
		free (ss);
	}
}

unsigned scns_get_char (scanner_t *scn)
{
	unsigned  ret;
	scn_str_t *ss;

	ss = (scn_str_t *) scn->ext;

	if (ss->i < ss->n) {
		ret = ss->str[ss->i++];
	}
	else {
		ret = SCN_EOF;
	}

	return (ret);
}
