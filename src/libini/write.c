/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/write.c                                         *
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


static
int ini_write_body (ini_sct_t *sct, FILE *fp, unsigned level, unsigned indent);


static
int ini_write_indent (FILE *fp, unsigned level)
{
  while (level > 0) {
    fputc ('\t', fp);
    level -= 1;
  }

  return (0);
}

static
int ini_write_val (ini_val_t *val, FILE *fp)
{
  fprintf (fp, "%s = ", val->name);

  switch (val->type) {
    case INI_VAL_U32:
      fprintf (fp, "%ld", val->val.u32);
      break;

    case INI_VAL_S32:
      fprintf (fp, "%ld", val->val.s32);
      break;

    case INI_VAL_DBL:
      fprintf (fp, "%f", val->val.dbl);
      break;

    case INI_VAL_STR:
      fprintf (fp, "\"%s\"", val->val.str);
      break;

    default:
      return (1);
  }

  return (0);
}

static
int ini_write_section1 (ini_sct_t *sct, FILE *fp, unsigned level, unsigned indent)
{
  if (ini_write_indent (fp, indent)) {
    return (1);
  }

  fprintf (fp, "section %s {\n", ini_sct_get_name (sct));

  if (ini_write_body (sct, fp, level, indent + 1)) {
    return (1);
  }

  if (ini_write_indent (fp, indent)) {
    return (1);
  }

  fputs ("}\n", fp);

  return (0);
}

static
int ini_write_section2 (ini_sct_t *sct, FILE *fp, unsigned level, unsigned indent)
{
  if (ini_write_indent (fp, indent)) {
    return (1);
  }

  fprintf (fp, "[%s]\n", ini_sct_get_name (sct));

  if (ini_write_body (sct, fp, level, indent)) {
    return (1);
  }

  return (0);
}

static
int ini_write_body (ini_sct_t *sct, FILE *fp, unsigned level, unsigned indent)
{
  ini_val_t *val;
  ini_sct_t *down;

  if (sct->val_head != NULL) {
    val = sct->val_head;
    while (val != NULL) {
      if (ini_write_indent (fp, indent)) {
        return (1);
      }

      if (ini_write_val (val, fp)) {
        return (1);
      }

      fputs ("\n", fp);

      val = val->next;
    }

    if (sct->head != NULL) {
      fputs ("\n", fp);
    }
  }

  down = sct->head;
  while (down != NULL) {
    if ((level == 0) && (ini_sct_get_format (down) == 2)) {
      if (ini_write_section2 (down, fp, level + 1, indent)) {
        return (1);
      }
    }
    else {
      if (ini_write_section1 (down, fp, level + 1, indent)) {
        return (1);
      }
    }

    down = down->next;

    if (down != NULL) {
      fputs ("\n", fp);
    }
  }

  return (0);
}

int ini_write_fp (ini_sct_t *sct, FILE *fp)
{
  int r;

  fputs (
    "# Generated automatically by libini "
    LIBINI_VERSION_STR " by Hampa Hug <hampa@hampa.ch>\n\n",
    fp
  );

  r = ini_write_body (sct, fp, 0, 0);

  return (r);
}

int ini_write (ini_sct_t *sct, const char *fname)
{
  int  r;
  FILE *fp;

  fp = fopen (fname, "wb");
  if (fp == NULL) {
    return (1);
  }

  r = ini_write_fp (sct, fp);

  fclose (fp);

  return (r);
}
