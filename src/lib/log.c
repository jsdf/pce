/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/log.c                                              *
 * Created:       2003-02-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-11-08 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: log.c,v 1.1 2003/11/08 14:40:34 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"


static unsigned log_level = MSG_DEB;

static FILE     *log_fp = NULL;
static int       log_close_fp = 0;
static int      log_stderr = 1;


void pce_log_set_level (unsigned level)
{
  log_level = level;
}

unsigned pce_log_get_level (void)
{
  return (log_level);
}

void pce_log_set_fp (FILE *fp, int close)
{
  if (log_close_fp) {
    fclose (log_fp);
  }

  log_fp = fp;
  log_close_fp = (fp != NULL) && close;
}

void pce_log_set_fname (const char *fname)
{
  if (log_close_fp) {
    fclose (log_fp);
  }

  log_fp = fopen (fname, "ab");
  log_close_fp = (log_fp != NULL);
}

void pce_log_set_stderr (int f)
{
  log_stderr = (f != 0);
}

void pce_log (unsigned level, const char *msg, ...)
{
  va_list va;

  if (level <= log_level) {
    if (log_fp != NULL) {
      va_start (va, msg);
      vfprintf (log_fp, msg, va);
      va_end (va);
      fflush (log_fp);
    }

    if (log_stderr) {
      va_start (va, msg);
      vfprintf (stderr, msg, va);
      va_end (va);
      fflush (stderr);
    }
  }
}
