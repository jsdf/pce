/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/expr.c                                            *
 * Created:     2010-09-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include <libini/scanner.h>


int ini_eval (scanner_t *scn, ini_sct_t *sct, ini_val_t *val);


static
int val_str_cat (ini_val_t *dst, const char *s1, const char *s2)
{
	unsigned n1, n2;
	char     *str;

	n1 = strlen (s1);
	n2 = strlen (s2);

	str = malloc (n1 + n2 + 1);

	if (str == NULL) {
		return (1);
	}

	if (n1 > 0) {
		memcpy (str, s1, n1);
	}

	if (n2 > 0) {
		memcpy (str + n1, s2, n2);
	}

	str[n1 + n2] = 0;

	ini_val_set_str (dst, str);

	free (str);

	return (0);
}

static
int val_plus (ini_val_t *val)
{
	if (val->type == INI_VAL_INT) {
		return (0);
	}

	return (1);
}

static
int val_neg (ini_val_t *val)
{
	if (val->type == INI_VAL_INT) {
		val->val.u32 = (~val->val.u32 + 1) & 0xffffffff;
		return (0);
	}

	return (1);
}

static
int val_bnot (ini_val_t *val)
{
	if (val->type == INI_VAL_INT) {
		val->val.u32 = ~val->val.u32 & 0xffffffff;
		return (0);
	}

	return (1);
}

static
int val_lnot (ini_val_t *val)
{
	if (val->type == INI_VAL_INT) {
		val->val.u32 = val->val.u32 == 0;
		return (0);
	}
	else if (val->type == INI_VAL_STR) {
		ini_val_set_uint32 (val, strcmp (val->val.str, "") == 0);
		return (0);
	}

	return (1);
}

static
int val_mul (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 * src->val.u32) & 0xffffffff;
		return (0);
	}

	return (1);
}

static
int val_div (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		if (src->val.u32 == 0) {
			return (1);
		}

		dst->val.u32 = (dst->val.u32 / src->val.u32) & 0xffffffff;

		return (0);
	}

	return (1);
}

static
int val_mod (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		if (src->val.u32 == 0) {
			return (1);
		}

		dst->val.u32 = (dst->val.u32 % src->val.u32) & 0xffffffff;

		return (0);
	}

	return (1);
}

static
int val_add (ini_val_t *dst, const ini_val_t *src)
{
	unsigned t1, t2;
	char     buf[32];

	t1 = dst->type;
	t2 = src->type;

	if ((t1 == INI_VAL_INT) && (t2 == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 + src->val.u32) & 0xffffffff;
		return (0);
	}
	else if ((t1 == INI_VAL_STR) && (t2 == INI_VAL_STR)) {
		if (val_str_cat (dst, dst->val.str, src->val.str)) {
			return (1);
		}

		return (0);
	}
	else if ((t1 == INI_VAL_STR) && (t2 == INI_VAL_INT)) {
		sprintf (buf, "%lu", src->val.u32);

		if (val_str_cat (dst, dst->val.str, buf)) {
			return (1);
		}

		return (0);
	}
	else if ((t1 == INI_VAL_INT) && (t2 == INI_VAL_STR)) {
		sprintf (buf, "%lu", dst->val.u32);

		if (val_str_cat (dst, buf, src->val.str)) {
			return (1);
		}

		return (0);
	}

	return (1);
}

static
int val_sub (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 - src->val.u32) & 0xffffffff;
		return (0);
	}

	return (1);
}

static
int val_shl (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 << (src->val.u32 & 31)) & 0xffffffff;
		return (0);
	}

	return (1);
}

static
int val_shr (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 >> (src->val.u32 & 31)) & 0xffffffff;
		return (0);
	}

	return (1);
}

