/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     term.h                                                     *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: term.h,v 1.1 2003/04/18 20:05:50 hampa Exp $ */


#ifndef PCE_TERM_H
#define PCE_TERM_H 1


#include <pce.h>


typedef struct {
  unsigned x;
  unsigned y;

  unsigned fg;
  unsigned bg;

  int      col_chg;

  FILE     *fp;
} term_t;


void trm_init (term_t *trm, FILE *fp);
void trm_free (term_t *trm);

void trm_clr_scn (term_t *trm);
void trm_set_pos (term_t *trm, unsigned x, unsigned y);
void trm_set_col (term_t *trm, unsigned fg, unsigned bg);
void trm_set_attr_mono (term_t *trm, unsigned char a);
void trm_set_attr_col (term_t *trm, unsigned char a);
void trm_set_chr (term_t *trm, unsigned char chr);
void trm_set_chr_xy (term_t *trm, unsigned x, unsigned y, unsigned char c);


#endif
