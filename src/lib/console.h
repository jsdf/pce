/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/console.h                                          *
 * Created:       2006-06-19 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2006 Hampa Hug <hampa@hampa.ch>                        *
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


#ifndef PCE_LIB_CONSOLE_H
#define PCE_LIB_CONSOLE_H 1


#include <stdio.h>


void pce_set_fd_interactive (int fd, int interactive);


/*!***************************************************************************
 * @short Check if a file descriptor is readable
 * @param fd The file descriptor
 * @param t  Wait at most t milliseconds for fd to become readable
 *****************************************************************************/
int pce_fd_readable (int fd, int t);

void pce_prt_sep (FILE *fp, const char *str, ...);

void pce_prt_prompt (FILE *fp, const char *str);


#endif
