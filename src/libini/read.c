/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/read.c                                          *
 * Created:       2001-08-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2005 Hampa Hug <hampa@hampa.ch>                   *
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


#include <libini/libini.h>
#include <libini/scanner.h>

#include <math.h>

#define INI_TOK_STRING 256
#define INI_TOK_IDENT  257
#define INI_TOK_DIGIT  258


static
int parse_sect_body (scanner_t *scn, ini_sct_t *sct, int subsct);


static
void ini_scn_scan (scanner_t *scn)
{
  unsigned i;
  unsigned c;

  scn_scan (scn);

  while (scn_chr (scn, '#')) {
    c = scn_get_chr (scn, 0);
    while ((c != 10) && (c != 13)) {
      if (c == SCN_EOF) {
        scn->tok_id = SCN_TOK_NONE;
        scn->tok_str[0] = 0;
        return;
      }

      scn_rmv_chr (scn, 1);
      c = scn_get_chr (scn, 0);
    }

    scn_scan (scn);
  }

  if (scn_chr (scn, '"')) {
    i = 0;
    c = scn_get_chr (scn, 0);

    while ((i < SCN_TOK_MAX) && (c != '"')) {
      scn->tok_str[i] = c;
      i += 1;

      scn_rmv_chr (scn, 1);
      c = scn_get_chr (scn, 0);
      if (c == SCN_EOF) {
        scn->tok_id = SCN_TOK_NONE;
        scn->tok_str[0] = 0;
      }
    }

    scn_rmv_chr (scn, 1);

    if (i >= SCN_TOK_MAX) {
      scn->tok_id = SCN_TOK_NONE;
      scn->tok_str[0] = 0;
      return;
    }
    else {
      scn->tok_id = INI_TOK_STRING;
      scn->tok_str[i] = 0;
      return;
    }
  }
}

static
scanner_t *get_scanner (FILE *fp)
{
  scanner_t *scn;
  scn_set_t   set;
  scn_class_t *cls;

  scn = scnf_new ();
  if (scn == NULL) {
    return (NULL);
  }

  scn->scan = &ini_scn_scan;

  scn_set_clear (&set);
  scn_set_add_el (&set, 9);
  scn_set_add_el (&set, 10);
  scn_set_add_el (&set, 13);
  scn_set_add_el (&set, 32);
  scn_set_white (scn, &set);

  cls = scn_cls_new (INI_TOK_IDENT);
  scn_set_add_els (&cls->start, 'a', 'z');
  scn_set_add_els (&cls->start, 'A', 'Z');
  scn_set_add_el (&cls->start, '_');
  scn_set_copy (&cls->run, &cls->start);
  scn_set_add_els (&cls->run, '0', '9');
  scn_class_add (scn, cls);

  cls = scn_cls_new (INI_TOK_DIGIT);
  scn_set_add_els (&cls->start, '0', '9');
  scn_set_copy (&cls->run, &cls->start);
  scn_set_add_els (&cls->run, 'a', 'f');
  scn_set_add_els (&cls->run, 'A', 'F');
  scn_set_add_el (&cls->run, 'x');
  scn_set_add_el (&cls->run, 'b');
  scn_class_add (scn, cls);

  scnf_set_file (scn, fp);

  return (scn);
}

static
int parse_unsigned (scanner_t *scn, ini_val_t *val, int neg)
{
  unsigned      i;
  unsigned long lng;
  unsigned long dig, base;
  char          *str;

  str = scn_str (scn);

  lng = 0;
  base = 10;

  if (str[0] == '0') {
    if (str[1] == 'b') {
      base = 2;
    }
    else if (str[1] == 'x') {
      base = 16;
    }
  }

  i = 2;
  while (str[i] != 0) {
    if ((str[i] >= '0') && (str[i] <= '9')) {
      dig = (str[i] - '0');
    }
    else if ((str[i] >= 'a') && (str[i] <= 'z')) {
      dig = (str[i] - 'a' + 10);
    }
    else if ((str[i] >= 'A') && (str[i] <= 'Z')) {
      dig = (str[i] - 'A' + 10);
    }
    else {
      return (1);
    }

    if (dig >= base) {
      return (1);
    }

    lng = base * lng + dig;

    i += 1;
  }

  ini_scn_scan (scn);

  if (neg) {
    ini_val_set_sint32 (val, -(long)lng);
  }
  else {
    ini_val_set_uint32 (val, lng);
  }

  return (0);
}

static
int parse_number (scanner_t *scn, ini_val_t *val, int neg)
{
  unsigned      i;
  unsigned long lng;
  unsigned      dig;
  double        dbl, tmp;
  char          *str;

  str = scn_str (scn);

  if ((str[0] == '0') && ((str[1] == 'b') || (str[1] == 'x'))) {
    return (parse_unsigned (scn, val, neg));
  }

  dbl = 0.0;
  lng = 0;

  i = 0;
  while ((str[i] >= '0') && (str[i] <= '9')) {
    dig = (unsigned)(str[i] - '0');

    lng = 10 * lng + dig;
    dbl = 10.0 * dbl + (double) dig;

    i += 1;
  }

  if (str[i] != 0) {
    return (1);
  }

  ini_scn_scan (scn);

  if (scn_chr (scn, '.') == 0) {
    if (neg) {
      ini_val_set_sint32 (val, -(long)lng);
    }
    else {
      ini_val_set_uint32 (val, lng);
    }

    return (0);
  }

  ini_scn_scan (scn);

  if (scn_tid (scn) != INI_TOK_DIGIT) {
    return (1);
  }

  str = scn_str (scn);
  tmp = 10.0;

  i = 0;
  while ((str[i] >= '0') && (str[i] <= '9')) {
    dbl += (double)(str[i] - '0') / tmp;
    tmp *= 10.0;
    i += 1;
  }

  if (str[i] != 0) {
    return (1);
  }

  if (neg) {
    dbl = -dbl;
  }

  ini_val_set_dbl (val, dbl);

  ini_scn_scan (scn);

  return (0);
}

