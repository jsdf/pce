/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/cmd_ppc.h                                  *
 * Created:       2004-06-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-11 by Hampa Hug <hampa@hampa.ch>                   *
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


int ppc_match_reg (cmd_t *cmd, sim405_t *sim, uint32_t **reg);

void ppc_disasm_str (char *dst, p405_disasm_t *op);
void prt_state_ppc (p405_t *c, FILE *fp);
void prt_state_spr (p405_t *c, FILE *fp);
void prt_state_uic (p405_uic_t *uic, FILE *fp);

int ppc_do_cmd (cmd_t *cmd, sim405_t *sim);

void ppc_cmd_init (sim405_t *sim);