static
int val_lt (ini_val_t *dst, const ini_val_t *src)
{
	if (dst->type != src->type) {
		return (1);
	}

	if (dst->type == INI_VAL_INT) {
		dst->val.u32 = dst->val.u32 < src->val.u32;
		return (0);
	}
	else if (dst->type == INI_VAL_STR) {
		ini_val_set_uint32 (dst, strcmp (dst->val.str, src->val.str) < 0);
		return (0);
	}

	return (1);
}

static
int val_le (ini_val_t *dst, const ini_val_t *src)
{
	if (dst->type != src->type) {
		return (1);
	}

	if (dst->type == INI_VAL_INT) {
		dst->val.u32 = dst->val.u32 <= src->val.u32;
		return (0);
	}
	else if (dst->type == INI_VAL_STR) {
		ini_val_set_uint32 (dst, strcmp (dst->val.str, src->val.str) <= 0);
		return (0);
	}

	return (1);
}

static
int val_gt (ini_val_t *dst, const ini_val_t *src)
{
	if (dst->type != src->type) {
		return (1);
	}

	if (dst->type == INI_VAL_INT) {
		dst->val.u32 = dst->val.u32 > src->val.u32;
		return (0);
	}
	else if (dst->type == INI_VAL_STR) {
		ini_val_set_uint32 (dst, strcmp (dst->val.str, src->val.str) > 0);
		return (0);
	}

	return (1);
}

static
int val_ge (ini_val_t *dst, const ini_val_t *src)
{
	if (dst->type != src->type) {
		return (1);
	}

	if (dst->type == INI_VAL_INT) {
		dst->val.u32 = dst->val.u32 >= src->val.u32;
		return (0);
	}
	else if (dst->type == INI_VAL_STR) {
		ini_val_set_uint32 (dst, strcmp (dst->val.str, src->val.str) >= 0);
		return (0);
	}

	return (1);
}

static
int val_equ (ini_val_t *dst, const ini_val_t *src)
{
	if (dst->type != src->type) {
		return (1);
	}

	if (dst->type == INI_VAL_INT) {
		dst->val.u32 = dst->val.u32 == src->val.u32;
		return (0);
	}
	else if (dst->type == INI_VAL_STR) {
		ini_val_set_uint32 (dst, strcmp (dst->val.str, src->val.str) == 0);
		return (0);
	}

	return (1);
}

static
int val_neq (ini_val_t *dst, const ini_val_t *src)
{
	if (dst->type != src->type) {
		return (1);
	}

	if (dst->type == INI_VAL_INT) {
		dst->val.u32 = dst->val.u32 != src->val.u32;
		return (0);
	}
	else if (dst->type == INI_VAL_STR) {
		ini_val_set_uint32 (dst, strcmp (dst->val.str, src->val.str) != 0);
		return (0);
	}

	return (1);
}

static
int val_band (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = dst->val.u32 & src->val.u32;
		return (0);
	}

	return (1);
}

static
int val_bxor (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = dst->val.u32 ^ src->val.u32;
		return (0);
	}

	return (1);
}

static
int val_bor (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = dst->val.u32 | src->val.u32;
		return (0);
	}

	return (1);
}

static
int val_land (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 != 0) && (src->val.u32 != 0);
		return (0);
	}

	return (1);
}

static
int val_lor (ini_val_t *dst, const ini_val_t *src)
{
	if ((dst->type == src->type) && (dst->type == INI_VAL_INT)) {
		dst->val.u32 = (dst->val.u32 != 0) || (src->val.u32 != 0);
		return (0);
	}

	return (1);
}


