/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/console.h                                            *
 * Created:     2006-06-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_CONSOLE_H
#define PCE_LIB_CONSOLE_H 1


#include <stdio.h>
#include <stdarg.h>


FILE *pce_get_redir_inp (void);
FILE *pce_get_redir_out (void);

FILE *pce_get_fp_inp (void);
FILE *pce_get_fp_out (void);

int pce_set_redir_inp (const char *fname);
int pce_set_redir_out (const char *fname, const char *mode);

int pce_gets (const char *prompt, char *str, unsigned max);
void pce_puts (const char *str);
void pce_printf (const char *msg, ...);
void pce_vprintf (const char *msg, va_list va);

void pce_prt_sep (const char *str);

void pce_console_init (FILE *inp, FILE *out);
void pce_console_done (void);


#endif
