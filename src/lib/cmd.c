/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/cmd.c                                              *
 * Created:       2003-11-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2006 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"


static FILE *cmd_fpi = NULL;
static FILE *cmd_fpo = NULL;


static void  *cmd_match_sym_ext = NULL;
static int   (*cmd_match_sym) (void *ext, cmd_t *cmd, unsigned long *ret);


int cmd_match_expr (cmd_t *cmd, unsigned long *val, unsigned base);


static
int str_is_space (char c)
{
	if ((c == ' ') || (c == '\t')) {
		return (1);
	}

	if ((c == '\n') || (c == '\r')) {
		return (1);
	}

	return (0);
}

static
char *str_ltrim (char *str)
{
	unsigned i, j;

	i = 0;
	j = 0;

	while ((str[i] != 0) && str_is_space (str[i])) {
		i += 1;
	}

	if (i == 0) {
		return (str);
	}

	while (str[i] != 0) {
		str[j] = str[i];
		i += 1;
		j += 1;
	}

	str[j] = 0;

	return (str);
}

static
char *str_rtrim (char *str)
{
	unsigned i, j;

	i = 0; j = 0;

	while (str[i] != 0) {
		if ((str[i] != ' ') && (str[i] != '\t') && (str[i] != '\n')) {
			j = i + 1;
		}
		i++;
	}

	str[j] = 0;

	return (str);
}

void cmd_get (cmd_t *cmd)
{
	fgets (cmd->str, PCE_CMD_MAX, cmd_fpi);

	str_ltrim (cmd->str);
	str_rtrim (cmd->str);

	cmd->i = 0;
}

void cmd_set_str (cmd_t *cmd, const char *str)
{
	unsigned i;

	cmd->i = 0;

	for (i = 0; i < PCE_CMD_MAX; i++) {
		cmd->str[i] = str[i];

		if (str[i] == 0) {
			return;
		}
	}

	cmd->str[PCE_CMD_MAX - 1] = 0;
}

void cmd_rewind (cmd_t *cmd)
{
	cmd->i = 0;
}

const char *cmd_get_str (cmd_t *cmd)
{
	return (cmd->str + cmd->i);
}

int cmd_match_space (cmd_t *cmd)
{
	int      r;
	unsigned i;

	i = cmd->i;

	while ((cmd->str[i] != 0) && str_is_space (cmd->str[i])) {
		i += 1;
	}

	r = (i > cmd->i);

	cmd->i = i;

	return (r);
}

void cmd_error (cmd_t *cmd, const char *str)
{
	fprintf (cmd_fpo, "*** %s [%s]\n", str, cmd->str + cmd->i);
	fflush (cmd_fpo);
}

int cmd_match_str (cmd_t *cmd, char *str, unsigned max)
{
	unsigned i, n;
	int      quote;

	cmd_match_space (cmd);

	i = cmd->i;
	n = 0;

	if (cmd->str[i] == '"') {
		quote = 1;
		i += 1;
	}
	else {
		quote = 0;
	}

	while (cmd->str[i] != 0) {
		if (quote) {
			if (cmd->str[i] == '"') {
				i += 1;
				break;
			}
		}
		else {
			if (str_is_space (cmd->str[i])) {
				break;
			}
		}

		*(str++) = cmd->str[i];
		i += 1;
		n += 1;

		if (n >= max) {
			cmd_error (cmd, "string too long");
			return (0);
		}
	}

	*str = 0;

	cmd->i = i;

	return (n > 0);
}

int cmd_match_eol (cmd_t *cmd)
{
	cmd_match_space (cmd);

	if (cmd->str[cmd->i] == 0) {
		return (1);
	}

	return (0);
}

int cmd_match_end (cmd_t *cmd)
{
	if (cmd_match_eol (cmd)) {
		return (1);
	}

	cmd_error (cmd, "syntax error");

	return (0);
}

int cmd_match (cmd_t *cmd, const char *str)
{
	unsigned i;

	cmd_match_space (cmd);

	i = cmd->i;

	while ((*str != 0) && (cmd->str[i] == *str)) {
		i += 1;
		str += 1;
	}

	if (*str != 0) {
		return (0);
	}

	cmd->i = i;

	return (1);
}

