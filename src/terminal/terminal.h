/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/terminal.h                                    *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-19 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: terminal.h,v 1.3 2003/08/19 17:06:20 hampa Exp $ */


#ifndef PCE_TERMINAL_TERMINAL_H
#define PCE_TERMINAL_TERMINAL_H 1


#include <stdio.h>


typedef void (*trm_del_f) (void *ext);
typedef void (*trm_set_size_f) (void *ext, unsigned mode, unsigned w, unsigned h);
typedef void (*trm_set_map_f) (void *ext, unsigned i, unsigned r, unsigned g, unsigned b);
typedef void (*trm_set_col_f) (void *ext, unsigned fg, unsigned bg);
typedef void (*trm_set_crs_f) (void *ext, unsigned y1, unsigned y2);
typedef void (*trm_set_pos_f) (void *ext, unsigned x, unsigned y);
typedef void (*trm_set_chr_f) (void *ext, unsigned x, unsigned y, unsigned char c);
typedef void (*trm_set_pxl_f) (void *ext, unsigned x, unsigned y);
typedef void (*trm_check_f) (void *ext);


typedef struct {
  void (*del) (void *ext);

  void *key_ext;
  void (*set_key) (void *ext, unsigned char val);
  void (*set_brk) (void *ext, unsigned char val);

  void (*set_size) (void *ext, unsigned mode, unsigned w, unsigned h);

  void (*set_map) (void *ext, unsigned i, unsigned r, unsigned g, unsigned b);
  void (*set_col) (void *ext, unsigned fg, unsigned bg);
  void (*set_crs) (void *ext, unsigned y1, unsigned y2);
  void (*set_pos) (void *ext, unsigned x, unsigned y);

  void (*set_chr) (void *ext, unsigned x, unsigned y, unsigned char c);
  void (*set_pxl) (void *ext, unsigned x, unsigned y);

  void (*check) (void *ext);

  void *ext;
} terminal_t;


void trm_init (terminal_t *trmp);
void trm_free (terminal_t *trm);

void trm_del (terminal_t *trm);

void trm_set_size (terminal_t *trm, unsigned mode, unsigned w, unsigned h);
void trm_set_map (terminal_t *trm, unsigned i, unsigned r, unsigned g, unsigned b);
void trm_set_col (terminal_t *trm, unsigned fg, unsigned bg);

/*****************************************************************************
 * @short Set the cursor size
 * @param trm The terminal
 * @param y1  The first (lower) cursor line
 * @param y2  The second (higher) cursor line
 *
 * The lines y1 and y2 are a percentage in the range [0, 255] of the whole
 * character height.
 *****************************************************************************/
void trm_set_crs (terminal_t *trm, unsigned y1, unsigned y2);

void trm_set_pos (terminal_t *trm, unsigned x, unsigned y);

void trm_set_chr (terminal_t *trm, unsigned x, unsigned y, unsigned char c);

void trm_set_pxl (terminal_t *trm, unsigned x, unsigned y);

void trm_check (terminal_t *trm);


#endif
