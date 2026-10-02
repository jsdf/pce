/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   pce.h                                                        *
 * Created:     2018-12-22 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCEUTILS_PCE_H
#define PCEUTILS_PCE_H 1


int pce_check (void);
int pce_get_version (unsigned *p);
int pce_get_version_str (char *str, unsigned max);

int pce_init (void);
int pce_putc (int c);
int pce_putc0 (int c);
int pce_puts (const char *s);
int pce_puts0 (const char *s);
int pce_getc (void);

void pce_stop (void);
void pce_abort (void);

int pce_set_msg (const char *msg, const char *val);

long long pce_get_time_unix (void);

int pce_read_open (const char *s);
int pce_read_close (void);
int pce_read16 (void *buf);

int pce_write_open (const char *s);
int pce_write_close (void);
int pce_write16 (void *buf, unsigned cnt);


#endif
