/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/aym/decode.c                                       *
 * Created:     2015-05-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2016 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static
int aym_decode_fp (FILE *inp, FILE *out, unsigned long th, unsigned long mark)
{
	unsigned           i;
	int                c1, c2;
	unsigned long      vers;
	unsigned long long delay;
	unsigned long long total;
	unsigned char      v1, v2;
	unsigned char      buf[8];
	unsigned char      reg[16];

	if (fread (buf, 1, 8, inp) != 8) {
		return (1);
	}

	if (aym_get_uint32_be (buf, 0) != AYM_MAGIC) {
		fprintf (stderr, "%s: not a aym file\n", arg0);
		return (1);
	}

	vers = aym_get_uint32_be (buf, 4);

	if (vers != 0) {
		fprintf (stderr, "%s: unknown aym version (%lu)\n", arg0, vers);
		return (1);
	}

	fprintf (out, "AYM %lu\n\n", vers);

	for (i = 0; i < 16; i++) {
		reg[i] = 0;
	}

	delay = 0;
	total = 0;

	while (1) {
		c1 = fgetc (inp);
		c2 = fgetc (inp);

		if ((c1 == EOF) || (c2 == EOF)) {
			fprintf (stderr, "%s: premature end of file\n", arg0);
			return (0);
		}

		v1 = c1 & 0xff;
		v2 = c2 & 0xff;

		switch ((v1 >> 4) & 0x0f) {
		case 0:
			if ((mark > 0) && (total <= mark) && ((total + delay) > mark)) {
				fprintf (out, "\n# MARK %lu.%06lu\n",
					mark / 1000000, mark % 1000000
				);

				for (i = 0; i < 14; i++) {
					fprintf (out, "# REG %02X %02X\n", i, reg[i]);
				}

				fputs ("\n", out);
			}

			if ((delay > 0) && (delay >= th)) {
				total += delay;
				fprintf (out, "\n# %llu.%06llu + %llu.%06llu = %llu.%06llu\n",
					(total - delay) / 1000000, (total - delay) % 1000000,
					delay / 1000000, delay % 1000000,
					total / 1000000, total % 1000000
				);
				fprintf (out, "DEL %llu\n\n", delay);
				delay = 0;
			}

			reg[v1 & 0x0f] = v2;

			fprintf (out, "REG %02X %02X\n", v1 & 0x0f, v2);
			break;

		case 1:
			delay += ((v1 & 0x0f) << 8) | v2;
			break;

		case 2:
			delay += (((unsigned long long) (v1 & 0x0f) << 8) | v2) << 12;
			break;

		case 3:
			delay += (((unsigned long long) (v1 & 0x0f) << 8) | v2) << 24;
			break;

		case 8:
			fprintf (out, "TXT %u ", v1 & 0x0f);
			while (v2 > 0) {
				if ((c1 = fgetc (inp)) == EOF) {
					return (1);
				}

				fputc (c1, out);

				v2 -= 1;
			}
			fputs ("\n\n", out);
			break;

		case 15:
			if ((v1 == 0xff) && (v2 == 0xff)) {
				fprintf (out, "END\n");
			}
			return (0);
		}
	}

	return (0);
}

int aym_decode (const char *inp, const char *out, unsigned long th, unsigned long mark)
{
	int  r;
	FILE *finp, *fout;

	if (inp == NULL) {
		finp = stdin;
	}
	else if ((finp = fopen (inp, "rb")) == NULL) {
		fprintf (stderr, "%s: can't open input file (%s)\n", arg0, inp);
		return (1);
	}

	if (out == NULL) {
		fout = stdout;
	}
	else if ((fout = fopen (out, "w")) == NULL) {
		fprintf (stderr, "%s: can't open output file (%s)\n", arg0, out);
		fclose (finp);
		return (1);
	}

	r = aym_decode_fp (finp, fout, th, mark);

	if (fout != stdout) {
		fclose (fout);
	}

	if (finp != stdin) {
		fclose (finp);
	}

	return (r);
}
