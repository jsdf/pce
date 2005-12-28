/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/section.c                                       *
 * Created:       2001-08-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2005 Hampa Hug <hampa@hampa.ch>                   *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by  the Free Software Foundation.                                         *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/

/* $Id$ */


#include <libini/libini.h>


ini_sct_t *ini_sct_new (const char *name)
{
	ini_sct_t *sct;

	sct = (ini_sct_t *) malloc (sizeof (ini_sct_t));
	if (sct == NULL) {
		return (NULL);
	}

	sct->name = NULL;
	sct->format = 0;

	sct->next = NULL;
	sct->head = NULL;
	sct->tail = NULL;

	sct->val_head = NULL;
	sct->val_tail = NULL;

	ini_sct_set_name (sct, name);

	return (sct);
}

void ini_sct_del (ini_sct_t *sct)
{
	if (sct == NULL) {
		return;
	}

	if (sct->next != NULL) {
		ini_sct_del (sct->next);
	}

	if (sct->head != NULL) {
		ini_sct_del (sct->head);
	}

	free (sct->name);
	free (sct);
}

void ini_sct_set_name (ini_sct_t *sct, const char *name)
{
	free (sct->name);
	sct->name = NULL;

	if (name == NULL) {
		return;
	}

	sct->name = (char *) malloc (strlen (name) + 1);
	if (sct->name == NULL) {
		return;
	}

	strcpy (sct->name, name);
}

const char *ini_sct_get_name (const ini_sct_t *sct)
{
	return (sct->name);
}

void ini_sct_set_format (ini_sct_t *sct, unsigned format, int rec)
{
	sct->format = format;

	if (rec) {
		ini_sct_t *tmp;

		tmp = sct->head;
		while (tmp != NULL) {
			ini_sct_set_format (tmp, format, 1);
			tmp = tmp->next;
		}
	}
}

unsigned ini_sct_get_format (const ini_sct_t *sct)
{
	return (sct->format);
}

ini_sct_t *ini_sct_get_next (const ini_sct_t *sct)
{
	return (sct->next);
}

ini_sct_t *ini_sct_get_head (const ini_sct_t *sct)
{
	return (sct->head);
}

ini_val_t *ini_sct_get_val_head (const ini_sct_t *sct)
{
	return (sct->val_head);
}

void ini_sct_add_sct (ini_sct_t *sct, ini_sct_t *add)
{
	if (sct->head == NULL) {
		sct->head = add;
	}
	else {
		sct->tail->next = add;
	}

	sct->tail = add;

	while (sct->tail->next != NULL) {
		sct->tail = sct->tail->next;
	}
}

void ini_sct_add_val (ini_sct_t *sct, ini_val_t *val)
{
	if (sct->val_head == NULL) {
		sct->val_head = val;
	}
	else {
		sct->val_tail->next = val;
	}

	sct->val_tail = val;

	while (sct->val_tail->next != NULL) {
		sct->val_tail = sct->val_tail->next;
	}
}

ini_val_t *ini_sct_new_val (ini_sct_t *sct, const char *name)
{
	unsigned  i, j;
	char      buf[256];
	ini_sct_t *tmp;
	ini_val_t *val;

	tmp = sct;

	while (1) {
		i = 0;
		while ((name[i] != 0) && (name[i] != '.')) {
			i += 1;
		}

		if ((i == 0) || (i > 255)) {
			return (NULL);
		}

		for (j = 0; j < i; j++) {
			buf[j] = name[j];
		}

		buf[i] = 0;

		if (name[i] == 0) {
			val = ini_sct_find_val (sct, buf);
			if (val == NULL) {
				val = ini_val_new (buf);
				ini_sct_add_val (sct, val);
			}

			return (val);
		}

		tmp = ini_sct_find_sct (sct, buf);
		if (tmp == NULL) {
			tmp = ini_sct_new (buf);
			ini_sct_add_sct (sct, tmp);
		}

		sct = tmp;

		name = name + i + 1;
	}
}

ini_val_t *ini_sct_find_val (const ini_sct_t *sct, const char *name)
{
	unsigned  i, j;
	ini_val_t *val;

	if (sct == NULL) {
		return (NULL);
	}

	i = 0;

	while ((name[i] != 0) && (name[i] != '.')) {
		i += 1;
	}

	if (i == 0) {
		return (NULL);
	}

	if (name[i] == 0) {
		val = sct->val_head;
		while (val != NULL) {
			if (strcmp (val->name, name) == 0) {
				return (val);
			}

			val = val->next;
		}

		return (NULL);
	}

	sct = sct->head;
	while (sct != NULL) {
		if (strlen (sct->name) == i) {
			j = 0;
			while ((j < i) && (sct->name[j] == name[j])) {
				j += 1;
			}

			if (j == i) {
				return (ini_sct_find_val (sct, name + i + 1));
			}
		}

		sct = sct->next;
	}

	return (NULL);
}

