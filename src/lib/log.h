/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/log.h                                                *
 * Created:     2003-02-02 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_LOG_H
#define PCE_LIB_LOG_H 1


#include <stdarg.h>
#include <stdio.h>


#define PCE_LOG_MAX 16

#define MSG_ERR 0
#define MSG_MSG 1
#define MSG_INF 2
#define MSG_DEB 3


void pce_log_init (void);
void pce_log_done (void);

int pce_log_add_fp (FILE *fp, int close, unsigned level);
int pce_log_add_fname (const char *fname, unsigned level);
void pce_log_rmv_fp (FILE *fp);

void pce_log_set_level (FILE *fp, unsigned level);
unsigned pce_log_get_level (FILE *fp);

void pce_log (unsigned level, const char *msg, ...);
void pce_log_va (unsigned level, const char *msg, va_list va);
void pce_log_deb (const char *msg, ...);
void pce_log_tag (unsigned level, const char *tag, const char *msg, ...);


#endif
