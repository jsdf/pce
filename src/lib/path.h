/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/path.h                                               *
 * Created:     2008-11-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_PATH_H
#define PCE_LIB_PATH_H 1


#include <libini/libini.h>


/*!***************************************************************************
 * @short Clear the search path
 *****************************************************************************/
void pce_path_clear (void);

/*!***************************************************************************
 * @short Add a directory to the search path
 * @param dir   The directory
 * @param atend If true, add the directory at the end of the path, otherwise
 *              add it to the beginning.
 *****************************************************************************/
int pce_path_add (const char *dir, int atend);

/*!***************************************************************************
 * @short Add a directory to the search path
 * @param dir   The directory
 *
 * The first character of dir determines where the directory is added:
 * '+': at end
 * '-': at beginning
 * '=': clear before adding
 *****************************************************************************/
int pce_path_set (const char *dir);

/*!***************************************************************************
 * @short  Get a path
 * @param  fname The file name
 * @return The path name or NULL if fname is NULL
 *
 * This function looks for a file named fname in every directory of the
 * search path. If a file is found its full path is returned. If no file
 * is found a copy of fname is returned.
 *****************************************************************************/
char *pce_path_get (const char *fname);

/*!***************************************************************************
 * @short Open a file for input
 *****************************************************************************/
FILE *pce_fopen_inp (const char *fname, const char *mode, char **path);

FILE *pce_fopen_out (const char *fname, const char *mode);

/*!***************************************************************************
 * @short Add all "path" entries from an ini section to the search path
 *****************************************************************************/
int pce_path_ini (ini_sct_t *sct);


#endif