ini_sct_t *ini_sct_find_sct (ini_sct_t *sct, const char *name)
{
	unsigned  i, j;

	if (sct == NULL) {
		return (NULL);
	}

	i = 0;

	while ((name[i] != 0) && (name[i] != '.')) {
		i += 1;
	}

	if (i == 0) {
		return (NULL);
	}

	sct = sct->head;
	while (sct != NULL) {
		if (strlen (sct->name) == i) {
			j = 0;
			while ((j < i) && (sct->name[j] == name[j])) {
				j += 1;
			}

			if (j == i) {
				if (name[i] == 0) {
					return (sct);
				}
				else {
					return (ini_sct_find_sct (sct, name + i + 1));
				}
			}
		}

		sct = sct->next;
	}

	return (NULL);
}

ini_sct_t *ini_sct_find_next (ini_sct_t *sct, const char *name)
{
	if (sct == NULL) {
		return (NULL);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (strcmp (sct->name, name) == 0) {
			return (sct);
		}

		sct = sct->next;
	}

	return (NULL);
}

int ini_set_uint32 (ini_sct_t *sct, const char *name, unsigned long v)
{
	ini_val_t *val;

	val = ini_sct_new_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_uint32 (val, v);

	return (0);
}

int ini_set_sint32 (ini_sct_t *sct, const char *name, long v)
{
	ini_val_t *val;

	val = ini_sct_new_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_sint32 (val, v);

	return (0);
}

int ini_set_dbl (ini_sct_t *sct, const char *name, double v)
{
	ini_val_t *val;

	val = ini_sct_new_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_dbl (val, v);

	return (0);
}

int ini_set_str (ini_sct_t *sct, const char *name, const char *v)
{
	ini_val_t *val;

	val = ini_sct_new_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_str (val, v);

	return (0);
}

int ini_get_uint32 (const ini_sct_t *sct, const char *name, unsigned long *ret, unsigned long def)
{
	ini_val_t *val;

	*ret = def;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_uint32 (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_sint32 (const ini_sct_t *sct, const char *name, long *ret, long def)
{
	ini_val_t *val;

	*ret = def;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_sint32 (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_uint16 (const ini_sct_t *sct, const char *name, unsigned *ret, unsigned def)
{
	ini_val_t *val;

	*ret = def;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_uint16 (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_sint16 (const ini_sct_t *sct, const char *name, int *ret, int def)
{
	ini_val_t *val;

	*ret = def;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_sint16 (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_dbl (const ini_sct_t *sct, const char *name, double *ret, double def)
{
	ini_val_t *val;

	*ret = def;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_dbl (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_string (const ini_sct_t *sct, const char *name, const char **ret, const char *def)
{
	const char *tmp;
	ini_val_t  *val;

	*ret = def;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	tmp = ini_val_get_str (val);

	if (tmp != NULL) {
		*ret = tmp;
	}

	return (0);
}

const char *ini_get_str (const ini_sct_t *sct, const char *name)
{
	ini_val_t  *val;

	val = ini_sct_find_val (sct, name);
	if (val == NULL) {
		return (NULL);
	}

	return (ini_val_get_str (val));
}

long ini_get_lng_def (ini_sct_t *sct, const char *name, long def)
{
	long      ret;
	ini_val_t *val;

	val = ini_sct_find_val (sct, name);

	if (val == NULL) {
		return (def);
	}

	if (ini_val_get_sint32 (val, &ret)) {
		return (def);
	}

	return (ret);
}

double ini_get_dbl_def (ini_sct_t *sct, const char *name, double def)
{
	ini_val_t *val;

	val = ini_sct_find_val (sct, name);

	if (val == NULL) {
		return (def);
	}

	if (val->type != INI_VAL_DBL) {
		return (def);
	}

	return (val->val.dbl);
}

const char *ini_get_str_def (ini_sct_t *sct, const char *name, const char *def)
{
	ini_val_t *val;

	val = ini_sct_find_val (sct, name);

	if (val == NULL) {
		return (def);
	}

	if (val->type != INI_VAL_STR) {
		return (def);
	}

	return (val->val.str);
}
