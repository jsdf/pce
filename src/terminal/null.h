/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/null.h                                        *
 * Created:       2003-10-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-10-18 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: null.h,v 1.1 2003/10/18 19:30:32 hampa Exp $ */


#ifndef PCE_TERMINAL_NULL_H
#define PCE_TERMINAL_NULL_H 1


#include <stdio.h>

#include <libini/libini.h>
#include <terminal/terminal.h>


typedef struct {
  terminal_t     trm;
} null_t;


void null_init (null_t *vt, ini_sct_t *ini);
terminal_t *null_new (ini_sct_t *ini);

void null_free (null_t *vt);
void null_del (null_t *vt);

void null_set_size (null_t *vt, unsigned m, unsigned w, unsigned h);
void null_set_col (null_t *vt, unsigned fg, unsigned bg);
void null_set_crs (null_t *vt, unsigned y1, unsigned y2, int show);
void null_set_pos (null_t *vt, unsigned x, unsigned y);
void null_set_chr (null_t *vt, unsigned x, unsigned y, unsigned char c);
void null_set_pxl (null_t *vt, unsigned x, unsigned y, unsigned w, unsigned h);
void null_check (null_t *vt);


#endif
