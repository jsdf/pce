/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/write.c                                           *
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


#include <config.h>

#include <stdio.h>
#include <string.h>

#include <libini/libini.h>


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
int ini_write_val (FILE *fp, ini_val_t *val)
{
	fprintf (fp, "%s = ", val->name);

	switch (val->type) {
	case INI_VAL_INT:
		fprintf (fp, "0x%lx", val->val.u32);
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
int ini_write_section (FILE *fp, ini_sct_t *sct, unsigned indent)
{
	ini_val_t *val;

	val = sct->val_head;

	while (val != NULL) {
		if (ini_write_indent (fp, indent)) {
			return (1);
		}

		if (ini_write_val (fp, val)) {
			return (1);
		}

		fputs ("\n", fp);

		val = val->next;
	}

	sct = sct->sub_head;

	while (sct != NULL) {
		fputs ("\n", fp);

		if (ini_write_indent (fp, indent)) {
			return (1);
		}

		fprintf (fp, "%s {\n", sct->name);

		if (ini_write_section (fp, sct, indent + 1)) {
			return (1);
		}

		if (ini_write_indent (fp, indent)) {
			return (1);
		}

		fputs ("}\n", fp);

		sct = sct->next;
	}

	return (0);
}

int ini_write_fp (FILE *fp, ini_sct_t *sct)
{
	fputs ("# Generated automatically by libini\n\n", fp);

	if (ini_write_section (fp, sct, 0)) {
		return (1);
	}

	return (0);
}

int ini_write (const char *fname, ini_sct_t *sct)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = ini_write_fp (fp, sct);

	fclose (fp);

	return (r);
}
