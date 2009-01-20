/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/libini.h                                          *
 * Created:     2001-08-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2009 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by  the Free Software Foundation.                                         *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef LIBINI_H
#define LIBINI_H 1


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*!***************************************************************************
 * @short The value type identifiers
 *****************************************************************************/
#define INI_VAL_NONE 0
#define INI_VAL_U32  1
#define INI_VAL_S32  2
#define INI_VAL_DBL  3
#define INI_VAL_STR  4


/*!***************************************************************************
 * @short The value struct
 *****************************************************************************/
typedef struct ini_val_s {
	char             *name;
	unsigned         type;

	union {
		char          *str;
		unsigned long u32;
		long          s32;
		double        dbl;
	} val;
} ini_val_t;


/*!***************************************************************************
 * @short The ini section struct
 *****************************************************************************/
typedef struct ini_sect_s {
	char              *name;
	unsigned          format;

	struct ini_sect_s *sct;
	unsigned          sctcnt;

	ini_val_t         *val;
	unsigned          valcnt;
} ini_sct_t;


/*!***************************************************************************
 * @short Initialize a value
 * @param name The value name
 *****************************************************************************/
void ini_val_init (ini_val_t *val, const char *name);

/*!***************************************************************************
 * @short Free a value
 * @param val The value
 *****************************************************************************/
void ini_val_free (ini_val_t *val);

void ini_val_set_none (ini_val_t *val);

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
 * @short Set a value to a double value
 * @param val The value
 * @param v   The double value
 *****************************************************************************/
void ini_val_set_dbl (ini_val_t *val, double v);

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
 * @short  Get a double value
 * @param  val The value
 * @retval v   The double value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_dbl (const ini_val_t *val, double *v);

/*!***************************************************************************
 * @short  Get a string value
 * @param  val The value
 * @return The string value or NULL on error
 *****************************************************************************/
const char *ini_val_get_str (const ini_val_t *val);


/*!***************************************************************************
 * @short Initialize a section
 * @param name The section name
 *****************************************************************************/
void ini_sct_init (ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short Free a section
 *****************************************************************************/
void ini_sct_free (ini_sct_t *sct);

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

void ini_sct_set_format (ini_sct_t *sct, unsigned format, int rec);

ini_sct_t *ini_get_sct_idx (const ini_sct_t *sct, unsigned i);
ini_val_t *ini_get_val_idx (const ini_sct_t *sct, unsigned i);

ini_sct_t *ini_next_sct (ini_sct_t *sct, ini_sct_t *val, const char *name);
ini_val_t *ini_next_val (ini_sct_t *sct, ini_val_t *val, const char *name);

ini_sct_t *ini_get_sct (ini_sct_t *sct, const char *name, int add);
ini_val_t *ini_get_val (ini_sct_t *sct, const char *name, int add);

/*!***************************************************************************
 * @short  Add a subsection
 * @param  sct  The parent section
 * @param  name The subsection name
 * @return The new section
 *****************************************************************************/
ini_sct_t *ini_new_sct (ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short  Add a value
 * @param  sct  The parent section
 * @param  name The value name
 * @return The new value
 *****************************************************************************/
ini_val_t *ini_new_val (ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short  Find a value in an ini tree
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @return A pointer to the value or NULL if the value can't be found
 *
 * The first value of the given name is returned. Use ini_val_find_next()
 * to find more values of the same name.
 *****************************************************************************/
ini_val_t *ini_find_val (const ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short  Find a section in an ini tree
 * @param  sct  The base section
 * @param  name The section name (as in "sect1.sect2")
 * @return A pointer to the section or NULL if the section can't be found
 *****************************************************************************/
ini_sct_t *ini_find_sct (ini_sct_t *sct, const char *name);


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
 * @short  Set a double value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @param  v    The double value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_set_dbl (ini_sct_t *sct, const char *name, double v);

/*!***************************************************************************
 * @short  Set a string value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @param  ret  The string value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_set_str (ini_sct_t *sct, const char *name, const char *ret);


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
 * @short  Get a double value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_dbl (const ini_sct_t *sct, const char *name, double *ret, double def);

/*!***************************************************************************
 * @short  Get a string value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @return The string or NULL on error
 *****************************************************************************/
int ini_get_string (const ini_sct_t *sct, const char *name, const char **ret, const char *def);


/*!***************************************************************************
 * @short  Read an ini tree from a file
 * @param  fp A C style stream that must be open for reading
 * @return A pointer to the base section of the ini tree or NULL on error.
 *****************************************************************************/
ini_sct_t *ini_read_fp (FILE *fp);

/*!***************************************************************************
 * @short  Read an ini tree from a file
 * @param  fname The file name
 * @return A pointer to the base section of the ini tree or NULL on error.
 *****************************************************************************/
ini_sct_t *ini_read (const char *fname);


/*!***************************************************************************
 * @short  Write an ini tree to a file
 * @param  fp A C style stream that must be open for writing
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_write_fp (ini_sct_t *sct, FILE *fp);

/*!***************************************************************************
 * @short  Write an ini tree to a file
 * @param  fname The file name
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_write (ini_sct_t *sct, const char *fname);


#endif
