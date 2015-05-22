/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/path.h                                          *
 * Created:     2013-01-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DOS_PATH_H
#define PCE_DOS_PATH_H 1


void sim_buf_init (void **buf, unsigned *cnt, unsigned *max);
int sim_buf_add (void **buf, unsigned *cnt, unsigned *max, unsigned char val);

char *sim_get_dir_name (const char *src, char sep);
char *sim_make_path (const char *s1, const char *s2);

int sim_get_dos_basename (char *dst, const char *src, char sep);
char *sim_get_dos_full_name (dos_t *sim, const char *name);
char *sim_get_host_name (dos_t *sim, const char *dosname);


#endif
