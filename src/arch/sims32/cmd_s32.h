/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sims32/cmd_s32.h                                  *
 * Created:       2004-09-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
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


int ss32_match_reg (cmd_t *cmd, sims32_t *sim, uint32_t **reg);

void ss32_dasm_str (char *dst, s32_dasm_t *op);

void ss32_prt_state_cpu (sparc32_t *c, FILE *fp);
void ss32_prt_state_mem (sims32_t *sim, FILE *fp);

int ss32_do_cmd (cmd_t *cmd, sims32_t *sim);

void ss32_cmd_init (sims32_t *sim);