int cmd_match_expr_const (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned       i;
	unsigned       cnt;
	unsigned long  ret;

	cmd_match_space (cmd);

	i = cmd->i;

	ret = 0;
	cnt = 0;

	while (cmd->str[i] != 0) {
		unsigned dig;

		if ((cmd->str[i] >= '0') && (cmd->str[i] <= '9')) {
			dig = cmd->str[i] - '0';
		}
		else if ((cmd->str[i] >= 'a') && (cmd->str[i] <= 'f')) {
			dig = cmd->str[i] - 'a' + 10;
		}
		else if ((cmd->str[i] >= 'A') && (cmd->str[i] <= 'F')) {
			dig = cmd->str[i] - 'A' + 10;
		}
		else {
			break;
		}

		if (dig >= base) {
			break;
		}

		ret = (base * ret + dig) & 0xffffffffUL;

		cnt += 1;
		i += 1;
	}

	if (cnt == 0) {
		return (0);
	}

	cmd->i = i;
	*val = ret;

	return (1);
}

int cmd_match_expr_literal (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned i;

	cmd_match_space (cmd);

	i = cmd->i;

	if (cmd_match_sym != NULL) {
		if (cmd_match_sym (cmd_match_sym_ext, cmd, val)) {
			*val &= 0xffffffff;
			return (1);
		}
	}

	if (cmd_match (cmd, "(")) {
		if (cmd_match_expr (cmd, val, base)) {
			if (cmd_match (cmd, ")")) {
				return (1);
			}
		}

		cmd->i = i;

		return (0);
	}

	if (cmd_match_expr_const (cmd, val, base)) {
		return (1);
	}

	return (0);
}

int cmd_match_expr_neg (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned i;
	unsigned op;

	i = cmd->i;

	if (cmd_match (cmd, "!")) {
		op = 1;
	}
	else if (cmd_match (cmd, "~")) {
		op = 2;
	}
	else if (cmd_match (cmd, "+")) {
		op = 3;
	}
	else if (cmd_match (cmd, "-")) {
		op = 4;
	}
	else {
		op = 0;
	}

	if (cmd_match_expr_literal (cmd, val, base) == 0) {
		cmd->i = i;
		return (0);
	}

	if (op == 1) {
		*val = !*val;
	}
	else if (op == 2) {
		*val = ~*val & 0xffffffffUL;
	}
	else if (op == 3) {
		;
	}
	else if (op == 4) {
		*val = -*val & 0xffffffffUL;
	}

	return (1);
}

int cmd_match_expr_product (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_neg (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "*")) {
			op = 1;
		}
		else if (cmd_match (cmd, "/")) {
			op = 2;
		}
		else if (cmd_match (cmd, "%")) {
			op = 3;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_neg (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = (*val * val2) & 0xffffffffUL;
		}
		else if (op == 2) {
			*val = (val2 == 0) ? 0xffffffffUL : (*val / val2);
		}
		else {
			*val = (val2 == 0) ? 0 : (*val % val2);
		}
	}

	return (0);
}

int cmd_match_expr_sum (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_product (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "+")) {
			op = 1;
		}
		else if (cmd_match (cmd, "-")) {
			op = 2;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_product (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = (*val + val2) & 0xffffffffUL;
		}
		else {
			*val = (*val - val2) & 0xffffffffUL;
		}
	}

	return (0);
}

int cmd_match_expr_shift (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_sum (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "<<<")) {
			op = 1;
		}
		else if (cmd_match (cmd, ">>>")) {
			op = 2;
		}
		else if (cmd_match (cmd, "<<")) {
			op = 3;
		}
		else if (cmd_match (cmd, ">>")) {
			op = 4;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_sum (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		val2 &= 0x1f;

		if (op == 1) {
			*val = ((*val << val2) | (*val >> (32 - val2))) & 0xffffffffUL;
		}
		else if (op == 2) {
			*val = ((*val >> val2) | (*val << (32 - val2))) & 0xffffffffUL;
		}
		else if (op == 3) {
			*val = (*val << val2) & 0xffffffffUL;
		}
		else if (op == 4) {
			*val = (*val >> val2) & 0xffffffffUL;
		}
	}

	return (0);
}

