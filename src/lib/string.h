/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/string.h                                             *
 * Created:     2009-06-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_LIB_STRING_H
#define PCE_LIB_STRING_H 1


/*!***************************************************************************
 * @short  Concatenate two strings
 * @param  str1  The first string
 * @param  str2  The second string
 * @return The concatenation of str1 and str2, allocated with malloc()
 *****************************************************************************/
char *str_cat_alloc (const char *str1, const char *str2);

/*!***************************************************************************
 * @short  Copy a part of a string
 * @param  str  The source string
 * @param  cnt  The maximum number of bytes to be copied
 * @return The new string, allocated with malloc()
 *****************************************************************************/
char *str_cut_alloc (const char *str, unsigned long cnt);

/*!***************************************************************************
 * @short  Copy a string
 * @param  str  The source string
 * @return The new string, allocated with malloc()
 *****************************************************************************/
char *str_copy_alloc (const char *str);

/*!***************************************************************************
 * @short  Remove characters from the start and/or end of a string
 * @param  str    The source string
 * @param  left   Characters to be removed from the start or NULL
 * @param  right  Characters to be removed from the right or NULL
 * @return The new string, allocated with malloc()
 *****************************************************************************/
char *str_trim (char *str, const char *left, const char *right);

/*!***************************************************************************
 * @short  Remove characters from the start of a string
 * @param  str    The source string
 * @param  left   Characters to be removed from the start or NULL
 * @return The new string
 *****************************************************************************/
const char *str_ltrim (const char *str, const char *left);

/*!***************************************************************************
 * @short  Extract a prefix of a string
 * @param  str    The source string
 * @param  sep    Characters that end the prefix
 * @retval rest   If not NULL, receives a pointer to the rest of the string
 * @return The new string, allocated with malloc()
 *****************************************************************************/
char *str_extract_alloc (const char *str, const char *sep, const char **rest);


#endif
