/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/brkpt.c                                              *
 * Created:     2004-05-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "brkpt.h"
#include "cmd.h"


breakpoint_t *bp_new (unsigned type)
{
	breakpoint_t *bp;

	bp = malloc (sizeof (breakpoint_t));
	if (bp == NULL) {
		return (NULL);
	}

	bp->type = type;

	bp->del = NULL;
	bp->match = NULL;
	bp->print = NULL;

	bp->pass = 0;
	bp->reset = 0;

	return (bp);
}

void bp_set_pass (breakpoint_t *bp, unsigned pass, unsigned reset)
{
	bp->pass = pass;
	bp->reset = reset;
}

unsigned bp_get_pass (breakpoint_t *bp)
{
	return (bp->pass);
}

void bp_del (breakpoint_t *bp)
{
	if (bp->del != NULL) {
		bp->del (bp);
	}
	else {
		free (bp);
	}
}

int bp_match (breakpoint_t *bp, unsigned seg, unsigned long addr)
{
	int r;

	if (bp->match != NULL) {
		r = bp->match (bp, seg, addr);
	}
	else {
		r = 0;
	}

	if (r) {
		if (bp->pass > 0) {
			bp->pass -= 1;
		}

		if (bp->pass == 0) {
			if (bp->reset > 0) {
				bp->pass = bp->reset;
			}

			return (1);
		}
	}

	return (0);
}

void bp_print (breakpoint_t *bp, FILE *fp)
{
	if (bp->print != NULL) {
		return (bp->print (bp, fp));
	}
}


static
void bp_addr_del (breakpoint_t *bp)
{
	free (bp);
}

static
int bp_addr_match (breakpoint_t *bp, unsigned seg, unsigned long addr)
{
	addr += (unsigned long) seg << 4;

	if (bp->addr == addr) {
		return (1);
	}

	return (0);
}

static
void bp_addr_print (breakpoint_t *bp, FILE *fp)
{
	printf ("A  %08lX  %04X  %04X\n",
		bp->addr, bp->pass, bp->reset
	);
}

breakpoint_t *bp_addr_new (unsigned long addr)
{
	breakpoint_t *bp;

	bp = bp_new (BP_TYPE_ADDR);
	if (bp == NULL) {
		return (NULL);
	}

	bp->del = bp_addr_del;
	bp->match = bp_addr_match;
	bp->print = bp_addr_print;

	bp->seg = 0;
	bp->addr = addr;

	return (bp);
}


static
void bp_segofs_del (breakpoint_t *bp)
{
	free (bp);
}

static
int bp_segofs_match (breakpoint_t *bp, unsigned seg, unsigned long addr)
{
	if ((bp->seg == seg) && (bp->addr == addr)) {
		return (1);
	}

	return (0);
}

static
void bp_segofs_print (breakpoint_t *bp, FILE *fp)
{
	printf ("S  %04X:%04lX  %04X  %04X\n",
		bp->seg, bp->addr, bp->pass, bp->reset
	);
}

breakpoint_t *bp_segofs_new (unsigned short seg, unsigned short ofs)
{
	breakpoint_t *bp;

	bp = bp_new (BP_TYPE_SEGOFS);

	if (bp == NULL) {
		return (NULL);
	}

	bp->del = bp_segofs_del;
	bp->match = bp_segofs_match;
	bp->print = bp_segofs_print;

	bp->seg = seg;
	bp->addr = ofs;

	return (bp);
}


static
void bp_expr_del (breakpoint_t *bp)
{
	free (bp->expr);
	free (bp);
}

static
int bp_expr_match (breakpoint_t *bp, unsigned seg, unsigned long addr)
{
	unsigned long val;
	cmd_t         cmd;

	cmd_set_str (&cmd, bp->expr);

	if (cmd_match_uint32 (&cmd, &val)) {
		if (val) {
			return (1);
		}
	}

	return (0);
}

static
void bp_expr_print (breakpoint_t *bp, FILE *fp)
{
	printf ("E  \"%s\"\n", bp->expr);
}

breakpoint_t *bp_expr_new (const char *expr)
{
	breakpoint_t *bp;

	bp = bp_new (BP_TYPE_EXPR);
	if (bp == NULL) {
		return (NULL);
	}

	bp->del = bp_expr_del;
	bp->match = bp_expr_match;
	bp->print = bp_expr_print;

	bp->expr = strdup (expr);

	return (bp);
}



void bps_init (bp_set_t *bps)
{
	bps->cnt = 0;
	bps->bp = NULL;
}

void bps_free (bp_set_t *bps)
{
	unsigned i;

	for (i = 0; i < bps->cnt; i++) {
		bp_del (bps->bp[i]);
	}

	free (bps->bp);
}

int bps_bp_add (bp_set_t *bps, breakpoint_t *bp)
{
	breakpoint_t **tmp;

	tmp = realloc (bps->bp, (bps->cnt + 1) * sizeof (breakpoint_t *));
	if (tmp == NULL) {
		return (1);
	}

	bps->bp = tmp;
	bps->bp[bps->cnt] = bp;
	bps->cnt += 1;

	return (0);
}

