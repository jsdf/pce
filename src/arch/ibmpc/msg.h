/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/msg.h                                       *
 * Created:       2004-09-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-04-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_MSG_H
#define PCE_MSG_H 1


int pc_set_msg (ibmpc_t *pc, const char *msg, const char *val);
int pc_get_msg (ibmpc_t *pc, const char *msg, char *val, unsigned max);

int pc_set_msgul (ibmpc_t *pc, const char *msg, unsigned long val);
int pc_get_msgul (ibmpc_t *pc, const char *msg, unsigned long *val);


#endif
