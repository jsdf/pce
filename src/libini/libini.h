/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/libini.h                                        *
 * Created:       2001-08-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-02-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2004 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#ifndef LIBINI_H
#define LIBINI_H 1


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define LIBINI_VERSION_MAJ 0
#define LIBINI_VERSION_MIN 1
#define LIBINI_VERSION_MIC 0
#define LIBINI_VERSION_STR "0.1.0"


/*!***************************************************************************
 * @short The value type identifiers
 *****************************************************************************/
typedef enum {
  INI_VAL_LNG = 1,
  INI_VAL_DBL = 2,
  INI_VAL_STR = 3
} ini_type_t;


/*!***************************************************************************
 * @short The value union
 *****************************************************************************/
typedef union {
  char   *str;
  long   lng;
  double dbl;
} ini_val_u;


/*!***************************************************************************
 * @short The value struct
 *****************************************************************************/
typedef struct ini_val_s {
  struct ini_val_s *next;
  char             *name;
  ini_type_t       type;
  ini_val_u        val;
} ini_val_t;


/*!***************************************************************************
 * @short  Create a new value
 * @param  name The value name
 * @return A pointer to the value or NULL on error
 *
 * The type and value of the new value are undefined
 *****************************************************************************/
ini_val_t *ini_val_new (const char *name);

/*!***************************************************************************
 * @short  Delete a list of values
 * @param  val The value
 *****************************************************************************/
void ini_val_del (ini_val_t *val);

/*!***************************************************************************
 * @short  Get a value's name
 * @param  val The value
 * @return The value's name
 *****************************************************************************/
const char *ini_val_get_name (const ini_val_t *val);

/*!***************************************************************************
 * @short  Get a value's type
 * @param  val The value
 * @return The value's type
 *****************************************************************************/
ini_type_t ini_val_get_type (const ini_val_t *val);

/*!***************************************************************************
 * @short  Get the next value in a list of values
 * @param  val The value
 * @return The next value
 *****************************************************************************/
ini_val_t *ini_val_get_next (const ini_val_t *val);

/*!***************************************************************************
 * @short  Find the next value by name
 * @param  val  The current value
 * @param  name The value name
 * @return A pointer to the value or NULL if the value can't be found
 *****************************************************************************/
ini_val_t *ini_val_find_next (const ini_val_t *val, const char *name);


/*!***************************************************************************
 * @short Set a value to a long value
 * @param val The value
 * @param v   The long value
 *****************************************************************************/
void ini_val_set_lng (ini_val_t *val, long v);

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
 * @short  Get a long value
 * @param  val The value
 * @retval v   The long value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_val_get_lng (const ini_val_t *val, long *v);

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
 * @short The ini section struct
 *****************************************************************************/
typedef struct ini_sect_s {
  char              *name;
  unsigned          format;
  struct ini_sect_s *next;
  struct ini_sect_s *head;
  struct ini_sect_s *tail;
  ini_val_t         *val_head;
  ini_val_t         *val_tail;
} ini_sct_t;


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


/*!***************************************************************************
 * @short Set the section name
 * @param sct  The section
 * @param name The new section name
 *****************************************************************************/
void ini_sct_set_name (ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short  Get the section name
 * @param  sct The section
 * @return The section name
 *****************************************************************************/
const char *ini_sct_get_name (const ini_sct_t *sct);


/*!***************************************************************************
 * @short Set the section format
 * @param sct    The section
 * @param format The new section format
 * @param rec    If true set the format on all subsections
 *****************************************************************************/
void ini_sct_set_format (ini_sct_t *sct, unsigned format, int rec);

/*!***************************************************************************
 * @short  Get the section format
 * @param  sct The section
 * @return The section format
 *****************************************************************************/
unsigned ini_sct_get_format (const ini_sct_t *sct);


/*!***************************************************************************
 * @short  Get the next section in a section list
 * @param  sct The section
 * @return The next section or NULL if this is the last section
 *****************************************************************************/
ini_sct_t *ini_sct_get_next (const ini_sct_t *sct);

/*!***************************************************************************
 * @short  Get the first subsection
 * @param  sct The section
 * @return The first subsection or NULL if there aren't any
 *****************************************************************************/
ini_sct_t *ini_sct_get_head (const ini_sct_t *sct);

/*!***************************************************************************
 * @short  Get the first value
 * @param  sct The section
 * @return The first value or NULL if there aren't any
 *****************************************************************************/
ini_val_t *ini_sct_get_val_head (const ini_sct_t *sct);


/*!***************************************************************************
 * @short Add a subsection
 * @param sct The section
 * @param add The subsection
 *****************************************************************************/
void ini_sct_add_sct (ini_sct_t *sct, ini_sct_t *add);

/*!***************************************************************************
 * @short Add a value to a section
 * @param sct The section
 * @param val The value
 *****************************************************************************/
void ini_sct_add_val (ini_sct_t *sct, ini_val_t *val);

/*!***************************************************************************
 * @short  Add a new value to a section
 * @param  sct  The section
 * @param  name The new value name
 * @return The new value
 *
 * If a value of the given name already exists no new value is created and
 * a pointer to the existing value is returned.
 *****************************************************************************/
ini_val_t *ini_sct_new_val (ini_sct_t *sct, const char *name);


/*!***************************************************************************
 * @short  Find a value in an ini tree
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @return A pointer to the value or NULL if the value can't be found
 *
 * The first value of the given name is returned. Use ini_val_find_next()
 * to find more values of the same name.
 *****************************************************************************/
ini_val_t *ini_sct_find_val (const ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short  Find a section in an ini tree
 * @param  sct  The base section
 * @param  name The section name (as in "sect1.sect2")
 * @return A pointer to the section or NULL if the section can't be found
 *****************************************************************************/
ini_sct_t *ini_sct_find_sct (ini_sct_t *sct, const char *name);

/*!***************************************************************************
 * @short  Find the next section
 * @param  sct The base section
 * @param  name The section name (as in "sect1")
 * @return A pointer to the section or NULL if the value can't be found
 *
 * This looks for the next section on the same level as the base section.
 * The section name can only be a simple name for now.
 *****************************************************************************/
ini_sct_t *ini_sct_find_next (ini_sct_t *sct, const char *name);


/*!***************************************************************************
 * @short  Set a long value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @param  v    The long value
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_set_lng (ini_sct_t *sct, const char *name, long v);

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
 * @short  Get a long value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_lng (const ini_sct_t *sct, const char *name, long *ret);

/*!***************************************************************************
 * @short  Get a double value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret  The return value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_dbl (const ini_sct_t *sct, const char *name, double *ret);

/*!***************************************************************************
 * @short  Get a string value
 * @param  sct  The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @return The string or NULL on error
 *****************************************************************************/
const char *ini_get_str (const ini_sct_t *sct, const char *name);

long ini_get_lng_def (ini_sct_t *sct, const char *name, long def);
double ini_get_dbl_def (ini_sct_t *sct, const char *name, double def);
const char *ini_get_str_def (ini_sct_t *sct, const char *name, const char *def);


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