breakpoint_t *bps_bp_get_index (bp_set_t *bps, unsigned idx)
{
	if (idx < bps->cnt) {
		return (bps->bp[idx]);
	}

	return (NULL);
}

void bps_bp_del_index (bp_set_t *bps, unsigned idx)
{
	if (idx >= bps->cnt) {
		return;
	}

	bp_del (bps->bp[idx]);

	idx += 1;
	while (idx < bps->cnt) {
		bps->bp[idx - 1] = bps->bp[idx];
		idx += 1;
	}

	bps->cnt -= 1;
}

void bps_bp_del (bp_set_t *bps, breakpoint_t *bp)
{
	unsigned i;

	for (i = 0; i < bps->cnt; i++) {
		if (bps->bp[i] == bp) {
			bps_bp_del_index (bps, i);
			return;
		}
	}
}

void bps_bp_del_all (bp_set_t *bps)
{
	unsigned i;

	for (i = 0; i < bps->cnt; i++) {
		bp_del (bps->bp[i]);
	}

	free (bps->bp);

	bps->cnt = 0;
	bps->bp = NULL;
}

void bps_list (bp_set_t *bps, FILE *fp)
{
	unsigned i;

	for (i = 0; i < bps->cnt; i++) {
		fprintf (fp, "%4u  ", i);
		bp_print (bps->bp[i], fp);
	}

	fflush (fp);
}

breakpoint_t *bps_match (bp_set_t *bps, unsigned seg, unsigned long addr)
{
	unsigned i;

	for (i = 0; i < bps->cnt; i++) {
		if (bp_match (bps->bp[i], seg, addr)) {
			return (bps->bp[i]);
		}
	}

	return (NULL);
}

/*
 * Check if a breakpoint is triggered
 */
int bps_check (bp_set_t *bps, unsigned seg, unsigned long addr, FILE *fp)
{
	breakpoint_t *bp;

	bp = bps_match (bps, seg, addr);

	if (bp != NULL) {
		bp_print (bp, fp);

		if (bp_get_pass (bp) == 0) {
			bps_bp_del (bps, bp);
		}

		return (1);
	}

	return (0);
}


static
void cmd_do_bsa (cmd_t *cmd, bp_set_t *bps)
{
	int            isseg;
	unsigned       seg;
	unsigned long  addr;
	unsigned short pass, reset;
	breakpoint_t   *bp;

	pass = 1;
	reset = 0;

	isseg = 0;
	seg = 0;
	addr = 0;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "expecting address");
		return;
	}

	if (cmd_match (cmd, ":")) {
		seg = addr & 0xffff;

		if (cmd_match_uint32 (cmd, &addr) == 0) {
			cmd_error (cmd, "expecting offset");
			return;
		}

		isseg = 1;
	}

	cmd_match_uint16 (cmd, &pass);
	cmd_match_uint16 (cmd, &reset);

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (pass > 0) {
		if (isseg) {
			bp = bp_segofs_new (seg, addr);
		}
		else {
			bp = bp_addr_new (addr);
		}

		bp_set_pass (bp, pass, reset);
		bp_print (bp, stdout);
		bps_bp_add (bps, bp);
	}
}

static
void cmd_do_bsx (cmd_t *cmd, bp_set_t *bps)
{
	breakpoint_t   *bp;
	unsigned short pass, reset;
	char           expr[256];

	pass = 1;
	reset = 0;

	if (!cmd_match_str (cmd, expr, 256)) {
		cmd_error (cmd, "expecting expression");
		return;
	}

	cmd_match_uint16 (cmd, &pass);
	cmd_match_uint16 (cmd, &reset);

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (pass > 0) {
		bp = bp_expr_new (expr);
		bp_set_pass (bp, pass, reset);

		bp_print (bp, stdout);

		bps_bp_add (bps, bp);
	}
}

static
void cmd_do_bs (cmd_t *cmd, bp_set_t *bps)
{
	if (cmd_match (cmd, "x")) {
		cmd_do_bsx (cmd, bps);
	}
	else {
		cmd_do_bsa (cmd, bps);
	}
}

static
void cmd_do_bc (cmd_t *cmd, bp_set_t *bps)
{
	unsigned short idx;

	if (cmd_match_eol (cmd)) {
		bps_bp_del_all (bps);
		return;
	}

	while (cmd_match_uint16 (cmd, &idx)) {
		bps_bp_del_index (bps, idx);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void cmd_do_bl (cmd_t *cmd, bp_set_t *bps)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	bps_list (bps, stdout);
}

void cmd_do_b (cmd_t *cmd, bp_set_t *bps)
{
	if (cmd_match (cmd, "l")) {
		cmd_do_bl (cmd, bps);
	}
	else if (cmd_match (cmd, "s")) {
		cmd_do_bs (cmd, bps);
	}
	else if (cmd_match (cmd, "c")) {
		cmd_do_bc (cmd, bps);
	}
	else {
		cmd_error (cmd, "b: unknown command");
	}
}
