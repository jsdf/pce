/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/cmd_arm.h                                  *
 * Created:       2004-11-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-11 by Hampa Hug <hampa@hampa.ch>                   *
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


int sarm_match_reg (cmd_t *cmd, simarm_t *sim, uint32_t **reg);

void sarm_dasm_str (char *dst, arm_dasm_t *op);
void sarm_prt_state_cpu (arm_t *c, FILE *fp);
void sarm_prt_state_mmu (arm_t *c, FILE *fp);

int sarm_do_cmd (cmd_t *cmd, simarm_t *sim);

void sarm_cmd_init (simarm_t *sim);
