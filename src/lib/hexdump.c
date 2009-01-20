/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/hexdump.c                                            *
 * Created:     2003-11-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/hexdump.h>


void pce_dump_hex (FILE *fp, void *buf, unsigned long n,
	unsigned long addr, unsigned cols, char *prefix, int ascii)
{
	unsigned long i;
	unsigned      col;
	unsigned char *data;
	unsigned char line[256];

	col = 0;
	data = (unsigned char *) buf;

	for (i = 0; i < n; i++) {
		fputs ((col == 0) ? prefix : " ", fp);

		fprintf (fp, "%02X", data[i]);

		if ((data[i] >= 0x20) && (data[i] <= 0x7f)) {
			line[col] = data[i];
		}
		else {
			line[col] = '.';
		}

		col += 1;
		if (col >= cols) {
			fprintf (fp, "\t# %08lX", addr + i - (i % cols));

			if (ascii) {
				fputs ("  ", fp);
				fwrite (line, 1, col, fp);
			}

			fputs ("\n", fp);

			col = 0;
		}
	}

	if (col > 0) {
		unsigned col1, col2;

		col1 = strlen (prefix) + 3 * col - 1;
		col2 = (strlen (prefix) + 3 * cols - 1 + 8) & ~7;

		do {
			fputs ("\t", fp);
			col1 = (col1 + 8) & ~7;
		} while (col1 < col2);

		fprintf (fp, "# %08lX", addr + i - (i % cols));

		if (ascii) {
			fputs ("  ", fp);
			fwrite (line, 1, col, fp);
		}

		fputs ("\n", fp);
	}
}