static
int ini_eval_uint (scanner_t *scn, ini_val_t *val)
{
	int           r;
	char          c;
	unsigned      base;
	unsigned long v, d;

	if (scn_match (scn, "0x")) {
		base = 16;
	}
	else if (scn_match (scn, "0b")) {
		base = 2;
	}
	else {
		base = 10;
	}

	r = 1;
	v = 0;

	while (1) {
		c = scn_get_chr (scn, 0);

		if (c == 0) {
			break;
		}

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
			break;
		}

		if (d >= base) {
			break;
		}

		v = base * v + d;

		scn_rmv_chr (scn, 1);

		r = 0;
	}

	if (r) {
		return (1);
	}

	if ((c == 'k') || (c == 'K')) {
		scn_rmv_chr (scn, 1);
		v *= 1024;
	}
	else if ((c == 'm') || (c == 'M')) {
		scn_rmv_chr (scn, 1);
		v *= 1024UL * 1024UL;
	}
	else if ((c == 'g') || (c == 'G')) {
		scn_rmv_chr (scn, 1);
		v *= 1024UL * 1024UL * 1024UL;
	}

	ini_val_set_uint32 (val, v);

	return (0);
}

static
int ini_eval_literal (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	unsigned  i;
	ini_sct_t *tmp;
	ini_val_t *val2;
	char      str[256];

	if (scn_match (scn, "(")) {
		if (ini_eval (scn, sct, val)) {
			return (1);
		}

		scn_match_space (scn);

		if (scn_match (scn, ")") == 0) {
			return (1);
		}

		return (0);
	}

	if (scn_match_string (scn, str, 256)) {
		ini_val_set_str (val, str);
		return (0);
	}

	if (scn_match_name (scn, str, 256)) {
		if (strcmp (str, "defined") == 0) {
			if (ini_eval_literal (scn, sct, val)) {
				return (1);
			}

			ini_val_set_uint32 (val, val->type != INI_VAL_NONE);

			return (0);
		}
		else if (strcmp (str, "true") == 0) {
			ini_val_set_uint32 (val, 1);
			return (0);
		}
		else if (strcmp (str, "false") == 0) {
			ini_val_set_uint32 (val, 0);
			return (0);
		}

		i = (str[0] == '$') ? 1 : 0;

		tmp = sct;
		val2 = NULL;

		while (tmp != NULL) {
			val2 = ini_get_val (tmp, str + i, 0);

			if (val2 != NULL) {
				break;
			}

			tmp = tmp->parent;
		}

		if (val2 == NULL) {
			ini_val_set_none (val);
			return (0);
		}

		ini_val_copy (val, val2);

		return (0);
	}

	if (ini_eval_uint (scn, val) == 0) {
		return (0);
	}

	return (1);
}

static
int ini_eval_neg (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int      r;
	unsigned op;

	if (scn_match (scn, "+")) {
		op = 1;
	}
	else if (scn_match (scn, "-")) {
		op = 2;
	}
	else if (scn_match (scn, "~")) {
		op = 3;
	}
	else if (scn_match (scn, "!")) {
		op = 4;
	}
	else {
		op = 0;
	}

	if (ini_eval_literal (scn, sct, val)) {
		return (1);
	}

	if (op == 1) {
		r = val_plus (val);
	}
	else if (op == 2) {
		r = val_neg (val);
	}
	else if (op == 3) {
		r = val_bnot (val);
	}
	else if (op == 4) {
		r = val_lnot (val);
	}
	else {
		r = 0;
	}

	return (r);
}

