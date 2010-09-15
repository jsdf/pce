/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/value.c                                           *
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


void ini_val_init (ini_val_t *val, const char *name)
{
	val->next = NULL;
	val->name = NULL;
	val->type = INI_VAL_NONE;

	if (name != NULL) {
		val->name = strdup (name);
	}
}

void ini_val_free (ini_val_t *val)
{
	if (val != NULL) {
		ini_val_set_none (val);
		free (val->name);
	}
}

ini_val_t *ini_val_new (const char *name)
{
	ini_val_t *val;

	val = malloc (sizeof (ini_val_t));

	if (val == NULL) {
		return (NULL);
	}

	ini_val_init (val, name);

	return (val);
}

void ini_val_del (ini_val_t *val)
{
	ini_val_t *tmp;

	while (val != NULL) {
		tmp = val;
		val = val->next;

		ini_val_free (tmp);

		free (tmp);
	}
}

void ini_val_set_none (ini_val_t *val)
{
	if (val->type == INI_VAL_STR) {
		free (val->val.str);
	}

	val->type = INI_VAL_NONE;
}

void ini_val_copy (ini_val_t *dst, const ini_val_t *src)
{
	switch (src->type) {
	case INI_VAL_INT:
		ini_val_set_uint32 (dst, src->val.u32);
		break;

	case INI_VAL_STR:
		ini_val_set_str (dst, src->val.str);
		break;

	default:
		ini_val_set_none (dst);
		break;
	}
}

void ini_val_set_uint32 (ini_val_t *val, unsigned long v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_INT;
	val->val.u32 = v;
}

void ini_val_set_sint32 (ini_val_t *val, long v)
{
	unsigned long t;

	ini_val_set_none (val);

	if (v < 0) {
		t = -v;
		t = (~t + 1) & 0xffffffff;
	}
	else {
		t = v;
	}

	val->type = INI_VAL_INT;
	val->val.u32 = t;
}

void ini_val_set_bool (ini_val_t *val, int v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_INT;
	val->val.u32 = (v != 0);
}

void ini_val_set_str (ini_val_t *val, const char *v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_STR;
	val->val.str = strdup (v);
}


int ini_val_get_uint32 (const ini_val_t *val, unsigned long *v)
{
	if (val->type == INI_VAL_INT) {
		*v = val->val.u32;
		return (0);
	}

	return (1);
}

int ini_val_get_sint32 (const ini_val_t *val, long *v)
{
	if (val->type == INI_VAL_INT) {
		if (val->val.u32 & 0x80000000) {
			*v = -(long) ((~val->val.u32 + 1) & 0x7fffffff);
		}
		else {
			*v = (long) val->val.u32;
		}

		return (0);
	}

	return (1);
}

int ini_val_get_uint16 (const ini_val_t *val, unsigned *v)
{
	if (val->type == INI_VAL_INT) {
		*v = val->val.u32 & 0xffff;
		return (0);
	}

	return (1);
}

int ini_val_get_sint16 (const ini_val_t *val, int *v)
{
	long tmp;

	if (ini_val_get_sint32 (val, &tmp)) {
		return (1);
	}

	if ((tmp < -0x7fff) || (tmp > 0x7fff)) {
		return (1);
	}

	*v = tmp;

	return (0);
}

int ini_val_get_bool (const ini_val_t *val, int *v)
{
	if (val->type == INI_VAL_INT) {
		*v = (val->val.u32 != 0);
		return (0);
	}

	return (1);
}

const char *ini_val_get_str (const ini_val_t *val)
{
	if (val->type != INI_VAL_STR) {
		return (NULL);
	}

	return (val->val.str);
}
