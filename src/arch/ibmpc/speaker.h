/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/speaker.h                                        *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: speaker.h,v 1.1 2003/12/20 01:01:34 hampa Exp $ */


#ifndef PCE_SPEAKER_H
#define PCE_SPEAKER_H 1


typedef struct {
  ibmpc_t            *pc;

  unsigned char      ppi_val;
  unsigned char      pit_val;
  unsigned char      cur_val;

  unsigned long long clk_cnt;
  unsigned long long clk_max;

  FILE               *fp;
} speaker_t;


#endif