static
int ini_eval_mul (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	unsigned  op;
	ini_val_t val2;

	if (ini_eval_neg (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "*")) {
			op = 1;
		}
		else if (scn_match (scn, "/")) {
			op = 2;
		}
		else if (scn_match (scn, "%")) {
			op = 3;
		}
		else {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_neg (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		if (op == 1) {
			r = val_mul (val, &val2);
		}
		else if (op == 2) {
			r = val_div (val, &val2);
		}
		else if (op == 3) {
			r = val_mod (val, &val2);
		}
		else {
			r = 1;
		}

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_add (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	unsigned  op;
	ini_val_t val2;

	if (ini_eval_mul (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "+")) {
			op = 1;
		}
		else if (scn_match (scn, "-")) {
			op = 2;
		}
		else {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_mul (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		if (op == 1) {
			r = val_add (val, &val2);
		}
		else if (op == 2) {
			r = val_sub (val, &val2);
		}
		else {
			r = 1;
		}

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_shift (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	unsigned  op;
	ini_val_t val2;

	if (ini_eval_add (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "<<")) {
			op = 1;
		}
		else if (scn_match (scn, ">>")) {
			op = 2;
		}
		else {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_add (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		if (op == 1) {
			r = val_shl (val, &val2);
		}
		else if (op == 2) {
			r = val_shr (val, &val2);
		}
		else {
			r = 1;
		}

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_cmp (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	unsigned  op;
	ini_val_t val2;

	if (ini_eval_shift (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "<=")) {
			op = 2;
		}
		else if (scn_match (scn, ">=")) {
			op = 4;
		}
		else if (scn_match (scn, "<")) {
			op = 1;
		}
		else if (scn_match (scn, ">")) {
			op = 3;
		}
		else {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_shift (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		if (op == 1) {
			r = val_lt (val, &val2);
		}
		else if (op == 2) {
			r = val_le (val, &val2);
		}
		else if (op == 3) {
			r = val_gt (val, &val2);
		}
		else if (op == 4) {
			r = val_ge (val, &val2);
		}
		else {
			r = 1;
		}

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_equ (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	unsigned  op;
	ini_val_t val2;

	if (ini_eval_cmp (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "==")) {
			op = 1;
		}
		else if (scn_match (scn, "!=")) {
			op = 2;
		}
		else {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_cmp (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		if (op == 1) {
			r = val_equ (val, &val2);
		}
		else if (op == 2) {
			r = val_neq (val, &val2);
		}
		else {
			r = 1;
		}

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_band (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	ini_val_t val2;

	if (ini_eval_equ (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_peek (scn, "&&")) {
			return (0);
		}

		if (scn_match (scn, "&") == 0) {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_equ (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		r = val_band (val, &val2);

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_bxor (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	ini_val_t val2;

	if (ini_eval_band (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_peek (scn, "^^")) {
			return (0);
		}

		if (scn_match (scn, "^") == 0) {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_band (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		r = val_bxor (val, &val2);

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_bor (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	ini_val_t val2;

	if (ini_eval_bxor (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_peek (scn, "||")) {
			return (0);
		}

		if (scn_match (scn, "|") == 0) {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_bxor (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		r = val_bor (val, &val2);

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_land (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	ini_val_t val2;

	if (ini_eval_bor (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "&&") == 0) {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_bor (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		r = val_land (val, &val2);

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_lor (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	int       r;
	ini_val_t val2;

	if (ini_eval_land (scn, sct, val)) {
		return (1);
	}

	while (1) {
		if (scn_match (scn, "||") == 0) {
			return (0);
		}

		ini_val_init (&val2, NULL);

		if (ini_eval_land (scn, sct, &val2)) {
			ini_val_free (&val2);
			return (1);
		}

		r = val_lor (val, &val2);

		ini_val_free (&val2);

		if (r) {
			return (1);
		}
	}
}

static
int ini_eval_cond (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	ini_val_t val2;
	ini_val_t *v1, *v2;

	if (ini_eval_lor (scn, sct, val)) {
		return (1);
	}

	if (scn_match (scn, "?") == 0) {
		return (0);
	}

	ini_val_init (&val2, NULL);

	if ((val->type == INI_VAL_INT) && (val->val.u32 != 0)) {
		v1 = val;
		v2 = &val2;
	}
	else {
		v1 = &val2;
		v2 = val;
	}

	if (ini_eval_lor (scn, sct, v1)) {
		ini_val_free (&val2);
		return (1);
	}

	if (scn_match (scn, ":") == 0) {
		ini_val_free (&val2);
		return (1);
	}

	if (ini_eval_lor (scn, sct, v2)) {
		ini_val_free (&val2);
		return (1);
	}

	ini_val_free (&val2);

	return (0);
}

int ini_eval (scanner_t *scn, ini_sct_t *sct, ini_val_t *val)
{
	if (ini_eval_cond (scn, sct, val)) {
		return (1);
	}

	return (0);
}
