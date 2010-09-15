/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/section.c                                         *
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

#include <stdlib.h>
#include <string.h>

#include <libini/libini.h>


/*
 * Check if c is a letter, a digit or both.
 */
static
int ini_check_char (char c, int alpha, int num)
{
	if (alpha) {
		if ((c >= 'a') && (c <= 'z')) {
			return (1);
		}

		if ((c >= 'A') && (c <= 'Z')) {
			return (1);
		}

		if (c == '_') {
			return (1);
		}
	}

	if (num) {
		if ((c >= '0') && (c <= '9')) {
			return (1);
		}
	}

	return (0);
}

/*
 * Extract the first name and index from a longer section name.
 */
static
int ini_get_name_and_index (const char **str, char *name, unsigned max,
	unsigned *idx, int *noidx, int *addnew, int *last)
{
	unsigned   i;
	const char *s;

	s = *str;

	*idx = 0;
	*noidx = 1;
	*addnew = 0;
	*last = 0;

	if (ini_check_char (*s, 1, 0) == 0) {
		return (1);
	}

	name[0] = *(s++);
	i = 1;

	while ((*s != 0) && (i < max)) {
		if (ini_check_char (*s, 1, 1) == 0) {
			break;
		}

		name[i++] = *(s++);
	}

	if (i >= max) {
		return (1);
	}

	name[i] = 0;

	if (*s == 0) {
		*str = s;
		return (0);
	}

	if (*s == '.') {
		*str = s + 1;
		return (0);
	}

	if (*s != '[') {
		return (1);
	}

	s += 1;

	if (*s == '+') {
		*addnew = 1;
		s += 1;
	}
	else if (*s == '-') {
		*last = 1;
		s += 1;
	}
	else {
		if (ini_check_char (*s, 0, 1) == 0) {
			return (1);
		}

		while (ini_check_char (*s, 0, 1)) {
			*idx = 10 * *idx + (*s - '0');
			s += 1;
		}
	}

	if (*s != ']') {
		return (1);
	}

	s += 1 ;

	if (*s == '.') {
		s += 1 ;
	}

	*str = s;
	*noidx = 0;

	return (0);
}

ini_sct_t *ini_sct_new (const char *name)
{
	ini_sct_t *sct;

	sct = malloc (sizeof (ini_sct_t));

	if (sct == NULL) {
		return (NULL);
	}

	sct->next = NULL;
	sct->parent = NULL;

	if (name == NULL) {
		name = "";
	}

	if (name != NULL) {
		sct->name = strdup (name);
	}
	else {
		sct->name = NULL;
	}

	sct->sub_head = NULL;
	sct->sub_tail = NULL;

	sct->val_head = NULL;
	sct->val_tail = NULL;

	return (sct);
}

void ini_sct_del (ini_sct_t *sct)
{
	ini_sct_t *tmp;

	while (sct != NULL) {
		tmp = sct;
		sct = sct->next;

		ini_val_del (tmp->val_head);
		ini_sct_del (tmp->sub_head);

		free (tmp->name);
		free (tmp);
	}
}

static
ini_sct_t *ini_new_sct (ini_sct_t *sct, const char *name)
{
	ini_sct_t *sub;

	sub = ini_sct_new (name);

	if (sub == NULL) {
		return (NULL);
	}

	if (sct != NULL) {
		sub->parent = sct;

		if (sct->sub_head == NULL) {
			sct->sub_head = sub;
		}
		else {
			sct->sub_tail->next = sub;
		}

		sct->sub_tail = sub;
	}

	return (sub);
}

static
ini_val_t *ini_new_val (ini_sct_t *sct, const char *name)
{
	ini_val_t *val;

	val = ini_val_new (name);

	if (val == NULL) {
		return (NULL);
	}

	if (sct != NULL) {
		if (sct->val_head == NULL) {
			sct->val_head = val;
		}
		else {
			sct->val_tail->next = val;
		}

		sct->val_tail = val;
	}

	return (val);
}

ini_sct_t *ini_next_sct (ini_sct_t *sct, ini_sct_t *val, const char *name)
{
	if (val == NULL) {
		if (sct == NULL) {
			return (NULL);
		}

		val = sct->sub_head;
	}
	else {
		val = val->next;
	}

	if (val == NULL) {
		return (NULL);
	}

	if (name == NULL) {
		return (val);
	}

	while (val != NULL) {
		if (strcmp (val->name, name) == 0) {
			return (val);
		}

		val = val->next;
	}

	return (NULL);
}

ini_val_t *ini_next_val (ini_sct_t *sct, ini_val_t *val, const char *name)
{
	if (val == NULL) {
		if (sct == NULL) {
			return (NULL);
		}

		val = sct->val_head;
	}
	else {
		val = val->next;
	}

	if (val == NULL) {
		return (NULL);
	}

	if (name == NULL) {
		return (NULL);
	}

	while (val != NULL) {
		if (strcmp (val->name, name) == 0) {
			return (val);
		}

		val = val->next;
	}

	return (NULL);
}

/*
 * Get the last subsection of sct with name name. If there is no such
 * subsection and add is true, add a new subsection and return it.
 */
