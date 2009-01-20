/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/cmd_arm.h                                    *
 * Created:     2004-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


void sarm_dasm_str (char *dst, arm_dasm_t *op);
void sarm_prt_state_cpu (arm_t *c, FILE *fp);
void sarm_prt_state_mmu (arm_t *c, FILE *fp);
void sarm_prt_state_timer (ixp_timer_t *tmr, FILE *fp);
void sarm_prt_state_intc (simarm_t *sim, FILE *fp);
void sarm_prt_state_mem (simarm_t *sim, FILE *fp);

void sarm_run (simarm_t *sim);

int sarm_do_cmd (simarm_t *sim, cmd_t *cmd);

void sarm_cmd_init (simarm_t *sim);
