/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/hook.h                                           *
 * Created:       2003-09-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-14 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: hook.h,v 1.3 2003/09/14 21:27:38 hampa Exp $ */


#ifndef PCE_HOOK_H
#define PCE_HOOK_H 1


#define PCEH_STOP      0x0000
#define PCEH_ABORT     0x0100

#define PCEH_SET_BOOT  0x0001

#define PCEH_GET_BOOT  0x0002
#define PCEH_GET_COM   0x0102
#define PCEH_GET_LPT   0x0202
#define PCEH_GET_VIDEO 0x0302

#define PCEH_XMS       0x0003
#define PCEH_XMS_INFO  0x0103

#define PCEH_INT       0x00cd


void pc_int_15 (ibmpc_t *pc);
void pc_int_1a (ibmpc_t *pc);

void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);


#endif
