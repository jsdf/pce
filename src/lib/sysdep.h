/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/sysdep.h                                             *
 * Created:     2006-06-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_SYSDEP_H
#define PCE_LIB_SYSDEP_H 1


#define PCE_BRK_STOP  1
#define PCE_BRK_ABORT 2


int pce_usleep (unsigned long usec);

/*!***************************************************************************
 * @short Get an interval in microseconds
 *
 * Returns the number of microseconds that passed since this function
 * was called the last time with the same parameter.
 *****************************************************************************/
unsigned long pce_get_interval_us (unsigned long *val);

void pce_srand (unsigned val);

void pce_set_fd_interactive (int fd, int interactive);

/*!***************************************************************************
 * @short Check if a file descriptor is readable without blocking
 * @param fd The file descriptor
 * @param t  Wait at most t milliseconds for fd to become readable
 *****************************************************************************/
int pce_fd_readable (int fd, int t);

/*!***************************************************************************
 * @short Check if a file descriptor is writeable without blocking
 * @param fd The file descriptor
 * @param t  Wait at most t milliseconds for fd to become writeable
 *****************************************************************************/
int pce_fd_writeable (int fd, int t);


void pce_start (unsigned *brk);

void pce_stop (void);


#endif
