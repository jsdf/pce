/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/value.c                                         *
 * Created:       2001-08-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2007 Hampa Hug <hampa@hampa.ch>                   *
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


void ini_val_init (ini_val_t *val, const char *name)
{
	val->name = strdup (name);
	val->type = INI_VAL_NONE;
}

void ini_val_free (ini_val_t *val)
{
	ini_val_set_none (val);
	free (val->name);
}

void ini_val_set_none (ini_val_t *val)
{
	if (val->type == INI_VAL_STR) {
		free (val->val.str);
	}

	val->type = INI_VAL_NONE;
}

void ini_val_set_uint32 (ini_val_t *val, unsigned long v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_U32;
	val->val.u32 = v;
}

void ini_val_set_sint32 (ini_val_t *val, long v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_S32;
	val->val.s32 = v;
}

void ini_val_set_bool (ini_val_t *val, int v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_S32;
	val->val.s32 = (v != 0);
}

void ini_val_set_dbl (ini_val_t *val, double v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_DBL;
	val->val.dbl = v;
}

void ini_val_set_str (ini_val_t *val, const char *v)
{
	ini_val_set_none (val);

	val->type = INI_VAL_STR;
	val->val.str = strdup (v);
}

int ini_val_get_uint32 (const ini_val_t *val, unsigned long *v)
{
	if (val->type == INI_VAL_U32) {
		*v = val->val.u32;
		return (0);
	}

	if (val->type == INI_VAL_S32) {
		*v = val->val.s32;
		return (0);
	}

	if (val->type == INI_VAL_DBL) {
		*v = val->val.dbl;
		return (0);
	}

	return (1);
}

int ini_val_get_sint32 (const ini_val_t *val, long *v)
{
	if (val->type == INI_VAL_S32) {
		*v = val->val.s32;
		return (0);
	}

	if (val->type == INI_VAL_U32) {
		if (val->val.u32 > 0x7fffffff) {
			return (1);
		}
		*v = val->val.u32;
		return (0);
	}

	if (val->type == INI_VAL_DBL) {
		if ((val->val.dbl < -0x7fffffff) || (val->val.dbl > 0x7fffffff)) {
			return (1);
		}
		*v = val->val.dbl;
		return (0);
	}

	return (1);
}

int ini_val_get_uint16 (const ini_val_t *val, unsigned *v)
{
	unsigned long tmp;

	if (ini_val_get_uint32 (val, &tmp)) {
		return (1);
	}

	*v = tmp & 0xffff;

	return (0);
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
	if (val->type == INI_VAL_S32) {
		*v = (val->val.s32 != 0);
		return (0);
	}

	if (val->type == INI_VAL_U32) {
		*v = (val->val.u32 != 0);
		return (0);
	}

	if (val->type == INI_VAL_DBL) {
		*v = (val->val.dbl != 0.0);
		return (0);
	}

	return (1);
}

int ini_val_get_dbl (const ini_val_t *val, double *v)
{
	if (val->type == INI_VAL_DBL) {
		*v = val->val.dbl;
		return (0);
	}

	if (val->type == INI_VAL_U32) {
		*v = val->val.u32;
		return (0);
	}

	if (val->type == INI_VAL_S32) {
		*v = val->val.s32;
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
