/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     value.c                                                    *
 * Created:       2001-08-24 by Hampa Hug <hhug@student.ethz.ch>             *
 * Last modified: 2002-08-27 by Hampa Hug <hhug@student.ethz.ch>             *
 * Copyright:     (C) 2001-2002 by Hampa Hug <hhug@student.ethz.ch>          *
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

/* $Id: value.c,v 1.2 2003/08/19 00:52:10 hampa Exp $ */


#include "libini.h"


ini_val_t *ini_val_new (const char *name, ini_type_t type)
{
  ini_val_t *val;

  val = (ini_val_t *) malloc (sizeof (ini_val_t));
  if (val == NULL) {
    return (NULL);
  }

  val->name = (char *) malloc (strlen (name) + 1);
  if (val->name == NULL) {
    free (val);
    return (NULL);
  }

  strcpy (val->name, name);

  val->next = NULL;
  val->type = type;
  val->val.str = NULL;

  return (val);
}

void ini_val_del_val (ini_val_t *val)
{
  if (val->type == INI_VAL_STR) {
    free (val->val.str);
    val->val.str = NULL;
  }
}

void ini_val_del (ini_val_t *val)
{
  if (val != NULL) {
    if (val->next != NULL) {
      ini_val_del (val->next);
    }

    ini_val_del_val (val);

    free (val->name);
    free (val);
  }
}

ini_type_t ini_val_get_type (const ini_val_t *val)
{
  return (val->type);
}

ini_val_t *ini_val_get_next (const ini_val_t *val)
{
  return (val->next);
}

char *ini_val_get_name (const ini_val_t *val)
{
  return (val->name);
}

ini_val_t *ini_val_find_next (const ini_val_t *val, const char *name)
{
  if (val != NULL) {
    val = val->next;
  }

  while (val != NULL) {
    if (strcmp (val->name, name) == 0) {
      return ((ini_val_t *) val);
    }

    val = val->next;
  }

  return (NULL);
}

void ini_val_set_lng (ini_val_t *val, long lng)
{
  ini_val_del_val (val);

  val->type = INI_VAL_LNG;
  val->val.lng = lng;
}

void ini_val_set_dbl (ini_val_t *val, double dbl)
{
  ini_val_del_val (val);

  val->type = INI_VAL_DBL;
  val->val.dbl = dbl;
}

void ini_val_set_str (ini_val_t *val, char *str)
{
  ini_val_del_val (val);

  val->type = INI_VAL_STR;

  val->val.str = (char *) malloc (strlen (str) + 1);
  if (val->val.str == NULL) {
    return;
  }

  strcpy (val->val.str, str);
}

int ini_val_get_lng (const ini_val_t *val, long *lng)
{
  if (val->type != INI_VAL_LNG) {
    return (1);
  }

  *lng = val->val.lng;

  return (0);
}

int ini_val_get_dbl (const ini_val_t *val, double *dbl)
{
  if (val->type != INI_VAL_DBL) {
    return (1);
  }

  *dbl = val->val.dbl;

  return (0);
}

int ini_val_get_str (const ini_val_t *val, char **str)
{
  if (val->type != INI_VAL_STR) {
    return (1);
  }

  *str = val->val.str;

  return (0);
}