int cmd_match_expr_cmp (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_shift (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "<=")) {
			op = 1;
		}
		else if (cmd_match (cmd, "<")) {
			op = 2;
		}
		else if (cmd_match (cmd, ">=")) {
			op = 3;
		}
		else if (cmd_match (cmd, ">")) {
			op = 4;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_shift (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val <= val2;
		}
		else if (op == 2) {
			*val = *val < val2;
		}
		else if (op == 3) {
			*val = *val >= val2;
		}
		else if (op == 4) {
			*val = *val > val2;
		}
	}

	return (0);
}

int cmd_match_expr_equ (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_cmp (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "==")) {
			op = 1;
		}
		else if (cmd_match (cmd, "!=")) {
			op = 2;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_cmp (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val == val2;
		}
		else if (op == 2) {
			*val = *val != val2;
		}
	}

	return (0);
}

int cmd_match_expr_band (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_equ (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "&")) {
			op = 1;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_equ (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val & val2;
		}
	}

	return (0);
}

int cmd_match_expr_bxor (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_band (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "^")) {
			op = 1;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_band (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val ^ val2;
		}
	}

	return (0);
}

int cmd_match_expr_bor (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_bxor (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "|")) {
			op = 1;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_bxor (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val | val2;
		}
	}

	return (0);
}

int cmd_match_expr_land (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_bor (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "&&")) {
			op = 1;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_bor (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val && val2;
		}
	}

	return (0);
}

int cmd_match_expr_lor (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned      op;
	unsigned long val2;

	i = cmd->i;

	if (cmd_match_expr_land (cmd, val, base) == 0) {
		return (0);
	}

	while (1) {
		if (cmd_match (cmd, "||")) {
			op = 1;
		}
		else {
			return (1);
		}

		if (cmd_match_expr_land (cmd, &val2, base) == 0) {
			cmd->i = i;
			return (0);
		}

		if (op == 1) {
			*val = *val || val2;
		}
	}

	return (0);
}

int cmd_match_expr (cmd_t *cmd, unsigned long *val, unsigned base)
{
	unsigned      i;
	unsigned long val2, val3;

	i = cmd->i;

	if (cmd_match_expr_lor (cmd, val, base) == 0) {
		return (0);
	}

	if (cmd_match (cmd, "?") == 0) {
		return (1);
	}

	if (cmd_match_expr_lor (cmd, &val2, base) == 0) {
		cmd->i = i;
		return (0);
	}

	if (cmd_match (cmd, ":") == 0) {
		cmd->i = i;
		return (0);
	}

	if (cmd_match_expr_lor (cmd, &val3, base) == 0) {
		cmd->i = i;
		return (0);
	}

	*val = *val ? val2 : val3;

	return (1);
}

int cmd_match_ulng (cmd_t *cmd, unsigned long *val, unsigned base)
{
	if (cmd_match_expr_const (cmd, val, base)) {
		return (1);
	}

	return (0);
}

int cmd_match_uint (cmd_t *cmd, unsigned *val, unsigned base)
{
	unsigned long tmp;

	if (cmd_match_expr_const (cmd, &tmp, base)) {
		*val = tmp;
		return (1);
	}

	return (0);
}

int cmd_match_uint16b (cmd_t *cmd, unsigned short *val, unsigned base)
{
	unsigned long tmp;

	if (cmd_match_expr (cmd, &tmp, base)) {
		*val = tmp & 0xffffU;
		return (1);
	}

	return (0);
}

int cmd_match_uint16 (cmd_t *cmd, unsigned short *val)
{
	return (cmd_match_uint16b (cmd, val, 16));
}

int cmd_match_uint32b (cmd_t *cmd, unsigned long *val, unsigned base)
{
	return (cmd_match_expr (cmd, val, base));
}

int cmd_match_uint32 (cmd_t *cmd, unsigned long *val)
{
	return (cmd_match_uint32b (cmd, val, 16));
}

int cmd_match_uint16_16 (cmd_t *cmd, unsigned short *seg, unsigned short *ofs)
{
	unsigned short val;

	if (!cmd_match_uint16 (cmd, &val)) {
		return (0);
	}

	if (!cmd_match (cmd, ":")) {
		*ofs = val;
		return (1);
	}

	*seg = val;

	cmd_match_uint16 (cmd, ofs);

	return (1);
}

void cmd_init (FILE *inp, FILE *out, void *ext, void *sym)
{
	cmd_fpi = inp;
	cmd_fpo = out;
	cmd_match_sym_ext = ext;
	cmd_match_sym = sym;
}