static
ini_sct_t *ini_get_last_sct (ini_sct_t *sct, const char *name, int add)
{
	ini_sct_t *sub, *ret;

	sub = sct->sub_head;
	ret = NULL;

	while (sub != NULL) {
		if (strcmp (sub->name, name) == 0) {
			ret = sub;
		}

		sub = sub->next;
	}

	if ((add == 0) || (ret != NULL)) {
		return (ret);
	}

	return (ini_new_sct (sct, name));
}

static
ini_sct_t *ini_get_indexed_sct (ini_sct_t *sct, const char *name,
	unsigned index, int add)
{
	ini_sct_t *sub;

	sub = sct->sub_head;

	while (sub != NULL) {
		if (strcmp (sub->name, name) == 0) {
			if (index == 0) {
				return (sub);
			}

			index -= 1;
		}

		sub = sub->next;
	}

	if (add == 0) {
		return (NULL);
	}

	while (index > 0) {
		if (ini_new_sct (sct, name) == NULL) {
			return (NULL);
		}

		index -= 1;
	}

	return (ini_new_sct (sct, name));
}

/*
 * Get the last value of sct with name name. If there is no such value
 * and add is true, add a new value and return it.
 */
static
ini_val_t *ini_get_last_val (ini_sct_t *sct, const char *name, int add)
{
	ini_val_t *val, *ret;

	val = sct->val_head;
	ret = NULL;

	while (val != NULL) {
		if (strcmp (val->name, name) == 0) {
			ret = val;
		}

		val = val->next;
	}

	if ((add == 0) || (ret != NULL)) {
		return (ret);
	}

	return (ini_new_val (sct, name));
}

static
ini_val_t *ini_get_indexed_val (ini_sct_t *sct, const char *name,
	unsigned index, int add)
{
	ini_val_t *val;

	val = sct->val_head;

	while (val != NULL) {
		if (strcmp (val->name, name) == 0) {
			if (index == 0) {
				return (val);
			}

			index -= 1;
		}

		val = val->next;
	}

	if (add == 0) {
		return (NULL);
	}

	while (index > 0) {
		if (ini_new_val (sct, name) == NULL) {
			return (NULL);
		}

		index -= 1;
	}

	return (ini_new_val (sct, name));
}

ini_sct_t *ini_get_sct (ini_sct_t *sct, const char *name, int add)
{
	int       simple, noidx, addnew, last;
	unsigned  index;
	char      name2[256];

	if (sct == NULL) {
		return (NULL);
	}

	simple = 1;

	while (*name != 0) {
		if (ini_get_name_and_index (&name, name2, 256, &index, &noidx, &addnew, &last)) {
			return (NULL);
		}

		if (*name != 0) {
			simple = 0;
		}

		if (addnew || (simple && add && noidx)) {
			sct = ini_new_sct (sct, name2);
		}
		else if (last) {
			sct = ini_get_last_sct (sct, name2, add);
		}
		else {
			sct = ini_get_indexed_sct (sct, name2, index, add);

		}

		if (sct == NULL) {
			return (NULL);
		}
	}

	return (sct);
}

ini_val_t *ini_get_val (ini_sct_t *sct, const char *name, int add)
{
	int      simple, noidx, addnew, last;
	unsigned index;
	char     name2[256];

	if (sct == NULL) {
		return (NULL);
	}

	simple = 1;

	while (*name != 0) {
		if (ini_get_name_and_index (&name, name2, 256, &index, &noidx, &addnew, &last)) {
			return (NULL);
		}

		if (*name == 0) {
			if (addnew || (simple && add && noidx)) {
				return (ini_new_val (sct, name2));
			}
			else if (last) {
				return (ini_get_last_val (sct, name2, add));
			}

			return (ini_get_indexed_val (sct, name2, index, add));
		}
		else {
			simple = 0;
		}

		if (addnew || (simple && add && noidx)) {
			sct = ini_new_sct (sct, name2);
		}
		else if (last) {
			sct = ini_get_last_sct (sct, name2, add);
		}
		else {
			sct = ini_get_indexed_sct (sct, name2, index, add);
		}

		if (sct == NULL) {
			return (NULL);
		}
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

	val = ini_get_val ((ini_sct_t *) sct, name, 0);

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

	val = ini_get_val ((ini_sct_t *) sct, name, 0);

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

	val = ini_get_val ((ini_sct_t *) sct, name, 0);

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

	val = ini_get_val ((ini_sct_t *) sct, name, 0);

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

	val = ini_get_val ((ini_sct_t *) sct, name, 0);

	if (val == NULL) {
		return (1);
	}

	if (ini_val_get_bool (val, ret)) {
		return (1);
	}

	return (0);
}

int ini_get_string (const ini_sct_t *sct, const char *name, const char **ret, const char *def)
{
	const char *tmp;
	ini_val_t  *val;

	*ret = def;

	val = ini_get_val ((ini_sct_t *) sct, name, 0);

	if (val == NULL) {
		return (1);
	}

	tmp = ini_val_get_str (val);

	if (tmp != NULL) {
		*ret = tmp;
	}

	return (0);
}
