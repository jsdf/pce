/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     section.c                                                  *
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

/* $Id: section.c,v 1.2 2003/04/29 00:51:24 hampa Exp $ */


#include "libini.h"


ini_sct_t *ini_sct_new (const char *name)
{
  ini_sct_t *sct;

  sct = (ini_sct_t *) malloc (sizeof (ini_sct_t));
  if (sct == NULL) {
    return (NULL);
  }

  sct->name = NULL;
  sct->format = 0;

  sct->next = NULL;
  sct->head = NULL;
  sct->tail = NULL;

  sct->val_head = NULL;
  sct->val_tail = NULL;

  ini_sct_set_name (sct, name);

  return (sct);
}

void ini_sct_del (ini_sct_t *sct)
{
  if (sct == NULL) {
    return;
  }

  if (sct->next != NULL) {
    ini_sct_del (sct->next);
  }

  if (sct->head != NULL) {
    ini_sct_del (sct->head);
  }

  free (sct->name);
  free (sct);
}

void ini_sct_set_name (ini_sct_t *sct, const char *name)
{
  free (sct->name);
  sct->name = NULL;

  if (name == NULL) {
    return;
  }

  sct->name = (char *) malloc (strlen (name) + 1);
  if (sct->name == NULL) {
    return;
  }

  strcpy (sct->name, name);
}

const char *ini_sct_get_name (const ini_sct_t *sct)
{
  return (sct->name);
}

void ini_sct_set_format (ini_sct_t *sct, unsigned format, int rec)
{
  sct->format = format;

  if (rec) {
    ini_sct_t *tmp;

    tmp = sct->head;
    while (tmp != NULL) {
      ini_sct_set_format (tmp, format, 1);
      tmp = tmp->next;
    }
  }
}

unsigned ini_sct_get_format (const ini_sct_t *sct)
{
  return (sct->format);
}

ini_sct_t *ini_sct_get_next (const ini_sct_t *sct)
{
  return (sct->next);
}

ini_sct_t *ini_sct_get_head (const ini_sct_t *sct)
{
  return (sct->head);
}

ini_val_t *ini_sct_get_val_head (const ini_sct_t *sct)
{
  return (sct->val_head);
}

void ini_sct_add_sct (ini_sct_t *sct, ini_sct_t *add)
{
  if (sct->head == NULL) {
    sct->head = add;
  }
  else {
    sct->tail->next = add;
  }

  sct->tail = add;

  while (sct->tail->next != NULL) {
    sct->tail = sct->tail->next;
  }
}

void ini_sct_add_val (ini_sct_t *sct, ini_val_t *val)
{
  if (sct->val_head == NULL) {
    sct->val_head = val;
  }
  else {
    sct->val_tail->next = val;
  }

  sct->val_tail = val;

  while (sct->val_tail->next != NULL) {
    sct->val_tail = sct->val_tail->next;
  }
}

ini_val_t *ini_sct_find_val (const ini_sct_t *sct, const char *name)
{
  unsigned  i, j;
  ini_val_t *val;

  i = 0;

  while ((name[i] != 0) && (name[i] != '.')) {
    i += 1;
  }

  if (i == 0) {
    return (NULL);
  }

  if (name[i] == 0) {
    val = sct->val_head;
    while (val != NULL) {
      if (strcmp (val->name, name) == 0) {
        return (val);
      }

      val = val->next;
    }

    return (NULL);
  }

  sct = sct->head;
  while (sct != NULL) {
    if (strlen (sct->name) == i) {
      j = 0;
      while ((j < i) && (sct->name[j] == name[j])) {
        j += 1;
      }

      if (j == i) {
        return (ini_sct_find_val (sct, name + i + 1));
      }
    }

    sct = sct->next;
  }

  return (NULL);
}

ini_sct_t *ini_sct_find_sct (ini_sct_t *sct, const char *name)
{
  unsigned  i, j;

  i = 0;

  while ((name[i] != 0) && (name[i] != '.')) {
    i += 1;
  }

  if (i == 0) {
    return (NULL);
  }

  sct = sct->head;
  while (sct != NULL) {
    if (strlen (sct->name) == i) {
      j = 0;
      while ((j < i) && (sct->name[j] == name[j])) {
        j += 1;
      }

      if (j == i) {
        if (name[i] == 0) {
          return (sct);
        }
        else {
          return (ini_sct_find_sct (sct, name + i + 1));
        }
      }
    }

    sct = sct->next;
  }

  return (NULL);
}

ini_sct_t *ini_sct_find_next (ini_sct_t *sct, const char *name)
{
  sct = sct->next;

  while (sct != NULL) {
    if (strcmp (sct->name, name) == 0) {
      return (sct);
    }

    sct = sct->next;
  }

  return (NULL);
}

int ini_get_slng (const ini_sct_t *sct, const char *name,
  long *ret, long def)
{
  ini_val_t *val;

  *ret = def;

  val = ini_sct_find_val (sct, name);
  if (val == NULL) {
    return (1);
  }

  if (val->type != INI_VAL_LNG) {
    return (1);
  }

  *ret = val->val.lng;

  return (0);
}

int ini_get_ulng (const ini_sct_t *sct, const char *name,
  unsigned long *ret, unsigned long def)
{
  long val;

  *ret = def;

  if (ini_get_slng (sct, name, &val, 0)) {
    return (1);
  }

  *ret = (unsigned long) val;

  return (0);
}

int ini_get_sint (const ini_sct_t *sct, const char *name,
  int *ret, int def)
{
  long val;

  *ret = def;

  if (ini_get_slng (sct, name, &val, 0)) {
    return (1);
  }

  *ret = (int) val;

  return (0);
}

int ini_get_uint (const ini_sct_t *sct, const char *name,
  unsigned *ret, unsigned def)
{
  long val;

  *ret = def;

  if (ini_get_slng (sct, name, &val, 0)) {
    return (1);
  }

  *ret = (unsigned) val;

  return (0);
}

int ini_get_sdbl (const ini_sct_t *sct, const char *name,
  double *ret, double def)
{
  ini_val_t *val;

  *ret = def;

  val = ini_sct_find_val (sct, name);
  if (val == NULL) {
    return (1);
  }

  if (val->type == INI_VAL_DBL) {
    *ret = val->val.dbl;
  }
  else if (val->type == INI_VAL_LNG) {
    *ret = (double) val->val.lng;
  }
  else {
    return (1);
  }

  return (0);
}

int ini_get_string (const ini_sct_t *sct, const char *name,
  char **ret, char *def)
{
  ini_val_t *val;

  *ret = def;

  val = ini_sct_find_val (sct, name);
  if (val == NULL) {
    return (1);
  }

  if (val->type != INI_VAL_STR) {
    return (1);
  }

  *ret = val->val.str;

  return (0);
}
