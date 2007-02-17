/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/libini/write.c                                         *
 * Created:       2001-08-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2007 Hampa Hug <hampa@hampa.ch>                   *
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
int ini_write_body (ini_sct_t *sct, FILE *fp, unsigned indent);


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
		fprintf (fp, "0x%lx", val->val.u32);
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
int ini_write_section1 (ini_sct_t *sct, FILE *fp, unsigned indent)
{
	if (ini_write_indent (fp, indent)) {
		return (1);
	}

	fprintf (fp, "section %s {\n", sct->name);

	if (ini_write_body (sct, fp, indent + 1)) {
		return (1);
	}

	if (ini_write_indent (fp, indent)) {
		return (1);
	}

	fputs ("}\n", fp);

	return (0);
}

static
int ini_write_section2 (ini_sct_t *sct, FILE *fp, unsigned indent)
{
	int subsct;

	subsct = (sct->sctcnt > 0);

	if (ini_write_indent (fp, indent)) {
		return (1);
	}

	fprintf (fp, "[%s]%s", sct->name, subsct ? " {\n" : "\n");

	if (ini_write_body (sct, fp, indent + subsct)) {
		return (1);
	}

	if (subsct) {
		fputs ("}\n", fp);
	}

	return (0);
}

static
int ini_write_body (ini_sct_t *sct, FILE *fp, unsigned indent)
{
	unsigned i;

	for (i = 0; i < sct->valcnt; i++) {
		if (ini_write_indent (fp, indent)) {
			return (1);
		}

		if (ini_write_val (&sct->val[i], fp)) {
			return (1);
		}

		fputs ("\n", fp);
	}

	for (i = 0; i < sct->sctcnt; i++) {
		fputs ("\n", fp);

		if (sct->sct[i].format == 2) {
			if (ini_write_section2 (&sct->sct[i], fp, indent)) {
				return (1);
			}
		}
		else {
			if (ini_write_section1 (&sct->sct[i], fp, indent)) {
				return (1);
			}
		}
	}

	return (0);
}

int ini_write_fp (ini_sct_t *sct, FILE *fp)
{
	int r;

	fputs ("# Generated automatically by libini\n\n", fp);

	r = ini_write_body (sct, fp, 0);

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
