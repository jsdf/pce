/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/terminal.h                                    *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-25 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: terminal.h,v 1.1 2003/04/24 23:18:17 hampa Exp $ */


#ifndef PCE_TERMINAL_TERMINAL_H
#define PCE_TERMINAL_TERMINAL_H 1


#include <stdio.h>


typedef void (*trm_del_f) (void *ext);
typedef void (*trm_set_size_f) (void *ext, unsigned w, unsigned h);
typedef void (*trm_set_col_f) (void *ext, unsigned fg, unsigned bg);
typedef void (*trm_set_crs_f) (void *ext, unsigned y1, unsigned y2);
typedef void (*trm_set_pos_f) (void *ext, unsigned x, unsigned y);
typedef void (*trm_set_chr_f) (void *ext, unsigned x, unsigned y, unsigned char c);
typedef void (*trm_check_f) (void *ext);


typedef struct {
  void (*del) (void *ext);

  void *key_ext;
  void (*set_key) (void *ext, unsigned char val);
  void (*set_brk) (void *ext, unsigned char val);

  void (*set_size) (void *ext, unsigned w, unsigned h);

  void (*set_col) (void *ext, unsigned fg, unsigned bg);
  void (*set_crs) (void *ext, unsigned y1, unsigned y2);
  void (*set_pos) (void *ext, unsigned x, unsigned y);

  void (*set_chr) (void *ext, unsigned x, unsigned y, unsigned char c);

  void (*check) (void *ext);

  void *ext;
} terminal_t;


void trm_init (terminal_t *trmp);
void trm_free (terminal_t *trm);

void trm_del (terminal_t *trm);

void trm_set_size (terminal_t *trm, unsigned w, unsigned h);
void trm_set_col (terminal_t *trm, unsigned fg, unsigned bg);
void trm_set_crs (terminal_t *trm, unsigned y1, unsigned y2);
void trm_set_pos (terminal_t *trm, unsigned x, unsigned y);
void trm_set_chr (terminal_t *trm, unsigned x, unsigned y, unsigned char c);
void trm_check (terminal_t *trm);


#endif
