/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/libini.h                                          *
 * Created:     2001-08-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef LIBINI_H
#define LIBINI_H 1


#include <stdio.h>


/*!***************************************************************************
 * @short The value type identifiers
 *****************************************************************************/
#define INI_VAL_NONE 0
#define INI_VAL_INT  1
#define INI_VAL_STR  2


/*!***************************************************************************
 * @short The value struct
 *****************************************************************************/
typedef struct ini_val_s {
	struct ini_val_s *next;

	char             *name;
	unsigned         type;

	union {
		char          *str;
		unsigned long u32;
	} val;
} ini_val_t;


/*!***************************************************************************
 * @short The ini section struct
 *****************************************************************************/
typedef struct ini_sct_s {
	struct ini_sct_s *next;

	struct ini_sct_s *parent;

	char             *name;

	struct ini_sct_s *sub_head;
	struct ini_sct_s *sub_tail;

	ini_val_t        *val_head;
	ini_val_t        *val_tail;
} ini_sct_t;


typedef struct {
	unsigned cnt;
	unsigned max;
	char     *str;
} ini_strings_t;


void ini_val_init (ini_val_t *val, const char *name);

void ini_val_free (ini_val_t *val);

ini_val_t *ini_val_new (const char *name);

void ini_val_del (ini_val_t *val);

void ini_val_set_none (ini_val_t *val);

void ini_val_copy (ini_val_t *dst, const ini_val_t *src);

/*!***************************************************************************
 * @short Set a value to an unsigned long value
 * @param val The value
 * @param v   The long value
 *****************************************************************************/
void ini_val_set_uint32 (ini_val_t *val, unsigned long v);

/*!***************************************************************************
 * @short Set a value to a long value
 * @param val The value
 * @param v   The long value
 *****************************************************************************/
void ini_val_set_sint32 (ini_val_t *val, long v);

/*!***************************************************************************
 * @short Set a value to a boolean value
 * @param val The value
 * @param v   The boolean value
 *****************************************************************************/
void ini_val_set_bool (ini_val_t *val, int v);

/*!***************************************************************************
 * @short Set a value to a string value
 * @param val The value
 * @param v   The string value
 *****************************************************************************/
void ini_val_set_str (ini_val_t *val, const char *v);


/*!***************************************************************************
 * @short  Get an unsigned long value
 * @param  val The value
 * @retval v   The uint32 value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_uint32 (const ini_val_t *val, unsigned long *v);

/*!***************************************************************************
 * @short  Get a long value
 * @param  val The value
 * @retval v   The sint32 value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_sint32 (const ini_val_t *val, long *v);

/*!***************************************************************************
 * @short  Get an unsigned int value
 * @param  val The value
 * @retval v   The uint16 value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_uint16 (const ini_val_t *val, unsigned *v);

/*!***************************************************************************
 * @short  Get an int value
 * @param  val The value
 * @retval v   The sint16 value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_sint16 (const ini_val_t *val, int *v);

/*!***************************************************************************
 * @short  Get a boolean value
 * @param  val The value
 * @retval v   The boolean value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_bool (const ini_val_t *val, int *v);

/*!***************************************************************************
 * @short  Get a string value
 * @param  val The value
 * @return The string value or NULL on error
 *****************************************************************************/
const char *ini_val_get_str (const ini_val_t *val);


/*!***************************************************************************
 * @short  Create a new section
 * @param  name The section name
 * @return The new section or NULL on error
 *****************************************************************************/
ini_sct_t *ini_sct_new (const char *name);

/*!***************************************************************************
 * @short Delete a section list and all subsections
 * @param sct The section
 *****************************************************************************/
void ini_sct_del (ini_sct_t *sct);

ini_sct_t *ini_next_sct (ini_sct_t *sct, ini_sct_t *val, const char *name);
ini_val_t *ini_next_val (ini_sct_t *sct, ini_val_t *val, const char *name);

ini_sct_t *ini_get_sct (ini_sct_t *sct, const char *name, int add);
ini_val_t *ini_get_val (ini_sct_t *sct, const char *name, int add);


/*!***************************************************************************
 * @short  Set an unsigned long value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @param  v    The value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_set_uint32 (ini_sct_t *sct, const char *name, unsigned long v);

/*!***************************************************************************
 * @short  Set a long value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @param  v    The long value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_set_sint32 (ini_sct_t *sct, const char *name, long v);

/*!***************************************************************************
 * @short  Set a string value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @param  v    The string value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_set_str (ini_sct_t *sct, const char *name, const char *v);


/*!***************************************************************************
 * @short  Get a value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_uint32 (const ini_sct_t *sct, const char *name, unsigned long *ret, unsigned long def);

/*!***************************************************************************
 * @short  Get a value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_sint32 (const ini_sct_t *sct, const char *name, long *ret, long def);

/*!***************************************************************************
 * @short  Get a value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_uint16 (const ini_sct_t *sct, const char *name, unsigned *ret, unsigned def);

/*!***************************************************************************
 * @short  Get a value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_sint16 (const ini_sct_t *sct, const char *name, int *ret, int def);

/*!***************************************************************************
 * @short  Get a boolean value
 * @param  val The value
 * @retval v   The boolean value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_bool (const ini_sct_t *sct, const char *name, int *ret, int def);

/*!***************************************************************************
 * @short  Get a string value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @return The string or NULL on error
 *****************************************************************************/
int ini_get_string (const ini_sct_t *sct, const char *name, const char **ret, const char *def);


void ini_str_init (ini_strings_t *is);
void ini_str_free (ini_strings_t *is);
int ini_str_add (ini_strings_t *is, const char *s1, const char *s2, const char *s3);
int ini_str_eval (ini_strings_t *is, ini_sct_t *sct, int free);


int ini_read_str (ini_sct_t *sct, const char *str);

/*!***************************************************************************
 * @short  Read an ini tree from a file
 * @param  fp A C style stream that must be open for reading
 * @return True on error
 *****************************************************************************/
int ini_read_fp (ini_sct_t *sct, FILE *fp, const char *fname);

/*!***************************************************************************
 * @short  Read an ini tree from a file
 * @param  fname The file name
 * @return True on error
 *****************************************************************************/
int ini_read (ini_sct_t *sct, const char *fname);


/*!***************************************************************************
 * @short  Write an ini tree to a file
 * @param  fp A C style stream that must be open for writing
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_write_fp (FILE *fp, ini_sct_t *sct);

/*!***************************************************************************
 * @short  Write an ini tree to a file
 * @param  fname The file name
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_write (const char *fname, ini_sct_t *sct);


#endif
