/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/section.c                                         *
 * Created:     2001-08-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <libini/libini.h>


void ini_sct_init (ini_sct_t *sct, const char *name)
{
	if (name == NULL) {
		name = "";
	}

	sct->name = strdup (name);
	sct->format = 0;

	sct->sct = NULL;
	sct->sctcnt = 0;

	sct->val = NULL;
	sct->valcnt = 0;
}

void ini_sct_free (ini_sct_t *sct)
{
	unsigned i;

	for (i = 0; i < sct->sctcnt; i++) {
		ini_sct_free (&sct->sct[i]);
	}

	free (sct->sct);

	for (i = 0; i < sct->valcnt; i++) {
		ini_val_free (&sct->val[i]);
	}

	free (sct->val);

	free (sct->name);
}

ini_sct_t *ini_sct_new (const char *name)
{
	ini_sct_t *sct;

	sct = malloc (sizeof (ini_sct_t));
	if (sct == NULL) {
		return (NULL);
	}

	ini_sct_init (sct, name);

	return (sct);
}

void ini_sct_del (ini_sct_t *sct)
{
	if (sct != NULL) {
		ini_sct_free (sct);
		free (sct);
	}
}

void ini_sct_set_format (ini_sct_t *sct, unsigned format, int rec)
{
	unsigned i;

	sct->format = format;

	if (rec) {
		for (i = 0; i < sct->sctcnt; i++) {
			ini_sct_set_format (&sct->sct[i], format, 1);
		}
	}
}

unsigned ini_sct_get_format (const ini_sct_t *sct)
{
	return (sct->format);
}

ini_sct_t *ini_get_sct_idx (const ini_sct_t *sct, unsigned i)
{
	if (i < sct->sctcnt) {
		return (&sct->sct[i]);
	}

	return (NULL);
}

ini_val_t *ini_get_val_idx (const ini_sct_t *sct, unsigned i)
{
	if (i < sct->valcnt) {
		return (&sct->val[i]);
	}

	return (NULL);
}

ini_sct_t *ini_next_sct (ini_sct_t *sct, ini_sct_t *val, const char *name)
{
	unsigned i;

	i = 0;

	if (val != NULL) {
		while ((i < sct->sctcnt) && (&sct->sct[i] != val)) {
			i += 1;
		}
		i += 1;
	}

	while (i < sct->sctcnt) {
		if (strcmp (sct->sct[i].name, name) == 0) {
			return (&sct->sct[i]);
		}
		i += 1;
	}

	return (NULL);
}

ini_val_t *ini_next_val (ini_sct_t *sct, ini_val_t *val, const char *name)
{
	unsigned i;

	i = 0;

	if (val != NULL) {
		while ((i < sct->valcnt) && (&sct->val[i] != val)) {
			i += 1;
		}
		i += 1;
	}

	while (i < sct->valcnt) {
		if (strcmp (sct->val[i].name, name) == 0) {
			return (&sct->val[i]);
		}
		i += 1;
	}

	return (NULL);
}

ini_sct_t *ini_get_sct (ini_sct_t *sct, const char *name, int add)
{
	unsigned i;

	for (i = 0; i < sct->sctcnt; i++) {
		if (strcmp (sct->sct[i].name, name) == 0) {
			return (&sct->sct[i]);
		}
	}

	if (add) {
		return (ini_new_sct (sct, name));
	}

	return (NULL);
}

ini_val_t *ini_get_val (ini_sct_t *sct, const char *name, int add)
{
	unsigned i;

	for (i = 0; i < sct->valcnt; i++) {
		if (strcmp (sct->val[i].name, name) == 0) {
			return (&sct->val[i]);
		}
	}

	if (add) {
		return (ini_new_val (sct, name));
	}

	return (NULL);
}

ini_sct_t *ini_new_sct (ini_sct_t *sct, const char *name)
{
	ini_sct_t *tmp;

	tmp = realloc (sct->sct, (sct->sctcnt + 1) * sizeof (ini_sct_t));
	if (tmp == NULL) {
		return (NULL);
	}

	sct->sct = tmp;
	sct->sctcnt += 1;

	tmp = &sct->sct[sct->sctcnt - 1];

	ini_sct_init (tmp, name);

	return (tmp);
}

ini_val_t *ini_new_val (ini_sct_t *sct, const char *name)
{
	ini_val_t *tmp;

	tmp = realloc (sct->val, (sct->valcnt + 1) * sizeof (ini_val_t));
	if (tmp == NULL) {
		return (NULL);
	}

	sct->val = tmp;
	sct->valcnt += 1;

	tmp = &sct->val[sct->valcnt - 1];

	ini_val_init (tmp, name);

	return (tmp);
}

ini_val_t *ini_find_val (const ini_sct_t *sct, const char *name)
{
	unsigned  i, j, k;
	ini_sct_t *tmp;
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
		val = sct->val;
		for (k = 0; k < sct->valcnt; k++) {
			if (strcmp (val->name, name) == 0) {
				return (val);
			}
			val += 1;
		}

		return (NULL);
	}

	tmp = sct->sct;
	for (k = 0; k < sct->sctcnt; k++) {
		if (strlen (tmp->name) == i) {
			j = 0;
			while ((j < i) && (tmp->name[j] == name[j])) {
				j += 1;
			}

			if (j == i) {
				return (ini_find_val (tmp, name + i + 1));
			}
		}

		tmp += 1;
	}

	return (NULL);
}

ini_sct_t *ini_find_sct (ini_sct_t *sct, const char *name)
{
	unsigned  i, j, k;
	ini_sct_t *tmp;

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

	tmp = sct->sct;
	for (k = 0; k < sct->sctcnt; k++) {
		if (strlen (tmp->name) == i) {
			j = 0;
			while ((j < i) && (tmp->name[j] == name[j])) {
				j += 1;
			}

			if (j == i) {
				if (name[i] == 0) {
					return (tmp);
				}
				else {
					return (ini_find_sct (tmp, name + i + 1));
				}
			}
		}

		tmp += 1;
	}

	return (NULL);
}

int ini_set_uint32 (ini_sct_t *sct, const char *name, unsigned long v)
{
	ini_val_t *val;

	val = ini_get_val (sct, name, 1);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_uint32 (val, v);

	return (0);
}

int ini_set_sint32 (ini_sct_t *sct, const char *name, long v)
{
	ini_val_t *val;

	val = ini_get_val (sct, name, 1);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_sint32 (val, v);

	return (0);
}

int ini_set_dbl (ini_sct_t *sct, const char *name, double v)
{
	ini_val_t *val;

	val = ini_get_val (sct, name, 1);
	if (val == NULL) {
		return (1);
	}

	ini_val_set_dbl (val, v);

	return (0);
}

int ini_set_str (ini_sct_t *sct, const char *name, const char *v)
{
	ini_val_t *val;

	val = ini_get_val (sct, name, 1);
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

	val = ini_find_val (sct, name);
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

	val = ini_find_val (sct, name);
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

	val = ini_find_val (sct, name);
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

	val = ini_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_sint16 (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_bool (const ini_sct_t *sct, const char *name, int *ret, int def)
{
	ini_val_t *val;

	*ret = (def != 0);

	val = ini_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_bool (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_dbl (const ini_sct_t *sct, const char *name, double *ret, double def)
{
	ini_val_t *val;

	*ret = def;

	val = ini_find_val (sct, name);
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

	val = ini_find_val (sct, name);
	if (val == NULL) {
		return (1);
	}

	tmp = ini_val_get_str (val);

	if (tmp != NULL) {
		*ret = tmp;
	}

	return (0);
}
