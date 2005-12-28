/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/brkpt.h                                            *
 * Created:       2004-05-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-06-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#ifndef PCE_LIB_BRKPT_H
#define PCE_LIB_BRKPT_H 1


typedef struct breakpoint_s {
	struct breakpoint_s *next;
	unsigned long       addr[2];
	unsigned            pass;
	unsigned            reset;
} breakpoint_t;


breakpoint_t *bp_get (breakpoint_t *lst, unsigned long addr1, unsigned long addr2);

void bp_add (breakpoint_t **lst, unsigned long addr1, unsigned long addr2,
	unsigned pass, unsigned reset);

int bp_clear (breakpoint_t **lst, unsigned long addr1, unsigned long addr2);

void bp_clear_all (breakpoint_t **lst);

void bp_print (breakpoint_t *bp, const char *str, int seg);

void bp_list (breakpoint_t *lst, int seg);

int bp_check (breakpoint_t **lst, unsigned long addr1, unsigned long addr2);


#endif