static
ini_val_t *parse_value (scanner_t *scn)
{
  ini_val_t *val;

  val = ini_val_new (scn_str (scn));
  if (val == NULL) {
    return (NULL);
  }

  ini_scn_scan (scn);

  if ((scn_chr (scn, '=') == 0) && (scn_chr (scn, ':') == 0)) {
    ini_val_del (val);
    return (NULL);
  }

  ini_scn_scan (scn);

  if (scn_chr (scn, '-')) {
    ini_scn_scan (scn);

    if (parse_number (scn, val, 1)) {
      ini_val_del (val);
      return (NULL);
    }

    return (val);
  }
  else if (scn_chr (scn, '+')) {
    ini_scn_scan (scn);

    if (parse_number (scn, val, 0)) {
      ini_val_del (val);
      return (NULL);
    }

    return (val);
  }

  if ((scn_tid (scn) == INI_TOK_IDENT) || (scn_tid (scn) == INI_TOK_STRING)) {
    ini_val_set_str (val, scn_str (scn));
    ini_scn_scan (scn);
    return (val);
  }

  if (scn_tid (scn) == INI_TOK_DIGIT) {
    if (parse_number (scn, val, 0)) {
      ini_val_del (val);
      return (NULL);
    }

    return (val);
  }

  ini_val_del (val);

  return (NULL);
}

static
ini_sct_t *parse_sect1 (scanner_t *scn)
{
  ini_sct_t *sct;

  ini_scn_scan (scn);

  if (scn_tid (scn) != INI_TOK_IDENT) {
    return (NULL);
  }

  sct = ini_sct_new (scn_str (scn));

  ini_sct_set_format (sct, 1, 0);

  ini_scn_scan (scn);

  if (scn_chr (scn, '{') == 0) {
    ini_sct_del (sct);
    return (NULL);
  }

  ini_scn_scan (scn);

  if (parse_sect_body (scn, sct, 1)) {
    ini_sct_del (sct);
    return (NULL);
  }

  if (scn_chr (scn, '}') == 0) {
    ini_sct_del (sct);
    return (NULL);
  }

  ini_scn_scan (scn);

  return (sct);
}

static
ini_sct_t *parse_sect2 (scanner_t *scn)
{
  int       subsct;
  ini_sct_t *sct;

  ini_scn_scan (scn);

  subsct = 0;

  if (scn_tid (scn) != INI_TOK_IDENT) {
    return (NULL);
  }

  sct = ini_sct_new (scn_str (scn));

  ini_sct_set_format (sct, 2, 0);

  ini_scn_scan (scn);

  if (scn_chr (scn, ']') == 0) {
    ini_sct_del (sct);
    return (NULL);
  }

  ini_scn_scan (scn);

  if (scn_chr (scn, '{')) {
    ini_scn_scan (scn);
    subsct = 1;
  }

  if (parse_sect_body (scn, sct, subsct)) {
    ini_sct_del (sct);
    return (NULL);
  }

  if (subsct) {
    if (scn_chr (scn, '}') == 0) {
      ini_sct_del (sct);
      return (NULL);
    }

    ini_scn_scan (scn);
  }
  return (sct);
}

static
int parse_sect_body (scanner_t *scn, ini_sct_t *sct, int subsct)
{
  ini_sct_t *newsct;
  ini_val_t  *val;

  while (scn_tid (scn) != SCN_TOK_NONE) {
    if (scn_tok (scn, INI_TOK_IDENT, "section")) {
      if (subsct) {
        newsct = parse_sect1 (scn);
        if (newsct == NULL) {
          return (1);
        }

        ini_sct_add_sct (sct, newsct);
      }
      else {
        break;
      }
    }
    else if (scn_tok (scn, SCN_TOK_CHAR, "[")) {
      if (subsct) {
        newsct = parse_sect2 (scn);
        if (newsct == NULL) {
          return (1);
        }

        ini_sct_add_sct (sct, newsct);
      }
      else {
        break;
      }
    }
    else if (scn_tid (scn) == INI_TOK_IDENT) {
      val = parse_value (scn);
      if (val == NULL) {
        return (1);
      }

      ini_sct_add_val (sct, val);
    }
    else {
      break;
    }
  }

  return (0);
}

ini_sct_t *ini_read_fp (FILE *fp)
{
  scanner_t *scn;
  ini_sct_t *sct;

  scn = get_scanner (fp);
  if (scn == NULL) {
    return (NULL);
  }

  sct = ini_sct_new (NULL);
  if (sct == NULL) {
    scn->del (scn);
    return (NULL);
  }

  ini_scn_scan (scn);

  if (parse_sect_body (scn, sct, 1)) {
    ini_sct_del (sct);
    return (NULL);
  }

  return (sct);
}

ini_sct_t *ini_read (const char *fname)
{
  FILE       *fp;
  ini_sct_t *sct;

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return (NULL);
  }

  sct = ini_read_fp (fp);

  fclose (fp);

  return (sct);
}
