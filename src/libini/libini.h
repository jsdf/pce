/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     libini.h                                                   *
 * Created:       2001-08-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: libini.h,v 1.2 2003/04/29 00:51:24 hampa Exp $ */


#ifndef LIBINI_H
#define LIBINI_H 1


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define LIBINI_VERSION_MAJ 0
#define LIBINI_VERSION_MIN 1
#define LIBINI_VERSION_MIC 0
#define LIBINI_VERSION_STR "0.1.0"


/*!************************************************************************
 * The value type identifiers
 **************************************************************************/
typedef enum {
  INI_VAL_LNG = 1,
  INI_VAL_DBL = 2,
  INI_VAL_STR = 3
} ini_type_t;


/*!************************************************************************
 * The value union
 **************************************************************************/
typedef union {
  char   *str;
  long   lng;
  double dbl;
} ini_val_u;


/*!************************************************************************
 * A value
 **************************************************************************/
typedef struct s_ini_val_t {
  struct s_ini_val_t *next;
  char               *name;
  ini_type_t         type;
  ini_val_u          val;
} ini_val_t;


ini_val_t *ini_val_new (const char *name, ini_type_t type);
void ini_val_del_val (ini_val_t *val);
void ini_val_del (ini_val_t *val);
ini_type_t ini_val_get_type (const ini_val_t *val);
ini_val_t *ini_val_get_next (const ini_val_t *val);
char *ini_val_get_name (const ini_val_t *val);
void ini_val_set_lng (ini_val_t *val, long lng);
void ini_val_set_dbl (ini_val_t *val, double dbl);
void ini_val_set_str (ini_val_t *val, char *str);
int ini_val_get_lng (const ini_val_t *val, long *lng);
int ini_val_get_dbl (const ini_val_t *val, double *dbl);
int ini_val_get_str (const ini_val_t *val, char **str);



typedef struct s_ini_sect_t {
  char                *name;
  unsigned            format;
  struct s_ini_sect_t *next;
  struct s_ini_sect_t *head;
  struct s_ini_sect_t *tail;
  ini_val_t           *val_head;
  ini_val_t           *val_tail;
} ini_sct_t;


ini_sct_t *ini_sct_new (const char *name);
void ini_sct_del (ini_sct_t *sct);

void ini_sct_set_name (ini_sct_t *sct, const char *name);
const char *ini_sct_get_name (const ini_sct_t *sct);

void ini_sct_set_format (ini_sct_t *sct, unsigned format, int rec);
unsigned ini_sct_get_format (const ini_sct_t *sct);

ini_sct_t *ini_sct_get_next (const ini_sct_t *sct);
ini_sct_t *ini_sct_get_head (const ini_sct_t *sct);
ini_val_t *ini_sct_get_val_head (const ini_sct_t *sct);

void ini_sct_add_sct (ini_sct_t *sct, ini_sct_t *add);
void ini_sct_add_val (ini_sct_t *sct, ini_val_t *val);


/*!***************************************************************************
 * @short Find a value in an ini tree
 * @param sct The base section
 * @param name The value name (as in "sect1.sect2.valname")
 * @return A pointer to the value or NULL if the value can't be found
 *****************************************************************************/
ini_val_t *ini_sct_find_val (const ini_sct_t *sct, const char *name);


/*!***************************************************************************
 * @short Find a section in an ini tree
 * @param sct The base section
 * @param name The section name (as in "sect1.sect2")
 * @return A pointer to the section or NULL if the value can't be found
 *****************************************************************************/
ini_sct_t *ini_sct_find_sct (ini_sct_t *sct, const char *name);


/*!***************************************************************************
 * @short Find the next section
 * @param sct The base section
 * @param name The section name (as in "sect1")
 * @return A pointer to the section or NULL if the value can't be found
 *
 * This looks for the next section on the same level as the base section.
 * The section name can only be a simple name for now.
 *****************************************************************************/
ini_sct_t *ini_sct_find_next (ini_sct_t *sct, const char *name);


/*!***************************************************************************
 * @short  Get a long value
 * @param  sct The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret The return value
 * @param  def The default value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_slng (const ini_sct_t *sct, const char *name,
  long *ret, long def);

int ini_get_ulng (const ini_sct_t *sct, const char *name,
  unsigned long *ret, unsigned long def);

int ini_get_sint (const ini_sct_t *sct, const char *name,
  int *ret, int def);

int ini_get_uint (const ini_sct_t *sct, const char *name,
  unsigned *ret, unsigned def);

/*!***************************************************************************
 * @short  Get a double value
 * @param  sct The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret The return value
 * @param  def The default value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_sdbl (const ini_sct_t *sct, const char *name,
  double *ret, double def);

/*!***************************************************************************
 * @short  Get a string value
 * @param  sct The base section
 * @param  name The value name (as in "sect1.sect2.valname")
 * @retval ret The string. This must not be changed by the caller.
 * @param  def The default value
 * @return 0 if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_string (const ini_sct_t *sct, const char *name,
  char **ret, char *def);


/*!***************************************************************************
 * @short Read an ini tree from a file
 * @param fp A C style stream that must be open for reading
 * @return A pointer to the base section of the ini tree. NULL on error.
 *****************************************************************************/
ini_sct_t *ini_read_fp (FILE *fp);

ini_sct_t *ini_read (const char *fname);


int ini_write_fp (ini_sct_t *sct, FILE *fp);
int ini_write (ini_sct_t *sct, const char *fname);


#endif
