/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/cmd_ppc.h                                  *
 * Created:       2004-06-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                       *
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

/* $Id$ */


int ppc_match_reg (cmd_t *cmd, sim405_t *sim, uint32_t **reg);

void ppc_disasm_str (char *dst, p405_disasm_t *op);

void s405_prt_state_ppc (sim405_t *sim, FILE *fp);
void s405_prt_state_spr (p405_t *c, FILE *fp);
void s405_prt_state_uic (p405_uic_t *uic, FILE *fp);
void s405_prt_state_mem (sim405_t *sim, FILE *fp);

int ppc_do_cmd (sim405_t *sim, cmd_t *cmd);

void ppc_cmd_init (sim405_t *sim);
