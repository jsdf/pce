/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/log.h                                            *
 * Created:       2003-02-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-23 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: log.h,v 1.1 2003/04/23 12:48:42 hampa Exp $ */


#ifndef PCE_LOG_H
#define PCE_LOG_H 1


#define MSG_ERR 0
#define MSG_MSG 1
#define MSG_INF 2
#define MSG_DEB 3


void pce_log_set_level (unsigned level);
unsigned pce_log_get_level (void);

void pce_log_set_fp (FILE *fp, int close);
void pce_log_set_fname (const char *fname);
void pce_log_set_stderr (int f);

void pce_log (unsigned level, const char *msg, ...);


#endif
