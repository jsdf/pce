/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/hook.h                                        *
 * Created:     2003-09-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_HOOK_H
#define PCE_HOOK_H 1


#include "ibmpc.h"


#define PCEH_STOP      0x0000
#define PCEH_ABORT     0x0100

#define PCEH_SET_BOOT  0x0001
#define PCEH_SET_INT28 0x0401
#define PCEH_SET_CPU   0x0501
#define PCEH_SET_AMSK  0x0601

#define PCEH_GET_BOOT  0x0002
#define PCEH_GET_COM   0x0102
#define PCEH_GET_LPT   0x0202
#define PCEH_GET_VIDEO 0x0302
#define PCEH_GET_INT28 0x0402
#define PCEH_GET_CPU   0x0502
#define PCEH_GET_AMSK  0x0602
#define PCEH_GET_VERS  0x0702
#define PCEH_GET_HDCNT 0x0802
#define PCEH_GET_FDCNT 0x0902

#define PCEH_XMS       0x0003
#define PCEH_XMS_INFO  0x0103

#define PCEH_EMS       0x0004
#define PCEH_EMS_INFO  0x0104

#define PCEH_MSG       0x0005

#define PCEH_CHECK_INT 0x0006

#define PCEH_INT       0x00cd


void pc_int_15 (ibmpc_t *pc);
void pc_int_1a (ibmpc_t *pc);

void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);


#endif
