/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pce-img/convert.c                                  *
 * Created:     2013-01-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include "pce-img.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <drivers/block/block.h>
#include <drivers/block/blkpbi.h>

#include <lib/getopt.h>


static pce_option_t opts_convert[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 1, "cylinders", "int", "Set the number of cylinders [0]" },
	{ 'C', 1, "min-cluster-size", "int", "Set the minimum cluster size for QED [0]" },
	{ 'f', 1, "offset", "int", "Set the data offset [0]" },
	{ 'F', 0, "flat", NULL, "Create a flat pbi file [no]" },
	{ 'g', 3, "geometry", "3*int", "Set the disk geometry (c h s)" },
	{ 'h', 1, "heads", "int", "Set the number of heads [0]" },
	{ 'i', 1, "input", "string", "Set the input file name [stdin]" },
	{ 'I', 1, "input-type", "string", "Set the input file type [auto]" },
	{ 'n', 1, "size", "int", "Set the disk size in 512 byte blocks [0]" },
	{ 'o', 1, "output", "string", "Set the output file name [stdout]" },
	{ 'O', 1, "output-type", "string", "Set the output file type [auto]" },
	{ 'q', 0, "quiet", NULL, "Be quiet [no]" },
	{ 's', 1, "sectors", "int", "Set the number of sectors per track [0]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{ 'w', 1, "cow", "string", "Set the COW file name [none]" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pce-img convert: Convert disk images",
		"usage: pce-img convert [options] [input [output]]",
		opts_convert
	);

	fflush (stdout);
}

static
void print_progress (FILE *fp, unsigned long i, unsigned long n, int done)
{
	fprintf (fp, "[%6.2f%%] block %lu of %lu\r",
		(100.0 * (i + 1)) / n,
		(unsigned long) i,
		(unsigned long) n
	);

	if (done) {
		fputc ('\n', fp);
	}

	fflush (fp);
}

static
unsigned long get_block_count (disk_t *dst, disk_t *src)
{
	unsigned long c1, c2;

	c1 = dsk_get_block_cnt (dst);
	c2 = dsk_get_block_cnt (src);

	return ((c1 < c2) ? c1 : c2);
}

static
int dsk_copy_to_pbi (disk_pbi_t *dst, disk_t *src)
{
	unsigned char *buf;
	unsigned long prg_i;
	unsigned long blidx, blcnt, bl512;
	unsigned long cnt;
	unsigned long val;

	if ((buf = pce_get_buf (dst->block_size)) == NULL) {
		return (1);
	}

	prg_i = 0;
	blidx = 0;
	blcnt = get_block_count (&dst->dsk, src);
	bl512 = dst->block_size / 512;

	while (blidx < blcnt) {
		cnt = blcnt - blidx;

		if (cnt > bl512) {
			cnt = bl512;
		}

		if (dsk_read_lba (src, buf, blidx, cnt)) {
			fprintf (stderr, "%s: read error at block %lu+%lu\n",
				arg0, blidx, cnt
			);
			return (1);
		}

		if (pce_block_is_null (buf, 512 * cnt)) {
			;
		}
		else if (pce_block_is_uniform_32 (buf, 512 * cnt, &val)) {
			if (dsk_pbi_set_uniform (dst, 512ULL * blidx, val)) {
				return (1);
			}
		}
		else {
			if (dsk_write_lba (&dst->dsk, buf, blidx, cnt)) {
				fprintf (stderr, "%s: write error at block %lu+%lu\n",
					arg0, blidx, cnt
				);
				return (1);
			}
		}

		blidx += cnt;

		if (par_quiet == 0) {
			prg_i += cnt;

			if (prg_i >= 4096) {
				print_progress (stdout, blidx, blcnt, 0);
				prg_i &= 0xfff;
			}
		}
	}

	if (par_quiet == 0) {
		print_progress (stdout, blcnt, blcnt, 1);
	}

	return (0);
}

