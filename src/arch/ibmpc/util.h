/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/util.h                                      *
 * Created:       2004-06-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-08-02 by Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_IBMPC_UTIL_H
#define PCE_IBMPC_UTIL_H 1


int str_istail (const char *str, const char *tail);
int str_isarg (const char *str, const char *arg1, const char *arg2);

void prt_error (const char *str, ...);
void pce_set_fd (int fd, int interactive);

int pce_load_blk_bin (mem_blk_t *blk, const char *fname);

int pce_load_mem_hex (memory_t *mem, const char *fname);

int pce_load_mem_bin (memory_t *mem, const char *fname, unsigned long base);

ini_sct_t *pce_load_config (const char *fname);


#endif
