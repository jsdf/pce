/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/brkpt.h                                              *
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


#ifndef PCE_LIB_BRKPT_H
#define PCE_LIB_BRKPT_H 1


#include <stdio.h>

#include <lib/cmd.h>


#define BP_TYPE_ADDR   1
#define BP_TYPE_SEGOFS 2
#define BP_TYPE_EXPR   3


/* a breakpoint */
typedef struct breakpoint_t {
	unsigned      type;

	void          (*del) (struct breakpoint_t *bp);
	int           (*match) (struct breakpoint_t *bp, unsigned seg, unsigned long addr);
	void          (*print) (struct breakpoint_t *bp, FILE *fp);

	unsigned      pass;
	unsigned      reset;

	unsigned      seg;
	unsigned long addr;

	char          *expr;
} breakpoint_t;


/* a set of breakpoints */
typedef struct {
	unsigned     cnt;
	breakpoint_t **bp;
} bp_set_t;


breakpoint_t *bp_new (unsigned type);

void bp_set_pass (breakpoint_t *bp, unsigned pass, unsigned reset);

unsigned bp_get_pass (breakpoint_t *bp);

void bp_del (breakpoint_t *bp);
int bp_match (breakpoint_t *bp, unsigned seg, unsigned long addr);
void bp_print (breakpoint_t *bp, FILE *fp);

breakpoint_t *bp_addr_new (unsigned long addr);
breakpoint_t *bp_segofs_new (unsigned short seg, unsigned short ofs);
breakpoint_t *bp_expr_new (const char *expr);


void bps_init (bp_set_t *bps);
void bps_free (bp_set_t *bps);

int bps_bp_add (bp_set_t *bps, breakpoint_t *bp);

breakpoint_t *bps_bp_get_index (bp_set_t *bps, unsigned idx);

void bps_bp_del_index (bp_set_t *bps, unsigned idx);
void bps_bp_del (bp_set_t *bps, breakpoint_t *bp);
void bps_bp_del_all (bp_set_t *bps);

void bps_list (bp_set_t *bps, FILE *fp);
breakpoint_t *bps_match (bp_set_t *bps, unsigned seg, unsigned long addr);

int bps_check (bp_set_t *bps, unsigned seg, unsigned long addr, FILE *fp);


void cmd_do_b (cmd_t *cmd, bp_set_t *bps);


#endif