static
int dsk_copy (disk_t *dst, disk_t *src)
{
	uint32_t      i, n, m;
	uint32_t      prg_i, prg_n;
	unsigned      k;
	uint16_t      msk;
	unsigned char *buf;

	if (dst->type == PCE_DISK_PBI) {
		return (dsk_copy_to_pbi (dst->ext, src));
	}

	n = get_block_count (dst, src);

	if ((buf = pce_get_buf (8192)) == NULL) {
		return (1);
	}

	prg_i = 0;
	prg_n = n;

	i = 0;
	while (n > 0) {
		m = (n < 16) ? n : 16;

		if (dsk_read_lba (src, buf, i, m)) {
			return (1);
		}

		msk = 0;
		for (k = 0; k < m; k++) {
			if (pce_block_is_null (buf + 512 * k, 512) == 0) {
				msk |= 1U << k;
			}
		}

		if (msk == 0xffff) {
			if (dsk_write_lba (dst, buf, i, m)) {
				return (1);
			}
		}
		else if (msk != 0) {
			for (k = 0; k < m; k++) {
				if (msk & (1U << k)) {
					if (dsk_write_lba (dst, buf + 512 * k, i + k, 1)) {
						return (1);
					}
				}
			}
		}

		i += m;
		n -= m;

		if (par_quiet == 0) {
			prg_i += m;
			if (prg_i >= 4096) {
				print_progress (stdout, i, prg_n, 0);
				prg_i &= 0xfff;
			}
		}
	}

	if (par_quiet == 0) {
		print_progress (stdout, prg_n, prg_n, 1);
	}

	return (0);
}

int main_convert (int argc, char **argv)
{
	int    r;
	char   **optarg;
	disk_t *inp, *out;

	inp = NULL;
	out = NULL;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts_convert);

		if (r == GETOPT_DONE) {
			break;
		}

		if (r < 0) {
			return (1);
		}

		switch (r) {
		case '?':
			print_help();
			return (0);

		case 'V':
			print_version();
			return (0);

		case 'c':
			if (pce_set_c (optarg[0])) {
				return (1);
			}
			break;

		case 'C':
			if (pce_set_min_cluster_size (optarg[0])) {
				return (1);
			}
			break;

		case 'f':
			if (pce_set_ofs (optarg[0])) {
				return (1);
			}
			break;

		case 'F':
			pce_set_flat (1);
			break;

		case 'g':
			if (pce_set_geo (optarg[0], optarg[1], optarg[2])) {
				return (1);
			}
			break;

		case 'h':
			if (pce_set_h (optarg[0])) {
				return (1);
			}
			break;

		case 'i':
			if ((inp = dsk_open_inp (optarg[0], inp, 1)) == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (pce_set_type_inp (optarg[0])) {
				return (1);
			}
			break;

		case 'n':
			if (pce_set_n (optarg[0])) {
				return (1);
			}
			break;

		case 'o':
			if ((out = dsk_create_out (optarg[0], out)) == NULL) {
				return (1);
			}
			break;

		case 'O':
			if (pce_set_type_out (optarg[0])) {
				return (1);
			}
			break;

		case 'q':
			pce_set_quiet (1);
			break;

		case 's':
			if (pce_set_s (optarg[0])) {
				return (1);
			}
			break;

		case 'w':
			if (out != NULL) {
				if ((out = pce_cow_open (out, optarg[0])) == NULL) {
					return (1);
				}
			}
			else if (inp != NULL) {
				if ((inp = pce_cow_open (inp, optarg[0])) == NULL) {
					return (1);
				}
			}
			else {
				return (1);
			}
			break;

		case 0:
			if (inp == NULL) {
				if ((inp = dsk_open_inp (optarg[0], inp, 1)) == NULL) {
					return (1);
				}
			}
			else if (out == NULL) {
				if ((out = dsk_create_out (optarg[0], out)) == NULL) {
					return (1);
				}
			}
			else {
				fprintf (stderr, "%s: too many file names (%s)\n",
					arg0, optarg[0]
				);
				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	if (inp == NULL) {
		fprintf (stderr, "%s: need an input file name\n", arg0);
		return (1);
	}

	if (out == NULL) {
		fprintf (stderr, "%s: need an output file name\n", arg0);
		return (1);
	}

	if (dsk_copy (out, inp)) {
		fprintf (stderr, "%s: copy failed\n", arg0);
		return (1);
	}

	dsk_del (out);
	dsk_del (inp);

	return (0);
}
